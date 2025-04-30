#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TitleGameMode.generated.h"

UCLASS()
class UE_FRUITMOUNTAIN_API ATitleGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ATitleGameMode();

    virtual void BeginPlay() override;

protected:
    UPROPERTY()
    TSubclassOf<class UMainMenuWidget> MainMenuWidgetClass;

    UPROPERTY()
    class UMainMenuWidget* MainMenuWidget;
};