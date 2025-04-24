#include "MergeAnimationState.h"
#include "Actors/FruitBall.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Gameplay/Controller/FruitPlayerController.h"
#include "Gameplay/Fruit/FruitSpawnHelper.h"
#include "FruitMergeFeedbackHelper.h"
#include "MergeAnimator.h"

UMergeAnimationState::UMergeAnimationState()
    : NextBallType(0)
    , CurrentBallType(0)
    , AnimDuration(0.15f)
    , ElapsedTime(0.0f)
    , bIsInitialized(false)
    , bIsCompleted(false)
    , bIsCleanedUp(false)
{
}

void UMergeAnimationState::Initialize(AFruitBall* InFruit1, AFruitBall* InFruit2, const FVector& InMergeLocation, 
                                    int32 InNextBallType, float InAnimDuration)
{
    // 유효성 검사
    if (!InFruit1 || !InFruit2 || !IsValid(InFruit1) || !IsValid(InFruit2))
    {
        UE_LOG(LogTemp, Error, TEXT("병합 애니메이션 초기화 실패: 유효하지 않은 과일"));
        return;
    }
    
    // 객체 참조
    Fruit1 = InFruit1;
    Fruit2 = InFruit2;
    MergeLocation = InMergeLocation;
    NextBallType = InNextBallType;
    AnimDuration = FMath::Min(InAnimDuration, 0.2f);
    CurrentBallType = InFruit1->GetBallType();
    
    // 초기 스케일 저장
    InitialScale1 = InFruit1->GetActorScale3D();
    InitialScale2 = InFruit2->GetActorScale3D();
    
    UE_LOG(LogTemp, Warning, TEXT("과일 병합 애니메이션 시작: %s와 %s (위치: %s"), 
           *InFruit1->GetName(), *InFruit2->GetName(), *MergeLocation.ToString());
    UE_LOG(LogTemp, Warning, TEXT("초기 스케일: 과일1=%.2f, 과일2=%.2f"), 
           InitialScale1.X, InitialScale2.X);
           
    // 물리/충돌 비활성화
    if (InFruit1->GetMeshComponent())
    {
        InFruit1->GetMeshComponent()->SetSimulatePhysics(false);
        InFruit1->GetMeshComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    
    if (InFruit2->GetMeshComponent())
    {
        InFruit2->GetMeshComponent()->SetSimulatePhysics(false);
        InFruit2->GetMeshComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    
    // 새 과일 생성
    SpawnNewFruit();
    
    // 초기화 완료
    bIsInitialized = true;
    
    // 애니메이션 타이머 시작
    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().SetTimer(AnimTimerHandle, 
            this, &UMergeAnimationState::UpdateAnimation, 0.01f, true);
    }
}

void UMergeAnimationState::UpdateAnimation()
{
    if (!bIsInitialized || bIsCompleted)
    {
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // 경과 시간 증가
    ElapsedTime += 0.01f; 
    float Progress = FMath::Clamp(ElapsedTime / AnimDuration, 0.0f, 1.0f);
    
    // 스케일 업데이트
    UpdateFruitScale(Progress);
    
    // 완료 체크
    if (Progress >= 1.0f && !bIsCompleted)
    {
        // 애니메이션 타이머 중지
        World->GetTimerManager().ClearTimer(AnimTimerHandle);
        
        // 원본 과일 제거
        DestroyOriginalFruits();
        
        // 새 과일 활성화
        AFruitBall* CurrentNewFruit = NewFruit.Get();
        if (CurrentNewFruit && CurrentNewFruit->GetMeshComponent())
        {
            CurrentNewFruit->GetMeshComponent()->SetSimulatePhysics(true);
            CurrentNewFruit->GetMeshComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        }
        
        UE_LOG(LogTemp, Display, TEXT("병합 애니메이션 완료"));
        
        // 완료 플래그 설정
        bIsCompleted = true;
        
        // 후처리 타이머 설정
        World->GetTimerManager().SetTimer(PostAnimTimerHandle,
            this, &UMergeAnimationState::FinishAnimation, 0.1f, false);
    }
}

void UMergeAnimationState::FinishAnimation()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // 최종 스케일 명시적으로 설정
    if (AFruitBall* CurrentNewFruit = NewFruit.Get())
    {
        // 타입에 맞는 정확한 최종 크기 설정
        float FinalScale = AFruitBall::CalculateBallSize(NextBallType) / 100.0f; // UE 스케일로 변환
        CurrentNewFruit->SetActorScale3D(FVector(FinalScale));
    }
    
    // 효과 실행
    UFruitMergeFeedbackHelper::PlayMergeEffect(World, MergeLocation, CurrentBallType);
    
    // 전역 병합 상태 해제
    UMergeAnimator::SetGlobalMergeInProgress(false);
    
    UE_LOG(LogTemp, Display, TEXT("병합 후처리 완료: 새 병합 가능 상태"));
    
    // 객체 정리
    Cleanup();
}

void UMergeAnimationState::Cleanup()
{
    if (bIsCleanedUp)
    {
        return;
    }
    
    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().ClearTimer(AnimTimerHandle);
        World->GetTimerManager().ClearTimer(PostAnimTimerHandle);
    }
    
    bIsCleanedUp = true;
}

void UMergeAnimationState::SpawnNewFruit()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    AFruitPlayerController* Controller = Cast<AFruitPlayerController>(
        UGameplayStatics::GetPlayerController(World, 0));
        
    if (Controller)
    {
        AActor* SpawnedActor = UFruitSpawnHelper::SpawnBall(
            Controller, MergeLocation, NextBallType, true);
        AFruitBall* NewBall = Cast<AFruitBall>(SpawnedActor);
        
        if (NewBall)
        {
            NewFruit = NewBall;
            
            // 회전값 평균 계산
            AFruitBall* Fruit1Ball = Fruit1.Get();
            AFruitBall* Fruit2Ball = Fruit2.Get();
            
            FRotator AvgRotation = Fruit1Ball && Fruit2Ball ?
                (Fruit1Ball->GetActorRotation() + Fruit2Ball->GetActorRotation()) * 0.5f :
                FRotator::ZeroRotator;
            
            NewBall->SetActorRotation(AvgRotation);
            NewBall->SetActorScale3D(FVector(0.05f)); // 초기 작은 크기로 시작
            
            if (NewBall->GetMeshComponent())
            {
                NewBall->GetMeshComponent()->SetSimulatePhysics(false);
                NewBall->GetMeshComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            }
        }
    }
}

void UMergeAnimationState::UpdateFruitScale(float Progress)
{
    // 원본 과일 축소
    AFruitBall* Fruit1Ball = Fruit1.Get();
    AFruitBall* Fruit2Ball = Fruit2.Get();
    AFruitBall* NewBall = NewFruit.Get();
    
    if (Fruit1Ball)
    {
        float ShrinkScale = 1.0f - Progress * 0.95f;
        Fruit1Ball->SetActorScale3D(FVector(ShrinkScale * InitialScale1.X));
    }
    
    if (Fruit2Ball)
    {
        float ShrinkScale = 1.0f - Progress * 0.95f;
        Fruit2Ball->SetActorScale3D(FVector(ShrinkScale * InitialScale2.X));
    }
    
    if (NewBall)
    {
        // 타입에 맞는 적절한 크기 계산 (FruitBall 클래스의 함수 활용)
        float TargetScale = AFruitBall::CalculateBallSize(NextBallType) / 100.0f; // UE 스케일로 변환
        
        // 작은 크기에서 목표 크기로 보간
        float GrowScale = 0.05f + Progress * (TargetScale - 0.05f);
        NewBall->SetActorScale3D(FVector(GrowScale));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("병합 진행도: %.2f - 원본 과일 스케일: %.2f, 새 과일 스케일: %.2f"), 
        Progress, 
        Fruit1Ball ? Fruit1Ball->GetActorScale3D().X : 0.0f,
        NewBall ? NewBall->GetActorScale3D().X : 0.0f);
}

void UMergeAnimationState::DestroyOriginalFruits()
{
    if (AFruitBall* Fruit1Ball = Fruit1.Get())
    {
        Fruit1Ball->Destroy();
    }
    
    if (AFruitBall* Fruit2Ball = Fruit2.Get())
    {
        Fruit2Ball->Destroy();
    }
}

UWorld* UMergeAnimationState::GetWorld() const
{
    if (Fruit1.IsValid())
    {
        return Fruit1->GetWorld();
    }
    
    if (Fruit2.IsValid())
    {
        return Fruit2->GetWorld();
    }
    
    if (NewFruit.IsValid())
    {
        return NewFruit->GetWorld();
    }
    
    return nullptr;
}