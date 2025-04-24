#include "MergeAnimator.h"
#include "Actors/FruitBall.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"
#include "FruitMergeHelper.h"
#include "FruitMergeFeedbackHelper.h"
#include "Kismet/GameplayStatics.h"
#include "Gameplay/Controller/FruitPlayerController.h"
#include "Gameplay/Fruit/FruitSpawnHelper.h"
#include "Gameplay/Score/ScoreManagerComponent.h"

FTimerHandle UMergeAnimator::AnimateMerge(AFruitBall* Fruit1, AFruitBall* Fruit2, const FVector& MergeLocation, int32 NextBallType, float AnimDuration)
{
    if (!Fruit1 || !Fruit2)
    {
        return FTimerHandle();
    }

    UWorld* World = Fruit1->GetWorld();
    if (!World)
    {
        return FTimerHandle();
    }

    // 로그 추가: 병합 애니메이션 시작
    UE_LOG(LogTemp, Warning, TEXT("Starting Merge Animation for Fruits: %s and %s at location %s"), 
           *Fruit1->GetName(), *Fruit2->GetName(), *MergeLocation.ToString());

    // 초기 스케일 값 확인
    FVector InitialScale1 = Fruit1->GetActorScale3D();
    FVector InitialScale2 = Fruit2->GetActorScale3D();
    UE_LOG(LogTemp, Warning, TEXT("Initial Scales: Fruit1=%.2f, Fruit2=%.2f"), 
           InitialScale1.X, InitialScale2.X);

    // 애니메이션 시작 전 물리 비활성화 및 병합 상태 설정
    if (Fruit1->GetMeshComponent())
    {
        Fruit1->GetMeshComponent()->SetSimulatePhysics(false);
        Fruit1->GetMeshComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    
    if (Fruit2->GetMeshComponent())
    {
        Fruit2->GetMeshComponent()->SetSimulatePhysics(false);
        Fruit2->GetMeshComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    // 현재 과일 타입 저장
    int32 CurrentType = Fruit1->GetBallType();
    
    // 애니메이션 진행을 위한 타이머 설정
    FTimerHandle AnimTimerHandle;
    FTimerDelegate AnimTimerDelegate;
    
    // 경과 시간과 총 시간 추적을 위한 변수들 캡처
    const float TotalAnimTime = AnimDuration;
    
    // 수정된 버전: 모든 변수를 값으로 캡처하고 내부에서 ElapsedTime 관리
    float* ElapsedTimePtr = new float(0.0f); // 힙에 할당하여 타이머 인스턴스 분리
    AnimTimerDelegate.BindLambda([=]() mutable {
        *ElapsedTimePtr += 0.016f;
        float AnimProgress = FMath::Clamp(*ElapsedTimePtr / TotalAnimTime, 0.0f, 1.0f);
        
        // 로그로 애니메이션 진행 상태 확인
        UE_LOG(LogTemp, Warning, TEXT("Merge Animation: Progress=%.2f, Scale=%.2f"), 
               AnimProgress, CalculateAnimationScale(AnimProgress, false));
        
        // 애니메이션 적용
        TickMergeAnimation(Fruit1, Fruit2, AnimProgress, MergeLocation);
        
        // 애니메이션 완료 시
        if (AnimProgress >= 1.0f)
        {
            UWorld* CurrentWorld = Fruit1->GetWorld();
            if (!CurrentWorld) return;
            
            // 타이머 중지
            CurrentWorld->GetTimerManager().ClearTimer(AnimTimerHandle);
            
            // 메모리 해제
            delete ElapsedTimePtr;
            
            // 1. 이펙트 및 사운드 재생
            UFruitMergeFeedbackHelper::PlayMergeEffect(CurrentWorld, MergeLocation, CurrentType);
            
            // 2. 점수 추가
            UScoreManagerComponent::AddScoreStatic(CurrentWorld, CurrentType);
            
            // 3. 최대 레벨 확인
            if (CurrentType >= AFruitBall::MaxBallType)
            {
                // 최대 레벨이면 두 과일 제거하고 종료
                Fruit1->Destroy();
                Fruit2->Destroy();
                return;
            }
            
            // 4. 새 과일 생성
            AFruitPlayerController* Controller = Cast<AFruitPlayerController>(
                UGameplayStatics::GetPlayerController(CurrentWorld, 0));
            if (Controller)
            {
                // 새 과일 스폰
                AActor* SpawnedActor = UFruitSpawnHelper::SpawnBall(
                    Controller, MergeLocation, NextBallType, true);
                AFruitBall* NewFruit = Cast<AFruitBall>(SpawnedActor);
                
                if (NewFruit)
                {
                    // 기존 과일의 평균 회전각 적용
                    FRotator AvgRotation = (Fruit1->GetActorRotation() + Fruit2->GetActorRotation()) * 0.5f;
                    NewFruit->SetActorRotation(AvgRotation);
                    
                    // 성장 애니메이션 시작
                    AnimateNewFruitGrowth(NewFruit, AnimDuration);
                    
                    // 병합 후 안정화 (약간 지연)
                    FTimerHandle StabilizeTimerHandle;
                    CurrentWorld->GetTimerManager().SetTimer(StabilizeTimerHandle, [=]() {
                        UFruitMergeFeedbackHelper::StabilizeFruits(
                            CurrentWorld,
                            MergeLocation,
                            5.0f,
                            NewFruit,
                            NextBallType
                        );
                    }, AnimDuration + 0.05f, false);
                }
            }
            
            // 원본 과일 제거
            Fruit1->Destroy();
            Fruit2->Destroy();
        }
    });
    
    // 0.016초 간격으로 애니메이션 업데이트 (약 60fps)
    World->GetTimerManager().SetTimer(AnimTimerHandle, AnimTimerDelegate, 0.016f, true);
    
    return AnimTimerHandle;
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
    MeshComp->SetSimulatePhysics(false);
    MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    
    // 애니메이션 진행을 위한 타이머 설정
    FTimerHandle GrowthTimerHandle;
    const float TotalAnimTime = AnimDuration;
    
    World->GetTimerManager().SetTimer(GrowthTimerHandle, 
        [=]() mutable {
            float* ElapsedTime = new float(0.0f);
            return [=]() mutable {
                *ElapsedTime += 0.016f;
                float AnimProgress = FMath::Clamp(*ElapsedTime / TotalAnimTime, 0.0f, 1.0f);
                
                // 애니메이션 적용
                TickGrowthAnimation(NewFruit, AnimProgress);
                
                // 애니메이션 완료 시
                if (AnimProgress >= 1.0f)
                {
                    NewFruit->GetWorld()->GetTimerManager().ClearTimer(GrowthTimerHandle);
                    
                    // 충돌 재활성화
                    UStaticMeshComponent* FruitMesh = NewFruit->GetMeshComponent();
                    if (FruitMesh)
                    {
                        FruitMesh->SetSimulatePhysics(true);
                        FruitMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                    }
                    
                    delete ElapsedTime; // 메모리 해제
                }
            };
        }(), 
        0.016f, true);
}

void UMergeAnimator::TickMergeAnimation(AFruitBall* Fruit1, AFruitBall* Fruit2, float AnimProgress, const FVector& MergeLocation)
{
    // 첫 번째 과일 애니메이션
    if (IsValid(Fruit1))
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
    if (IsValid(Fruit2))
    {
        // 스케일 계산 - 축소 애니메이션
        float Scale = CalculateAnimationScale(AnimProgress, false);
        Fruit2->SetActorScale3D(FVector(Scale));
        
        // 병합 위치로 이동
        FVector StartPos = Fruit2->GetActorLocation();
        FVector NewPos = FMath::Lerp(StartPos, MergeLocation, AnimProgress * 0.7f);
        Fruit2->SetActorLocation(NewPos);
    }

    // 로그 추가
    UE_LOG(LogTemp, Warning, TEXT("Fruit1 Scale: %.2f, Fruit2 Scale: %.2f, Progress: %.2f"),
           Fruit1->GetActorScale3D().X, Fruit2->GetActorScale3D().X, AnimProgress);
}

void UMergeAnimator::TickGrowthAnimation(AFruitBall* Fruit, float AnimProgress)
{
    if (IsValid(Fruit))
    {
        // 스케일 계산 - 성장 애니메이션
        float Scale = CalculateAnimationScale(AnimProgress, true);
        Fruit->SetActorScale3D(FVector(Scale));
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