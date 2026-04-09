#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

class UButton;
class UTextBlock;

UCLASS()
class AVATAR_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()
    
public:
	virtual void NativeConstruct() override;
    
	UPROPERTY(meta = (BindWidget))
	UButton* StartButton;
    
	UPROPERTY(meta = (BindWidget))
	UButton* ExitButton;
    
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TitleText;
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> ChatWidgetClass;
    
private:
	UFUNCTION()
	void OnStartClicked();
    
	UFUNCTION()
	void OnExitClicked();
};