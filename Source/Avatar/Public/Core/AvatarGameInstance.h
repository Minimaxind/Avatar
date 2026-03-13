#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "AvatarGameInstance.generated.h"

// Forward declarations
class ULLMClient;
class UTTSClient;

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
    
	// Состояние диалога
	UPROPERTY(BlueprintAssignable, Category = "Chat")
	FOnChatMessageReceived OnChatMessageReceived;
    
	UPROPERTY(BlueprintAssignable, Category = "Chat")
	FOnAvatarResponse OnAvatarResponse;
    
	// Отправка сообщения
	UFUNCTION(BlueprintCallable, Category = "Chat")
	void SendUserMessage(const FString& Message);
    
	// История диалога
	UPROPERTY(BlueprintReadOnly, Category = "Chat")
	TArray<FString> ChatHistory;
    
	// Настройки
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FString LLMEndpoint = TEXT("http://localhost:11434/api/generate");
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FString AvatarName = TEXT("Ассистент");
    
private:
	UPROPERTY()
	ULLMClient* LLMClient;
    
	UPROPERTY()
	UTTSClient* TTSClient;
};