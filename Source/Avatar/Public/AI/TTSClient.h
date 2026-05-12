// AI/TTSClient.h
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
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TTS Settings")
	int32 SpeechRate = 0;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TTS Settings")
	int32 SpeechVolume = 100;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TTS Settings")
	FString VoiceName = TEXT("Microsoft Irina Desktop");
    
	virtual UWorld* GetWorld() const override;
    
private:
	FTimerHandle SpeechTimerHandle;
};