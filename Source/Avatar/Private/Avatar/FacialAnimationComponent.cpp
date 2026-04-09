#include "Avatar/FacialAnimationComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Avatar/AvatarAnimInstance.h"
#include "Animation/AnimSequence.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Animation/AnimInstance.h"

UFacialAnimationComponent::UFacialAnimationComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UFacialAnimationComponent::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("=== FacialAnimation::BeginPlay START ==="));
    
    AActor* Owner = GetOwner();
    if (Owner)
    {
        SkeletalMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
        if (SkeletalMesh)
        {
            AnimInstance = Cast<UAvatarAnimInstance>(SkeletalMesh->GetAnimInstance());
            UE_LOG(LogTemp, Warning, TEXT("FacialAnimation: SkeletalMesh found - %s"), *SkeletalMesh->GetName());
            
            // Проверяем скелет
            if (SkeletalMesh->GetSkeletalMeshAsset())
            {
                USkeleton* Skeleton = SkeletalMesh->GetSkeletalMeshAsset()->GetSkeleton();
                UE_LOG(LogTemp, Warning, TEXT("FacialAnimation: Skeleton - %s"), *Skeleton->GetName());
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("FacialAnimation: SkeletalMesh NOT found!"));
            return;
        }
    }
    
    // Заполняем карту буква -> анимация
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
    CharToAnimMap.Add(TEXT('в'), AS_V1);
    CharToAnimMap.Add(TEXT('ф'), AS_V1);
    CharToAnimMap.Add(TEXT('м'), AS_M1);
    CharToAnimMap.Add(TEXT('т'), AS_T1);
    CharToAnimMap.Add(TEXT('д'), AS_T1);
    CharToAnimMap.Add(TEXT('н'), AS_T1);
    CharToAnimMap.Add(TEXT('с'), AS_S1);
    CharToAnimMap.Add(TEXT('з'), AS_S1);
    CharToAnimMap.Add(TEXT('ш'), AS_Sh1);
    CharToAnimMap.Add(TEXT('ж'), AS_Sh1);
    CharToAnimMap.Add(TEXT('щ'), AS_Sh1);
    CharToAnimMap.Add(TEXT('ч'), AS_Ch1);
    CharToAnimMap.Add(TEXT('к'), AS_K1);
    CharToAnimMap.Add(TEXT('г'), AS_K1);
    CharToAnimMap.Add(TEXT('х'), AS_K1);
    CharToAnimMap.Add(TEXT('л'), AS_L1);
    CharToAnimMap.Add(TEXT('р'), AS_R1);
    
    UE_LOG(LogTemp, Warning, TEXT("FacialAnimation: CharMap size = %d"), CharToAnimMap.Num());
    
    // Проверяем назначенные анимации
    UE_LOG(LogTemp, Warning, TEXT("FacialAnimation: Checking assigned animations:"));
    UE_LOG(LogTemp, Warning, TEXT("  AS_Idle = %s"), AS_Idle ? *AS_Idle->GetName() : TEXT("NULL"));
    UE_LOG(LogTemp, Warning, TEXT("  AS_A1 = %s"), AS_A1 ? *AS_A1->GetName() : TEXT("NULL"));
    UE_LOG(LogTemp, Warning, TEXT("  AS_O1 = %s"), AS_O1 ? *AS_O1->GetName() : TEXT("NULL"));
    
    // ТЕСТ: Пробуем проиграть анимацию через 2 секунды
    FTimerHandle TestTimer;
    GetWorld()->GetTimerManager().SetTimer(TestTimer, this, &UFacialAnimationComponent::TestAnimation, 2.0f, false);
    
    UE_LOG(LogTemp, Warning, TEXT("=== FacialAnimation::BeginPlay END ==="));
}

void UFacialAnimationComponent::TestAnimation()
{
    UE_LOG(LogTemp, Warning, TEXT("=== TEST ANIMATION START ==="));
    
    if (!SkeletalMesh)
    {
        UE_LOG(LogTemp, Error, TEXT("TEST: SkeletalMesh is NULL!"));
        return;
    }
    
    if (!AS_Idle)
    {
        UE_LOG(LogTemp, Error, TEXT("TEST: AS_Idle is NULL! Cannot test!"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("TEST: Playing %s on %s"), *AS_Idle->GetName(), *SkeletalMesh->GetName());
    
    // Способ 1: SetAnimationMode + Play
    SkeletalMesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);
    SkeletalMesh->SetAnimation(AS_Idle);
    SkeletalMesh->Play(true);
    UE_LOG(LogTemp, Warning, TEXT("TEST: AnimationSingleNode mode - SetAnimation + Play"));
    
    UE_LOG(LogTemp, Warning, TEXT("=== TEST ANIMATION END ==="));
}

bool UFacialAnimationComponent::IsAnimationCompatible(UAnimSequence* Animation)
{
    if (!SkeletalMesh || !Animation) return false;
    
    USkeletalMesh* SkMesh = SkeletalMesh->GetSkeletalMeshAsset();
    if (!SkMesh) return false;
    
    USkeleton* MeshSkeleton = SkMesh->GetSkeleton();
    USkeleton* AnimSkeleton = Animation->GetSkeleton();
    
    if (!MeshSkeleton || !AnimSkeleton) return false;
    
    bool bCompatible = (MeshSkeleton == AnimSkeleton);
    
    UE_LOG(LogTemp, Warning, TEXT("Compatibility: MeshSkeleton=%s, AnimSkeleton=%s, Same=%d"),
        *MeshSkeleton->GetName(), *AnimSkeleton->GetName(), bCompatible);
    
    return bCompatible;
}

void UFacialAnimationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsTalking) return;
    
    CurrentSpeechTime += DeltaTime;
    
    if (CurrentSpeechTime >= SpeechDuration)
    {
        StopTalking();
        return;
    }
    
    TimeInCurrentViseme += DeltaTime;
    
    if (TimeInCurrentViseme >= CurrentVisemeDuration)
    {
        CurrentVisemeIndex++;
        
        if (CurrentVisemeIndex >= VisemeSequence.Num())
        {
            CurrentVisemeIndex = 0;
        }
        
        PlayCurrentViseme();
        TimeInCurrentViseme = 0.0f;
    }
}

void UFacialAnimationComponent::StartTalking(const FString& Text, float Duration)
{
    UE_LOG(LogTemp, Warning, TEXT("=== FacialAnimation::StartTalking: '%s' ==="), *Text);
    
    if (Text.IsEmpty()) return;
    if (!SkeletalMesh)
    {
        UE_LOG(LogTemp, Error, TEXT("FacialAnimation: SkeletalMesh is NULL!"));
        return;
    }
    
    bIsTalking = true;
    SpeechDuration = (Duration > 0.0f) ? Duration : Text.Len() * 0.15f;
    CurrentSpeechTime = 0.0f;
    
    // Создаем последовательность анимаций
    VisemeSequence.Empty();
    FString LowerText = Text.ToLower();
    
    for (int32 i = 0; i < LowerText.Len(); i++)
    {
        TCHAR Char = LowerText[i];
        
        if (Char == ' ') continue;
        
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
    
    UE_LOG(LogTemp, Warning, TEXT("FacialAnimation: Created %d visemes"), VisemeSequence.Num());
    
    if (VisemeSequence.Num() > 0)
    {
        CurrentVisemeIndex = 0;
        CurrentVisemeDuration = SpeechDuration / VisemeSequence.Num();
        PlayCurrentViseme();
        TimeInCurrentViseme = 0.0f;
    }
    
    if (AnimInstance)
    {
        AnimInstance->SetTalking(true);
    }
    
    SetComponentTickEnabled(true);
}

void UFacialAnimationComponent::StopTalking()
{
    UE_LOG(LogTemp, Warning, TEXT("FacialAnimation::StopTalking"));
    
    bIsTalking = false;
    
    if (SkeletalMesh && AS_Idle)
    {
        SkeletalMesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);
        SkeletalMesh->SetAnimation(AS_Idle);
        SkeletalMesh->Play(true);
    }
    
    if (AnimInstance)
    {
        AnimInstance->SetTalking(false);
    }
    
    SetComponentTickEnabled(false);
}

void UFacialAnimationComponent::SetEmotion(const FString& Emotion, float Intensity)
{
    if (AnimInstance)
    {
        AnimInstance->SetEmotion(Emotion, Intensity);
    }
}

void UFacialAnimationComponent::PlayCurrentViseme()
{
    if (!SkeletalMesh) return;
    if (VisemeSequence.Num() == 0) return;
    if (CurrentVisemeIndex >= VisemeSequence.Num()) return;
    
    UAnimSequence* AnimToPlay = VisemeSequence[CurrentVisemeIndex];
    if (!AnimToPlay) return;
    
    UE_LOG(LogTemp, Warning, TEXT("FacialAnimation: Playing %s"), *AnimToPlay->GetName());
    
    // Используем AnimationSingleNode - самый надёжный способ
    SkeletalMesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);
    SkeletalMesh->SetAnimation(AnimToPlay);
    SkeletalMesh->Play(true);
}

UAnimSequence* UFacialAnimationComponent::GetAnimationForChar(TCHAR Char)
{
    UAnimSequence** Found = CharToAnimMap.Find(Char);
    return Found ? *Found : AS_Idle;
}

bool UFacialAnimationComponent::IsVowel(TCHAR Char)
{
    return Char == 'а' || Char == 'я' || Char == 'о' || Char == 'ё' || 
           Char == 'у' || Char == 'ю' || Char == 'э' || Char == 'е' || 
           Char == 'и' || Char == 'ы';
}