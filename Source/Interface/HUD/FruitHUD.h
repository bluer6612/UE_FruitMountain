#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Interface/UI/TextureDisplayWidget.h"
#include "FruitHUD.generated.h"

UCLASS()
class UE_FRUITMOUNTAIN_API AFruitHUD : public AHUD
{
    GENERATED_BODY()
    
public:
    AFruitHUD();
    
    virtual void BeginPlay() override;
    
protected:
    // UMG 위젯 참조
    UPROPERTY()
    UTextureDisplayWidget* TextureWidget;
    
    // 위젯 생성 함수
    void CreateAndAddWidgets();
};