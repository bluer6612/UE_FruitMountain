#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FruitMergeHelper.generated.h"

// 전방 선언
class AFruitBall;

// UFruitMergeHelper: 병합 관련 유틸리티 함수들을 제공하는 정적 함수 라이브러리
UCLASS()
class UE_FRUITMOUNTAIN_API UFruitMergeHelper : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
    
public:
    // 충돌 이벤트 관리
    UFUNCTION(BlueprintCallable, Category = "Fruit Merging")
    static void RegisterCollisionHandlers(AFruitBall* Fruit);

    // 과일 충돌 처리 및 병합 조건 검사
    UFUNCTION(BlueprintCallable, Category = "Fruit Merging")
    static void ProcessFruitCollision(AFruitBall* FruitA, AFruitBall* FruitB, const FVector& CollisionPoint);
    
    // 실제 병합 수행 (기존 함수)
    UFUNCTION(BlueprintCallable, Category = "Fruit Merging")
    static void MergeFruits(AFruitBall* FruitA, AFruitBall* FruitB, const FVector& MergeLocation);
    
    // 모든 메시 사전 로드 (기존 함수)
    UFUNCTION(BlueprintCallable, Category = "Fruit Merging")
    static void PreloadAllFruitMeshes(UWorld* World);
};