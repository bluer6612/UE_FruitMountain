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

    // "IncreaseAngle" 매핑: Up키
    {
        bool bFound = false;
        for (const FInputActionKeyMapping& Mapping : InputSettings->GetActionMappings())
        {
            if (Mapping.ActionName == "IncreaseAngle" && Mapping.Key == EKeys::Up)
            {
                bFound = true;
                break;
            }
        }
        if (!bFound)
        {
            InputSettings->AddActionMapping(FInputActionKeyMapping("IncreaseAngle", EKeys::Up));
            bMappingsChanged = true;
        }
    }

    // "DecreaseAngle" 매핑: Down키
    {
        bool bFound = false;
        for (const FInputActionKeyMapping& Mapping : InputSettings->GetActionMappings())
        {
            if (Mapping.ActionName == "DecreaseAngle" && Mapping.Key == EKeys::Down)
            {
                bFound = true;
                break;
            }
        }
        if (!bFound)
        {
            InputSettings->AddActionMapping(FInputActionKeyMapping("DecreaseAngle", EKeys::Down));
            bMappingsChanged = true;
        }
    }

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