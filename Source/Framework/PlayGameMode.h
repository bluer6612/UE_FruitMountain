#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Interface/UI/PlayLevel/Score/ScoreDisplayWidget.h"
#include "Interface/UI/PlayLevel/Score/Total/TotalScoreWidget.h"
#include "Interface/UI/PlayLevel/Score/Combo/ComboCountWidget.h"
#include "Interface/UI/PlayLevel/Start/PlayStartSequenceManager.h"
#include "Gameplay/Score/ScoreManagerComponent.h"
#include "PlayGameMode.generated.h"

class APlateActor;
class AFruitBall;

UCLASS()
class UE_FRUITMOUNTAIN_API APlayGameMode : public AGameModeBase
{
    GENERATED_BODY()
    
public:
    APlayGameMode();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UPROPERTY(BlueprintReadOnly, Category = "Game")
    class UScoreManagerComponent* ScoreManager;

    UFUNCTION()
    void OnGameStartSequenceFinished();

    void InitializeGameWidgets();

    UPROPERTY(EditDefaultsOnly, Category = "Game")
    TSubclassOf<AActor> FruitBallClass;
};