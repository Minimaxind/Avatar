#include "UI/MainMenuWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Kismet/GameplayStatics.h"

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
    
	if (StartButton)
	{
		StartButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnStartClicked);
	}
    
	if (ExitButton)
	{
		ExitButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnExitClicked);
	}
    
	if (TitleText)
	{
		TitleText->SetText(FText::FromString(TEXT("Виртуальный аватар")));
	}
}

void UMainMenuWidget::OnStartClicked()
{
	// Скрываем меню и показываем чат
	SetVisibility(ESlateVisibility::Hidden);
    
	if (ChatWidgetClass)
	{
		UUserWidget* ChatWidget = CreateWidget<UUserWidget>(this, ChatWidgetClass);
		if (ChatWidget)
		{
			ChatWidget->AddToViewport();
		}
	}
    
	// Запускаем анимацию аватара
	APlayerController* PlayerController = GetOwningPlayer();
	if (PlayerController)
	{
		PlayerController->SetShowMouseCursor(true);
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PlayerController->SetInputMode(InputMode);
	}
}

void UMainMenuWidget::OnExitClicked()
{
	// Выход из приложения
	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		UKismetSystemLibrary::QuitGame(this, PlayerController, EQuitPreference::Quit, false);
	}
}