#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FruitMergeFeedbackHelper.generated.h"

class UWorld;
class AFruitBall;

UCLASS()
class UE_FRUITMOUNTAIN_API UFruitMergeFeedbackHelper : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
    
public:
    // 병합 이펙트 재생
    UFUNCTION(BlueprintCallable, Category = "Fruit Feedback")
    static void PlayMergeEffect(UWorld* World, const FVector& Location, int32 BallType);
    
    // 통합된 과일 안정화 함수
    UFUNCTION(BlueprintCallable, Category = "Fruit Physics")
    static void StabilizeFruits(UWorld* World, AFruitBall* SingleFruit = nullptr, float DampingMultiplier = 20.0f, bool bIsNewFruit = false);
    
    // 단일 과일 물리 속성 안정화 (내부 헬퍼 함수)
    static void StabilizeSingleFruit(AFruitBall* Fruit, float InitialDampingMultiplier, bool bIsNewFruit);
};