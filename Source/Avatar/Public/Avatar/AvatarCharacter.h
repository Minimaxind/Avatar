#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AvatarCharacter.generated.h"

class UFacialAnimationComponent;
class UAudioComponent;
class UAvatarGameInstance;

UCLASS()
class AVATAR_API AAvatarCharacter : public ACharacter
{
	GENERATED_BODY()
    
public:
	AAvatarCharacter();
    
	virtual void BeginPlay() override;
    
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UFacialAnimationComponent* FacialAnimation;
    
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UAudioComponent* VoiceAudioComponent;
    
	UFUNCTION(BlueprintCallable, Category = "Avatar")
	void Speak(const FString& Text, float Duration = 0.0f);
    
	UFUNCTION(BlueprintImplementableEvent, Category = "Avatar")
	void OnStartSpeaking();
    
	UFUNCTION(BlueprintImplementableEvent, Category = "Avatar")
	void OnStopSpeaking();
    
private:
	UFUNCTION()
	void HandleAvatarResponse(const FString& Response);
    
	UPROPERTY()
	UAvatarGameInstance* GameInstance;
    
	FTimerHandle SpeechTimerHandle;
};