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
    // 과일 병합 시도
    UFUNCTION(BlueprintCallable, Category="Fruit|Merge")
    static void TryMergeFruits(AFruitBall* FruitA, AFruitBall* FruitB, const FVector& CollisionPoint);
    
    // 과일 병합 수행
    UFUNCTION(BlueprintCallable, Category="Fruit|Merge")
    static void MergeFruits(AFruitBall* FruitA, AFruitBall* FruitB, const FVector& MergeLocation);

    // 모든 과일 메시를 미리 로드
    UFUNCTION(BlueprintCallable, Category = "Fruit")
    static void PreloadAllFruitMeshes(UWorld* World);
};