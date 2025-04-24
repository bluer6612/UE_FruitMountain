#include "MergeAnimator.h"
#include "MergeAnimationState.h"
#include "Actors/FruitBall.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"
#include "Gameplay/Merging/Core/FruitMergeHelper.h"
#include "Gameplay/Merging/Core/FruitMergeStabilizer.h"
#include "Kismet/GameplayStatics.h"
#include "Gameplay/Controller/FruitPlayerController.h"
#include "Gameplay/Fruit/FruitSpawnHelper.h"

// 정적 변수 정의
bool UMergeAnimator::bGlobalMergeInProgress = false;

// 코드 시작 부분에 초기화
bool UMergeAnimator::bAnimationCompletionInProgress = false;

bool UMergeAnimator::IsGlobalMergeInProgress()
{
    return bGlobalMergeInProgress;
}

void UMergeAnimator::SetGlobalMergeInProgress(bool bInProgress)
{
    bGlobalMergeInProgress = bInProgress;
    
    // 디버깅 로그 추가
    UE_LOG(LogTemp, Warning, TEXT("전역 병합 상태 변경: %s"), 
           bInProgress ? TEXT("병합 진행 중") : TEXT("병합 없음"));
}

FTimerHandle UMergeAnimator::AnimateMerge(AFruitBall* Fruit1, AFruitBall* Fruit2, const FVector& MergeLocation, int32 NextBallType, float AnimDuration)
{
    // 이미 병합 중이면 중단
    if (bGlobalMergeInProgress)
    {
        UE_LOG(LogTemp, Warning, TEXT("이미 다른 병합이 진행 중입니다. 병합 요청 무시"));
        return FTimerHandle();
    }
    
    // 병합 시작 - 전역 플래그 설정
    SetGlobalMergeInProgress(true);
    
    // 기본 유효성 검사
    if (!Fruit1 || !Fruit2 || !IsValid(Fruit1) || !IsValid(Fruit2))
    {
        SetGlobalMergeInProgress(false);
        return FTimerHandle();
    }

    UWorld* World = Fruit1->GetWorld();
    if (!World)
    {
        SetGlobalMergeInProgress(false);
        return FTimerHandle();
    }
    
    // UObject 애니메이션 상태 생성
    UMergeAnimationState* AnimState = NewObject<UMergeAnimationState>();
    if (AnimState)
    {
        // GC에서 제거되지 않도록 루트에 추가
        AnimState->AddToRoot();
        
        // 애니메이션 초기화 및 시작
        AnimState->Initialize(Fruit1, Fruit2, MergeLocation, NextBallType, AnimDuration);
        
        // 애니메이션 완료 여부를 주기적으로 확인
        FTimerHandle CleanupTimerHandle;
        World->GetTimerManager().SetTimer(CleanupTimerHandle, [AnimState]() {
            if (AnimState->IsCompleted())
            {
                // 완료되면 GC 허용
                AnimState->RemoveFromRoot();
            }
        }, 0.5f, false);
    }
    
    // 비어 있는 타이머 핸들 반환 (이전과 호환성 유지)
    return FTimerHandle();
}

float UMergeAnimator::CalculateAnimationScale(float Progress, bool IsGrowing)
{
    if (IsGrowing)
    {
        // 단순하게: 0.1에서 1.0으로 선형 증가
        return 0.1f + Progress * 0.9f;
    }
    else
    {
        // 단순하게: 1.0에서 0.1로 선형 감소
        return 1.0f - (Progress * 0.9f);
    }
}

void UMergeAnimator::CleanupMergeAnimation(UWorld* World, FTimerHandle& TimerHandle, float* ElapsedTimePtr)
{
    // 타이머가 활성화되어 있으면 중지
    if (World && World->GetTimerManager().TimerExists(TimerHandle))
    {
        World->GetTimerManager().ClearTimer(TimerHandle);
    }
    
    // 할당된 메모리 해제
    if (ElapsedTimePtr)
    {
        delete ElapsedTimePtr;
    }
    
    // 전역 병합 상태 해제
    SetGlobalMergeInProgress(false);
}