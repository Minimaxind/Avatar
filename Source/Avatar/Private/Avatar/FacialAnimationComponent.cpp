// Avatar/FacialAnimationComponent.cpp
#include "Avatar/FacialAnimationComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "TimerManager.h"

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
        SkeletalMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
        if (SkeletalMesh)
        {
            UE_LOG(LogTemp, Warning, TEXT("FacialAnimationComponent initialized"));
        }
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
        }
        else
        {
            UpdateCurrentViseme(DeltaTime);
        }
    }
}

FString UFacialAnimationComponent::CharToViseme(TCHAR Char)
{
    switch (Char)
    {
        case 'а': case 'я': case 'э': case 'е': return TEXT("A");
        case 'о': case 'ё': return TEXT("O");
        case 'у': case 'ю': return TEXT("U");
        case 'и': case 'ы': return TEXT("I");
        case 'б': case 'п': return TEXT("B");
        case 'м': return TEXT("M");
        case 'в': case 'ф': return TEXT("F");
        case 'т': case 'д': return TEXT("T");
        case 'с': case 'з': return TEXT("S");
        case 'ш': case 'щ': case 'ж': return TEXT("SH");
        case 'ч': return TEXT("CH");
        case 'к': case 'г': case 'х': return TEXT("K");
        case 'л': return TEXT("L");
        case 'р': return TEXT("R");
        default: return TEXT("IDLE");
    }
}

void UFacialAnimationComponent::SetEmotion(const FString& Emotion, float Intensity)
{
    UE_LOG(LogTemp, Verbose, TEXT("SetEmotion: %s, Intensity: %.2f"), *Emotion, Intensity);
}

void UFacialAnimationComponent::StartSpeaking(const FString& Text, float Duration)
{
    if (Text.IsEmpty()) return;
    
    StopSpeaking();
    
    bIsSpeaking = true;
    CurrentSpeechTime = 0.0f;
    TotalSpeechDuration = Duration > 0.0f ? Duration : FMath::Clamp(Text.Len() * 0.12f, 1.0f, 15.0f);
    CurrentVisemeIndex = 0;
    VisemeTimer = 0.0f;
    
    GenerateVisemeTimings(Text, TotalSpeechDuration);
    
    if (VisemeTimings.Num() > 0)
    {
        // Показываем первую визему с плавным появлением
        PlayVisemeWithBlend(VisemeTimings[0].VisemeName, 0.1f);
    }
    
    UE_LOG(LogTemp, Log, TEXT("StartSpeaking: %d visemes over %.2f seconds"), VisemeTimings.Num(), TotalSpeechDuration);
}

void UFacialAnimationComponent::StopSpeaking()
{
    if (!bIsSpeaking) return;
    
    bIsSpeaking = false;
    VisemeTimings.Empty();
    CurrentVisemeIndex = 0;
    
    // Возвращаемся в нейтральное положение
    UAnimSequence* IdleAnim = VisemeAnimations.FindRef(TEXT("IDLE"));
    if (IdleAnim && SkeletalMesh)
    {
        SkeletalMesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);
        SkeletalMesh->SetAnimation(IdleAnim);
        SkeletalMesh->Play(true);
    }
    
    UE_LOG(LogTemp, Log, TEXT("StopSpeaking"));
}

void UFacialAnimationComponent::GenerateVisemeTimings(const FString& Text, float Duration)
{
    VisemeTimings.Empty();
    
    FString LowerText = Text.ToLower();
    
    // Сначала разбиваем текст на фонемы (группируем похожие звуки)
    TArray<FString> Phonemes;
    FString CurrentPhoneme;
    FString LastViseme = TEXT("");
    
    for (TCHAR Char : LowerText)
    {
        if (Char == ' ') continue;
        
        FString Viseme = CharToViseme(Char);
        
        // Группируем одинаковые виземы подряд
        if (Viseme != LastViseme && !LastViseme.IsEmpty())
        {
            Phonemes.Add(LastViseme);
        }
        LastViseme = Viseme;
    }
    if (!LastViseme.IsEmpty())
    {
        Phonemes.Add(LastViseme);
    }
    
    if (Phonemes.Num() == 0) return;
    
    // Минимальное время показа одной виземы (чтобы рот успел открыться)
    const float MinVisemeDuration = 0.12f;  // 120 мс минимум
    const float MaxVisemeDuration = 0.35f;  // 350 мс максимум
    
    // Распределяем время между виземами
    float TimePerViseme = FMath::Clamp(Duration / Phonemes.Num(), MinVisemeDuration, MaxVisemeDuration);
    float CurrentTime = 0.0f;
    
    for (const FString& VisemeName : Phonemes)
    {
        FVisemeTiming Timing;
        Timing.VisemeName = VisemeName;
        Timing.StartTime = CurrentTime;
        Timing.EndTime = CurrentTime + TimePerViseme;
        Timing.Intensity = 0.9f;
        
        VisemeTimings.Add(Timing);
        CurrentTime += TimePerViseme;
    }
    
    // Корректируем последнюю визему, чтобы она не обрывалась резко
    if (VisemeTimings.Num() > 0)
    {
        FVisemeTiming& LastTiming = VisemeTimings.Last();
        LastTiming.EndTime = Duration;
        LastTiming.Intensity = 0.7f;  // Затухание в конце
    }
    
    UE_LOG(LogTemp, Log, TEXT("Generated %d viseme groups (was %d chars)"), Phonemes.Num(), LowerText.Len());
}

void UFacialAnimationComponent::UpdateCurrentViseme(float DeltaTime)
{
    if (VisemeTimings.Num() == 0) return;
    
    VisemeTimer += DeltaTime;
    
    // Находим текущую визему по времени
    int32 NewVisemeIndex = CurrentVisemeIndex;
    for (int32 i = 0; i < VisemeTimings.Num(); i++)
    {
        if (VisemeTimer >= VisemeTimings[i].StartTime && VisemeTimer <= VisemeTimings[i].EndTime)
        {
            NewVisemeIndex = i;
            break;
        }
    }
    
    // Если визема изменилась
    if (NewVisemeIndex != CurrentVisemeIndex)
    {
        CurrentVisemeIndex = NewVisemeIndex;
        
        if (CurrentVisemeIndex < VisemeTimings.Num())
        {
            const FVisemeTiming& Viseme = VisemeTimings[CurrentVisemeIndex];
            
            // Рассчитываем интенсивность на основе прогресса внутри виземы
            float ProgressInViseme = (VisemeTimer - Viseme.StartTime) / (Viseme.EndTime - Viseme.StartTime);
            float IntensityMultiplier = 1.0f;
            
            // Плавное нарастание в начале и затухание в конце
            if (ProgressInViseme < 0.2f)
            {
                // Нарастание (первые 20% времени)
                IntensityMultiplier = ProgressInViseme / 0.2f;
            }
            else if (ProgressInViseme > 0.8f)
            {
                // Затухание (последние 20% времени)
                IntensityMultiplier = 1.0f - ((ProgressInViseme - 0.8f) / 0.2f);
            }
            else
            {
                // Полная интенсивность (середина)
                IntensityMultiplier = 1.0f;
            }
            
            float FinalIntensity = Viseme.Intensity * IntensityMultiplier;
            
            // Показываем визему с нужной интенсивностью
            PlayVisemeWithIntensity(Viseme.VisemeName, FinalIntensity);
            
            UE_LOG(LogTemp, Verbose, TEXT("Viseme %d: %s at %.2f intensity (progress %.2f)"), 
                CurrentVisemeIndex, *Viseme.VisemeName, FinalIntensity, ProgressInViseme);
        }
    }
}

void UFacialAnimationComponent::PlayVisemeWithBlend(const FString& VisemeName, float BlendTime)
{
    UAnimSequence* Anim = VisemeAnimations.FindRef(VisemeName);
    if (!Anim || !SkeletalMesh) return;
    
    // Используем AnimationSingleNode - он автоматически делает плавный переход
    SkeletalMesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);
    SkeletalMesh->SetAnimation(Anim);
    SkeletalMesh->Play(true);
}

void UFacialAnimationComponent::PlayVisemeWithIntensity(const FString& VisemeName, float Intensity)
{
    UAnimSequence* Anim = VisemeAnimations.FindRef(VisemeName);
    if (!Anim || !SkeletalMesh) return;
    
    // Для интенсивности можно масштабировать время или вес
    // Просто проигрываем анимацию, UE5 автоматически сделает переход
    
    SkeletalMesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);
    SkeletalMesh->SetAnimation(Anim);
    SkeletalMesh->Play(true);
    
    // Опционально: можно настроить скорость воспроизведения для имитации интенсивности
    // SkeletalMesh->SetPlayRate(0.8f + Intensity * 0.4f);
}