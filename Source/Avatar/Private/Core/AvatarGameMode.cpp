#include "Core/AvatarGameMode.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

AAvatarGameMode::AAvatarGameMode()
{
	// Устанавливаем класс персонажа по умолчанию
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/Blueprints/BP_AvatarCharacter"));
	if (PlayerPawnClassFinder.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnClassFinder.Class;
	}
}

void AAvatarGameMode::BeginPlay()
{
	Super::BeginPlay();
    
	SetupInputMode();
    
	// Создаем и показываем главное меню
	if (MainMenuWidgetClass)
	{
		MainMenuWidget = CreateWidget<UUserWidget>(GetWorld(), MainMenuWidgetClass);
		if (MainMenuWidget)
		{
			MainMenuWidget->AddToViewport();
		}
	}
}

void AAvatarGameMode::SetupInputMode()
{
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (PlayerController)
	{
		PlayerController->SetShowMouseCursor(true);
		FInputModeUIOnly InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PlayerController->SetInputMode(InputMode);
	}
}