#include "FruitMergeFeedbackHelper.h"
#include "Kismet/GameplayStatics.h"
#include "Actors/FruitBall.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"
#include "Actors/PlateActor.h"

// 모든 과일 안정화 처리
void UFruitMergeFeedbackHelper::StabilizeFruits(UWorld* World, float DampingMultiplier)
{
    if (!World)
    {
        return;
    }
    
    // 월드의 모든 과일 처리
    TArray<AActor*> FoundFruits;
    UGameplayStatics::GetAllActorsOfClass(World, AFruitBall::StaticClass(), FoundFruits);
    
    for (AActor* Actor : FoundFruits)
    {
        AFruitBall* Fruit = Cast<AFruitBall>(Actor);
        
        // 미리보기 공이나 이미 병합, 투척 중인 과일 제외
        if (Fruit->IsPreviewBall() || Fruit->IsMerging() || !Fruit->IsHasCollided())
        {
            continue;
        }
        
        // 각 과일 안정화 처리
        StabilizeSingleFruit(Fruit, DampingMultiplier);
    }
}

// 통합된 안정화 처리 로직 (매개변수 간소화)
// StabilizeSingleFruit 함수를 간소화합니다
void UFruitMergeFeedbackHelper::StabilizeSingleFruit(AFruitBall* Fruit, float DampingMultiplier)
{
    // 유효성 검사
    UStaticMeshComponent* MeshComp = Fruit->GetMeshComponent();
    UWorld* World = Fruit->GetWorld();
    if (!MeshComp || !World)
    {
        return;
    }

    // 1. 기본 물리 속성 안정화 - 감쇠 값 높임
    const float StabilizedAngularDamping = 2.0f * DampingMultiplier;
    
    MeshComp->SetAngularDamping(StabilizedAngularDamping);
    
    // 2. 타이머로 일정 시간 후 안정화 완화
    FTimerHandle StabilizeTimerHandle;
    World->GetTimerManager().SetTimer(StabilizeTimerHandle,
        [WeakFruit=TWeakObjectPtr<AFruitBall>(Fruit)]()
        {
            if (WeakFruit.IsValid() && WeakFruit->GetMeshComponent())
            {
                // 기본 감쇠 값으로 복원
                WeakFruit->GetMeshComponent()->SetAngularDamping(2.0f);
            }
        },
        1.0f, false);
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