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
            break;
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
        UE_LOG(LogTemp, Error, TEXT("[FAC] Go to BP_MetaHuman_Avatarus -> Face component -> Anim Class -> set ABP_Face_PostProcess"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[FAC] AnimInstance OK: %s"), *AnimInst->GetClass()->GetName());
    }

    BuildCharMap();
    NextBlinkTime = FMath::RandRange(2.0f, 5.0f);
}

void UFacialAnimationComponent::BuildCharMap()
{
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
    CharToAnimMap.Add(TEXT('п'), AS_B1);
    CharToAnimMap.Add(TEXT('м'), AS_M1);
    CharToAnimMap.Add(TEXT('в'), AS_V1);
    CharToAnimMap.Add(TEXT('ф'), AS_V1);
    CharToAnimMap.Add(TEXT('т'), AS_T1);
    CharToAnimMap.Add(TEXT('д'), AS_T1);
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
        if (!Pair.Value) NullCount++;
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

void UFacialAnimationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsSpeaking)
    {
        CurrentSpeechTime += DeltaTime;
        if (CurrentSpeechTime >= TotalSpeechDuration)
        {
            StopSpeaking();
            return;
        }

        TimeInCurrentViseme += DeltaTime;
        if (TimeInCurrentViseme >= CurrentVisemeDuration)
        {
            TimeInCurrentViseme = 0.0f;
            CurrentVisemeIndex++;
            if (CurrentVisemeIndex >= VisemeQueue.Num())
            {
                CurrentVisemeIndex = FMath::Max(0, VisemeQueue.Num() - 1);
            }
            PlayVisemeAtIndex(CurrentVisemeIndex);
        }
    }

    UpdateBlinking(DeltaTime);
}

void UFacialAnimationComponent::StartSpeaking(const FString& Text, float Duration)
{
    if (Text.IsEmpty()) return;

    if (!FaceMeshComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("[FAC] StartSpeaking: FaceMeshComponent is NULL!"));
        return;
    }

    UAnimInstance* AnimInst = FaceMeshComponent->GetAnimInstance();
    if (!AnimInst)
    {
        UE_LOG(LogTemp, Error, TEXT("[FAC] StartSpeaking: AnimInstance is NULL!"));
        return;
    }

    StopSpeaking();

    FString Lower = Text.ToLower();
    static const FString Skip = TEXT(" ,.!?-;:\n\r\"'()[]");

    for (int32 i = 0; i < Lower.Len(); i++)
    {
        TCHAR Ch = Lower[i];
        if (Skip.Contains(FString(1, &Ch))) continue;

        UAnimSequence** Found = CharToAnimMap.Find(Ch);
        if (Found && *Found)
        {
            VisemeQueue.Add(*Found);
        }
        else if (AS_Idle)
        {
            VisemeQueue.Add(AS_Idle);
        }
    }

    if (VisemeQueue.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[FAC] No visemes for text: %s"), *Text);
        return;
    }

    TotalSpeechDuration = (Duration > 0.0f) ? Duration : Text.Len() * 0.08f;
    CurrentVisemeDuration = FMath::Clamp(TotalSpeechDuration / VisemeQueue.Num(), 0.05f, 0.3f);
    bIsSpeaking = true;
    CurrentSpeechTime = 0.0f;
    CurrentVisemeIndex = 0;
    TimeInCurrentViseme = 0.0f;

    PlayVisemeAtIndex(0);
}

void UFacialAnimationComponent::PlayVisemeAtIndex(int32 Index)
{
    if (!FaceMeshComponent) return;
    if (!VisemeQueue.IsValidIndex(Index)) return;

    UAnimSequence* Anim = VisemeQueue[Index];
    if (!Anim) return;

    UAnimInstance* AnimInst = FaceMeshComponent->GetAnimInstance();
    if (!AnimInst) return;

    AnimInst->PlaySlotAnimationAsDynamicMontage(
        Anim, VisemeSlotName,
        VisemeBlendIn, VisemeBlendOut,
        VisemePlayRate, 1, 0.0f
    );
}

void UFacialAnimationComponent::StopSpeaking()
{
    if (!bIsSpeaking) return;

    bIsSpeaking = false;
    CurrentSpeechTime = 0.0f;
    CurrentVisemeIndex = 0;
    TimeInCurrentViseme = 0.0f;
    VisemeQueue.Empty();

    if (FaceMeshComponent && AS_Idle)
    {
        UAnimInstance* AnimInst = FaceMeshComponent->GetAnimInstance();
        if (AnimInst)
        {
            AnimInst->PlaySlotAnimationAsDynamicMontage(
                AS_Idle, VisemeSlotName,
                0.1f, 0.1f, 1.0f, 1, 0.0f
            );
        }
    }
}

void UFacialAnimationComponent::UpdateBlinking(float DeltaTime)
{
    if (!FaceMeshComponent) return;

    if (!bIsBlinking)
    {
        TimeSinceLastBlink += DeltaTime;
        if (TimeSinceLastBlink >= NextBlinkTime)
        {
            bIsBlinking = true;
            BlinkProgress = 0.0f;
            TimeSinceLastBlink = 0.0f;
            NextBlinkTime = FMath::RandRange(2.0f, 5.0f);
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

        FaceMeshComponent->SetMorphTarget(FName("eyeBlink_L"), V, false);
        FaceMeshComponent->SetMorphTarget(FName("eyeBlink_R"), V, false);
    }
}

void UFacialAnimationComponent::SetEmotion(const FString& Emotion, float Intensity)
{
    if (!FaceMeshComponent) return;
    float I = FMath::Clamp(Intensity, 0.0f, 1.0f);

    if (Emotion.Equals(TEXT("happy"), ESearchCase::IgnoreCase))
    {
        FaceMeshComponent->SetMorphTarget(FName("mouthSmileLeft"), 0.6f * I, false);
        FaceMeshComponent->SetMorphTarget(FName("mouthSmileRight"), 0.6f * I, false);
        FaceMeshComponent->SetMorphTarget(FName("mouthFrownLeft"), 0.0f, false);
        FaceMeshComponent->SetMorphTarget(FName("mouthFrownRight"), 0.0f, false);
    }
    else if (Emotion.Equals(TEXT("sad"), ESearchCase::IgnoreCase))
    {
        FaceMeshComponent->SetMorphTarget(FName("mouthFrownLeft"), 0.5f * I, false);
        FaceMeshComponent->SetMorphTarget(FName("mouthFrownRight"), 0.5f * I, false);
        FaceMeshComponent->SetMorphTarget(FName("browInnerUp"), 0.4f * I, false);
        FaceMeshComponent->SetMorphTarget(FName("mouthSmileLeft"), 0.0f, false);
        FaceMeshComponent->SetMorphTarget(FName("mouthSmileRight"), 0.0f, false);
    }
    else
    {
        FaceMeshComponent->SetMorphTarget(FName("mouthSmileLeft"), 0.0f, false);
        FaceMeshComponent->SetMorphTarget(FName("mouthSmileRight"), 0.0f, false);
        FaceMeshComponent->SetMorphTarget(FName("mouthFrownLeft"), 0.0f, false);
        FaceMeshComponent->SetMorphTarget(FName("mouthFrownRight"), 0.0f, false);
        FaceMeshComponent->SetMorphTarget(FName("browInnerUp"), 0.0f, false);
    }
}