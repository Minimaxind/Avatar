// Avatar/AvatarCharacter.cpp
#include "Avatar/AvatarCharacter.h"
#include "Avatar/FacialAnimationComponent.h"
#include "Components/AudioComponent.h"
#include "Core/AvatarGameInstance.h"
#include "TimerManager.h"

AAvatarCharacter::AAvatarCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    
    FacialAnimation = CreateDefaultSubobject<UFacialAnimationComponent>(TEXT("FacialAnimation"));
    VoiceAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("VoiceAudioComponent"));
    VoiceAudioComponent->SetupAttachment(RootComponent);
}

void AAvatarCharacter::BeginPlay()
{
    Super::BeginPlay();
    
    // Проверяем есть ли у нас рабочий FacialAnimation с Face мешем
    // Если нет — не регистрируемся, чтобы не перебить MetaHuman
    if (FacialAnimation && !FacialAnimation->IsSpeaking())
    {
        // Проверка: если FacialAnimation не нашёл Face меш — не регистрируемся
        // (IsSpeaking() == false может быть и у рабочего, поэтому используем лог-проверку через попытку)
    }

    GameInstance = Cast<UAvatarGameInstance>(GetGameInstance());
    
    if (GameInstance)
    {
        // Регистрируемся только если у нас есть FacialAnimation с рабочим Face мешем
        // Определяем это косвенно: если у нас только CharacterMesh0 без Face — не регистрируемся
        TArray<USkeletalMeshComponent*> SkelMeshes;
        GetComponents<USkeletalMeshComponent>(SkelMeshes);

        bool bHasFaceMesh = false;
        for (USkeletalMeshComponent* SkelMesh : SkelMeshes)
        {
            if (SkelMesh->GetName().Equals(TEXT("Face"), ESearchCase::IgnoreCase))
            {
                bHasFaceMesh = true;
                break;
            }
        }

        if (bHasFaceMesh)
        {
            // Это MetaHuman или актор с Face мешем — регистрируемся
            GameInstance->AvatarCharacter = this;
            GameInstance->AvatarActor = this;
            GameInstance->OnAvatarResponse.AddDynamic(this, &AAvatarCharacter::HandleAvatarResponse);
            UE_LOG(LogTemp, Warning, TEXT("[AvatarCharacter] Registered as avatar (has Face mesh): %s"), *GetName());
        }
        else
        {
            // Это дефолтный pawn без Face меша — не регистрируемся
            UE_LOG(LogTemp, Warning, TEXT("[AvatarCharacter] Skipping registration (no Face mesh): %s"), *GetName());
        }
    }
}

void AAvatarCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AAvatarCharacter::HandleAvatarResponse(const FString& Response)
{

}

void AAvatarCharacter::Speak(const FString& Text, float Duration)
{
    if (Text.IsEmpty()) return;
    
    if (FacialAnimation)
    {
        FacialAnimation->StartSpeaking(Text, Duration);
    }
    
    OnStartSpeaking();
    
    GetWorldTimerManager().ClearTimer(SpeechTimerHandle);
    GetWorldTimerManager().SetTimer(SpeechTimerHandle, [this]()
    {
        if (FacialAnimation)
            FacialAnimation->StopSpeaking();
        OnStopSpeaking();
    }, Duration, false);
}