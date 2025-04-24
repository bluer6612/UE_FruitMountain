#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MergeAnimator.generated.h"

class AFruitBall;

// UMergeAnimator: 병합 애니메이션 상태를 전역적으로 관리하는 유틸리티 클래스
UCLASS()
class UE_FRUITMOUNTAIN_API UMergeAnimator : public UObject
{
    GENERATED_BODY()
    
public:
    // 병합 애니메이션 시작
    static FTimerHandle AnimateMerge(AFruitBall* Fruit1, AFruitBall* Fruit2, const FVector& MergeLocation, int32 NextBallType, float AnimDuration);
    
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