#include "AI/LLMClient.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"

ULLMClient::ULLMClient()
{
}

void ULLMClient::SendPrompt(const FString& Prompt, FLLMResponseDelegate ResponseCallback)
{
    UE_LOG(LogTemp, Warning, TEXT("=== LLMClient::SendPrompt ==="));
    
    FHttpModule* HttpModule = &FHttpModule::Get();
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = HttpModule->CreateRequest();
    
    // Важно: устанавливаем таймаут побольше для первого запроса
    Request->SetTimeout(10.0f);
    
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    JsonObject->SetStringField(TEXT("model"), TEXT("qwen2.5:1.5b"));
    
    // Простой промпт для теста
    JsonObject->SetStringField(TEXT("prompt"), Prompt);
    JsonObject->SetBoolField(TEXT("stream"), false);
    
    FString JsonString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
    
    UE_LOG(LogTemp, Warning, TEXT("JSON Request: %s"), *JsonString);
    
    // Используем 127.0.0.1 вместо localhost
    Request->SetURL(TEXT("http://127.0.0.1:11434/api/generate"));
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetHeader(TEXT("Accept"), TEXT("application/json"));  
    Request->SetContentAsString(JsonString);
    
    Request->OnProcessRequestComplete().BindUObject(this, &ULLMClient::OnResponseReceived, ResponseCallback);
    
    if (!Request->ProcessRequest())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to process HTTP request!"));
    }
}

void ULLMClient::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FLLMResponseDelegate Callback)
{
    UE_LOG(LogTemp, Warning, TEXT("=== LLMClient::OnResponseReceived ==="));
    UE_LOG(LogTemp, Warning, TEXT("bWasSuccessful: %d"), bWasSuccessful);
    
    FString ResponseString = TEXT("");
    
    if (bWasSuccessful && Response.IsValid())
    {
        int32 ResponseCode = Response->GetResponseCode();
        UE_LOG(LogTemp, Warning, TEXT("Response code: %d"), ResponseCode);
        
        FString Content = Response->GetContentAsString();
        UE_LOG(LogTemp, Warning, TEXT("Response body: %s"), *Content);
        
        if (ResponseCode == 200)
        {
            TSharedPtr<FJsonObject> JsonObject;
            TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Content);
            
            if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
            {
                JsonObject->TryGetStringField(TEXT("response"), ResponseString);
                UE_LOG(LogTemp, Warning, TEXT("Parsed response: %s"), *ResponseString);
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to parse JSON response!"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("HTTP error %d"), ResponseCode);
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("HTTP request failed!"));
        if (Response.IsValid())
        {
            UE_LOG(LogTemp, Error, TEXT("Response code: %d"), Response->GetResponseCode());
        }
    }
    
    if (ResponseString.IsEmpty())
    {
        ResponseString = TEXT("Извините, произошла ошибка при обращении к языковой модели.");
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Calling callback with response: %s"), *ResponseString);
    Callback.ExecuteIfBound(ResponseString);
}