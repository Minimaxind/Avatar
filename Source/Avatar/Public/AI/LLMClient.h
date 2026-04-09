#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "LLMClient.generated.h"

DECLARE_DELEGATE_OneParam(FLLMResponseDelegate, const FString&);

UCLASS(BlueprintType)
class AVATAR_API ULLMClient : public UObject
{
	GENERATED_BODY()
    
public:
	ULLMClient();
    
	void SendPrompt(const FString& Prompt, FLLMResponseDelegate ResponseCallback);
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LLM Settings")
	FString ModelName = TEXT("qwen2.5:1.5b");
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LLM Settings")
	float Temperature = 0.7f;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LLM Settings")
	int32 MaxTokens = 500;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LLM Settings")
	FString APIEndpoint = TEXT("http://127.0.0.1:11434/api/generate");
    
private:
	void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FLLMResponseDelegate Callback);
};