#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "AvatarGameInstance.generated.h"

class ULLMClient;
class UTTSClient;
class AAvatarCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChatMessageReceived, const FString&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAvatarResponse, const FString&, Response);

UCLASS()
class AVATAR_API UAvatarGameInstance : public UGameInstance
{
	GENERATED_BODY()
    
public:
	UAvatarGameInstance();
    
	virtual void Init() override;
	virtual void Shutdown() override;
    
	UPROPERTY(BlueprintAssignable, Category = "Chat")
	FOnChatMessageReceived OnChatMessageReceived;
    
	UPROPERTY(BlueprintAssignable, Category = "Chat")
	FOnAvatarResponse OnAvatarResponse;
    
	UFUNCTION(BlueprintCallable, Category = "Chat")
	void SendUserMessage(const FString& Message);
    
	UFUNCTION()
	void OnTTSStarted(float Duration);
    
	UPROPERTY(BlueprintReadOnly, Category = "Chat")
	TArray<FString> ChatHistory;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FString AvatarName = TEXT("Ассистент");
    
	UPROPERTY()
	AAvatarCharacter* AvatarCharacter;
    
	FString CurrentResponse;
    
private:
	UPROPERTY()
	ULLMClient* LLMClient;
    
	UPROPERTY()
	UTTSClient* TTSClient;
};