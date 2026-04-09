#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FacialAnimationComponent.generated.h"

class UAvatarAnimInstance;
class UAnimSequence;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AVATAR_API UFacialAnimationComponent : public UActorComponent
{
    GENERATED_BODY()
    
public:
    UFacialAnimationComponent();
    
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    
    UFUNCTION(BlueprintCallable, Category = "Facial Animation")
    void StartTalking(const FString& Text, float Duration = 0.0f);
    
    UFUNCTION(BlueprintCallable, Category = "Facial Animation")
    void StopTalking();
    
    UFUNCTION(BlueprintCallable, Category = "Facial Animation")
    void SetEmotion(const FString& Emotion, float Intensity = 1.0f);
    
    // Viseme Animations
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
    
    UPROPERTY()
    class USkeletalMeshComponent* SkeletalMesh;
    
    UPROPERTY()
    UAvatarAnimInstance* AnimInstance;
    
private:
    void PlayCurrentViseme();
    UAnimSequence* GetAnimationForChar(TCHAR Char);
    bool IsVowel(TCHAR Char);
    void TestAnimation();
    bool IsAnimationCompatible(UAnimSequence* Animation);
    
    TMap<TCHAR, UAnimSequence*> CharToAnimMap;
    TArray<UAnimSequence*> VisemeSequence;
    
    int32 CurrentVisemeIndex = 0;
    float CurrentVisemeDuration = 0.1f;
    float TimeInCurrentViseme = 0.0f;
    float CurrentSpeechTime = 0.0f;
    float SpeechDuration = 0.0f;
    bool bIsTalking = false;
};