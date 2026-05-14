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
    void StartSpeaking(const FString& Text, float Duration = 0.0f);
    
    UFUNCTION(BlueprintCallable, Category = "Facial Animation")
    void StopSpeaking();
    
    UFUNCTION(BlueprintCallable, Category = "Facial Animation")
    bool IsSpeaking() const { return bIsSpeaking; }
    
    UFUNCTION(BlueprintCallable, Category = "Facial Animation")
    void SetEmotion(const FString& Emotion, float Intensity = 1.0f);
    
    UFUNCTION(BlueprintCallable, Category = "Facial Animation")
    void SetBlendshapeValue(const FString& BlendshapeName, float Value);
    
    UFUNCTION(BlueprintCallable, Category = "Facial Animation")
    void ResetBlendshapes();
    
    // ВСЕ ВАШИ ВИЗЕМЫ
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viseme Animations")
    UAnimSequence* AS_A1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viseme Animations")
    UAnimSequence* AS_O1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viseme Animations")
    UAnimSequence* AS_Yo1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viseme Animations")
    UAnimSequence* AS_U1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viseme Animations")
    UAnimSequence* AS_E1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viseme Animations")
    UAnimSequence* AS_I1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viseme Animations")
    UAnimSequence* AS_B1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viseme Animations")
    UAnimSequence* AS_V1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viseme Animations")
    UAnimSequence* AS_M1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viseme Animations")
    UAnimSequence* AS_T1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viseme Animations")
    UAnimSequence* AS_S1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viseme Animations")
    UAnimSequence* AS_Sh1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viseme Animations")
    UAnimSequence* AS_Ch1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viseme Animations")
    UAnimSequence* AS_K1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viseme Animations")
    UAnimSequence* AS_L1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viseme Animations")
    UAnimSequence* AS_R1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viseme Animations")
    UAnimSequence* AS_Idle;
    
    static const TArray<FString> GetARKitBlendshapeNames();
    
private:
    void GenerateVisemeTimings(const FString& Text, float Duration);
    void UpdateViseme(float DeltaTime);
    void ApplyBlendshapesToMetaHuman();
    void UpdateBlinking(float DeltaTime);
    void PlayCurrentViseme();  // <--- ДОБАВИТЬ ЭТУ СТРОКУ!
    
    UPROPERTY()
    class USkeletalMeshComponent* SkeletalMeshComponent;
    
    // Карта символов на виземы
    TMap<TCHAR, UAnimSequence*> CharToAnimMap;
    TArray<UAnimSequence*> VisemeSequence;
    
    bool bIsSpeaking = false;
    float CurrentSpeechTime = 0.0f;
    float TotalSpeechDuration = 0.0f;
    int32 CurrentVisemeIndex = 0;
    float CurrentVisemeDuration = 0.1f;
    float TimeInCurrentViseme = 0.0f;
    
    // Эмоции
    FString CurrentEmotion = TEXT("neutral");
    float EmotionIntensity = 0.0f;
    
    // Моргание
    float TimeSinceLastBlink = 0.0f;
    float NextBlinkTime = 0.0f;
    bool bIsBlinking = false;
    float BlinkProgress = 0.0f;
};