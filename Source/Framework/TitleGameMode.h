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
    // 타이틀 위젯 인스턴스
    UPROPERTY()
    TSubclassOf<class UMainMenuWidget> TitleWidgetClass;

    UPROPERTY()
    class UMainMenuWidget* TitleWidget;
};