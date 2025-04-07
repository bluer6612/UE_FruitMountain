#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FruitMergeHelper.generated.h"

// 전방 선언
class AFruitBall;

UCLASS()
class UE_FRUITMOUNTAIN_API UFruitMergeHelper : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
    
public:
    // 두 과일을 합치는 함수
    UFUNCTION(BlueprintCallable, Category="Fruit")
    static void MergeFruits(AFruitBall* FruitA, AFruitBall* FruitB, const FVector& MergeLocation);
    
    // 점수 적립
    UFUNCTION(BlueprintCallable, Category="Fruit")
    static void AddScore(int32 BallType);
    
    // 병합 효과 재생
    UFUNCTION(BlueprintCallable, Category="Fruit")
    static void PlayMergeEffect(UWorld* World, const FVector& Location, int32 BallType);
};