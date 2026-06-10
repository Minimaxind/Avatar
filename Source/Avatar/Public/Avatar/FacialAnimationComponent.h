// Avatar/FacialAnimationComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FacialAnimationComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AVATAR_API UFacialAnimationComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Test")
    void EmergencyTest();
    
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

    // -------------------------------------------------------
    // Viseme Settings
    // -------------------------------------------------------
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viseme Settings")
    FName VisemeSlotName = FName("DefaultGroup.VisemeSlot");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viseme Settings")
    float VisemeBlendIn = 0.04f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viseme Settings")
    float VisemeBlendOut = 0.04f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viseme Settings")
    float VisemePlayRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viseme Settings")
    bool bUseMetaHumanCurves = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viseme Settings")
    float VisemeMorphIntensity = 1.0f;

    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viseme Settings")
    float InterpolationSpeed = 10.0f;

    // -------------------------------------------------------
    // Head Movement Settings
    // -------------------------------------------------------

 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Head Movement")
    bool bEnableHeadMovement = true;

    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Head Movement")
    float SpeakingNodAmplitude = 3.5f;

   
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Head Movement")
    float SpeakingTiltAmplitude = 2.5f;

    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Head Movement")
    float SpeakingYawAmplitude = 2.0f;

   
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Head Movement")
    float IdleMovementSpeed = 0.4f;

   
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Head Movement")
    float IdleMovementAmplitude = 1.2f;

   
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Head Movement")
    float HeadInterpolationSpeed = 6.0f;

    // -------------------------------------------------------
    // Lip Realism Settings
    // -------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lip Realism")
    bool bEnableLipAsymmetry = true;


    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lip Realism")
    float LipAsymmetryAmount = 0.08f;

  
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lip Realism")
    bool bEnableCheekMovement = true;


    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lip Realism")
    bool bEnableMicroExpressions = true;

    // -------------------------------------------------------
    // Viseme Animations
    // -------------------------------------------------------
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viseme Animations")
    UAnimSequence* AS_P1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viseme Animations")
    UAnimSequence* AS_F1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viseme Animations")
    UAnimSequence* AS_D1;

private:
    UPROPERTY()
    USkeletalMeshComponent* FaceMeshComponent;

    UPROPERTY()
    USkeletalMeshComponent* BodyMeshComponent;

    TMap<TCHAR, UAnimSequence*> CharToAnimMap;
    TArray<UAnimSequence*> VisemeQueue;

    bool bIsSpeaking = false;
    float CurrentSpeechTime = 0.0f;
    float TotalSpeechDuration = 0.0f;
    int32 CurrentVisemeIndex = 0;
    float CurrentVisemeDuration = 0.1f;
    float TimeInCurrentViseme = 0.0f;

  
    float TimeSinceLastBlink = 0.0f;
    float NextBlinkTime = 0.0f;
    bool bIsBlinking = false;
    float BlinkProgress = 0.0f;

   
    float CurrentJawOpen = 0.0f;
    float CurrentMouthFunnel = 0.0f;
    float CurrentMouthPucker = 0.0f;
    float CurrentMouthStretch = 0.0f;

    float TargetJawOpen = 0.0f;
    float TargetMouthFunnel = 0.0f;
    float TargetMouthPucker = 0.0f;
    float TargetMouthStretch = 0.0f;

  
    float CurrentMouthCornerUp = 0.0f;
    float CurrentMouthCornerDown = 0.0f;
    float CurrentBrowRaise = 0.0f;

    float TargetMouthCornerUp = 0.0f;
    float TargetMouthCornerDown = 0.0f;
    float TargetBrowRaise = 0.0f;

    // -------------------------------------------------------
    // Head Movement — внутреннее состояние
    // -------------------------------------------------------

    // Текущий и целевой поворот головы
    FRotator CurrentHeadRotation = FRotator::ZeroRotator;
    FRotator TargetHeadRotation  = FRotator::ZeroRotator;

    // Базовый RelativeRotation меша
    FRotator HeadMeshBaseRotation = FRotator::ZeroRotator;
    bool bHeadBaseSet = false;
    
    float HeadNoisePhaseP = 0.0f;   // Pitch
    float HeadNoisePhaseY = 0.0f;   // Yaw
    float HeadNoisePhaseR = 0.0f;   // Roll


    float HeadNodTimer = 0.0f;
    float HeadNodInterval = 0.0f;


    float LipAsymmetrySign = 1.0f;
    float LipAsymmetryTimer = 0.0f;


    float MicroExprTimer = 0.0f;
    float MicroExprInterval = 0.0f;
    float CurrentMicroExpr = 0.0f;
    float TargetMicroExpr = 0.0f;


    float CurrentCheekPuff = 0.0f;
    float TargetCheekPuff = 0.0f;

    // Общее время 
    float TotalTime = 0.0f;

    void BuildCharMap();
    void PlayVisemeAtIndex(int32 Index);
    void PlayVisemeWithCurves(int32 Index);
    void UpdateBlinking(float DeltaTime);
    void UpdateHeadMovement(float DeltaTime);
    void UpdateLipRealism(float DeltaTime);
    void SetMetaHumanCurve(const FName& CurveName, float Value, UAnimInstance* AnimInst);


    static float SmoothNoise(float Phase);
};