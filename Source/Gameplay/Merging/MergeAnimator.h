#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MergeAnimator.generated.h"

class AFruitBall;

UCLASS()
class UE_FRUITMOUNTAIN_API UMergeAnimator : public UObject
{
    GENERATED_BODY()
    
public:
    // 병합 애니메이션 시작
    static FTimerHandle AnimateMerge(AFruitBall* Fruit1, AFruitBall* Fruit2, 
                                    const FVector& MergeLocation, 
                                    int32 NextBallType, 
                                    float AnimDuration = 0.15f);
    
    // 새 과일 성장 애니메이션
    static void AnimateNewFruitGrowth(AFruitBall* NewFruit, float AnimDuration = 0.1f);
    
    // 애니메이션 틱 처리
    static void TickMergeAnimation(AFruitBall* Fruit1, AFruitBall* Fruit2, 
                                  float AnimProgress, const FVector& MergeLocation);
    
    // 성장 애니메이션 틱 처리
    static void TickGrowthAnimation(AFruitBall* NewFruit, float AnimProgress);
    
    // 애니메이션 정리 함수 - 매개변수 수정
    static void CleanupMergeAnimation(UWorld* World, FTimerHandle& TimerHandle, float* ElapsedTimePtr);
    
    // 애니메이션 스케일 계산
    static float CalculateAnimationScale(float Progress, bool bIsGrowing = true);
    
    // 전역 병합 상태 확인
    static bool IsGlobalMergeInProgress();
    
    // 전역 병합 상태 설정
    static void SetGlobalMergeInProgress(bool bInProgress);
    
private:
    // 전역 병합 진행 중 플래그
    static bool bGlobalMergeInProgress;
    
    // 애니메이션 완료 처리 중 플래그
    static bool bAnimationCompletionInProgress;
};