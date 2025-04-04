#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "FruitHUD.generated.h"

UCLASS()
class UE_FRUITMOUNTAIN_API AFruitHUD : public AHUD
{
    GENERATED_BODY()

public:
    AFruitHUD();
    virtual void DrawHUD() override;
};