#include "Avatar/FacialAnimationComponent.h"
#include "Components/SkeletalMeshComponent.h"

UFacialAnimationComponent::UFacialAnimationComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Инициализация карты фонем (пример для русского языка)
    PhonemeToMorphMap = {
        {TEXT("а"), TEXT("jaw_open")},
        {TEXT("о"), TEXT("jaw_open")},
        {TEXT("у"), TEXT("lips_pucker")},
        {TEXT("ы"), TEXT("jaw_open")},
        {TEXT("э"), TEXT("jaw_open")},
        {TEXT("я"), TEXT("jaw_open")},
        {TEXT("ё"), TEXT("jaw_open")},
        {TEXT("ю"), TEXT("lips_pucker")},
        {TEXT("е"), TEXT("jaw_open")},
        {TEXT("и"), TEXT("mouth_smile")},
        {TEXT("б"), TEXT("lips_close")},
        {TEXT("п"), TEXT("lips_close")},
        {TEXT("м"), TEXT("lips_close")},
        {TEXT("в"), TEXT("jaw_open")},
        {TEXT("ф"), TEXT("jaw_open")}
    };
}

void UFacialAnimationComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Получаем ссылку на skeletal mesh
    AActor* Owner = GetOwner();
    if (Owner)
    {
        SkeletalMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
    }
}

void UFacialAnimationComponent::StartTalking(float Duration)
{
    SpeechDuration = Duration;
    CurrentSpeechTime = 0.0f;
    
    GetWorld()->GetTimerManager().SetTimer(LipSyncTimerHandle, this, 
        &UFacialAnimationComponent::UpdateLipSync, 0.03f, true); // ~30fps для анимации рта
    
    // Устанавливаем базовую анимацию речи
    if (SkeletalMesh)
    {
        // Активируем морф-таргет для речи
        SkeletalMesh->SetMorphTarget(TEXT("talking"), 1.0f);
    }
}

void UFacialAnimationComponent::StopTalking()
{
    GetWorld()->GetTimerManager().ClearTimer(LipSyncTimerHandle);
    
    // Сбрасываем морф-таргеты
    if (SkeletalMesh)
    {
        SkeletalMesh->SetMorphTarget(TEXT("talking"), 0.0f);
        SkeletalMesh->SetMorphTarget(TEXT("jaw_open"), 0.0f);
        SkeletalMesh->SetMorphTarget(TEXT("lips_pucker"), 0.0f);
        SkeletalMesh->SetMorphTarget(TEXT("mouth_smile"), 0.0f);
        SkeletalMesh->SetMorphTarget(TEXT("lips_close"), 0.0f);
    }
}

void UFacialAnimationComponent::SetEmotion(const FString& Emotion)
{
    if (!SkeletalMesh)
        return;
    
    // Сбрасываем эмоции
    SkeletalMesh->SetMorphTarget(TEXT("happy"), 0.0f);
    SkeletalMesh->SetMorphTarget(TEXT("sad"), 0.0f);
    SkeletalMesh->SetMorphTarget(TEXT("angry"), 0.0f);
    SkeletalMesh->SetMorphTarget(TEXT("surprised"), 0.0f);
    
    // Устанавливаем новую эмоцию
    if (Emotion == TEXT("happy"))
        SkeletalMesh->SetMorphTarget(TEXT("happy"), 1.0f);
    else if (Emotion == TEXT("sad"))
        SkeletalMesh->SetMorphTarget(TEXT("sad"), 1.0f);
    else if (Emotion == TEXT("angry"))
        SkeletalMesh->SetMorphTarget(TEXT("angry"), 1.0f);
    else if (Emotion == TEXT("surprised"))
        SkeletalMesh->SetMorphTarget(TEXT("surprised"), 1.0f);
}

void UFacialAnimationComponent::UpdateLipSync()
{
    if (!SkeletalMesh || SpeechDuration <= 0.0f)
        return;
    
    CurrentSpeechTime += 0.03f;
    
    if (CurrentSpeechTime >= SpeechDuration)
    {
        StopTalking();
        return;
    }
    
    // Простая симуляция движения губ
    // В реальном проекте здесь должен быть анализ аудио или фонем
    float JawValue = FMath::Sin(CurrentSpeechTime * 20.0f) * 0.5f + 0.5f;
    SkeletalMesh->SetMorphTarget(TEXT("jaw_open"), JawValue);
    
    // Добавляем вариации
    float PuckerValue = FMath::Sin(CurrentSpeechTime * 15.0f) * 0.3f;
    SkeletalMesh->SetMorphTarget(TEXT("lips_pucker"), FMath::Max(0.0f, PuckerValue));
}