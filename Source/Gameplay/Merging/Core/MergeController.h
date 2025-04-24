#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MergeController.generated.h"

class AFruitBall;
class UMergeAnimationState;

// AMergeController: 병합 프로세스 전체를 관리하는 싱글톤 컨트롤러
UCLASS()
class UE_FRUITMOUNTAIN_API AMergeController : public AActor
{
    GENERATED_BODY()
    
public:
    AMergeController();
    virtual void Tick(float DeltaTime) override;
    
    // 싱글톤 인스턴스 접근
    UFUNCTION(BlueprintCallable, Category="Merge System")
    static AMergeController* Get(const UObject* WorldContext);
    
    // 병합 프로세스 시작
    UFUNCTION(BlueprintCallable, Category="Merge System")
    bool StartMerge(AFruitBall* Fruit1, AFruitBall* Fruit2, const FVector& CollisionPoint);
    
    // 기존 병합 상태 함수
    UFUNCTION(BlueprintCallable, Category="Merge System")
    bool IsMergeInProgress() const { return bMergeInProgress; }

    UFUNCTION(BlueprintCallable, Category = "Merging")
    void SetMergeInProgress(bool bInProgress) { bMergeInProgress = bInProgress; }
    
    // 병합 완료 처리 함수
    void CompleteMerge();
    
    // 병합 이펙트 재생 함수
    UFUNCTION(BlueprintCallable, Category = "Fruit Merging")
    static void PlayMergeEffect(UWorld* World, const FVector& Location, int32 BallType);
    
    // 병합 애니메이션 시작
    UFUNCTION(BlueprintCallable, Category = "Merging Animation")
    FTimerHandle AnimateMerge(AFruitBall* Fruit1, AFruitBall* Fruit2, 
                             const FVector& MergeLocation, 
                             int32 NextBallType, 
                             float AnimDuration = 0.15f);
    
    // 애니메이션 정리 함수
    void CleanupMergeAnimation(UWorld* World, FTimerHandle& TimerHandle, float* ElapsedTimePtr);
    
    // 객체 풀링 관련 함수
    UMergeAnimationState* GetAnimationStateFromPool();
    void ReturnAnimationStateToPool(UMergeAnimationState* AnimState);
    void EnsurePoolSize(int32 MinSize);

protected:
    virtual void BeginPlay() override;

private:
    // 싱글톤 인스턴스
    static AMergeController* Instance;
    
    // 병합 진행 중 상태 변수
    UPROPERTY()
    bool bMergeInProgress;
    
    // 애니메이션 처리 중 플래그 (UMergeAnimator에서 이전)
    UPROPERTY()
    bool bAnimationCompletionInProgress;
    
    // 애니메이션 상태 객체 풀
    UPROPERTY()
    TArray<UMergeAnimationState*> AnimationStatePool;
    
    // 풀의 기본 크기
    static constexpr int32 DEFAULT_POOL_SIZE = 5;
};