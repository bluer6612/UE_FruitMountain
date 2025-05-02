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
    virtual void StartPlay() override;

protected:
    UPROPERTY()
    TSubclassOf<class UMainMenuWidget> MainMenuWidgetClass;

    UPROPERTY()
    class UMainMenuWidget* MainMenuWidget;

    // Plate 관련 멤버 추가
    UPROPERTY(EditDefaultsOnly, Category = "Game")
    TSubclassOf<AActor> PlateClass;
};