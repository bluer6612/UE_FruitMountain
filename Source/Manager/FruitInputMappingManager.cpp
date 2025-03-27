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

    // "IncreaseAngle" 매핑: W 키
    {
        bool bFound = false;
        for (const FInputActionKeyMapping& Mapping : InputSettings->GetActionMappings())
        {
            if (Mapping.ActionName == "IncreaseAngle" && Mapping.Key == EKeys::W)
            {
                bFound = true;
                break;
            }
        }
        if (!bFound)
        {
            InputSettings->AddActionMapping(FInputActionKeyMapping("IncreaseAngle", EKeys::W));
            bMappingsChanged = true;
        }
    }

    // "DecreaseAngle" 매핑: S 키
    {
        bool bFound = false;
        for (const FInputActionKeyMapping& Mapping : InputSettings->GetActionMappings())
        {
            if (Mapping.ActionName == "DecreaseAngle" && Mapping.Key == EKeys::S)
            {
                bFound = true;
                break;
            }
        }
        if (!bFound)
        {
            InputSettings->AddActionMapping(FInputActionKeyMapping("DecreaseAngle", EKeys::S));
            bMappingsChanged = true;
        }
    }

    // "ThrowFruit" 매핑: SpaceBar (변경 없음)
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

    // "RotateCameraLeft" 매핑: A 키
    {
        bool bFound = false;
        for (const FInputActionKeyMapping& Mapping : InputSettings->GetActionMappings())
        {
            if (Mapping.ActionName == "RotateCameraLeft" && Mapping.Key == EKeys::A)
            {
                bFound = true;
                break;
            }
        }
        if (!bFound)
        {
            InputSettings->AddActionMapping(FInputActionKeyMapping("RotateCameraLeft", EKeys::A));
            bMappingsChanged = true;
        }
    }

    // "RotateCameraRight" 매핑: D 키
    {
        bool bFound = false;
        for (const FInputActionKeyMapping& Mapping : InputSettings->GetActionMappings())
        {
            if (Mapping.ActionName == "RotateCameraRight" && Mapping.Key == EKeys::D)
            {
                bFound = true;
                break;
            }
        }
        if (!bFound)
        {
            InputSettings->AddActionMapping(FInputActionKeyMapping("RotateCameraRight", EKeys::D));
            bMappingsChanged = true;
        }
    }

    if (bMappingsChanged)
    {
        InputSettings->SaveKeyMappings();
        UE_LOG(LogTemp, Log, TEXT("프로젝트 키 매핑이 업데이트 되었습니다."));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("프로젝트 키 매핑이 이미 설정되어 있습니다."));
    }
}