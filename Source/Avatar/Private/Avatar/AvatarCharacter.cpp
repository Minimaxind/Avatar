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
    
    GameInstance = Cast<UAvatarGameInstance>(GetGameInstance());
    
    if (GameInstance)
    {
        GameInstance->AvatarCharacter = this;
        GameInstance->OnAvatarResponse.AddDynamic(this, &AAvatarCharacter::HandleAvatarResponse);
    }
}

void AAvatarCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AAvatarCharacter::HandleAvatarResponse(const FString& Response)
{
    float SpeechDuration = FMath::Max(1.0f, Response.Len() * 0.08f);
    Speak(Response, SpeechDuration);
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