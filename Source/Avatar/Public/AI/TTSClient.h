#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TTSClient.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSpeechStart, float, Duration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSpeechEnd);

UCLASS(BlueprintType)
class AVATAR_API UTTSClient : public UObject
{
	GENERATED_BODY()
    
public:
	UTTSClient();
    
	UFUNCTION(BlueprintCallable, Category = "TTS")
	void SynthesizeAndPlay(const FString& Text);
    
	UPROPERTY(BlueprintAssignable, Category = "TTS")
	FOnSpeechStart OnSpeechStart;
    
	UPROPERTY(BlueprintAssignable, Category = "TTS")
	FOnSpeechEnd OnSpeechEnd;
    
	// Настройки голоса
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TTS Settings")
	int32 SpeechRate = 0;  // -10 до 10, 0 = нормально
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TTS Settings")
	int32 SpeechVolume = 100;  // 0-100
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TTS Settings")
	FString VoiceName = TEXT("Microsoft Irina Desktop");  // Русский женский
    
	// ВАЖНО: Добавляем объявление функции GetWorld
	virtual UWorld* GetWorld() const override;
    
private:
	FTimerHandle SpeechTimerHandle;
};