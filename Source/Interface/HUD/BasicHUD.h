// 새 파일: BasicHUD.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BasicHUD.generated.h"

UCLASS()
class UE_FRUITMOUNTAIN_API ABasicHUD : public AHUD
{
    GENERATED_BODY()
    
public:
    virtual void DrawHUD() override;
};