// Core/AvatarGameInstance.h
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
	UFUNCTION(BlueprintCallable, Category = "Avatar")
	void RefreshAvatarActor();

	// В AvatarGameInstance.cpp
	
	
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

	// Старый указатель — оставляем для совместимости с AvatarCharacter.cpp
	UPROPERTY()
	AAvatarCharacter* AvatarCharacter;

	// Новый универсальный указатель — работает с любым актором у которого есть FacialAnimationComponent
	UPROPERTY()
	AActor* AvatarActor;
    
	FString CurrentResponse;
    
private:
    bool bIsPlayingTTS = false;
    
	UPROPERTY()
	ULLMClient* LLMClient;
    
	UPROPERTY()
	UTTSClient* TTSClient;

	// Ищет актора с FacialAnimationComponent в мире
	AActor* FindAvatarActor();
};