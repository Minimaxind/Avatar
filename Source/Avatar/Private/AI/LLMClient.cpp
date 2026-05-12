// AI/LLMClient.cpp
#include "AI/LLMClient.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"

ULLMClient::ULLMClient()
{
}

void ULLMClient::SendPrompt(const FString& Prompt, FLLMResponseDelegate ResponseCallback)
{
    FHttpModule* HttpModule = &FHttpModule::Get();
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = HttpModule->CreateRequest();
    
    Request->SetTimeout(30.0f);
    Request->SetHeader(TEXT("Connection"), TEXT("close"));
    
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    JsonObject->SetStringField(TEXT("model"), ModelName);
    JsonObject->SetStringField(TEXT("prompt"), Prompt);
    JsonObject->SetBoolField(TEXT("stream"), false);
    
    TSharedPtr<FJsonObject> OptionsObject = MakeShareable(new FJsonObject);
    OptionsObject->SetNumberField(TEXT("temperature"), Temperature);
    OptionsObject->SetNumberField(TEXT("num_predict"), MaxTokens);
    JsonObject->SetObjectField(TEXT("options"), OptionsObject);
    
    FString JsonString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
    
    Request->SetURL(TEXT("http://localhost:11434/api/generate"));
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetContentAsString(JsonString);
    
    // Используем лямбду вместо BindUObject для совместимости
    Request->OnProcessRequestComplete().BindLambda([this, ResponseCallback](FHttpRequestPtr Req, FHttpResponsePtr Res, bool bSuccess)
    {
        OnResponseReceived(Req, Res, bSuccess, ResponseCallback);
    });
    
    if (!Request->ProcessRequest())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to process HTTP request!"));
        ResponseCallback.ExecuteIfBound(TEXT("Ошибка отправки запроса"));
    }
}

void ULLMClient::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful, FLLMResponseDelegate Callback)
{
    FString ResponseString = TEXT("");
    
    if (bWasSuccessful && Response.IsValid())
    {
        int32 ResponseCode = Response->GetResponseCode();
        
        if (ResponseCode == 200)
        {
            FString Content = Response->GetContentAsString();
            TSharedPtr<FJsonObject> JsonObject;
            TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Content);
            
            if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
            {
                JsonObject->TryGetStringField(TEXT("response"), ResponseString);
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("LLM API Error: %d"), ResponseCode);
        }
    }
    
    if (ResponseString.IsEmpty())
    {
        ResponseString = TEXT("Извините, произошла ошибка при обращении к языковой модели.");
    }
    
    Callback.ExecuteIfBound(ResponseString);
}