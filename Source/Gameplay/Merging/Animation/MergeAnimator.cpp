#include "MergeAnimator.h"
#include "Gameplay/Merging/Core/MergeController.h"

// 호환성을 위한 리디렉션 함수들
FTimerHandle UMergeAnimator::AnimateMerge(AFruitBall* Fruit1, AFruitBall* Fruit2, const FVector& MergeLocation, int32 NextBallType, float AnimDuration)
{
    // MergeController로 모든 호출 리디렉션
    UWorld* World = Fruit1 ? Fruit1->GetWorld() : nullptr;
    if (!World)
    {
        return FTimerHandle();
    }
    
    AMergeController* Controller = AMergeController::Get(World);
    if (!Controller)
    {
        return FTimerHandle();
    }
    
    return Controller->AnimateMerge(Fruit1, Fruit2, MergeLocation, NextBallType, AnimDuration);
}

bool UMergeAnimator::IsGlobalMergeInProgress()
{
    // 첫 번째 월드 컨텍스트 확인
    UWorld* World = GEngine->GetWorldContexts().Num() > 0 ? 
                   GEngine->GetWorldContexts()[0].World() : nullptr;
    
    if (!World)
    {
        return false;
    }
    
    AMergeController* Controller = AMergeController::Get(World);
    return Controller ? Controller->IsMergeInProgress() : false;
}

void UMergeAnimator::SetGlobalMergeInProgress(bool bInProgress)
{
    UWorld* World = GEngine->GetWorldContexts().Num() > 0 ? 
                   GEngine->GetWorldContexts()[0].World() : nullptr;
    
    if (!World)
    {
        return;
    }
    
    AMergeController* Controller = AMergeController::Get(World);
    if (Controller)
    {
        Controller->SetMergeInProgress(bInProgress);
    }
}