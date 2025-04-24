#include "MergeController.h"
#include "Actors/FruitBall.h"
#include "FruitMergeHelper.h"
#include "MergeAnimator.h"
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

AMergeController* AMergeController::Get(const UObject* WorldContext)
{
    // 인스턴스 있으면 반환
    if (Instance)
    {
        return Instance;
    }
    
    // 없으면 월드에서 찾거나 생성
    UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
    if (World)
    {
        // 기존 액터 찾기
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(World, AMergeController::StaticClass(), FoundActors);
        
        if (FoundActors.Num() > 0)
        {
            Instance = Cast<AMergeController>(FoundActors[0]);
        }
        else
        {
            // 새로 생성
            FActorSpawnParameters SpawnParams;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            Instance = World->SpawnActor<AMergeController>(AMergeController::StaticClass(), 
                                                           FVector::ZeroVector, 
                                                           FRotator::ZeroRotator, 
                                                           SpawnParams);
        }
    }
    
    return Instance;
}

bool AMergeController::StartMerge(AFruitBall* Fruit1, AFruitBall* Fruit2, const FVector& CollisionPoint)
{
    // 이미 병합 중인지 확인
    if (bMergeInProgress || UMergeAnimator::IsGlobalMergeInProgress())
    {
        UE_LOG(LogTemp, Warning, TEXT("이미 병합이 진행 중입니다. 요청 무시"));
        return false;
    }
    
    // 유효성 검사 추가
    if (!IsValid(Fruit1) || !IsValid(Fruit2))
    {
        UE_LOG(LogTemp, Error, TEXT("병합 실패: 유효하지 않은 과일 객체"));
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

// IsMergeInProgress 호출 후 병합을 종료하는 함수 추가
void AMergeController::CompleteMerge()
{
    bMergeInProgress = false;
}

void AMergeController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // 추가 로직이 필요한 경우 여기에 구현
}