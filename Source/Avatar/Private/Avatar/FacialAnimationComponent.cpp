// Avatar/FacialAnimationComponent.cpp
#include "Avatar/FacialAnimationComponent.h"
#include "Components/SkeletalMeshComponent.h"

UFacialAnimationComponent::UFacialAnimationComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UFacialAnimationComponent::BeginPlay()
{
    Super::BeginPlay();
    
    AActor* Owner = GetOwner();
    if (Owner)
    {
        SkeletalMeshComponent = Owner->FindComponentByClass<USkeletalMeshComponent>();
        if (SkeletalMeshComponent)
        {
            UE_LOG(LogTemp, Warning, TEXT("FacialAnimationComponent: SkeletalMesh found"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("FacialAnimationComponent: No SkeletalMesh found!"));
        }
    }
    
    // Карта символов
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
    CharToAnimMap.Add(TEXT('й'), AS_I1);
    
    NextBlinkTime = FMath::RandRange(2.0f, 5.0f);
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
            CurrentVisemeIndex++;
            
            if (CurrentVisemeIndex >= VisemeSequence.Num())
            {
                CurrentVisemeIndex = FMath::Max(0, VisemeSequence.Num() - 1);
            }
            
            PlayCurrentViseme();
            TimeInCurrentViseme = 0.0f;
        }
    }
    
    // Моргание (опционально)
    UpdateBlinking(DeltaTime);
}

void UFacialAnimationComponent::PlayCurrentViseme()
{
    if (!SkeletalMeshComponent) return;
    if (VisemeSequence.Num() == 0) return;
    if (CurrentVisemeIndex >= VisemeSequence.Num()) return;
    
    UAnimSequence* AnimToPlay = VisemeSequence[CurrentVisemeIndex];
    if (!AnimToPlay) return;
    
    SkeletalMeshComponent->SetAnimationMode(EAnimationMode::AnimationSingleNode);
    SkeletalMeshComponent->SetAnimation(AnimToPlay);
    SkeletalMeshComponent->Play(true);
}

void UFacialAnimationComponent::StartSpeaking(const FString& Text, float Duration)
{
    if (Text.IsEmpty()) return;
    if (!SkeletalMeshComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("StartSpeaking: No SkeletalMesh!"));
        return;
    }
    
    StopSpeaking();
    
    bIsSpeaking = true;
    CurrentSpeechTime = 0.0f;
    TotalSpeechDuration = (Duration > 0.0f) ? Duration : Text.Len() * 0.12f;
    TimeInCurrentViseme = 0.0f;
    
    VisemeSequence.Empty();
    FString LowerText = Text.ToLower();
    
    for (int32 i = 0; i < LowerText.Len(); i++)
    {
        TCHAR Char = LowerText[i];
        
        if (Char == ' ' || Char == ',' || Char == '.' || Char == '!' || Char == '?' || Char == '-' || Char == ';' || Char == ':')
        {
            continue;
        }
        
        UAnimSequence** FoundAnim = CharToAnimMap.Find(Char);
        if (FoundAnim && *FoundAnim)
        {
            VisemeSequence.Add(*FoundAnim);
        }
        else if (AS_Idle)
        {
            VisemeSequence.Add(AS_Idle);
        }
    }
    
    if (VisemeSequence.Num() == 0)
    {
        StopSpeaking();
        return;
    }
    
    CurrentVisemeDuration = TotalSpeechDuration / VisemeSequence.Num();
    CurrentVisemeIndex = 0;
    TimeInCurrentViseme = 0.0f;
    
    PlayCurrentViseme();
    
    UE_LOG(LogTemp, Log, TEXT("StartSpeaking: %d visemes, duration %.2f"), VisemeSequence.Num(), TotalSpeechDuration);
}

void UFacialAnimationComponent::StopSpeaking()
{
    if (!bIsSpeaking) return;
    
    bIsSpeaking = false;
    VisemeSequence.Empty();
    CurrentVisemeIndex = 0;
    CurrentSpeechTime = 0.0f;
    
    if (SkeletalMeshComponent && AS_Idle)
    {
        SkeletalMeshComponent->SetAnimationMode(EAnimationMode::AnimationSingleNode);
        SkeletalMeshComponent->SetAnimation(AS_Idle);
        SkeletalMeshComponent->Play(true);
    }
}

void UFacialAnimationComponent::UpdateBlinking(float DeltaTime)
{
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
        
        float BlinkValue = 1.0f;
        if (BlinkProgress < 0.5f)
            BlinkValue = BlinkProgress * 2.0f;
        else if (BlinkProgress < 1.0f)
            BlinkValue = 1.0f - (BlinkProgress - 0.5f) * 2.0f;
        else
            BlinkValue = 0.0f;
        
        if (BlinkProgress >= 1.0f)
        {
            bIsBlinking = false;
        }
        
        SetBlendshapeValue("EyeBlinkLeft", BlinkValue);
        SetBlendshapeValue("EyeBlinkRight", BlinkValue);
    }
}

void UFacialAnimationComponent::SetBlendshapeValue(const FString& BlendshapeName, float Value)
{
    if (SkeletalMeshComponent)
    {
        FName MorphName = FName(*BlendshapeName);
        SkeletalMeshComponent->SetMorphTarget(MorphName, Value);
    }
}

void UFacialAnimationComponent::ResetBlendshapes()
{
    if (SkeletalMeshComponent)
    {
        SkeletalMeshComponent->ClearMorphTargets();
    }
}

void UFacialAnimationComponent::SetEmotion(const FString& Emotion, float Intensity)
{
    CurrentEmotion = Emotion;
    EmotionIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
}

const TArray<FString> UFacialAnimationComponent::GetARKitBlendshapeNames()
{
    return { "EyeBlinkLeft", "EyeBlinkRight", "JawOpen", "MouthSmileLeft", "MouthSmileRight" };
}