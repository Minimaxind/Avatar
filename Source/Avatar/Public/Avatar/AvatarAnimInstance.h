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
    
    // Основные параметры анимации
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Facial")
    float JawOpen = 0.0f;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Facial")
    float LipsPucker = 0.0f;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Facial")
    float MouthSmile = 0.0f;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Facial")
    float EyeBlink = 0.0f;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    bool bIsTalking = false;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    bool bIsListening = false;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    float EmotionalIntensity = 1.0f;
    
    // Управление анимацией
    UFUNCTION(BlueprintCallable, Category = "Facial")
    void SetTalking(bool bTalking);
    
    UFUNCTION(BlueprintCallable, Category = "Facial")
    void SetListening(bool bListening);
    
    UFUNCTION(BlueprintCallable, Category = "Facial")
    void SetEmotion(const FString& Emotion, float Intensity = 1.0f);
    
    UFUNCTION(BlueprintCallable, Category = "Facial")
    void UpdateLipSync(float JawIntensity);
    
private:
    void UpdateBlinking(float DeltaSeconds);
    
    float TimeSinceLastBlink;
    float NextBlinkTime;
    float BlinkDuration;
    bool bIsBlinking;
    float BlinkProgress;
    
    FString CurrentEmotion;
};