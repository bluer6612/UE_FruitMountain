#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FruitMergeStabilizer.generated.h"

class AFruitBall;

// UFruitMergeStabilizer: 병합 과정에서 주변 과일들을 안정화시키는 유틸리티
UCLASS()
class UE_FRUITMOUNTAIN_API UFruitMergeStabilizer : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
    
public:
    // 통합된 안정화 함수
    UFUNCTION(BlueprintCallable, Category = "Fruit Feedback")
    static void StabilizeFruits(
        UWorld* World,                      // 월드 컨텍스트
        const FVector& Center,              // 중심 위치 (병합 위치)
        float DampingMultiplier = 3.0f,     // 감쇠 계수
        AFruitBall* TargetFruit = nullptr,  // 특정 대상 과일 (병합 후 새 과일)
        int32 FruitType = 1                // 과일 타입 (크기 계산용)
    );
};