#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FacialAnimationComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AVATAR_API UFacialAnimationComponent : public UActorComponent
{
	GENERATED_BODY()
    
public:
	UFacialAnimationComponent();
    
	// Инициализация
	virtual void BeginPlay() override;
    
	// Управление анимацией
	UFUNCTION(BlueprintCallable, Category = "Facial Animation")
	void StartTalking(float Duration);
    
	UFUNCTION(BlueprintCallable, Category = "Facial Animation")
	void StopTalking();
    
	UFUNCTION(BlueprintCallable, Category = "Facial Animation")
	void SetEmotion(const FString& Emotion);
    
	// Морф-таргеты для лицевой анимации
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Morph Targets")
	TMap<FString, float> MorphTargets;
    
	// Ссылка на skeletal mesh
	UPROPERTY()
	class USkeletalMeshComponent* SkeletalMesh;
    
private:
	// Обновление морф-таргетов для речи
	void UpdateLipSync();
    
	FTimerHandle LipSyncTimerHandle;
	float CurrentSpeechTime;
	float SpeechDuration;
    
	// Карта фонем к морф-таргетам
	TMap<FString, FString> PhonemeToMorphMap;
};