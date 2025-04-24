#include "FruitMergeHelper.h"
#include "Actors/FruitBall.h"
#include "Gameplay/Fruit/FruitSpawnHelper.h"
#include "Kismet/GameplayStatics.h"
#include "Gameplay/Controller/FruitPlayerController.h"
#include "FruitMergeFeedbackHelper.h"
#include "Gameplay/Score/ScoreManagerComponent.h"
#include "Engine/StaticMesh.h"
#include "MergeAnimator.h"

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
    // 통합된 유효성 검사 (ValidateMergeConditions 내용을 인라인으로 포함)
    
    // 1. 기본 유효성 검사
    if (!FruitA || !FruitB || !IsValid(FruitA) || !IsValid(FruitB))
    {
        UE_LOG(LogTemp, Warning, TEXT("병합 실패: 유효하지 않은 과일 객체"));
        return;
    }

    // 2. 전역 병합 상태 검사
    if (UMergeAnimator::IsGlobalMergeInProgress())
    {
        UE_LOG(LogTemp, Warning, TEXT("병합 무시: 다른 병합이 이미 진행 중 (%s, %s)"),
               *FruitA->GetName(), *FruitB->GetName());
        return;
    }
    
    // 3. 미리보기 공 검사
    if (FruitA->IsPreviewBall() || FruitB->IsPreviewBall())
    {
        UE_LOG(LogTemp, Verbose, TEXT("병합 무시: 미리보기 공"));
        return;
    }
    
    // 4. 병합 중인 과일 검사
    if (FruitA->IsMerging() || FruitB->IsMerging())
    {
        UE_LOG(LogTemp, Verbose, TEXT("병합 무시: 이미 병합 중인 과일"));
        return;
    }
    
    // 5. 과일 타입 일치 검사
    int32 TypeA = FruitA->GetBallType();
    int32 TypeB = FruitB->GetBallType();
    if (TypeA != TypeB)
    {
        return;
    }
    
    // 6. 스케일 유효성 검사
    if (FruitA->GetActorScale3D().IsNearlyZero() || FruitB->GetActorScale3D().IsNearlyZero())
    {
        UE_LOG(LogTemp, Warning, TEXT("병합 실패: 과일 스케일이 0에 가까움"));
        return;
    }
    
    // 모든 검사 통과 - 병합 실행
    UE_LOG(LogTemp, Warning, TEXT("병합 시작: %s(타입:%d) + %s(타입:%d) -> 타입:%d"),
           *FruitA->GetName(), TypeA, *FruitB->GetName(), TypeB, TypeA + 1);
           
    // 병합 처리 함수 호출
    MergeFruits(FruitA, FruitB, CollisionPoint);
}

void UFruitMergeHelper::MergeFruits(AFruitBall* Fruit1, AFruitBall* Fruit2, const FVector& ImpactPoint)
{
    // 병합 플래그 설정 (유효성 검사는 이미 ProcessFruitCollision에서 완료됨)
    Fruit1->SetIsMerging(true);
    Fruit2->SetIsMerging(true);
    
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
    int32 NextType = CurrentType + 1;
    
    // 1. 병합 전 주변 과일 공간 확보
    UFruitMergeFeedbackHelper::StabilizeFruits(
        World,
        MergeLocation,
        3.0f,
        nullptr,
        NextType
    );
    
    // 애니메이션 시작
    UMergeAnimator::AnimateMerge(Fruit1, Fruit2, MergeLocation, NextType, 0.15f);
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