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
    
    // 애니메이션 시간 설정 (0.2초 이내 완료)
    const float TotalAnimTime = FMath::Min(AnimDuration, 0.2f);
    
    // 로그
    UE_LOG(LogTemp, Warning, TEXT("과일 병합 애니메이션 시작: %s와 %s (위치: %s"), 
           *Fruit1->GetName(), *Fruit2->GetName(), *MergeLocation.ToString());

    // 초기 스케일 값 저장
    FVector InitialScale1 = Fruit1->GetActorScale3D();
    FVector InitialScale2 = Fruit2->GetActorScale3D();
    UE_LOG(LogTemp, Warning, TEXT("초기 스케일: 과일1=%.2f, 과일2=%.2f"), 
           InitialScale1.X, InitialScale2.X);

    // 두 과일의 물리/충돌 비활성화
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

    // 과일 타입 저장
    int32 CurrentType = Fruit1->GetBallType();
    
    // 1. 중앙 지점에 새 과일 미리 생성 - 초기에는 매우 작은 크기로
    AFruitPlayerController* Controller = Cast<AFruitPlayerController>(
        UGameplayStatics::GetPlayerController(World, 0));
    
    AFruitBall* NewFruit = nullptr;
    if (Controller && CurrentType < AFruitBall::MaxBallType)
    {
        // 새 과일 스폰
        AActor* SpawnedActor = UFruitSpawnHelper::SpawnBall(
            Controller, MergeLocation, NextBallType, true);
        NewFruit = Cast<AFruitBall>(SpawnedActor);
        
        if (NewFruit)
        {
            // 기존 과일의 평균 회전각 적용
            FRotator AvgRotation = (Fruit1->GetActorRotation() + Fruit2->GetActorRotation()) * 0.5f;
            NewFruit->SetActorRotation(AvgRotation);
            
            // 초기에는 매우 작게 설정
            NewFruit->SetActorScale3D(FVector(0.05f));
            
            // 물리 비활성화
            if (NewFruit->GetMeshComponent())
            {
                NewFruit->GetMeshComponent()->SetSimulatePhysics(false);
                NewFruit->GetMeshComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            }
        }
    }
    
    // 타이머 설정
    FTimerHandle AnimTimerHandle;
    
    // 람다 함수에서 참조할 포인터를 힙에 할당
    float* ElapsedTimePtr = new float(0.0f);
    
    // 타이머 델리게이트 설정
    FTimerDelegate AnimTimerDelegate;

    AnimTimerDelegate.BindLambda([=]() mutable {
        // 경과 시간 증가
        *ElapsedTimePtr += 0.01f;
        float AnimProgress = FMath::Clamp(*ElapsedTimePtr / TotalAnimTime, 0.0f, 1.0f);
        
        // 애니메이션 완료 검사를 가장 먼저 수행
        if (AnimProgress >= 1.0f)
        {
            // 클래스 메서드 호출
            UMergeAnimator::CleanupMergeAnimation(World, AnimTimerHandle, ElapsedTimePtr);
            
            bool bFruit1Valid = IsValid(Fruit1);
            bool bFruit2Valid = IsValid(Fruit2);
            
            // 원본 과일 제거
            if (bFruit1Valid) Fruit1->Destroy();
            if (bFruit2Valid) Fruit2->Destroy();
            
            // 점수 추가
            UScoreManagerComponent::AddScoreStatic(World, CurrentType);
            
            // 새 과일 생성
            if (IsValid(NewFruit) && NewFruit->GetMeshComponent())
            {
                // 물리만 활성화
                NewFruit->GetMeshComponent()->SetSimulatePhysics(true);
                NewFruit->GetMeshComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            }
            
            UE_LOG(LogTemp, Display, TEXT("병합 애니메이션 완료"));
            
            // 후처리 타이머는 별도의 안전장치 추가
            FTimerHandle PostMergeTimerHandle;
            World->GetTimerManager().SetTimer(PostMergeTimerHandle, [=]() {
                // 효과 실행
                if (IsValid(NewFruit))
                {
                    UFruitMergeFeedbackHelper::PlayMergeEffect(World, MergeLocation, CurrentType);
                }
                
                // 병합 플래그 해제
                SetGlobalMergeInProgress(false);
                
                // 클래스 멤버 변수 사용
                bAnimationCompletionInProgress = false;
                
                UE_LOG(LogTemp, Display, TEXT("병합 후처리 완료: 새 병합 가능 상태"));
            }, 0.1f, false); // 딜레이 증가
            
            return; // 중요: 완료 후 즉시 반환
        }
        
        // 기존 과일들 축소 애니메이션
        bool bFruit1Valid = IsValid(Fruit1);
        bool bFruit2Valid = IsValid(Fruit2);
        
        if (bFruit1Valid)
        {
            float ShrinkScale = 1.0f - AnimProgress * 0.95f;
            Fruit1->SetActorScale3D(FVector(ShrinkScale * InitialScale1.X));
        }
        
        if (bFruit2Valid)
        {
            float ShrinkScale = 1.0f - AnimProgress * 0.95f;
            Fruit2->SetActorScale3D(FVector(ShrinkScale * InitialScale2.X));
        }
        
        // 새 과일 성장 애니메이션
        if (IsValid(NewFruit))
        {
            float GrowScale = 0.05f + AnimProgress * 0.95f;
            NewFruit->SetActorScale3D(FVector(GrowScale));
        }
        
        // 진행 로그
        UE_LOG(LogTemp, Warning, TEXT("병합 진행도: %.2f - 원본 과일 스케일: %.2f, 새 과일 스케일: %.2f"), 
            AnimProgress, 
            bFruit1Valid ? Fruit1->GetActorScale3D().X : 0.0f,
            IsValid(NewFruit) ? NewFruit->GetActorScale3D().X : 0.0f);
    });
    
    // 타이머 설정 변경 - 중복 호출 방지
    if (World->GetTimerManager().TimerExists(AnimTimerHandle))
    {
        World->GetTimerManager().ClearTimer(AnimTimerHandle);
    }
    World->GetTimerManager().SetTimer(AnimTimerHandle, AnimTimerDelegate, 0.01f, true);
    
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

void UMergeAnimator::CleanupMergeAnimation(UWorld* World, FTimerHandle& TimerHandle, float* TimePtr)
{
    if (World && World->GetTimerManager().TimerExists(TimerHandle))
    {
        World->GetTimerManager().ClearTimer(TimerHandle);
    }
    
    if (TimePtr)
    {
        delete TimePtr;
        TimePtr = nullptr;
    }
}