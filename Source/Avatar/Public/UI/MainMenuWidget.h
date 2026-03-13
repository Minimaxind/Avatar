#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

// Forward declarations
class UButton;
class UTextBlock;
class UImage;

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
    
	UPROPERTY(meta = (BindWidget))
	UImage* BackgroundImage;
    
	// Класс виджета чата
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> ChatWidgetClass;
    
private:
	UFUNCTION()
	void OnStartClicked();
    
	UFUNCTION()
	void OnExitClicked();
};