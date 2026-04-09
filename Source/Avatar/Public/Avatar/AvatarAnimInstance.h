#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "AvatarAnimInstance.generated.h"

UCLASS()
class AVATAR_API UAvatarAnimInstance : public UAnimInstance
{
    GENERATED_BODY()
    
public:
    UAvatarAnimInstance();
    
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    bool bIsTalking = false;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    bool bIsListening = false;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    float EmotionalIntensity = 1.0f;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Facial")
    float EyeBlink = 0.0f;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Facial")
    float JawOpen = 0.0f;
    
    UFUNCTION(BlueprintCallable, Category = "Facial")
    void SetTalking(bool bTalking);
    
    UFUNCTION(BlueprintCallable, Category = "Facial")
    void SetListening(bool bListening);
    
    UFUNCTION(BlueprintCallable, Category = "Facial")
    void SetEmotion(const FString& Emotion, float Intensity = 1.0f);
    
private:
    void UpdateBlinking(float DeltaSeconds);
    
    float TimeSinceLastBlink = 0.0f;
    float NextBlinkTime = 0.0f;
    float BlinkDuration = 0.15f;
    bool bIsBlinking = false;
    float BlinkProgress = 0.0f;
    
    FString CurrentEmotion = TEXT("neutral");
};