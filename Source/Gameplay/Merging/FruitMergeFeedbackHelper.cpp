#include "FruitMergeFeedbackHelper.h"
#include "Kismet/GameplayStatics.h"
#include "Framework/UE_FruitMountainGameMode.h"
#include "Gameplay/Merging/ScoreManagerComponent.h"
#include "Actors/FruitBall.h"
#include "Components/StaticMeshComponent.h"
#include "Gameplay/Controller/FruitPlayerController.h"

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

// 통합된 과일 안정화 함수
void UFruitMergeFeedbackHelper::StabilizeFruits(UWorld* World, AFruitBall* SingleFruit, float DampingMultiplier, bool bIsNewFruit)
{
    if (!World)
    {
        return;
    }
    
    // 1. 단일 과일만 처리하는 경우
    // SingleFruit가 투척 중이면 안정화하지 않음
    if (SingleFruit && !SingleFruit->IsBeingThrown())
    {
        // 과일 안정화 로직 적용
        StabilizeSingleFruit(SingleFruit, DampingMultiplier, bIsNewFruit);
        return;
    }
    
    // 2. 월드의 모든 과일 처리
    TArray<AActor*> FoundFruits;
    UGameplayStatics::GetAllActorsOfClass(World, AFruitBall::StaticClass(), FoundFruits);
    
    for (AActor* Actor : FoundFruits)
    {
        AFruitBall* Fruit = Cast<AFruitBall>(Actor);
        if (!Fruit || !Fruit->GetMeshComponent())
        {
            continue;
        }
        
        // 미리보기 공이나 이미 병합, 투척 중인 과일 제외
        if (Fruit->IsPreviewBall() || Fruit->IsMerging() || Fruit->IsBeingThrown())
        {
            continue;
        }
        
        // 각 과일 안정화 처리
        StabilizeSingleFruit(Fruit, DampingMultiplier, false);
    }
}

// 단일 과일 안정화 작업을 수행하는 내부 함수
void UFruitMergeFeedbackHelper::StabilizeSingleFruit(AFruitBall* Fruit, float InitialDampingMultiplier, bool bIsNewFruit)
{
    if (!Fruit || !Fruit->GetMeshComponent())
    {
        return;
    }
    
    UStaticMeshComponent* MeshComp = Fruit->GetMeshComponent();
    UWorld* World = Fruit->GetWorld();
    if (!World)
    {
        return;
    }
    
    // !!! 중요: 투척 중인 과일은 안정화하지 않음 !!!
    if (Fruit->IsBeingThrown())
    {
        return;
    }
    
    // 1. 크기 인자 계산
    int32 FruitType = Fruit->GetBallType();
    float SizeFactor = FMath::Min(2.0f, 0.5f + (FruitType * 0.2f));
    
    // 2. 현재 위치 기반 중앙 방향 힘 계산
    FVector ToCenterXY = FVector::ZeroVector - Fruit->GetActorLocation();
    ToCenterXY.Z = 0;
    
    float DistanceToCenter = ToCenterXY.Size();
    float PlateRadius = 100.0f;
    
    // 3. 새 과일 또는 기존 과일에 따라 다르게 처리
    if (bIsNewFruit)
    {
        // 새로 생성된 과일에 대한 특별 처리
        // 물리 시뮬레이션과 충돌 함께 비활성화
        MeshComp->SetSimulatePhysics(false);
        MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        
        // 업워드 힘 + 중심 힘 적용을 위한 값 계산
        float UpwardForce = FMath::Max(-20.0f, -5.0f - (FruitType * 0.5f));
        
        // 중앙으로 향하는 힘 계산
        FVector CenteringForce = FVector::ZeroVector;
        if (DistanceToCenter > PlateRadius * 0.5f)
        {
            float CenteringStrength = FMath::Min(1.0f, DistanceToCenter / PlateRadius) * 10.0f;
            CenteringForce = ToCenterXY.GetSafeNormal() * CenteringStrength;
        }
        
        FVector FinalVelocity = FVector(CenteringForce.X, CenteringForce.Y, UpwardForce);
        
        // 0.1초 후 충돌 및 물리 시뮬레이션 재활성화
        FTimerHandle CollisionHandle;
        World->GetTimerManager().SetTimer(CollisionHandle, 
            [WeakFruit=TWeakObjectPtr<AFruitBall>(Fruit), FinalVelocity]() 
            {
                if (WeakFruit.IsValid() && WeakFruit->GetMeshComponent())
                {
                    UStaticMeshComponent* MeshComp = WeakFruit->GetMeshComponent();
                    
                    // 올바른 순서로 재활성화: 먼저 충돌 설정 후 물리 시뮬레이션 활성화
                    MeshComp->SetCollisionProfileName(TEXT("PhysicsActor"));
                    MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                    MeshComp->SetSimulatePhysics(true);
                    
                    // 이제 물리 속성 적용
                    MeshComp->SetPhysicsLinearVelocity(FinalVelocity);
                }
            }, 
            0.1f, false);
    }
    else
    {
        // 기존 과일 안정화 (물리 시뮬레이션은 유지)
        if (MeshComp->IsSimulatingPhysics())
        {
            FVector CurrentVel = MeshComp->GetPhysicsLinearVelocity();
            float ReductionFactor = 0.05f * SizeFactor;
            MeshComp->SetPhysicsLinearVelocity(CurrentVel * (1.0f - ReductionFactor));
            
            FVector AngVel = MeshComp->GetPhysicsAngularVelocityInDegrees();
            MeshComp->SetPhysicsAngularVelocityInDegrees(AngVel * (1.0f - ReductionFactor));
            
            // 중앙에서 멀리 있는 과일에 추가 힘 적용
            if (DistanceToCenter > PlateRadius * 0.4f)
            {
                float CenteringStrength = FMath::Min(1.0f, DistanceToCenter / PlateRadius) * 5.0f * SizeFactor;
                FVector StabilizingForce = ToCenterXY.GetSafeNormal() * CenteringStrength;
                MeshComp->AddForce(StabilizingForce, NAME_None, true);
            }
            
            // 4. 감쇠 설정
            MeshComp->SetLinearDamping(InitialDampingMultiplier * SizeFactor);
            MeshComp->SetAngularDamping(InitialDampingMultiplier * SizeFactor);
            
            // 5. 감쇠 복원 타이머
            FTimerHandle DampingTimerHandle;
            World->GetTimerManager().SetTimer(DampingTimerHandle, 
                [WeakFruit=TWeakObjectPtr<AFruitBall>(Fruit), SizeFactor]() 
                {
                    if (WeakFruit.IsValid() && WeakFruit->GetMeshComponent())
                    {
                        UStaticMeshComponent* MeshComp = WeakFruit->GetMeshComponent();
                        MeshComp->SetLinearDamping(2.0f * SizeFactor);
                        MeshComp->SetAngularDamping(2.0f * SizeFactor);
                    }
                }, 
                0.5f, false);
        }
    }
}