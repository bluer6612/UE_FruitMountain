#include "FruitInputMappingManager.h"
#include "GameFramework/InputSettings.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "InputCoreTypes.h"

void UFruitInputMappingManager::ConfigureKeyMappings()
{
    UInputSettings* InputSettings = const_cast<UInputSettings*>(GetDefault<UInputSettings>());
    if (!InputSettings)
    {
        UE_LOG(LogTemp, Warning, TEXT("InputSettings를 찾을 수 없습니다."));
        return;
    }

    bool bMappingsChanged = false;

    // "ThrowFruit" 매핑: SpaceBar
    {
        bool bFound = false;
        for (const FInputActionKeyMapping& Mapping : InputSettings->GetActionMappings())
        {
            if (Mapping.ActionName == "ThrowFruit" && Mapping.Key == EKeys::SpaceBar)
            {
                bFound = true;
                break;
            }
        }
        if (!bFound)
        {
            InputSettings->AddActionMapping(FInputActionKeyMapping("ThrowFruit", EKeys::SpaceBar));
            bMappingsChanged = true;
        }
    }
    
    // "AdjustAngle" Axis 매핑
    bool bAdjustAngleAxisMappingChanged = false;
    TArray<FInputAxisKeyMapping> AxisMappings = InputSettings->GetAxisMappings();
    bool bFoundAdjustAngleNegative = false;
    bool bFoundAdjustAnglePositive = false;
    for (const FInputAxisKeyMapping& Mapping : AxisMappings)
    {
        if (Mapping.AxisName == "AdjustAngle")
        {
            if (Mapping.Key == EKeys::S && Mapping.Scale < 0.f)
            {
                bFoundAdjustAngleNegative = true;
            }
            if (Mapping.Key == EKeys::W && Mapping.Scale > 0.f)
            {
                bFoundAdjustAnglePositive = true;
            }
        }
    }
    if (!bFoundAdjustAngleNegative)
    {
        InputSettings->AddAxisMapping(FInputAxisKeyMapping("AdjustAngle", EKeys::S, -1.f));
        bAdjustAngleAxisMappingChanged = true;
    }
    if (!bFoundAdjustAnglePositive)
    {
        InputSettings->AddAxisMapping(FInputAxisKeyMapping("AdjustAngle", EKeys::W, 1.f));
        bAdjustAngleAxisMappingChanged = true;
    }
    if (bAdjustAngleAxisMappingChanged)
    {
        bMappingsChanged = true;
    }

    // "RotateCamera" Axis 매핑
    bool bAxisMappingChanged = false;
    bool bFoundNegative = false;
    bool bFoundPositive = false;
    for (const FInputAxisKeyMapping& Mapping : AxisMappings)
    {
        if (Mapping.AxisName == "RotateCamera")
        {
            if (Mapping.Key == EKeys::A && Mapping.Scale < 0.f)
            {
                bFoundNegative = true;
            }
            if (Mapping.Key == EKeys::D && Mapping.Scale > 0.f)
            {
                bFoundPositive = true;
            }
        }
    }
    if (!bFoundNegative)
    {
        InputSettings->AddAxisMapping(FInputAxisKeyMapping("RotateCamera", EKeys::A, -1.f));
        bAxisMappingChanged = true;
    }
    if (!bFoundPositive)
    {
        InputSettings->AddAxisMapping(FInputAxisKeyMapping("RotateCamera", EKeys::D, 1.f));
        bAxisMappingChanged = true;
    }
    if (bAxisMappingChanged)
    {
        bMappingsChanged = true;
    }

    if (bMappingsChanged)
    {
        InputSettings->SaveKeyMappings();
        UE_LOG(LogTemp, Log, TEXT("프로젝트 Axis & 키 매핑이 업데이트 되었습니다."));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("프로젝트 키 매핑이 이미 설정되어 있습니다."));
    }
}