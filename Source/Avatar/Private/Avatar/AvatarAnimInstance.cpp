#include "Avatar/AvatarAnimInstance.h"

UAvatarAnimInstance::UAvatarAnimInstance()
{
    NextBlinkTime = FMath::RandRange(2.0f, 5.0f);
}

void UAvatarAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
}

void UAvatarAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);
    UpdateBlinking(DeltaSeconds);
}

void UAvatarAnimInstance::SetTalking(bool bTalking)
{
    bIsTalking = bTalking;
}

void UAvatarAnimInstance::SetListening(bool bListening)
{
    bIsListening = bListening;
}

void UAvatarAnimInstance::SetEmotion(const FString& Emotion, float Intensity)
{
    CurrentEmotion = Emotion;
    EmotionalIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
}

void UAvatarAnimInstance::UpdateBlinking(float DeltaSeconds)
{
    TimeSinceLastBlink += DeltaSeconds;
    
    if (!bIsBlinking && TimeSinceLastBlink >= NextBlinkTime)
    {
        bIsBlinking = true;
        BlinkProgress = 0.0f;
    }
    
    if (bIsBlinking)
    {
        BlinkProgress += DeltaSeconds / BlinkDuration;
        
        if (BlinkProgress <= 0.5f)
        {
            EyeBlink = BlinkProgress * 2.0f;
        }
        else
        {
            EyeBlink = 1.0f - (BlinkProgress - 0.5f) * 2.0f;
        }
        
        if (BlinkProgress >= 1.0f)
        {
            bIsBlinking = false;
            TimeSinceLastBlink = 0.0f;
            NextBlinkTime = FMath::RandRange(2.0f, 5.0f);
            EyeBlink = 0.0f;
        }
    }
}