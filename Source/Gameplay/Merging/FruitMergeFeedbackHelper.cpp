#include "FruitMergeFeedbackHelper.h"
#include "Kismet/GameplayStatics.h"
#include "Actors/FruitBall.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"
#include "Actors/PlateActor.h"

// 통합된 안정화 함수
void UFruitMergeFeedbackHelper::StabilizeFruits(UWorld* World, const FVector& Center, float DampingMultiplier, AFruitBall* TargetFruit, int32 FruitType)
{
    if (!World)
    {
        return;
    }

    // 과일 타입에 따라 안정화 반경 계산
    float BaseFruitRadius = AFruitBall::CalculateBallSize(FruitType);
    
    // 과일 크기의 3배로 영향 반경 설정 (큰 과일은 더 넓은 영향 반경)
    float Radius = BaseFruitRadius * 3.0f;
    
    // 최소 반경 보장
    Radius = FMath::Max(Radius, 100.0f);
    
    // 대상 과일이 있는 경우 (병합 후 새 과일) 특별 처리
    if (TargetFruit && TargetFruit->GetMeshComponent())
    {
        UStaticMeshComponent* MeshComp = TargetFruit->GetMeshComponent();
        
        // 1. 새 과일에 감쇠값 적용
        MeshComp->SetAngularDamping(4.0f);
        MeshComp->SetLinearDamping(2.0f);
        
        // 2. 속도 초기화
        MeshComp->SetPhysicsLinearVelocity(FVector::ZeroVector);
        MeshComp->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
        
        // 3. 타이머로 감쇠값 복원
        ResetDamping(TargetFruit, 1.2f);
    }
    
    // 주변 과일 검색
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
        if (!Fruit || Fruit->IsMerging() || Fruit->IsPreviewBall())
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
            
            // 거리에 따른 효과 강도 계산 (가까울수록 강함)
            float DistanceFactor = FMath::Clamp(1.0f - Distance/Radius, 0.2f, 1.0f);
            
            // 대상 과일이 없을 경우 (병합 전) - 밀어내기 효과 적용
            if (TargetFruit == nullptr)
            {
                // 1. 중심에서 과일 방향으로 벡터 계산
                FVector PushDirection = (Fruit->GetActorLocation() - Center).GetSafeNormal();
                
                // 2. 거리에 따른 힘 계산 (가까울수록 강하게)
                float PushForce = FMath::Max(400.0f * DistanceFactor, 100.0f);
                
                // 3. 약간 위쪽으로 들어올리는 성분 추가 (쌓인 과일이 넘어지지 않도록)
                PushDirection.Z += 0.01f;
                PushDirection.Normalize();
                
                // 4. 부드러운 밀어내기 적용
                MeshComp->AddImpulse(PushDirection * PushForce);
            }
            
            // 공통: 감쇠 적용 및 복원 타이머 설정
            float AdjustedDamping = DampingMultiplier * DistanceFactor;
            MeshComp->SetAngularDamping(5.0f * AdjustedDamping);
            MeshComp->SetLinearDamping(1.5f * AdjustedDamping);
            
            // 타이머로 감쇠 설정 복원
            ResetDamping(Fruit, 1.0f);
        }
    }
}

// 내부 타이머 함수 - 감쇠 값 복원
void UFruitMergeFeedbackHelper::ResetDamping(TWeakObjectPtr<AFruitBall> Fruit, float Delay)
{
    if (!Fruit.IsValid() || !Fruit->GetWorld())
    {
        return;
    }
    
    FTimerHandle TimerHandle;
    Fruit->GetWorld()->GetTimerManager().SetTimer(
        TimerHandle,
        [WeakFruit=Fruit]()
        {
            if (WeakFruit.IsValid() && WeakFruit->GetMeshComponent())
            {
                // 기본 감쇠값 복원
                WeakFruit->GetMeshComponent()->SetAngularDamping(2.0f);
                WeakFruit->GetMeshComponent()->SetLinearDamping(0.5f);
            }
        },
        Delay,
        false
    );
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