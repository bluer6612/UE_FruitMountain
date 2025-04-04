#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
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
    virtual void Tick(float DeltaTime) override;
    
    // EndPlay 함수 선언 추가
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // 접시 클래스 지정 변수
    UPROPERTY(EditDefaultsOnly, Category = "Game")
    TSubclassOf<AActor> PlateClass;
    
    // 과일 볼 클래스 지정 변수
    UPROPERTY(EditDefaultsOnly, Category = "Game")
    TSubclassOf<AActor> FruitBallClass;
};