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

void UFruitMergeHelper::TryMergeFruits(AFruitBall* FruitA, AFruitBall* FruitB, const FVector& CollisionPoint)
{
    if (!FruitA || !FruitB) {
        UE_LOG(LogTemp, Error, TEXT("TryMergeFruits: 과일 참조가 유효하지 않음"));
        return;
    }
    
    // 두 과일의 타입 가져오기
    int32 TypeA = FruitA->GetBallType();
    int32 TypeB = FruitB->GetBallType();
    
    // 타입이 서로 다르면 병합하지 않음
    if (TypeA != TypeB) {
        return; 
    }
    
    // 이미 병합 중인 과일이면 무시
    if (FruitA->IsMerging() || FruitB->IsMerging()) {
        UE_LOG(LogTemp, Warning, TEXT("이미 병합중인 과일이 있음"));
        return;
    }
    
    // 두 과일 모두 병합 상태로 설정
    FruitA->SetMerging(true);
    FruitB->SetMerging(true);
    
    // 병합 처리 수행
    MergeFruits(FruitA, FruitB, CollisionPoint);
}

void UFruitMergeHelper::MergeFruits(AFruitBall* FruitA, AFruitBall* FruitB, const FVector& MergeLocation)
{
    if (!FruitA || !FruitB) {
        UE_LOG(LogTemp, Error, TEXT("MergeFruits: 과일 참조가 유효하지 않음"));
        return;
    }
    
    // 두 과일의 타입 가져오기
    int32 TypeA = FruitA->GetBallType();
    int32 TypeB = FruitB->GetBallType();
    
    UE_LOG(LogTemp, Warning, TEXT("같은 타입의 과일 병합 시작: 타입=%d"), TypeA);
    
    UWorld* World = FruitA->GetWorld();
    if (!World) return;
    
    // 마지막 레벨 체크
    if (TypeA >= 10)
    {
        UE_LOG(LogTemp, Warning, TEXT("최대 레벨 과일 병합"));
        AddScore(TypeA * 2);
        PlayMergeEffect(World, MergeLocation, TypeA);
        
        FruitA->Destroy();
        FruitB->Destroy();
        return;
    }
    
    // 다음 레벨의 과일 생성
    int32 NextType = TypeA + 1;
    UE_LOG(LogTemp, Warning, TEXT("다음 레벨 과일 생성: %d -> %d"), TypeA, NextType);
    
    // 이펙트 및 점수 처리
    PlayMergeEffect(World, MergeLocation, TypeA);
    AddScore(TypeA);
    
    // 새 과일 생성
    AFruitPlayerController* PC = Cast<AFruitPlayerController>(UGameplayStatics::GetPlayerController(World, 0));
    if (PC)
    {
        UFruitSpawnHelper::SpawnBall(PC, MergeLocation, NextType, true);
        UE_LOG(LogTemp, Warning, TEXT("새 과일 생성 완료: 레벨=%d, 위치=%s"), 
               NextType, *MergeLocation.ToString());
    } else {
        UE_LOG(LogTemp, Error, TEXT("PlayerController를 찾을 수 없음"));
    }
    
    // 기존 과일들 제거
    FruitA->Destroy();
    FruitB->Destroy();
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