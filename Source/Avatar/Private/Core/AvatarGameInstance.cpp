#include "Core/AvatarGameInstance.h"
#include "AI/LLMClient.h"
#include "AI/TTSClient.h"

UAvatarGameInstance::UAvatarGameInstance()
{
}

void UAvatarGameInstance::Init()
{
	Super::Init();
    
	LLMClient = NewObject<ULLMClient>(this);
	TTSClient = NewObject<UTTSClient>(this);
}

void UAvatarGameInstance::Shutdown()
{
	Super::Shutdown();
}

void UAvatarGameInstance::SendUserMessage(const FString& Message)
{
	UE_LOG(LogTemp, Warning, TEXT("=== SendUserMessage: %s ==="), *Message); // ДОБАВИТЬ
    
	if (Message.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("Message is empty!")); // ДОБАВИТЬ
		return;
	}
    
	// Добавляем в историю
	ChatHistory.Add(FString::Printf(TEXT("Пользователь: %s"), *Message));
	OnChatMessageReceived.Broadcast(Message);
    
	// Формируем контекст для LLM
	FString Context = TEXT("Ты виртуальный ассистент по имени ") + AvatarName + 
					 TEXT(". Отвечай дружелюбно и по делу.\n\n");
    
	for (const auto& Entry : ChatHistory)
	{
		Context += Entry + TEXT("\n");
	}
    
	UE_LOG(LogTemp, Warning, TEXT("Context prepared, length: %d"), Context.Len()); // ДОБАВИТЬ
    
	// Отправляем запрос к LLM
	if (LLMClient)
	{
		UE_LOG(LogTemp, Warning, TEXT("Calling LLMClient->SendPrompt...")); // ДОБАВИТЬ
		LLMClient->SendPrompt(Context, FLLMResponseDelegate::CreateLambda([this](const FString& Response)
		{
			UE_LOG(LogTemp, Warning, TEXT("=== LLM RESPONSE RECEIVED: %s ==="), *Response); // ДОБАВИТЬ
            
			// Добавляем ответ в историю
			FString FullResponse = AvatarName + TEXT(": ") + Response;
			ChatHistory.Add(FullResponse);
            
			// Отправляем на озвучку
			if (TTSClient)
			{
				UE_LOG(LogTemp, Warning, TEXT("Calling TTSClient->SynthesizeAndPlay...")); // ДОБАВИТЬ
				TTSClient->SynthesizeAndPlay(Response);
			}
            
			// Уведомляем UI
			UE_LOG(LogTemp, Warning, TEXT("Broadcasting OnAvatarResponse...")); // ДОБАВИТЬ
			OnAvatarResponse.Broadcast(Response);
		}));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("LLMClient is NULL!")); // ДОБАВИТЬ
	}
}