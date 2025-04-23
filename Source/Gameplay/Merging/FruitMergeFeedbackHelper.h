#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FruitMergeFeedbackHelper.generated.h"

class AFruitBall;

UCLASS()
class UE_FRUITMOUNTAIN_API UFruitMergeFeedbackHelper : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
    
public:
    // 통합된 안정화 함수 - PrepareSpaceForMerge와 StabilizeAfterMerge 기능 포함
    UFUNCTION(BlueprintCallable, Category = "Fruit Feedback")
    static void StabilizeFruits(
        UWorld* World,                      // 월드 컨텍스트
        const FVector& Center,              // 중심 위치 (병합 위치)
        float DampingMultiplier = 3.0f,     // 감쇠 계수
        AFruitBall* TargetFruit = nullptr,  // 특정 대상 과일 (병합 후 새 과일)
        int32 FruitType = 1                // 과일 타입 (크기 계산용)
    );
    
    // 병합 이펙트 재생
    UFUNCTION(BlueprintCallable, Category = "Fruit Feedback")
    static void PlayMergeEffect(UWorld* World, const FVector& Location, int32 BallType);
};