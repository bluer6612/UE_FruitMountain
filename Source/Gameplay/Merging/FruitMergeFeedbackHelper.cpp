#include "FruitMergeFeedbackHelper.h"
#include "Kismet/GameplayStatics.h"
#include "Actors/FruitBall.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"
#include "Actors/PlateActor.h"

void UFruitMergeFeedbackHelper::StabilizeFruits(UWorld* World, const FVector& Center, float DampingMultiplier, AFruitBall* TargetFruit, int32 FruitType)
{
    if (!World)
    {
        return;
    }

    // 과일 크기에 따라 안정화 반경 계산
    float Radius = AFruitBall::CalculateBallSize(FruitType) * 3.0f;
    Radius = FMath::Max(Radius, 100.0f);
    
    // 물리 속성 복원을 위해 필요한 정보 저장
    TArray<TWeakObjectPtr<AFruitBall>> FruitsToRestore;
    
    // 1. 대상 과일이 있는 경우 (병합 후 새 과일) 특별 처리
    if (TargetFruit && TargetFruit->GetMeshComponent() && !TargetFruit->IsThrowingInProgress())
    {
        UStaticMeshComponent* MeshComp = TargetFruit->GetMeshComponent();
        
        // 물리 시뮬레이션 활성화 보장
        if (!MeshComp->IsSimulatingPhysics())
        {
            MeshComp->SetSimulatePhysics(true);
        }
        
        FruitsToRestore.Add(TargetFruit);
        
        // 높은 감쇠값 적용
        MeshComp->SetAngularDamping(8.0f);
        MeshComp->SetLinearDamping(2.0f);
        
        // 속도 초기화
        MeshComp->SetPhysicsLinearVelocity(FVector::ZeroVector);
        MeshComp->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
    }
    
    // 2. 주변 과일 검색
    TArray<AActor*> NearbyFruits;
    UGameplayStatics::GetAllActorsOfClass(World, AFruitBall::StaticClass(), NearbyFruits);
    
    for (AActor* Actor : NearbyFruits)
    {
        // 대상 과일은 위에서 처리했으므로 제외
        if (Actor == TargetFruit)
        {
            continue;
        }
        
        AFruitBall* Fruit = Cast<AFruitBall>(Actor);
        if (!Fruit || Fruit->IsMerging() || Fruit->IsPreviewBall() || Fruit->IsThrowingInProgress())
        {
            continue;
        }
        
        float Distance = FVector::Dist(Fruit->GetActorLocation(), Center);
        
        // 작용 반경 내의 과일만 처리
        if (Distance < Radius && Distance > 1.0f)
        {
            UStaticMeshComponent* MeshComp = Fruit->GetMeshComponent();
            if (!MeshComp)
            {
                continue;
            }
            
            // 물리 시뮬레이션 활성화 보장
            if (!MeshComp->IsSimulatingPhysics())
            {
                MeshComp->SetSimulatePhysics(true);
            }

            FruitsToRestore.Add(Fruit);
            
            // 거리에 따른 요소 계산
            float DistanceFactor = FMath::Clamp(1.0f - Distance/Radius, 0.2f, 1.0f);

            // 매우 높은 감쇠값 적용하여 거의 움직이지 않게 함
            float AdjustedDamping = DampingMultiplier * DistanceFactor * 2.0f;
            MeshComp->SetAngularDamping(8.0f * AdjustedDamping); // 회전 감쇠 증가
            MeshComp->SetLinearDamping(4.0f * AdjustedDamping);  // 선형 감쇠 증가
            
            // 현재 속도 감소시켜 움직임 제한
            FVector CurrentVelocity = MeshComp->GetPhysicsLinearVelocity();
            MeshComp->SetPhysicsLinearVelocity(CurrentVelocity * 0.1f); // 속도 90% 감소
            
            // 회전 속도도 감소
            FVector CurrentAngularVelocity = MeshComp->GetPhysicsAngularVelocityInDegrees();
            MeshComp->SetPhysicsAngularVelocityInDegrees(CurrentAngularVelocity * 0.1f);
        }
    }
    
    // 감쇠값만 복원하는 타이머 설정 (질량 복원 코드 제거)
    if (FruitsToRestore.Num() > 0)
    {
        FTimerHandle RestoreTimerHandle;
        World->GetTimerManager().SetTimer(
            RestoreTimerHandle,
            [FruitsToRestore]()
            {
                for (const TWeakObjectPtr<AFruitBall>& WeakFruit : FruitsToRestore)
                {
                    if (WeakFruit.IsValid() && WeakFruit->GetMeshComponent())
                    {
                        UStaticMeshComponent* MeshComp = WeakFruit->GetMeshComponent();
                        
                        // 감쇠값 복원 (기본값으로)
                        MeshComp->SetAngularDamping(2.0f);
                        MeshComp->SetLinearDamping(0.5f);
                    }
                }
            },
            1.0f, // 1초 후 복원
            false
        );
    }
}

// 병합 이펙트 재생
void UFruitMergeFeedbackHelper::PlayMergeEffect(UWorld* World, const FVector& Location, int32 BallType)
{
    if (!World)
    {
        return;
    }
    
    // 1. 시각적 효과 (블루프린트 액터)
    static TSubclassOf<AActor> MergeEffectClass = nullptr;
    if (!MergeEffectClass)
    {
        // 블루프린트 액터 클래스 로드
        MergeEffectClass = LoadClass<AActor>(nullptr, TEXT("/Game/Particle/02_Blueprints/BP_Particle_Burst_Lvl_1.BP_Particle_Burst_Lvl_1_C"));
        
        if (!MergeEffectClass)
        {
            UE_LOG(LogTemp, Error, TEXT("병합 이펙트 클래스를 로드할 수 없습니다"));
            return;
        }
    }
    
    // 2. 이펙트 스폰
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
    AActor* SpawnedEffect = World->SpawnActor<AActor>(MergeEffectClass, Location, FRotator::ZeroRotator, SpawnParams);
    if (SpawnedEffect)
    {
        // 과일 타입에 따라 이펙트 스케일 조정
        float EffectScale = 1.0f + (BallType * 0.025f);
        SpawnedEffect->SetActorScale3D(FVector(EffectScale, EffectScale, EffectScale));
        
        // 자동 삭제
        SpawnedEffect->SetLifeSpan(1.5f);
    }
    
    // 3. 사운드 효과 재생
    static USoundBase* MergeSound = nullptr;
    if (!MergeSound)
    {
        MergeSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/Sounds/S_FruitMerge"));
    }
    
    if (MergeSound)
    {
        // 과일 크기에 따라 볼륨과 피치 조정
        float VolumeMultiplier = FMath::Min(1.5f, 0.7f + (BallType * 0.1f));
        float PitchMultiplier = FMath::Max(0.7f, 1.1f - (BallType * 0.05f)); // 큰 과일은 낮은 소리
        
        UGameplayStatics::PlaySoundAtLocation(
            World, 
            MergeSound, 
            Location, 
            VolumeMultiplier, 
            PitchMultiplier
        );
    }
}