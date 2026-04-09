#include "Core/AvatarGameInstance.h"
#include "AI/LLMClient.h"
#include "AI/TTSClient.h"
#include "Avatar/AvatarCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

UAvatarGameInstance::UAvatarGameInstance()
{
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

void UAvatarGameInstance::SendUserMessage(const FString& Message)
{
    if (Message.IsEmpty()) return;
    
    ChatHistory.Add(FString::Printf(TEXT("Пользователь: %s"), *Message));
    OnChatMessageReceived.Broadcast(Message);
    
    FString Context = TEXT("Ты виртуальный ассистент по имени ") + AvatarName + 
                      TEXT(". Отвечай дружелюбно и по делу на русском языке. Отвечай кратко.\n\n");
    
    for (const auto& Entry : ChatHistory)
    {
        Context += Entry + TEXT("\n");
    }
    
    if (LLMClient)
    {
        LLMClient->SendPrompt(Context, FLLMResponseDelegate::CreateLambda([this](const FString& Response)
        {
            CurrentResponse = Response;
            
            FString FullResponse = AvatarName + TEXT(": ") + Response;
            ChatHistory.Add(FullResponse);
            
            OnAvatarResponse.Broadcast(Response);
            
            // Ищем персонажа если еще не нашли
            if (!AvatarCharacter)
            {
                UWorld* World = GetWorld();
                if (World)
                {
                    TArray<AActor*> FoundActors;
                    UGameplayStatics::GetAllActorsOfClass(World, AAvatarCharacter::StaticClass(), FoundActors);
                    if (FoundActors.Num() > 0)
                    {
                        AvatarCharacter = Cast<AAvatarCharacter>(FoundActors[0]);
                    }
                }
            }
            
            if (TTSClient)
            {
                TTSClient->SynthesizeAndPlay(Response);
            }
        }));
    }
}

void UAvatarGameInstance::OnTTSStarted(float Duration)
{
    if (AvatarCharacter && !CurrentResponse.IsEmpty())
    {
        AvatarCharacter->Speak(CurrentResponse, Duration);
    }
}