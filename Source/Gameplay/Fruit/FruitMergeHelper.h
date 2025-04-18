#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FruitMergeHelper.generated.h"

// 전방 선언 추가
class UScoreManagerComponent;
class AFruitBall;
class UWorld;

UCLASS()
class UE_FRUITMOUNTAIN_API UFruitMergeHelper : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
    
public:
    // 과일 병합 시도 (새로 추가)
    static void TryMergeFruits(AFruitBall* FruitA, AFruitBall* FruitB, const FVector& CollisionPoint);
    
    // 과일 병합 수행
    static void MergeFruits(AFruitBall* FruitA, AFruitBall* FruitB, const FVector& MergeLocation);
    
    // 점수 추가
    UFUNCTION(BlueprintCallable, Category = "Score")
    static void AddScore(UWorld* World, int32 BallType);
    
    // 병합 이펙트 재생
    static void PlayMergeEffect(UWorld* World, const FVector& Location, int32 BallType);
    
    // 병합 시 과일들을 안정화하는 함수
    static void StabilizeFruitPhysicscs(AFruitBall* Fruit, float InitialDampingMultiplier, bool bIsNewFruit);
    
    // 연쇄 초기화 함수
    UFUNCTION(BlueprintCallable, Category = "Score")
    static void ResetCombo(UWorld* World);

    // 모든 과일 메시를 미리 로드
    UFUNCTION(BlueprintCallable, Category = "Fruit")
    static void PreloadAllFruitMeshes(UWorld* World);
};