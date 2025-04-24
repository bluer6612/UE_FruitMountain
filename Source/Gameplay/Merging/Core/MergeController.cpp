#include "MergeController.h"
#include "Actors/FruitBall.h"
#include "FruitMergeHelper.h"
#include "Gameplay/Merging/Animation/MergeAnimator.h"
#include "Kismet/GameplayStatics.h"

AMergeController* AMergeController::Instance = nullptr;

AMergeController::AMergeController()
{
    PrimaryActorTick.bCanEverTick = true;
    bMergeInProgress = false;
}

void AMergeController::BeginPlay()
{
    Super::BeginPlay();
    Instance = this;
}

// 싱글톤 인스턴스 접근
AMergeController* AMergeController::Get(const UObject* WorldContext)
{
    if (!WorldContext)
    {
        return nullptr;
    }
    
    UWorld* World = WorldContext->GetWorld();
    if (!World)
    {
        return nullptr;
    }
    
    // 모든 MergeController 찾기
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, AMergeController::StaticClass(), FoundActors);
    
    if (FoundActors.Num() > 0)
    {
        return Cast<AMergeController>(FoundActors[0]);
    }
    else
    {
        // 필요하다면 생성 로직 추가
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        return World->SpawnActor<AMergeController>(AMergeController::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
    }
}

bool AMergeController::StartMerge(AFruitBall* Fruit1, AFruitBall* Fruit2, const FVector& CollisionPoint)
{
    // 이미 병합 중인지 확인
    if (bMergeInProgress || UMergeAnimator::IsGlobalMergeInProgress())
    {
        //UE_LOG(LogTemp, Warning, TEXT("이미 병합이 진행 중입니다. 요청 무시"));
        return false;
    }
    
    // 유효성 검사 추가
    if (!IsValid(Fruit1) || !IsValid(Fruit2))
    {
        //UE_LOG(LogTemp, Error, TEXT("병합 실패: 유효하지 않은 과일 객체"));
        return false;
    }
    
    // 같은 타입인지 확인
    if (Fruit1->GetBallType() != Fruit2->GetBallType())
    {
        return false;
    }
    
    // 병합 시작
    bMergeInProgress = true;
    
    // 기존 로직 재활용 (반환 값 없이 호출)
    UFruitMergeHelper::ProcessFruitCollision(Fruit1, Fruit2, CollisionPoint);
    
    // 병합 프로세스가 시작되었으므로 성공으로 간주
    return true;
}

bool AMergeController::IsMergeInProgress() const
{
    // 내부 상태와 UMergeAnimator 양쪽 모두 확인
    return bMergeInProgress || UMergeAnimator::IsGlobalMergeInProgress();
}

void AMergeController::SetMergeInProgress(bool bInProgress)
{
    bMergeInProgress = bInProgress;
}

// IsMergeInProgress 호출 후 병합을 종료하는 함수
void AMergeController::CompleteMerge()
{
    bMergeInProgress = false;
}

// 병합 이펙트 재생
void AMergeController::PlayMergeEffect(UWorld* World, const FVector& Location, int32 BallType)
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
    //static USoundBase* MergeSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/Sounds/S_FruitMerge"));
    
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