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
    
	UE_LOG(LogTemp, Warning, TEXT("=== AvatarCharacter::BeginPlay ==="));
    
	GameInstance = Cast<UAvatarGameInstance>(GetGameInstance());
    
	if (GameInstance)
	{
		GameInstance->OnAvatarResponse.AddDynamic(this, &AAvatarCharacter::HandleAvatarResponse);
	}
    
	// Проверяем компонент
	if (FacialAnimation)
	{
		UE_LOG(LogTemp, Warning, TEXT("FacialAnimation component exists"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("FacialAnimation component is NULL!"));
	}
    
	// ТЕСТ: запустить анимацию через 2 секунды
	FTimerHandle TestTimer;
	GetWorldTimerManager().SetTimer(TestTimer, [this]()
	{
		UE_LOG(LogTemp, Warning, TEXT("=== TEST ANIMATION START ==="));
		Speak(TEXT("Привет мир тест анимации"), 3.0f);
	}, 2.0f, false);
}

void AAvatarCharacter::HandleAvatarResponse(const FString& Response)
{
	Speak(Response);
}

void AAvatarCharacter::Speak(const FString& Text, float Duration)
{
	if (Text.IsEmpty()) return;
    
	if (Duration <= 0.0f)
	{
		Duration = Text.Len() * 0.1f;
	}
    
	OnStartSpeaking();
    
	if (FacialAnimation)
	{
		FacialAnimation->StartTalking(Text, Duration);
	}
    
	GetWorldTimerManager().ClearTimer(SpeechTimerHandle);
	GetWorldTimerManager().SetTimer(SpeechTimerHandle, [this]()
	{
		OnStopSpeaking();
		if (FacialAnimation)
		{
			FacialAnimation->StopTalking();
		}
	}, Duration, false);
}