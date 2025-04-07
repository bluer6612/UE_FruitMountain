#include "FruitMergeHelper.h"
#include "Actors/FruitBall.h"
#include "FruitSpawnHelper.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Components/AudioComponent.h"
#include "Interface/UI/TextureDisplayWidget.h"
#include "Gameplay/Controller/FruitPlayerController.h"
#include "Framework/UE_FruitMountainGameMode.h"
#include "Components/StaticMeshComponent.h"

void UFruitMergeHelper::MergeFruits(AFruitBall* FruitA, AFruitBall* FruitB, const FVector& MergeLocation)
{
    if (!FruitA || !FruitB) return;
    
    UWorld* World = FruitA->GetWorld();
    if (!World) return;
    
    // 현재 과일 레벨 저장
    int32 CurrentType = FruitA->GetBallType();
    
    // 마지막 레벨 체크: 예를 들어 10이 최대 레벨이라면
    if (CurrentType >= 10)
    {
        // 최대 레벨 도달 시 처리 (예: 추가 점수)
        AddScore(CurrentType * 2); // 보너스 점수
        PlayMergeEffect(World, MergeLocation, CurrentType);
        
        // 두 과일 모두 제거
        FruitA->Destroy();
        FruitB->Destroy();
        return;
    }
    
    // 다음 레벨의 과일 계산
    int32 NextType = CurrentType + 1;
    
    // 병합 효과 재생
    PlayMergeEffect(World, MergeLocation, CurrentType);
    
    // 점수 추가
    AddScore(CurrentType);
    
    // 상위 레벨 과일 스폰
    AFruitPlayerController* PC = Cast<AFruitPlayerController>(UGameplayStatics::GetPlayerController(World, 0));
    if (PC)
    {
        // 기존 과일들의 물리 속성 (속도) 계산
        UStaticMeshComponent* MeshA = Cast<UStaticMeshComponent>(FruitA->GetComponentByClass(UStaticMeshComponent::StaticClass()));
        UStaticMeshComponent* MeshB = Cast<UStaticMeshComponent>(FruitB->GetComponentByClass(UStaticMeshComponent::StaticClass()));
        
        FVector CombinedVelocity = FVector::ZeroVector;
        if (MeshA && MeshB)
        {
            // 두 과일의 평균 속도 계산
            CombinedVelocity = (MeshA->GetPhysicsLinearVelocity() + MeshB->GetPhysicsLinearVelocity()) * 0.5f;
        }
        
        // 두 과일 제거
        FruitA->Destroy();
        FruitB->Destroy();
        
        // 새 과일 생성
        AActor* NewFruit = UFruitSpawnHelper::SpawnBall(PC, MergeLocation, NextType, true);
        if (NewFruit)
        {
            UStaticMeshComponent* NewMesh = Cast<UStaticMeshComponent>(NewFruit->GetComponentByClass(UStaticMeshComponent::StaticClass()));
            if (NewMesh)
            {
                // 이전 과일들의 평균 속도를 적용
                NewMesh->SetPhysicsLinearVelocity(CombinedVelocity * 0.8f); // 약간 감속
            }
        }
    }
}

void UFruitMergeHelper::AddScore(int32 BallType)
{
    // 레벨에 따른 점수 계산 (예: 레벨 * 10)
    int32 ScoreToAdd = BallType * 10;
    
    // 게임 모드나 UI를 통해 점수 업데이트
    UWorld* World = GEngine->GetWorld();
    if (World)
    {
        AGameModeBase* GameMode = World->GetAuthGameMode();
        AUE_FruitMountainGameMode* FruitGameMode = Cast<AUE_FruitMountainGameMode>(GameMode);
        if (FruitGameMode)
        {
            // 게임 모드에 점수 추가 함수 호출 (구현 필요)
            // FruitGameMode->AddScore(ScoreToAdd);
            
            // UI 업데이트
            UE_LOG(LogTemp, Warning, TEXT("과일 병합 점수 추가: %d (레벨 %d)"), ScoreToAdd, BallType);
        }
    }
}

void UFruitMergeHelper::PlayMergeEffect(UWorld* World, const FVector& Location, int32 BallType)
{
    if (!World) return;
    
    // 1. 시각적 효과 (파티클)
    static UParticleSystem* MergeParticle = nullptr;
    if (!MergeParticle)
    {
        // 파티클 에셋 로드 (게임에 맞는 경로로 수정 필요)
        MergeParticle = LoadObject<UParticleSystem>(nullptr, TEXT("/Game/Effects/P_FruitMerge"));
    }
    
    if (MergeParticle)
    {
        // 파티클 크기를 과일 레벨에 따라 조정
        float ParticleScale = 1.0f + (BallType * 0.2f);
        UGameplayStatics::SpawnEmitterAtLocation(
            World, MergeParticle, Location, FRotator::ZeroRotator, 
            FVector(ParticleScale), true, EPSCPoolMethod::AutoRelease);
    }
    
    // 2. 소리 효과
    static USoundBase* MergeSound = nullptr;
    if (!MergeSound)
    {
        // 사운드 에셋 로드 (게임에 맞는 경로로 수정 필요)
        MergeSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/Sounds/S_FruitMerge"));
    }
    
    if (MergeSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            World, MergeSound, Location, 
            1.0f + (BallType * 0.1f),  // 볼륨 (레벨이 높을수록 더 큰 소리)
            0.8f + (BallType * 0.05f)  // 피치 (레벨이 높을수록 더 높은 소리)
        );
    }
}