#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AvatarGameMode.generated.h"

UCLASS()
class AVATAR_API AAvatarGameMode : public AGameModeBase
{
	GENERATED_BODY()
    
public:
	AAvatarGameMode();
    
	virtual void BeginPlay() override;
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<class UUserWidget> MainMenuWidgetClass;
    
	UPROPERTY()
	class UUserWidget* MainMenuWidget;
    
private:
	void SetupInputMode();
};