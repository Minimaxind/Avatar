// Avatar/FacialAnimationComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FacialAnimationComponent.generated.h"

USTRUCT(BlueprintType)
struct FVisemeTiming
{
    GENERATED_BODY()
    
    UPROPERTY()
    FString VisemeName;
    
    UPROPERTY()
    float StartTime;
    
    UPROPERTY()
    float EndTime;
    
    UPROPERTY()
    float Intensity;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AVATAR_API UFacialAnimationComponent : public UActorComponent
{
    GENERATED_BODY()
    
public:
    UFacialAnimationComponent();
    
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    
    UFUNCTION(BlueprintCallable, Category = "Facial Animation")
    void StartSpeaking(const FString& Text, float Duration);
    
    UFUNCTION(BlueprintCallable, Category = "Facial Animation")
    void StopSpeaking();
    
    UFUNCTION(BlueprintCallable, Category = "Facial Animation")
    bool IsSpeaking() const { return bIsSpeaking; }
    
    UFUNCTION(BlueprintCallable, Category = "Facial Animation")
    void SetEmotion(const FString& Emotion, float Intensity = 1.0f);
    
    // Видимые для назначения в Blueprint
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viseme Animations")
    TMap<FString, UAnimSequence*> VisemeAnimations;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float BlendSpeed = 15.0f;
    
private:
    void GenerateVisemeTimings(const FString& Text, float Duration);
    void UpdateCurrentViseme(float DeltaTime);
    void PlayVisemeWithBlend(const FString& VisemeName, float BlendTime);
    void PlayVisemeWithIntensity(const FString& VisemeName, float Intensity);
    FString CharToViseme(TCHAR Char);
    
    UPROPERTY()
    class USkeletalMeshComponent* SkeletalMesh;
    
    TArray<FVisemeTiming> VisemeTimings;
    int32 CurrentVisemeIndex = 0;
    bool bIsSpeaking = false;
    float CurrentSpeechTime = 0.0f;
    float TotalSpeechDuration = 0.0f;
    
    // Для плавного блендинга
    float VisemeTimer = 0.0f;
};