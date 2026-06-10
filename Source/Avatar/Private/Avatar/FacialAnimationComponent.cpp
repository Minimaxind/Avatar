// Avatar/FacialAnimationComponent.cpp
#include "Avatar/FacialAnimationComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"

UFacialAnimationComponent::UFacialAnimationComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UFacialAnimationComponent::BeginPlay()
{
    Super::BeginPlay();

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        UE_LOG(LogTemp, Error, TEXT("[FAC] No owner actor!"));
        return;
    }

    TArray<USkeletalMeshComponent*> SkelMeshes;
    Owner->GetComponents<USkeletalMeshComponent>(SkelMeshes);

    UE_LOG(LogTemp, Warning, TEXT("[FAC] SkeletalMesh components on owner (%d):"), SkelMeshes.Num());
    for (USkeletalMeshComponent* Mesh : SkelMeshes)
    {
        UAnimInstance* Inst = Mesh->GetAnimInstance();
        UE_LOG(LogTemp, Warning, TEXT("[FAC]   [%s]  AnimInstance: %s"),
            *Mesh->GetName(),
            Inst ? *Inst->GetClass()->GetName() : TEXT("NULL"));
    }

    for (USkeletalMeshComponent* Mesh : SkelMeshes)
    {
        if (Mesh->GetName().Equals(TEXT("Face"), ESearchCase::IgnoreCase))
        {
            FaceMeshComponent = Mesh;
            UE_LOG(LogTemp, Warning, TEXT("[FAC] >>> Face mesh found: %s"), *Mesh->GetName());
        }
        else if (Mesh->GetName().Equals(TEXT("Body"), ESearchCase::IgnoreCase))
        {
            BodyMeshComponent = Mesh;
            UE_LOG(LogTemp, Warning, TEXT("[FAC] >>> Body mesh found: %s"), *Mesh->GetName());
        }
    }

    if (!FaceMeshComponent)
    {
        for (USkeletalMeshComponent* Mesh : SkelMeshes)
        {
            FString N = Mesh->GetName().ToLower();
            if (N.Contains(TEXT("body")) || N.Contains(TEXT("hair")) ||
                N.Contains(TEXT("eyebrow")) || N.Contains(TEXT("fuzz")) ||
                N.Contains(TEXT("eyelash")) || N.Contains(TEXT("beard")))
            {
                continue;
            }
            if (Mesh->GetAnimInstance())
            {
                FaceMeshComponent = Mesh;
                UE_LOG(LogTemp, Warning, TEXT("[FAC] >>> Fallback face mesh: %s"), *Mesh->GetName());
                break;
            }
        }
    }

    if (!FaceMeshComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("[FAC] CRITICAL: Face mesh not found!"));
        return;
    }

    UAnimInstance* AnimInst = FaceMeshComponent->GetAnimInstance();
    if (!AnimInst)
    {
        UE_LOG(LogTemp, Error, TEXT("[FAC] CRITICAL: AnimInstance is NULL on Face mesh!"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[FAC] AnimInstance OK: %s"), *AnimInst->GetClass()->GetName());

        
        {
            FProperty* Prop = AnimInst->GetClass()->FindPropertyByName(FName("Eable Facial Animation"));
            if (!Prop) Prop = AnimInst->GetClass()->FindPropertyByName(FName("EnableFacialAnimation"));
            if (Prop && Prop->IsA<FBoolProperty>())
            {
                CastField<FBoolProperty>(Prop)->SetPropertyValue_InContainer(AnimInst, true);
                UE_LOG(LogTemp, Warning, TEXT("[FAC] Set facial anim bool = true via reflection"));
            }
        }
    }

    BuildCharMap();
    NextBlinkTime = FMath::RandRange(2.0f, 5.0f);

    /
    {
        USkeletalMeshComponent* HeadMesh = BodyMeshComponent ? BodyMeshComponent : FaceMeshComponent;
        if (HeadMesh)
        {
            HeadMeshBaseRotation = HeadMesh->GetRelativeRotation();
            bHeadBaseSet = true;
        }
    }

    
    HeadNoisePhaseP = FMath::RandRange(0.0f, 100.0f);
    HeadNoisePhaseY = FMath::RandRange(0.0f, 100.0f);
    HeadNoisePhaseR = FMath::RandRange(0.0f, 100.0f);

    
    HeadNodInterval = FMath::RandRange(0.8f, 1.6f);

    LipAsymmetrySign = (FMath::RandBool()) ? 1.0f : -1.0f;

   
    MicroExprInterval = FMath::RandRange(2.0f, 5.0f);
}

// ---------------------------------------------------------------------------
// Простой "шум" 
// ---------------------------------------------------------------------------
float UFacialAnimationComponent::SmoothNoise(float Phase)
{
    return FMath::Sin(Phase * 1.0f) * 0.5f
         + FMath::Sin(Phase * 2.3f) * 0.3f
         + FMath::Sin(Phase * 5.1f) * 0.2f;
}

// ---------------------------------------------------------------------------
// TICK
// ---------------------------------------------------------------------------
void UFacialAnimationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    TotalTime += DeltaTime;

    
    if (bIsSpeaking)
    {
        CurrentSpeechTime += DeltaTime;
        if (CurrentSpeechTime >= TotalSpeechDuration)
        {
            StopSpeaking();
            
        }
        else
        {
            TimeInCurrentViseme += DeltaTime;
            if (TimeInCurrentViseme >= CurrentVisemeDuration)
            {
                TimeInCurrentViseme = 0.0f;
                CurrentVisemeIndex++;
                if (CurrentVisemeIndex >= VisemeQueue.Num())
                    CurrentVisemeIndex = FMath::Max(0, VisemeQueue.Num() - 1);
                PlayVisemeAtIndex(CurrentVisemeIndex);
            }
        }
    }

    // ---- Interpolate lipsync targets ----
    float LipSpeed = bIsSpeaking ? InterpolationSpeed : InterpolationSpeed * 0.6f;
    CurrentJawOpen      = FMath::FInterpTo(CurrentJawOpen,      TargetJawOpen,      DeltaTime, LipSpeed);
    CurrentMouthFunnel  = FMath::FInterpTo(CurrentMouthFunnel,  TargetMouthFunnel,  DeltaTime, LipSpeed);
    CurrentMouthPucker  = FMath::FInterpTo(CurrentMouthPucker,  TargetMouthPucker,  DeltaTime, LipSpeed);
    CurrentMouthStretch = FMath::FInterpTo(CurrentMouthStretch, TargetMouthStretch, DeltaTime, LipSpeed);

    // ---- Interpolate emotion targets ----
    CurrentMouthCornerUp   = FMath::FInterpTo(CurrentMouthCornerUp,   TargetMouthCornerUp,   DeltaTime, 12.0f);
    CurrentMouthCornerDown = FMath::FInterpTo(CurrentMouthCornerDown, TargetMouthCornerDown, DeltaTime, 12.0f);
    CurrentBrowRaise       = FMath::FInterpTo(CurrentBrowRaise,       TargetBrowRaise,       DeltaTime, 12.0f);

    // ---- Apply curves every frame ----
    if (bUseMetaHumanCurves && FaceMeshComponent)
    {
        UAnimInstance* AnimInst = FaceMeshComponent->GetAnimInstance();
        if (AnimInst)
        {
          
            UpdateLipRealism(DeltaTime);

            
            float JawL = CurrentJawOpen;
            float JawR = CurrentJawOpen;
            float StretchL = CurrentMouthStretch;
            float StretchR = CurrentMouthStretch;

            if (bEnableLipAsymmetry)
            {
                float Asym = CurrentJawOpen * LipAsymmetryAmount * LipAsymmetrySign;
                JawL = FMath::Clamp(CurrentJawOpen + Asym, 0.0f, 1.0f);
                JawR = FMath::Clamp(CurrentJawOpen - Asym, 0.0f, 1.0f);

                float StrAsym = CurrentMouthStretch * LipAsymmetryAmount * 0.5f * LipAsymmetrySign;
                StretchL = FMath::Clamp(CurrentMouthStretch + StrAsym, 0.0f, 1.0f);
                StretchR = FMath::Clamp(CurrentMouthStretch - StrAsym, 0.0f, 1.0f);
            }

            // ---- ARKit curves ----
            SetMetaHumanCurve(FName("jawOpen"), (JawL + JawR) * 0.5f, AnimInst);
            SetMetaHumanCurve(FName("mouthFunnel"),  CurrentMouthFunnel,  AnimInst);
            SetMetaHumanCurve(FName("mouthPucker"),  CurrentMouthPucker,  AnimInst);
            SetMetaHumanCurve(FName("mouthStretchLeft"),  StretchL, AnimInst);
            SetMetaHumanCurve(FName("mouthStretchRight"), StretchR, AnimInst);

            // Lips follow jaw
            float UpperLipL = FMath::Clamp(JawL * 0.3f, 0.0f, 0.5f);
            float UpperLipR = FMath::Clamp(JawR * 0.3f, 0.0f, 0.5f);
            float LowerLipL = FMath::Clamp(JawL * 0.5f, 0.0f, 0.7f);
            float LowerLipR = FMath::Clamp(JawR * 0.5f, 0.0f, 0.7f);
            SetMetaHumanCurve(FName("mouthUpperUpLeft"),    UpperLipL, AnimInst);
            SetMetaHumanCurve(FName("mouthUpperUpRight"),   UpperLipR, AnimInst);
            SetMetaHumanCurve(FName("mouthLowerDownLeft"),  LowerLipL, AnimInst);
            SetMetaHumanCurve(FName("mouthLowerDownRight"), LowerLipR, AnimInst);

            // Эмоции ARKit
            float CornerUpL = CurrentMouthCornerUp + (bEnableMicroExpressions ? CurrentMicroExpr * 0.5f : 0.0f);
            float CornerUpR = CurrentMouthCornerUp - (bEnableMicroExpressions ? CurrentMicroExpr * 0.3f : 0.0f);
            SetMetaHumanCurve(FName("mouthCornerPullLeft"),    FMath::Clamp(CornerUpL, 0.0f, 1.0f), AnimInst);
            SetMetaHumanCurve(FName("mouthCornerPullRight"),   FMath::Clamp(CornerUpR, 0.0f, 1.0f), AnimInst);
            SetMetaHumanCurve(FName("mouthCornerDepressLeft"), CurrentMouthCornerDown, AnimInst);
            SetMetaHumanCurve(FName("mouthCornerDepressRight"),CurrentMouthCornerDown, AnimInst);
            SetMetaHumanCurve(FName("browInnerUp"),            CurrentBrowRaise,       AnimInst);

            // Щёки
            if (bEnableCheekMovement)
            {
                SetMetaHumanCurve(FName("cheekPuff"), CurrentCheekPuff, AnimInst);
                SetMetaHumanCurve(FName("cheekSquintLeft"),  CurrentCheekPuff * 0.4f, AnimInst);
                SetMetaHumanCurve(FName("cheekSquintRight"), CurrentCheekPuff * 0.4f, AnimInst);
            }

            // ---- Control Rig curves ----
            SetMetaHumanCurve(FName("CTRL_expressions_jawOpen"),        (JawL + JawR) * 0.5f, AnimInst);
            SetMetaHumanCurve(FName("CTRL_expressions_mouthFunnel"),    CurrentMouthFunnel, AnimInst);
            SetMetaHumanCurve(FName("CTRL_expressions_mouthPucker"),    CurrentMouthPucker, AnimInst);
            SetMetaHumanCurve(FName("CTRL_expressions_mouthCornerWideL"), StretchL, AnimInst);
            SetMetaHumanCurve(FName("CTRL_expressions_mouthCornerWideR"), StretchR, AnimInst);

            SetMetaHumanCurve(FName("CTRL_expressions_mouthUpperLipTowardsTeethL"), UpperLipL, AnimInst);
            SetMetaHumanCurve(FName("CTRL_expressions_mouthUpperLipTowardsTeethR"), UpperLipR, AnimInst);
            SetMetaHumanCurve(FName("CTRL_expressions_mouthLowerLipTowardsTeethL"), LowerLipL, AnimInst);
            SetMetaHumanCurve(FName("CTRL_expressions_mouthLowerLipTowardsTeethR"), LowerLipR, AnimInst);

            SetMetaHumanCurve(FName("CTRL_expressions_mouthCornerUpL"),   FMath::Clamp(CornerUpL, 0.0f, 1.0f), AnimInst);
            SetMetaHumanCurve(FName("CTRL_expressions_mouthCornerUpR"),   FMath::Clamp(CornerUpR, 0.0f, 1.0f), AnimInst);
            SetMetaHumanCurve(FName("CTRL_expressions_mouthCornerDownL"), CurrentMouthCornerDown, AnimInst);
            SetMetaHumanCurve(FName("CTRL_expressions_mouthCornerDownR"), CurrentMouthCornerDown, AnimInst);
            SetMetaHumanCurve(FName("CTRL_expressions_browRaiselnL"),     CurrentBrowRaise, AnimInst);
            SetMetaHumanCurve(FName("CTRL_expressions_browRaiselnR"),     CurrentBrowRaise, AnimInst);
        }
    }

    UpdateBlinking(DeltaTime);
    UpdateHeadMovement(DeltaTime);
}

// ---------------------------------------------------------------------------
// HEAD MOVEMENT
// ---------------------------------------------------------------------------
void UFacialAnimationComponent::UpdateHeadMovement(float DeltaTime)
{
    if (!bEnableHeadMovement) return;

    // Выбираем меш для управления костью (предпочитаем Body, иначе Face)
    USkeletalMeshComponent* MeshForBone = BodyMeshComponent ? BodyMeshComponent : FaceMeshComponent;
    if (!MeshForBone) return;

    // ---- Продвигаем фазы шума ----
    float NoiseSpeakMult = bIsSpeaking ? 1.8f : 1.0f;
    HeadNoisePhaseP += DeltaTime * (0.7f * NoiseSpeakMult);
    HeadNoisePhaseY += DeltaTime * (0.5f * NoiseSpeakMult);
    HeadNoisePhaseR += DeltaTime * (0.6f * NoiseSpeakMult);

    // ---- Idle-движение в покое ----
    FRotator IdleRot = FRotator::ZeroRotator;
    {
        float Amp = IdleMovementAmplitude;
        IdleRot.Pitch = SmoothNoise(HeadNoisePhaseP * IdleMovementSpeed) * Amp;
        IdleRot.Yaw   = SmoothNoise(HeadNoisePhaseY * IdleMovementSpeed) * Amp * 0.8f;
        IdleRot.Roll  = SmoothNoise(HeadNoisePhaseR * IdleMovementSpeed) * Amp * 0.5f;
    }

    // ---- Speaking nod ----
    FRotator SpeakRot = FRotator::ZeroRotator;
    if (bIsSpeaking)
    {
        HeadNodTimer += DeltaTime;
        if (HeadNodTimer >= HeadNodInterval)
        {
            HeadNodTimer = 0.0f;
            HeadNodInterval = FMath::RandRange(0.6f, 1.4f);

            
            TargetHeadRotation.Pitch = FMath::RandRange(-SpeakingNodAmplitude, SpeakingNodAmplitude * 0.4f);
            TargetHeadRotation.Yaw   = FMath::RandRange(-SpeakingYawAmplitude,  SpeakingYawAmplitude);
            TargetHeadRotation.Roll  = FMath::RandRange(-SpeakingTiltAmplitude, SpeakingTiltAmplitude);
        }

        
        SpeakRot.Pitch = SmoothNoise(HeadNoisePhaseP * 2.5f) * SpeakingNodAmplitude  * 0.4f;
        SpeakRot.Yaw   = SmoothNoise(HeadNoisePhaseY * 2.0f) * SpeakingYawAmplitude  * 0.3f;
        SpeakRot.Roll  = SmoothNoise(HeadNoisePhaseR * 1.8f) * SpeakingTiltAmplitude * 0.3f;
    }
    else
    {
   
        TargetHeadRotation = FMath::RInterpTo(TargetHeadRotation, FRotator::ZeroRotator, DeltaTime, 2.0f);
    }

  
    FRotator DesiredRot;
    DesiredRot.Pitch = TargetHeadRotation.Pitch + IdleRot.Pitch + SpeakRot.Pitch;
    DesiredRot.Yaw   = TargetHeadRotation.Yaw   + IdleRot.Yaw   + SpeakRot.Yaw;
    DesiredRot.Roll  = TargetHeadRotation.Roll  + IdleRot.Roll  + SpeakRot.Roll;


    const float MaxPitch = 15.0f, MaxYaw = 12.0f, MaxRoll = 8.0f;
    DesiredRot.Pitch = FMath::Clamp(DesiredRot.Pitch, -MaxPitch, MaxPitch);
    DesiredRot.Yaw   = FMath::Clamp(DesiredRot.Yaw,   -MaxYaw,   MaxYaw);
    DesiredRot.Roll  = FMath::Clamp(DesiredRot.Roll,  -MaxRoll,  MaxRoll);


    CurrentHeadRotation = FMath::RInterpTo(CurrentHeadRotation, DesiredRot, DeltaTime, HeadInterpolationSpeed);


    if (bHeadBaseSet)
    {
        USkeletalMeshComponent* HeadMesh = BodyMeshComponent ? BodyMeshComponent : FaceMeshComponent;
        if (HeadMesh)
        {
            FRotator FinalRot;
            FinalRot.Pitch = HeadMeshBaseRotation.Pitch + CurrentHeadRotation.Pitch;
            FinalRot.Yaw   = HeadMeshBaseRotation.Yaw   + CurrentHeadRotation.Yaw;
            FinalRot.Roll  = HeadMeshBaseRotation.Roll  + CurrentHeadRotation.Roll;
            HeadMesh->SetRelativeRotation(FinalRot);
        }
    }
}

// ---------------------------------------------------------------------------
// LIP REALISM 
// ---------------------------------------------------------------------------
void UFacialAnimationComponent::UpdateLipRealism(float DeltaTime)
{
    // --- Асимметрия: периодически меняем сторону ---
    if (bEnableLipAsymmetry)
    {
        LipAsymmetryTimer += DeltaTime;
        if (LipAsymmetryTimer > FMath::RandRange(3.0f, 7.0f))
        {
            LipAsymmetryTimer = 0.0f;
            LipAsymmetrySign = (FMath::RandBool()) ? 1.0f : -1.0f;
        }
    }

 
    if (bEnableMicroExpressions)
    {
        MicroExprTimer += DeltaTime;
        if (MicroExprTimer >= MicroExprInterval)
        {
            MicroExprTimer = 0.0f;
            MicroExprInterval = FMath::RandRange(1.5f, 4.0f);
            // Во время речи micro-expressions сильнее
            float MaxMicro = bIsSpeaking ? 0.12f : 0.05f;
            TargetMicroExpr = FMath::RandRange(-MaxMicro, MaxMicro);
        }
        CurrentMicroExpr = FMath::FInterpTo(CurrentMicroExpr, TargetMicroExpr, DeltaTime, 8.0f);
    }

   
    if (bEnableCheekMovement)
    {
        
        float CheekTarget = 0.0f;
        if (bIsSpeaking)
        {
            
            CheekTarget = FMath::Clamp(CurrentMouthFunnel * 0.3f + CurrentMouthPucker * 0.2f, 0.0f, 0.25f);
            
            CheekTarget += FMath::Clamp(CurrentJawOpen * 0.1f, 0.0f, 0.1f);
        }
        TargetCheekPuff = CheekTarget;
        CurrentCheekPuff = FMath::FInterpTo(CurrentCheekPuff, TargetCheekPuff, DeltaTime, 10.0f);
    }
}

// ---------------------------------------------------------------------------
// BUILD CHAR MAP
// ---------------------------------------------------------------------------
void UFacialAnimationComponent::BuildCharMap()
{
    CharToAnimMap.Empty();

    CharToAnimMap.Add(TEXT('а'), AS_A1);
    CharToAnimMap.Add(TEXT('я'), AS_A1);
    CharToAnimMap.Add(TEXT('о'), AS_O1);
    CharToAnimMap.Add(TEXT('ё'), AS_Yo1);
    CharToAnimMap.Add(TEXT('у'), AS_U1);
    CharToAnimMap.Add(TEXT('ю'), AS_U1);
    CharToAnimMap.Add(TEXT('э'), AS_E1);
    CharToAnimMap.Add(TEXT('е'), AS_E1);
    CharToAnimMap.Add(TEXT('и'), AS_I1);
    CharToAnimMap.Add(TEXT('ы'), AS_I1);
    CharToAnimMap.Add(TEXT('й'), AS_I1);
    CharToAnimMap.Add(TEXT('б'), AS_B1);
    CharToAnimMap.Add(TEXT('п'), AS_P1 ? AS_P1 : AS_B1);
    CharToAnimMap.Add(TEXT('м'), AS_M1);
    CharToAnimMap.Add(TEXT('в'), AS_V1);
    CharToAnimMap.Add(TEXT('ф'), AS_F1 ? AS_F1 : AS_V1);
    CharToAnimMap.Add(TEXT('т'), AS_T1);
    CharToAnimMap.Add(TEXT('д'), AS_D1 ? AS_D1 : AS_T1);
    CharToAnimMap.Add(TEXT('н'), AS_T1);
    CharToAnimMap.Add(TEXT('с'), AS_S1);
    CharToAnimMap.Add(TEXT('з'), AS_S1);
    CharToAnimMap.Add(TEXT('ц'), AS_S1);
    CharToAnimMap.Add(TEXT('ш'), AS_Sh1);
    CharToAnimMap.Add(TEXT('щ'), AS_Sh1);
    CharToAnimMap.Add(TEXT('ж'), AS_Sh1);
    CharToAnimMap.Add(TEXT('ч'), AS_Ch1);
    CharToAnimMap.Add(TEXT('к'), AS_K1);
    CharToAnimMap.Add(TEXT('г'), AS_K1);
    CharToAnimMap.Add(TEXT('х'), AS_K1);
    CharToAnimMap.Add(TEXT('л'), AS_L1);
    CharToAnimMap.Add(TEXT('р'), AS_R1);

    int32 NullCount = 0;
    for (auto& Pair : CharToAnimMap)
    {
        if (!Pair.Value) { NullCount++; }
    }

    if (NullCount > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[FAC] WARNING: %d viseme slots are NULL"), NullCount);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[FAC] All %d viseme slots assigned OK."), CharToAnimMap.Num());
    }
}

// ---------------------------------------------------------------------------
// START / STOP SPEAKING
// ---------------------------------------------------------------------------
void UFacialAnimationComponent::StartSpeaking(const FString& Text, float Duration)
{
    if (Text.IsEmpty()) return;

    if (!FaceMeshComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("[FAC] StartSpeaking: FaceMeshComponent is NULL!"));
        return;
    }

    StopSpeaking();

    FString Lower = Text.ToLower();
    static const FString Skip = TEXT(" ,.!?-;:\n\r\"'()[]");

  
    UAnimSequence* LastAdded = nullptr;
    int32 ConsecutiveConsonants = 0;

    for (int32 i = 0; i < Lower.Len(); i++)
    {
        TCHAR Ch = Lower[i];
        if (Skip.Contains(FString(1, &Ch)))
        {
         
            if (AS_Idle && LastAdded != AS_Idle)
            {
                VisemeQueue.Add(AS_Idle);
                LastAdded = AS_Idle;
            }
            ConsecutiveConsonants = 0;
            continue;
        }

        UAnimSequence** Found = CharToAnimMap.Find(Ch);
        UAnimSequence* Anim = (Found && *Found) ? *Found : AS_Idle;

  
        if (Anim == LastAdded)
        {
            continue;
        }

        VisemeQueue.Add(Anim);
        LastAdded = Anim;

    
        bool bIsVowel = (Anim == AS_A1 || Anim == AS_O1 || Anim == AS_Yo1 ||
                         Anim == AS_U1 || Anim == AS_E1 || Anim == AS_I1);
        if (bIsVowel)
        {
            ConsecutiveConsonants = 0;
        }
        else
        {
            ConsecutiveConsonants++;
            if (ConsecutiveConsonants >= 3 && AS_Idle)
            {
                VisemeQueue.Add(AS_Idle);
                LastAdded = AS_Idle;
                ConsecutiveConsonants = 0;
            }
        }
    }

    if (VisemeQueue.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[FAC] No visemes for text: %s"), *Text);
        return;
    }

    TotalSpeechDuration   = (Duration > 0.0f) ? Duration : Text.Len() * 0.08f;

    CurrentVisemeDuration = FMath::Clamp(TotalSpeechDuration / VisemeQueue.Num(), 0.18f, 0.5f);
    bIsSpeaking          = true;
    CurrentSpeechTime    = 0.0f;
    CurrentVisemeIndex   = 0;
    TimeInCurrentViseme  = 0.0f;

   
    HeadNodInterval = FMath::RandRange(0.6f, 1.2f);
    HeadNodTimer    = HeadNodInterval; 

    PlayVisemeAtIndex(0);
}

void UFacialAnimationComponent::StopSpeaking()
{
    if (!bIsSpeaking) return;

    bIsSpeaking          = false;
    CurrentSpeechTime    = 0.0f;
    CurrentVisemeIndex   = 0;
    TimeInCurrentViseme  = 0.0f;
    VisemeQueue.Empty();

    TargetJawOpen      = 0.0f;
    TargetMouthFunnel  = 0.0f;
    TargetMouthPucker  = 0.0f;
    TargetMouthStretch = 0.0f;
    
    TargetHeadRotation = FRotator::ZeroRotator;
}

// ---------------------------------------------------------------------------
// PLAY VISEME
// ---------------------------------------------------------------------------
void UFacialAnimationComponent::PlayVisemeAtIndex(int32 Index)
{
    if (!FaceMeshComponent) return;

    if (bUseMetaHumanCurves)
        PlayVisemeWithCurves(Index);
    else
    {
        if (!VisemeQueue.IsValidIndex(Index)) return;
        UAnimSequence* Anim = VisemeQueue[Index];
        if (!Anim) return;

        UAnimInstance* AnimInst = FaceMeshComponent->GetAnimInstance();
        if (!AnimInst) return;

        AnimInst->PlaySlotAnimationAsDynamicMontage(Anim, VisemeSlotName,
            VisemeBlendIn, VisemeBlendOut, VisemePlayRate, 1, 0.0f);
    }
}

void UFacialAnimationComponent::PlayVisemeWithCurves(int32 Index)
{
    if (!FaceMeshComponent) return;
    if (!VisemeQueue.IsValidIndex(Index)) return;

    UAnimSequence* Viseme = VisemeQueue[Index];
    if (!Viseme) return;

    float JawOpen      = 0.0f;
    float MouthFunnel  = 0.0f;
    float MouthPucker  = 0.0f;
    float MouthStretch = 0.0f;

    if (Viseme == AS_A1)
    {
        
        JawOpen      = 0.85f;
        MouthStretch = 0.25f;
    }
    else if (Viseme == AS_O1)
    {
        
        JawOpen      = 0.70f;
        MouthFunnel  = 0.35f;
        MouthPucker  = 0.10f;
    }
    else if (Viseme == AS_Yo1)
    {
        
        JawOpen      = 0.55f;
        MouthFunnel  = 0.30f;
        MouthStretch = 0.15f;
    }
    else if (Viseme == AS_U1)
    {
        
        JawOpen      = 0.35f;
        MouthFunnel  = 0.75f;
        MouthPucker  = 0.60f;
    }
    else if (Viseme == AS_E1)
    {
       
        JawOpen      = 0.50f;
        MouthStretch = 0.50f;
    }
    else if (Viseme == AS_I1)
    {
        
        JawOpen      = 0.18f;
        MouthStretch = 0.70f;
    }
    else if (Viseme == AS_B1 || Viseme == AS_P1)
    {
        
        JawOpen = 0.03f;
    }
    else if (Viseme == AS_M1)
    {
        
        JawOpen = 0.02f;
    }
    else if (Viseme == AS_V1 || Viseme == AS_F1)
    {
        
        JawOpen      = 0.12f;
        MouthStretch = 0.25f;
    }
    else if (Viseme == AS_T1)
    {
        
        JawOpen      = 0.18f;
        MouthStretch = 0.10f;
    }
    else if (Viseme == AS_D1)
    {
        
        JawOpen      = 0.22f;
        MouthStretch = 0.12f;
    }
    else if (Viseme == AS_S1)
    {
        
        JawOpen      = 0.08f;
        MouthStretch = 0.55f;
    }
    else if (Viseme == AS_Sh1)
    {
        
        JawOpen      = 0.22f;
        MouthFunnel  = 0.35f;
        MouthPucker  = 0.25f;
    }
    else if (Viseme == AS_Ch1)
    {
       
        JawOpen      = 0.15f;
        MouthFunnel  = 0.28f;
        MouthPucker  = 0.20f;
    }
    else if (Viseme == AS_K1)
    {
       
        JawOpen      = 0.14f;
        MouthStretch = 0.05f;
    }
    else if (Viseme == AS_L1)
    {
        
        JawOpen      = 0.22f;
        MouthStretch = 0.30f;
    }
    else if (Viseme == AS_R1)
    {
      
        JawOpen      = 0.20f;
        MouthStretch = 0.15f;
    }

  
    TargetJawOpen      = JawOpen      * VisemeMorphIntensity;
    TargetMouthFunnel  = MouthFunnel  * VisemeMorphIntensity;
    TargetMouthPucker  = MouthPucker  * VisemeMorphIntensity;
    TargetMouthStretch = MouthStretch * VisemeMorphIntensity;
}

// ---------------------------------------------------------------------------
// BLINKING
// ---------------------------------------------------------------------------
void UFacialAnimationComponent::UpdateBlinking(float DeltaTime)
{
    if (!FaceMeshComponent) return;

    UAnimInstance* AnimInst = FaceMeshComponent->GetAnimInstance();
    if (!AnimInst) return;

    if (!bIsBlinking)
    {
        TimeSinceLastBlink += DeltaTime;
        if (TimeSinceLastBlink >= NextBlinkTime)
        {
            bIsBlinking        = true;
            BlinkProgress      = 0.0f;
            TimeSinceLastBlink = 0.0f;
            
            NextBlinkTime = bIsSpeaking
                ? FMath::RandRange(1.5f, 4.0f)
                : FMath::RandRange(2.5f, 6.0f);
        }
    }

    if (bIsBlinking)
    {
        BlinkProgress += DeltaTime / 0.15f;
        float V = 0.0f;

        if (BlinkProgress < 0.5f)
        {
            V = BlinkProgress * 2.0f;
        }
        else if (BlinkProgress < 1.0f)
        {
            V = 1.0f - (BlinkProgress - 0.5f) * 2.0f;
        }

        if (BlinkProgress >= 1.0f)
        {
            bIsBlinking = false;
            V = 0.0f;
        }

        SetMetaHumanCurve(FName("eyeBlinkLeft"),  V, AnimInst);
        SetMetaHumanCurve(FName("eyeBlinkRight"), V, AnimInst);
        SetMetaHumanCurve(FName("CTRL_expressions_eyeBlinkL"), V, AnimInst);
        SetMetaHumanCurve(FName("CTRL_expressions_eyeBlinkR"), V, AnimInst);
    }
}

// ---------------------------------------------------------------------------
// SET CURVE HELPER
// ---------------------------------------------------------------------------
void UFacialAnimationComponent::SetMetaHumanCurve(const FName& CurveName, float Value, UAnimInstance* AnimInst)
{
    if (AnimInst)
    {
        AnimInst->AddCurveValue(CurveName, Value, true, false);
    }

    if (BodyMeshComponent)
    {
        UAnimInstance* BodyAnimInst = BodyMeshComponent->GetAnimInstance();
        if (BodyAnimInst)
        {
            BodyAnimInst->AddCurveValue(CurveName, Value, true, false);
        }
    }
}

// ---------------------------------------------------------------------------
// EMOTION
// ---------------------------------------------------------------------------
void UFacialAnimationComponent::SetEmotion(const FString& Emotion, float Intensity)
{
    float I = FMath::Clamp(Intensity, 0.0f, 1.0f);

    if (Emotion.Equals(TEXT("happy"), ESearchCase::IgnoreCase))
    {
        TargetMouthCornerUp   = 0.6f * I;
        TargetMouthCornerDown = 0.0f;
        TargetBrowRaise       = 0.3f * I;
    }
    else if (Emotion.Equals(TEXT("sad"), ESearchCase::IgnoreCase))
    {
        TargetMouthCornerUp   = 0.0f;
        TargetMouthCornerDown = 0.5f * I;
        TargetBrowRaise       = 0.4f * I;
    }
    else if (Emotion.Equals(TEXT("surprised"), ESearchCase::IgnoreCase))
    {
        TargetMouthCornerUp = 0.1f * I;
        TargetBrowRaise     = 0.8f * I;
        
        TargetJawOpen = 0.25f * I;
    }
    else if (Emotion.Equals(TEXT("thinking"), ESearchCase::IgnoreCase))
    {
        
        TargetMouthCornerUp   = 0.0f;
        TargetMouthCornerDown = 0.1f * I;
        TargetBrowRaise       = 0.2f * I;
    }
    else
    {
        TargetMouthCornerUp   = 0.0f;
        TargetMouthCornerDown = 0.0f;
        TargetBrowRaise       = 0.0f;
    }
}

// ---------------------------------------------------------------------------
// EMERGENCY TEST
// ---------------------------------------------------------------------------
void UFacialAnimationComponent::EmergencyTest()
{
    UE_LOG(LogTemp, Error, TEXT("=== EMERGENCY TEST START ==="));

    if (!FaceMeshComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("FaceMeshComponent is NULL!"));
        return;
    }

    UAnimInstance* AnimInst = FaceMeshComponent->GetAnimInstance();
    if (!AnimInst)
    {
        UE_LOG(LogTemp, Error, TEXT("AnimInstance is NULL!"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("1. Face Mesh: %s"), *FaceMeshComponent->GetName());
    UE_LOG(LogTemp, Warning, TEXT("2. AnimInstance: %s"), *AnimInst->GetClass()->GetName());

    TargetJawOpen       = 0.8f;
    TargetMouthCornerUp = 0.6f;

    UE_LOG(LogTemp, Error, TEXT("=== EMERGENCY TEST END ==="));

    FTimerHandle ResetTimer;
    GetWorld()->GetTimerManager().SetTimer(ResetTimer, [this]()
    {
        TargetJawOpen       = 0.0f;
        TargetMouthCornerUp = 0.0f;
        UE_LOG(LogTemp, Warning, TEXT("4. Test targets cleared"));
    }, 2.0f, false);
}