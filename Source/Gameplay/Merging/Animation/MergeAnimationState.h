#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MergeAnimationState.generated.h"

class AFruitBall;

// UMergeAnimationState: 개별 병합 애니메이션의 진행과 상태를 관리하는 UObject
namespace MergeAnimConstants
{
    // 기본 애니메이션 지속 시간 (초)
    constexpr float DEFAULT_ANIMATION_DURATION = 0.1f;
    
    // 애니메이션 업데이트 간격 (초)
    constexpr float ANIMATION_UPDATE_INTERVAL = 0.005f;
    
    // 애니메이션 완료 후 후처리 지연 시간 (초)
    constexpr float POST_ANIMATION_DELAY = 0.1f;
}

// 병합 애니메이션의 상태를 관리하는 UObject 클래스
UCLASS()
class UE_FRUITMOUNTAIN_API UMergeAnimationState : public UObject
{
    GENERATED_BODY()
    
public:
    // 생성자
    UMergeAnimationState();
    
    // 초기화 함수
    void Initialize(AFruitBall* InFruit1, AFruitBall* InFruit2, const FVector& InMergeLocation, int32 InNextBallType);
    
    // 애니메이션 틱 함수
    UFUNCTION()
    void UpdateAnimation();
    
    // 애니메이션 후처리 함수
    UFUNCTION()
    void FinishAnimation();
    
    // 애니메이션 정리 함수
    void Cleanup();
    
    // 애니메이션이 완료되었는지 확인
    bool IsCompleted() const
    {
        return bIsCompleted;
    }
    
    FTimerHandle GetAnimTimerHandle() const
    {
        return AnimTimerHandle;
    }
    
private:
    // 애니메이션 대상 과일
    UPROPERTY()
    TWeakObjectPtr<AFruitBall> Fruit1;
    
    UPROPERTY()
    TWeakObjectPtr<AFruitBall> Fruit2;
    
    UPROPERTY()
    TWeakObjectPtr<AFruitBall> NewFruit;
    
    // 애니메이션 파라미터
    FVector MergeLocation;
    int32 NextBallType;
    int32 CurrentBallType;
    float AnimDuration;
    
    // 초기 스케일 값
    FVector InitialScale1;
    FVector InitialScale2;
    
    // 애니메이션 진행 타이머
    float ElapsedTime;
    
    // 애니메이션 타이머 핸들
    FTimerHandle AnimTimerHandle;
    FTimerHandle PostAnimTimerHandle;
    
    // 상태 플래그
    bool bIsInitialized;
    bool bIsCompleted;
    bool bIsCleanedUp;
    
    // 필요한 유틸리티 함수
    void SpawnNewMergingFruit();
    void UpdateFruitScale(float Progress);
    void DestroyOriginalFruits();
    
    // 안전한 월드 참조 획득
    UWorld* GetWorld() const override;
};