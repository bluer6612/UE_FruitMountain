#pragma once

#include "CoreMinimal.h"
#include "GameFramework/InputSettings.h"  // 기존 include
#include "FruitInputMappingManager.generated.h"  // 반드시 마지막에 포함

/**
 * 주석: 비블루프린트 함수로 프로젝트 키 매핑을 설정하는 Manager 클래스
 */
UCLASS()
class UE_FRUITMOUNTAIN_API UFruitInputMappingManager : public UObject
{
    GENERATED_BODY()

public:
    // 주석: 프로젝트 키 매핑 설정 함수를 C++로 호출
    UFUNCTION(BlueprintCallable, Category = "Input")
    static void ConfigureKeyMappings();
};