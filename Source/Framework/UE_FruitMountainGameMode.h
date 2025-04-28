#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Interface/UI/PlayLevel/Score/ScoreDisplayWidget.h"
#include "Interface/UI/PlayLevel/Score/Total/TotalScoreWidget.h"
#include "Interface/UI/PlayLevel/Score/Combo/ComboCountWidget.h"
#include "Interface/UI/PlayLevel/Start/PlayStartSequenceManager.h"
#include "Gameplay/Score/ScoreManagerComponent.h"
#include "UE_FruitMountainGameMode.generated.h"

class APlateActor;
class AFruitBall;

UCLASS()
class UE_FRUITMOUNTAIN_API AUE_FruitMountainGameMode : public AGameModeBase
{
    GENERATED_BODY()
    
public:
    AUE_FruitMountainGameMode();

    // 기존 함수들
    virtual void BeginPlay() override;
    virtual void StartPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    
    // 접시 클래스 지정 변수
    UPROPERTY(EditDefaultsOnly, Category = "Game")
    TSubclassOf<AActor> PlateClass;
    
    // 과일 볼 클래스 지정 변수
    UPROPERTY(EditDefaultsOnly, Category = "Game")
    TSubclassOf<AActor> FruitBallClass;

    UPROPERTY(BlueprintReadOnly, Category = "Components")
    class UScoreManagerComponent* ScoreManager;

    // 클래스 선언부에 추가
    UFUNCTION()
    void OnGameStartSequenceFinished();

    // 게임 UI 위젯 초기화 함수
    void InitializeGameWidgets();
};