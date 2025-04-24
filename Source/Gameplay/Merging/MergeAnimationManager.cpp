// MergeAnimationManager.cpp
#include "MergeAnimationManager.h"
#include "Actors/FruitBall.h"
#include "Gameplay/Score/ScoreManagerComponent.h"
#include "Gameplay/Fruit/FruitSpawnHelper.h"
#include "Gameplay/Controller/FruitPlayerController.h"
#include "Kismet/GameplayStatics.h"

UMergeAnimationManager::UMergeAnimationManager()
    : Fruit1(nullptr)
    , Fruit2(nullptr)
    , NewFruit(nullptr)
    , bIsAnimating(false)
    , bIsCompleted(false)
    , AnimationTime(0.0f)
    , TotalDuration(0.15f)
{
}

void UMergeAnimationManager::StartAnimation(AFruitBall* SourceFruit1, AFruitBall* SourceFruit2, const FVector& MergeLocation, int32 NextBallType)
{
    // 기본 유효성 검사
    if (!SourceFruit1 || !SourceFruit2 || !IsValid(SourceFruit1) || !IsValid(SourceFruit2))
    {
        return;
    }
    
    // 상태 초기화
    bIsAnimating = true;
    bIsCompleted = false;
    AnimationTime = 0.0f;
    
    // 대상 과일 설정
    Fruit1 = SourceFruit1;
    Fruit2 = SourceFruit2;
    MergePoint = MergeLocation;
    NextFruitType = NextBallType;
    
    // 초기 스케일 저장
    InitialScale1 = Fruit1->GetActorScale3D();
    InitialScale2 = Fruit2->GetActorScale3D();
    
    UE_LOG(LogTemp, Warning, TEXT("병합 애니메이션 시작: %s + %s (위치: %s)"), 
           *Fruit1->GetName(), *Fruit2->GetName(), *MergePoint.ToString());
    
    // 물리 비활성화
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
    
    // 새 과일 생성
    UWorld* World = Fruit1->GetWorld();
    if (World)
    {
        AFruitPlayerController* Controller = Cast<AFruitPlayerController>(
            UGameplayStatics::GetPlayerController(World, 0));
            
        if (Controller)
        {
            AActor* SpawnedActor = UFruitSpawnHelper::SpawnBall(
                Controller, MergePoint, NextFruitType, true);
            NewFruit = Cast<AFruitBall>(SpawnedActor);
            
            if (NewFruit)
            {
                // 초기 설정
                FRotator AvgRotation = (Fruit1->GetActorRotation() + Fruit2->GetActorRotation()) * 0.5f;
                NewFruit->SetActorRotation(AvgRotation);
                NewFruit->SetActorScale3D(FVector(0.05f));
                
                // 물리 비활성화
                if (NewFruit->GetMeshComponent())
                {
                    NewFruit->GetMeshComponent()->SetSimulatePhysics(false);
                    NewFruit->GetMeshComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                }
            }
        }
    }
}

void UMergeAnimationManager::UpdateAnimation(float DeltaTime)
{
    // 애니메이션이 활성화되지 않았거나 이미 완료된 경우
    if (!bIsAnimating || bIsCompleted)
    {
        return;
    }
    
    // 시간 업데이트
    AnimationTime += DeltaTime;
    float Progress = FMath::Clamp(AnimationTime / TotalDuration, 0.0f, 1.0f);
    
    // 애니메이션 업데이트 로직
    bool bFruit1Valid = IsValid(Fruit1);
    bool bFruit2Valid = IsValid(Fruit2);
    
    // 원본 과일 축소
    if (bFruit1Valid)
    {
        float ShrinkScale = 1.0f - Progress * 0.95f;
        Fruit1->SetActorScale3D(FVector(ShrinkScale * InitialScale1.X));
    }
    
    if (bFruit2Valid)
    {
        float ShrinkScale = 1.0f - Progress * 0.95f;
        Fruit2->SetActorScale3D(FVector(ShrinkScale * InitialScale2.X));
    }
    
    // 새 과일 성장
    if (IsValid(NewFruit))
    {
        float GrowScale = 0.05f + Progress * 0.95f;
        NewFruit->SetActorScale3D(FVector(GrowScale));
    }
    
    // 로그
    UE_LOG(LogTemp, Verbose, TEXT("병합 진행도: %.2f - 원본 과일 스케일: %.2f, 새 과일 스케일: %.2f"), 
        Progress, 
        bFruit1Valid ? Fruit1->GetActorScale3D().X : 0.0f,
        IsValid(NewFruit) ? NewFruit->GetActorScale3D().X : 0.0f);
    
    // 애니메이션 완료 체크
    if (Progress >= 1.0f && !bIsCompleted)
    {
        CompleteAnimation();
    }
}

void UMergeAnimationManager::CompleteAnimation()
{
    // 이미 완료된 경우 중복 실행 방지
    if (bIsCompleted)
    {
        return;
    }
    
    bIsCompleted = true;
    
    // 안전 검사
    bool bFruit1Valid = IsValid(Fruit1);
    bool bFruit2Valid = IsValid(Fruit2);
    
    UWorld* World = nullptr;
    if (bFruit1Valid) World = Fruit1->GetWorld();
    else if (bFruit2Valid) World = Fruit2->GetWorld();
    
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("병합 애니메이션 완료 실패: 유효한 월드 없음"));
        bIsAnimating = false;
        return;
    }
    
    // 원본 과일 제거
    if (bFruit1Valid) Fruit1->Destroy();
    if (bFruit2Valid) Fruit2->Destroy();
    
    // 점수 추가
    int32 CurrentType = 0;
    if (bFruit1Valid) CurrentType = Fruit1->GetBallType();
    else if (bFruit2Valid) CurrentType = Fruit2->GetBallType();
    
    UScoreManagerComponent::AddScoreStatic(World, CurrentType);
    
    // 새 과일 활성화
    if (IsValid(NewFruit) && NewFruit->GetMeshComponent())
    {
        NewFruit->GetMeshComponent()->SetSimulatePhysics(true);
        NewFruit->GetMeshComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        NewFruit->SetActorScale3D(FVector(1.0f));
    }
    
    UE_LOG(LogTemp, Display, TEXT("병합 애니메이션 완료"));
    
    // 디레퍼런스 후 애니메이션 비활성화 
    // (컨트롤러에서 시간차를 두고 정리하도록 함, 즉시 제거하지 않음)
    bIsAnimating = false;
}