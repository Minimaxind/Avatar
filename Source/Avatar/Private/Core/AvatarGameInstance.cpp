// Core/AvatarGameInstance.cpp
#include "Core/AvatarGameInstance.h"

#include "AI/LLMClient.h"
#include "AI/TTSClient.h"
#include "Avatar/AvatarCharacter.h"
#include "Avatar/FacialAnimationComponent.h"
#include "Kismet/GameplayStatics.h"

UAvatarGameInstance::UAvatarGameInstance()
{
    AvatarCharacter = nullptr;
    LLMClient = nullptr;
    TTSClient = nullptr;
}

void UAvatarGameInstance::Init()
{
    Super::Init();
    
    LLMClient = NewObject<ULLMClient>(this);
    TTSClient = NewObject<UTTSClient>(this);
    
    if (TTSClient)
    {
        TTSClient->OnSpeechStart.AddDynamic(this, &UAvatarGameInstance::OnTTSStarted);
    }
}

void UAvatarGameInstance::Shutdown()
{
    Super::Shutdown();
}

// Ищем актора у которого есть FacialAnimationComponent
AActor* UAvatarGameInstance::FindAvatarActor()
{
    UWorld* World = GetWorld();
    if (!World) return nullptr;

    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);

    for (AActor* Actor : AllActors)
    {
        // Пропускаем AvatarCharacter по умолчанию (у него нет Face меша)
        if (Actor->IsA<AAvatarCharacter>() && !Actor->GetName().Contains(TEXT("MetaHuman")))
        {
            continue;
        }
        
        UFacialAnimationComponent* FAC = Actor->FindComponentByClass<UFacialAnimationComponent>();
        if (FAC)
        {
            UE_LOG(LogTemp, Warning, TEXT("[GI] Found avatar actor: %s"), *Actor->GetName());
            return Actor;
        }
    }

    UE_LOG(LogTemp, Error, TEXT("[GI] Avatar actor with FacialAnimationComponent not found!"));
    return nullptr;
}

void UAvatarGameInstance::SendUserMessage(const FString& Message)
{
    if (Message.IsEmpty()) return;
    
    ChatHistory.Add(FString::Printf(TEXT("Пользователь: %s"), *Message));
    OnChatMessageReceived.Broadcast(Message);
    
    FString Context = TEXT("Ты виртуальный ассистент по имени ") + AvatarName + 
                      TEXT(". Отвечай дружелюбно и по делу на русском языке. Отвечай кратко, не более 2-3 предложений. Запомни, тебя зовут Ави, ты персональный виртуальный аватар студии ДримМейкерс, если тебя спросят представься, обязательно упомяни к какой студии ты принадлежишь!\n\n");
    
    int32 StartIndex = FMath::Max(0, ChatHistory.Num() - 10);
    for (int32 i = StartIndex; i < ChatHistory.Num(); i++)
    {
        Context += ChatHistory[i] + TEXT("\n");
    }
    Context += TEXT("\nАви: ");
    
    if (LLMClient)
    {
        LLMClient->SendPrompt(Context, FLLMResponseDelegate::CreateLambda([this](const FString& Response)
        {
            CurrentResponse = Response;
            
            FString FullResponse = AvatarName + TEXT(": ") + Response;
            ChatHistory.Add(FullResponse);
            
            OnAvatarResponse.Broadcast(Response);
            
            // Ищем актора с FacialAnimationComponent если ещё не нашли
            if (!AvatarActor)
            {
                AvatarActor = FindAvatarActor();
            }
            
            if (TTSClient)
            {
                TTSClient->SynthesizeAndPlay(Response);
            }
        }));
    }
}

void UAvatarGameInstance::RefreshAvatarActor()
{
    AActor* NewActor = FindAvatarActor();
    if (NewActor != AvatarActor)
    {
        AvatarActor = NewActor;
        if (AvatarActor)
        {
            UE_LOG(LogTemp, Warning, TEXT("[GI] Avatar actor updated to: %s"), *AvatarActor->GetName());
        }
    }
}

void UAvatarGameInstance::OnTTSStarted(float Duration)
{
    // Защита от рекурсии
    if (bIsPlayingTTS) return;
    bIsPlayingTTS = true;
    
    if (!AvatarActor)
    {
        AvatarActor = FindAvatarActor();
    }

    if (!AvatarActor || CurrentResponse.IsEmpty())
    {
        bIsPlayingTTS = false;
        return;
    }

    UFacialAnimationComponent* FAC = AvatarActor->FindComponentByClass<UFacialAnimationComponent>();
    if (FAC)
    {
        // ИСПРАВЛЕНО: Запускаем полноценную анимацию речи вместо тестового метода EmergencyTest
        FAC->StartSpeaking(CurrentResponse, Duration);
    }
    
    // Сбрасываем флаг после окончания речи
    UWorld* World = GetWorld();
    if (World && TTSClient)
    {
        FTimerHandle ResetTimer;
        World->GetTimerManager().SetTimer(ResetTimer, [this]()
        {
            bIsPlayingTTS = false;
        }, Duration + 0.5f, false);
    }
    else
    {
        bIsPlayingTTS = false;
    }
}