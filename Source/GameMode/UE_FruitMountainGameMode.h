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

protected:
    // Plate 액터 스폰을 위한 클래스 (에디터에서 지정)
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Plate")
    TSubclassOf<AActor> PlateClass;
};