#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "UE_FruitMountainGameMode.generated.h"

UCLASS()
class UE_FRUITMOUNTAIN_API AUE_FruitMountainGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AUE_FruitMountainGameMode();

    virtual void StartPlay() override;
    virtual void Tick(float DeltaTime) override;

    // BeginPlay 함수 선언 추가
    virtual void BeginPlay() override;

    // 공(FruitBall) 액터를 스폰하기 위한 클래스
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gameplay")
    TSubclassOf<AActor> FruitBallClass;

protected:
    // 접시(Plate) 액터를 스폰하기 위한 클래스
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gameplay")
    TSubclassOf<AActor> PlateClass;
};