#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameMode.generated.h"

UCLASS()
class UE_FRUITMOUNTAIN_API AGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AGameMode();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Add your custom functions and variables here
};