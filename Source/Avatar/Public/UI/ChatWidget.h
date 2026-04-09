#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/EditableTextBox.h"
#include "Components/ScrollBox.h"
#include "Components/Button.h"
#include "ChatWidget.generated.h"

class UAvatarGameInstance;

UCLASS()
class AVATAR_API UChatWidget : public UUserWidget
{
	GENERATED_BODY()
    
public:
	virtual void NativeConstruct() override;
    
	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* MessageInputBox;
    
	UPROPERTY(meta = (BindWidget))
	UScrollBox* ChatHistoryBox;
    
	UPROPERTY(meta = (BindWidget))
	UButton* SendButton;
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> MessageWidgetClass;
    
private:
	UFUNCTION()
	void OnSendMessage();
    
	UFUNCTION()
	void OnMessageCommitted(const FText& Text, ETextCommit::Type CommitMethod);
    
	UFUNCTION()
	void OnUserMessageReceived(const FString& Message);
    
	UFUNCTION()
	void OnAvatarResponseReceived(const FString& Response);
    
	void AddMessage(const FString& Sender, const FString& Message);
    
	UPROPERTY()
	UAvatarGameInstance* GameInstance;
};