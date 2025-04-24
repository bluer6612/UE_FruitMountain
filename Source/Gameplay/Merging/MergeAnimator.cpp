#include "MergeAnimator.h"
#include "MergeAnimationState.h" // 추가된 부분
#include "Actors/FruitBall.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"
#include "FruitMergeHelper.h"
#include "FruitMergeFeedbackHelper.h"
#include "Kismet/GameplayStatics.h"
#include "Gameplay/Controller/FruitPlayerController.h"
#include "Gameplay/Fruit/FruitSpawnHelper.h"
#include "Gameplay/Score/ScoreManagerComponent.h"

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

void UMergeAnimator::AnimateNewFruitGrowth(AFruitBall* NewFruit, float AnimDuration)
{
    if (!NewFruit || !NewFruit->GetMeshComponent())
    {
        return;
    }
    
    UWorld* World = NewFruit->GetWorld();
    if (!World)
    {
        return;
    }
    
    // 초기 크기를 매우 작게 설정
    NewFruit->SetActorScale3D(FVector(0.1f));
    
    // 물리 일시 비활성화 및 충돌 비활성화
    UStaticMeshComponent* MeshComp = NewFruit->GetMeshComponent();
    bool bWasSimulating = MeshComp->IsSimulatingPhysics();
    MeshComp->SetSimulatePhysics(false);
    MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    
    // AnimateNewFruitGrowth 함수 내 추가 로그
    UE_LOG(LogTemp, Warning, TEXT("새 과일 성장 애니메이션 시작: %s"), *NewFruit->GetName());
    
    // 애니메이션 진행을 위한 타이머 설정
    FTimerHandle GrowthTimerHandle;
    FTimerDelegate GrowthTimerDelegate;
    float* ElapsedTimePtr = new float(0.0f); // 힙에 할당
    const float TotalAnimTime = AnimDuration;
    
    GrowthTimerDelegate.BindLambda([=]() mutable {
        *ElapsedTimePtr += 0.01f;
        float AnimProgress = FMath::Clamp(*ElapsedTimePtr / TotalAnimTime, 0.0f, 1.0f);
        
        // 애니메이션 적용
        TickGrowthAnimation(NewFruit, AnimProgress);
        
        // 애니메이션 완료 시
        if (AnimProgress >= 1.0f)
        {
            if (!IsValid(NewFruit)) // 안전 체크
            {
                delete ElapsedTimePtr;
                return;
            }
            
            UWorld* CurrentWorld = NewFruit->GetWorld();
            if (!CurrentWorld) 
            {
                delete ElapsedTimePtr;
                return;
            }
            
            // 타이머 중지
            CurrentWorld->GetTimerManager().ClearTimer(GrowthTimerHandle);
            
            // 충돌 재활성화 - 안전 체크 추가
            UStaticMeshComponent* FruitMesh = NewFruit->GetMeshComponent();
            if (FruitMesh && IsValid(FruitMesh))
            {
                FruitMesh->SetSimulatePhysics(bWasSimulating);
                FruitMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            }
            
            // 정확한 최종 스케일 설정
            if (IsValid(NewFruit))
            {
                NewFruit->SetActorScale3D(FVector(1.0f));
            }
            
            // 메모리 해제
            delete ElapsedTimePtr;

            // 애니메이션 완료 시 로그
            UE_LOG(LogTemp, Display, TEXT("성장 애니메이션 완료: 과일 물리 속성 복원"));
        }
    });
    
    // 타이머 설정
    World->GetTimerManager().SetTimer(GrowthTimerHandle, GrowthTimerDelegate, 0.01f, true);
}

void UMergeAnimator::TickMergeAnimation(AFruitBall* Fruit1, AFruitBall* Fruit2, float AnimProgress, const FVector& MergeLocation)
{
    // 안전 검사를 더 철저히
    bool bFruit1Valid = IsValid(Fruit1);
    bool bFruit2Valid = IsValid(Fruit2);
    
    // 둘 다 유효하지 않으면 즉시 반환
    if (!bFruit1Valid && !bFruit2Valid)
    {
        return;
    }
    
    // 첫 번째 과일 애니메이션
    if (bFruit1Valid)
    {
        // 스케일 계산 - 축소 애니메이션
        float Scale = CalculateAnimationScale(AnimProgress, false);
        Fruit1->SetActorScale3D(FVector(Scale));
        
        // 병합 위치로 이동
        FVector StartPos = Fruit1->GetActorLocation();
        FVector NewPos = FMath::Lerp(StartPos, MergeLocation, AnimProgress * 0.7f);
        Fruit1->SetActorLocation(NewPos);
    }
    
    // 두 번째 과일 애니메이션
    if (bFruit2Valid)
    {
        // 스케일 계산 - 축소 애니메이션
        float Scale = CalculateAnimationScale(AnimProgress, false);
        Fruit2->SetActorScale3D(FVector(Scale));
        
        // 병합 위치로 이동
        FVector StartPos = Fruit2->GetActorLocation();
        FVector NewPos = FMath::Lerp(StartPos, MergeLocation, AnimProgress * 0.7f);
        Fruit2->SetActorLocation(NewPos);
    }

    // 과일 스케일 및 진행 상태 로그
    if (bFruit1Valid && bFruit2Valid)
    {
        UE_LOG(LogTemp, Warning, TEXT("과일1 스케일: %.2f, 과일2 스케일: %.2f, 진행도: %.2f"),
               Fruit1->GetActorScale3D().X, Fruit2->GetActorScale3D().X, AnimProgress);
    }
}

void UMergeAnimator::TickGrowthAnimation(AFruitBall* Fruit, float AnimProgress)
{
    if (IsValid(Fruit))
    {
        // 스케일 계산 - 성장 애니메이션
        float Scale = CalculateAnimationScale(AnimProgress, true);
        Fruit->SetActorScale3D(FVector(Scale));

        // TickGrowthAnimation 함수 내 추가 로그
        UE_LOG(LogTemp, Warning, TEXT("성장 애니메이션: 진행도=%.2f, 스케일=%.2f"),
               AnimProgress, Scale);
    }
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