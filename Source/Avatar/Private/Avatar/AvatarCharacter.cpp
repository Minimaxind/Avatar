#include "Avatar/AvatarCharacter.h"
#include "Avatar/FacialAnimationComponent.h"
#include "Components/AudioComponent.h"
#include "Core/AvatarGameInstance.h"

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
		
		GameInstance->OnAvatarResponse.AddDynamic(this, &AAvatarCharacter::HandleAvatarResponse);
	}
}

void AAvatarCharacter::HandleAvatarResponse(const FString& Response)
{
	Speak(Response);
}

void AAvatarCharacter::Speak(const FString& Text, float Duration)
{
	if (Duration <= 0.0f)
	{
		// Примерная оценка длительности
		Duration = Text.Len() * 0.1f;
	}
    
	OnStartSpeaking();
    
	if (FacialAnimation)
	{
		FacialAnimation->StartTalking(Duration);
	}
    
	// Таймер окончания речи
	GetWorldTimerManager().SetTimer(SpeechTimerHandle, [this]()
	{
		OnStopSpeaking();
		if (FacialAnimation)
		{
			FacialAnimation->StopTalking();
		}
	}, Duration, false);
}

void AAvatarCharacter::SetIdleAnimation(bool bIsIdle)
{
	// Здесь можно управлять idle анимациями
	if (FacialAnimation && bIsIdle)
	{
		FacialAnimation->SetEmotion(TEXT("neutral"));
	}
}