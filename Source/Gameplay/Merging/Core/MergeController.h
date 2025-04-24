#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MergeController.generated.h"

class AFruitBall;
class UMergeAnimationState;

UCLASS()
class UE_FRUITMOUNTAIN_API AMergeController : public AActor
{
    GENERATED_BODY()
    
public:
    AMergeController();

    virtual void BeginPlay() override;
    
    // 싱글톤 접근자
    UFUNCTION(BlueprintCallable, Category = "Fruit Merging")
    static AMergeController* Get(const UObject* WorldContextObject);

    // 과일 충돌 처리 및 병합 조건 검사
    UFUNCTION(BlueprintCallable, Category = "Fruit Merging")
    static void StartMerge(AFruitBall* FruitA, AFruitBall* FruitB, const FVector& CollisionPoint);
    
    // 실제 병합 수행 (기존 함수)
    UFUNCTION(BlueprintCallable, Category = "Fruit Merging")
    static void MergeFruits(AFruitBall* FruitA, AFruitBall* FruitB, const FVector& MergeLocation);
    
    // 병합 애니메이션
    UFUNCTION(BlueprintCallable, Category = "Fruit Merging")
    FTimerHandle AnimateMerge(AFruitBall* Fruit1, AFruitBall* Fruit2, const FVector& MergeLocation, int32 NextBallType);
    
    // 병합 상태 관리
    UFUNCTION(BlueprintCallable, Category = "Fruit Merging")
    bool IsMergeInProgress() const
    {
        return bMergeInProgress;
    }
    
    UFUNCTION(BlueprintCallable, Category = "Fruit Merging")
    void SetMergeInProgress(bool bInProgress)
    {
        bMergeInProgress = bInProgress;
    }
    
private:
    // 병합 진행 중 상태
    UPROPERTY()
    bool bMergeInProgress;
    
    // 싱글톤 인스턴스
    static AMergeController* Instance;
    
    // 레벨이 변경될 때 초기화 필요
    UFUNCTION()
    static void HandleLevelChange(UWorld* World);
};