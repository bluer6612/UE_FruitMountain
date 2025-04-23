#include "FruitMergeHelper.h"
#include "Actors/FruitBall.h"
#include "Gameplay/Fruit/FruitSpawnHelper.h"
#include "Kismet/GameplayStatics.h"
#include "Gameplay/Controller/FruitPlayerController.h"
#include "FruitMergeFeedbackHelper.h"
#include "Gameplay/Score/ScoreManagerComponent.h"
#include "Engine/StaticMesh.h"

void UFruitMergeHelper::RegisterCollisionHandlers(AFruitBall* Fruit)
{
    if (!Fruit || !Fruit->GetMeshComponent())
    {
        UE_LOG(LogTemp, Error, TEXT("UFruitMergeHelper: 유효하지 않은 과일 또는 메시 컴포넌트"));
        return;
    }

    // 미리보기 과일 체크 (추가 안전장치)
    if (Fruit->bIsPreviewBall)
    {
        return;
    }

    // 충돌 이벤트에 연결
    Fruit->GetMeshComponent()->OnComponentHit.AddDynamic(Fruit, &AFruitBall::OnBallHit);
    //UE_LOG(LogTemp, Log, TEXT("과일 충돌 핸들러 등록 완료: %s"), *Fruit->GetName());
}

void UFruitMergeHelper::ProcessFruitCollision(AFruitBall* FruitA, AFruitBall* FruitB, const FVector& CollisionPoint)
{
    // 유효성 검사
    if (!FruitA || !FruitB)
    {
        return;
    }
    
    // 1. 병합 제외 조건 확인 (빠른 실패)
    
    // 미리보기 공 체크
    if (FruitA->IsPreviewBall() || FruitB->IsPreviewBall())
    {
        return;
    }
    
    // 이미 병합 중인 과일 체크
    if (FruitA->IsMerging() || FruitB->IsMerging())
    {
        return;
    }
    
    // 타입이 다른 과일 체크
    int32 TypeA = FruitA->GetBallType();
    int32 TypeB = FruitB->GetBallType();
    if (TypeA != TypeB)
    {
        return;
    }
    
    // 2. 병합 실행 (모든 조건 통과)
    
    // 병합 상태 설정
    FruitA->SetMerging(true);
    FruitB->SetMerging(true);
    
    // 병합 실행
    MergeFruits(FruitA, FruitB, CollisionPoint);
}

void UFruitMergeHelper::MergeFruits(AFruitBall* Fruit1, AFruitBall* Fruit2, const FVector& ImpactPoint)
{
    if (!Fruit1 || !Fruit2)
    {
        return;
    }
    
    UWorld* World = Fruit1->GetWorld();
    if (!World)
    {
        return;
    }
    
    // 병합 위치 계산 (충돌 지점 또는 두 과일의 중간점)
    FVector MergeLocation = ImpactPoint;
    if (MergeLocation == FVector::ZeroVector)
    {
        MergeLocation = (Fruit1->GetActorLocation() + Fruit2->GetActorLocation()) * 0.5f;
    }
    
    int32 CurrentType = Fruit1->GetBallType();
    
    // 1. 병합 전 주변 과일 공간 확보 - 통합 함수 호출
    UFruitMergeFeedbackHelper::StabilizeFruits(
        World,             // 월드
        MergeLocation,     // 병합 위치
        3.0f,              // 기본 감쇠 계수
        nullptr,           // 대상 과일 없음 (병합 전)
        CurrentType + 1    // 새 과일 타입
    );
    
    // 약간의 딜레이 (0.1초) 후 실제 병합 진행
    FTimerHandle MergeTimerHandle;
    World->GetTimerManager().SetTimer(MergeTimerHandle, [=]()
    {
        // 1. 피드백 및 점수 처리
        UFruitMergeFeedbackHelper::PlayMergeEffect(World, MergeLocation, CurrentType);
        UScoreManagerComponent::AddScoreStatic(World, CurrentType);
        
        // 2. 주변 물리 안정화 (폭발적 충돌 방지)
        UFruitMergeFeedbackHelper::StabilizeFruits(
            World,             // 월드
            MergeLocation,     // 병합 위치
            5.0f,              // 높은 감쇠 계수 (병합 후)
            nullptr,           // 대상 과일 없음
            CurrentType        // 현재 과일 타입
        );
        
        // 3. 최대 레벨 확인
        if (CurrentType >= AFruitBall::MaxBallType)
        {
            // 최대 레벨이면 두 과일만 제거하고 종료
            Fruit1->Destroy();
            Fruit2->Destroy();
            return;
        }
        
        // 4. 새 과일 생성
        FRotator ExistingRotation = Fruit1->GetActorRotation();
        int32 NextType = CurrentType + 1;
        
        // 플레이어 컨트롤러를 통한 생성
        AFruitPlayerController* Controller = Cast<AFruitPlayerController>(UGameplayStatics::GetPlayerController(World, 0));
        if (Controller)
        {
            // 새 과일 스폰
            AActor* SpawnedActor = UFruitSpawnHelper::SpawnBall(Controller, MergeLocation, NextType, true);
            AFruitBall* NewFruit = Cast<AFruitBall>(SpawnedActor);
            
            // 생성된 과일에 자연스러운 움직임 적용
            if (NewFruit && NewFruit->GetMeshComponent())
            {
                // 기존 과일의 회전각 적용
                NewFruit->SetActorRotation(ExistingRotation);
                
                // 병합 후 안정화
                UFruitMergeFeedbackHelper::StabilizeFruits(
                    World,             // 월드
                    MergeLocation,     // 병합 위치
                    5.0f,              // 높은 감쇠 계수 (병합 후)
                    NewFruit,          // 생성된 새 과일
                    NextType           // 새 과일 타입
                );
            }
            
            UE_LOG(LogTemp, Warning, TEXT("새 과일 생성 완료: 레벨=%d, 위치=%s"), 
                   NextType, *MergeLocation.ToString());
        }
        
        // 기존 과일들 제거
        Fruit1->Destroy();
        Fruit2->Destroy();
        
    }, 0.1f, false);
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