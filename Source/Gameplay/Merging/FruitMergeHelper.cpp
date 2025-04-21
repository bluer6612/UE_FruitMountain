#include "FruitMergeHelper.h"
#include "Actors/FruitBall.h"
#include "Gameplay/Fruit/FruitSpawnHelper.h"
#include "Kismet/GameplayStatics.h"
#include "Gameplay/Controller/FruitPlayerController.h"
#include "FruitMergeFeedbackHelper.h"
#include "ScoreManagerComponent.h" // 이 헤더 파일 추가

void UFruitMergeHelper::TryMergeFruits(AFruitBall* FruitA, AFruitBall* FruitB, const FVector& CollisionPoint)
{
    if (!FruitA || !FruitB) {
        UE_LOG(LogTemp, Error, TEXT("TryMergeFruits: 과일 참조가 유효하지 않음"));
        return;
    }
    
    // 미리보기 공 체크 추가 - 둘 중 하나라도 미리보기 공이면 병합하지 않음
    if (FruitA->IsPreviewBall() || FruitB->IsPreviewBall()) {
        UE_LOG(LogTemp, Verbose, TEXT("미리보기 공과의 충돌 무시"));
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
    
    UWorld* World = FruitA->GetWorld();
    if (!World) return;
    
    // 이펙트 및 점수 처리
    UFruitMergeFeedbackHelper::PlayMergeEffect(World, MergeLocation, TypeA);
    UScoreManagerComponent::AddScoreStatic(World, TypeA);
    
    // 병합 위치 주변 과일들의 속도 감소 (폭발적 충돌 방지)
    UFruitMergeFeedbackHelper::StabilizeFruits(World);
    
    // 마지막 레벨 체크
    if (TypeA >= AFruitBall::MaxBallType)
    {
        UE_LOG(LogTemp, Warning, TEXT("병합 완료: 최대 레벨 과일 병합"));
        
        FruitA->Destroy();
        FruitB->Destroy();
        return;
    }
    
    // 새 과일 생성 전에 기존 과일의 회전값 저장
    FRotator ExistingRotation = FruitA->GetActorRotation();
    
    // 새 과일 생성
    AFruitPlayerController* Controller = Cast<AFruitPlayerController>(UGameplayStatics::GetPlayerController(World, 0));
    if (Controller)
    {
        // 다음 레벨의 과일 생성
        int32 NextType = TypeA + 1;

        // 정확히 병합 위치에 생성
        AActor* SpawnedActor = UFruitSpawnHelper::SpawnBall(Controller, MergeLocation, NextType, true);
        AFruitBall* NewFruit = Cast<AFruitBall>(SpawnedActor);
        
        // 생성된 과일에 자연스러운 움직임 적용
        if (NewFruit && NewFruit->GetMeshComponent())
        {
            // 기존 과일의 회전각 적용
            NewFruit->SetActorRotation(ExistingRotation);
            
            // 새 과일 물리 속성 설정
            UFruitMergeFeedbackHelper::StabilizeFruits(World, NewFruit, 8.0f, true);
        }
        
        UE_LOG(LogTemp, Warning, TEXT("새 과일 생성 완료: 레벨=%d, 위치=%s"), 
               NextType, *MergeLocation.ToString());
    }
    
    // 기존 과일들 제거
    FruitA->Destroy();
    FruitB->Destroy();
}

// 모든 메시 사전 로드
void UFruitMergeHelper::PreloadAllFruitMeshes(UWorld* World)
{
    UE_LOG(LogTemp, Display, TEXT("게임 에셋 사전 로드 시작..."));
    
    // 1. 모든 과일 메시 미리 로드 (최대 레벨까지)
    for (int32 i = 1; i <= AFruitBall::MaxBallType; i++)
    {
        // 메시 경로 - 게임의 실제 경로와 일치하게 수정
        FString MeshPath = FString::Printf(TEXT("/Game/Fruit/Meshes/Fruit%d.Fruit%d"), i, i);
        
        // 동기적 로딩 사용
        UStaticMesh* FruitMesh = LoadObject<UStaticMesh>(nullptr, *MeshPath);
        
        if (FruitMesh)
        {
            // 메시가 완전히 로드되도록 보장
            FruitMesh->ConditionalPostLoad();
            UE_LOG(LogTemp, Warning, TEXT("과일 메시 #%d 사전 로드 완료: %s"), i, *MeshPath);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("과일 메시 #%d 로드 실패: %s"), i, *MeshPath);
        }
    }
    
    // 2. 파티클 효과 미리 로드
    if (World)
    {
        TSubclassOf<AActor> PreloadParticleClass = LoadClass<AActor>(nullptr, TEXT("/Game/Particle/02_Blueprints/BP_Particle_Burst_Lvl_1.BP_Particle_Burst_Lvl_1_C"));
        if (PreloadParticleClass)
        {
            // 보이지 않는 위치에 미리 인스턴스 생성 후 즉시 제거 (렌더링 캐시 준비)
            FVector HiddenLocation = FVector(0, 0, -10000);
            AActor* PreloadActor = World->SpawnActor<AActor>(PreloadParticleClass, HiddenLocation, FRotator::ZeroRotator);
            if (PreloadActor)
            {
                PreloadActor->SetActorHiddenInGame(true);
                PreloadActor->Destroy();
            }
        }
    }
    
    // 3. 사운드 미리 로드
    USoundBase* PreloadSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/Sounds/S_FruitMerge"));
    if (PreloadSound)
    {
        UE_LOG(LogTemp, Display, TEXT("병합 사운드 미리 로드 완료"));
    }
    
    UE_LOG(LogTemp, Display, TEXT("게임 에셋 사전 로드 완료"));
}