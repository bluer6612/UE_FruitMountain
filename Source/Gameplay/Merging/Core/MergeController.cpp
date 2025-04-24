#include "MergeController.h"
#include "Actors/FruitBall.h"
#include "FruitMergeStabilizer.h"
#include "Gameplay/Merging/Animation/MergeAnimationState.h" 
#include "Kismet/GameplayStatics.h"

AMergeController* AMergeController::Instance = nullptr;

AMergeController::AMergeController()
{
    PrimaryActorTick.bCanEverTick = true;
    bMergeInProgress = false;
    
    // 레벨 변경 시 이벤트 등록
    FCoreUObjectDelegates::PostLoadMapWithWorld.AddStatic(&AMergeController::HandleLevelChange);
}

void AMergeController::BeginPlay()
{
    Super::BeginPlay();
    Instance = this;
}

// 싱글톤 접근자
AMergeController* AMergeController::Get(const UObject* WorldContextObject)
{
    // 이미 인스턴스가 있다면 반환
    if (Instance && IsValid(Instance))
    {
        return Instance;
    }
    
    // 없다면 월드 컨텍스트로 찾거나 생성
    if (!WorldContextObject)
    {
        UE_LOG(LogTemp, Warning, TEXT("MergeController 싱글톤 접근 실패: 유효하지 않은 WorldContextObject"));
        return nullptr;
    }
    
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("MergeController 싱글톤 접근 실패: 유효한 World를 얻을 수 없음"));
        return nullptr;
    }
    
    // 인스턴스를 찾지 못했으면 새로 생성하는 코드 추가
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    Instance = World->SpawnActor<AMergeController>(AMergeController::StaticClass(), 
                                                  FVector::ZeroVector, 
                                                  FRotator::ZeroRotator, 
                                                  SpawnParams);
    
    return Instance;
}

void AMergeController::HandleLevelChange(UWorld* World)
{
    // 레벨 변경 시 싱글톤 인스턴스 초기화
    Instance = nullptr;
}

void AMergeController::StartMerge(AFruitBall* FruitA, AFruitBall* FruitB, const FVector& CollisionPoint)
{
    // 1. 기본 유효성 검사
    if (!FruitA || !FruitB || !IsValid(FruitA) || !IsValid(FruitB))
    {
        return;
    }

    // 2. 미리보기 공 검사
    if (FruitA->IsPreviewBall() || FruitB->IsPreviewBall())
    {
        return;
    }
    
    // 3. 병합 중인 과일 검사
    if (FruitA->IsMerging() || FruitB->IsMerging())
    {
        return;
    }
    
    // 4. 스케일 유효성 검사
    if (FruitA->GetActorScale3D().IsNearlyZero() || FruitB->GetActorScale3D().IsNearlyZero())
    {
        return;
    }
    
    // 모든 검사 통과 - 병합 실행
    MergeFruits(FruitA, FruitB, CollisionPoint);
}

void AMergeController::MergeFruits(AFruitBall* Fruit1, AFruitBall* Fruit2, const FVector& ImpactPoint)
{
    // 병합 위치 계산
    FVector MergeLocation = ImpactPoint;
    if (MergeLocation == FVector::ZeroVector)
    {
        MergeLocation = (Fruit1->GetActorLocation() + Fruit2->GetActorLocation()) * 0.5f;
    }
    
    // 다음 과일 타입 계산 (1단계 업그레이드)
    int32 CurrentType = Fruit1->GetBallType();
    int32 NextType = CurrentType + 1;
    
    UWorld* World = Fruit1->GetWorld();
    if (!World)
    {
        return;
    }
    
    // MergeController 가져오기
    AMergeController* MergeController = AMergeController::Get(World);
    if (!MergeController)
    {
        return;
    }
    
    // 병합 상태 설정
    MergeController->SetMergeInProgress(true);
    Fruit1->SetIsMerging(true);
    Fruit2->SetIsMerging(true);

    // 주변 과일 안정화 
    UFruitMergeStabilizer::StabilizeFruits(World, MergeLocation, 3.0f, nullptr, NextType);
    
    // 병합 애니메이션 시작
    MergeController->AnimateMerge(Fruit1, Fruit2, MergeLocation, NextType);
}

FTimerHandle AMergeController::AnimateMerge(AFruitBall* Fruit1, AFruitBall* Fruit2, const FVector& MergeLocation, int32 NextBallType)
{
    // 기본 유효성 검사
    if (!Fruit1 || !Fruit2 || !IsValid(Fruit1) || !IsValid(Fruit2))
    {
        bMergeInProgress = false;
        return FTimerHandle();
    }
    
    // 매번 새 UMergeAnimationState 객체 생성 (풀링 없이 원래 방식대로)
    UMergeAnimationState* AnimState = NewObject<UMergeAnimationState>();
    
    // GC에서 보호하기 위해 Root에 추가
    AnimState->AddToRoot();
    
    // 애니메이션 상태 초기화 및 시작
    AnimState->Initialize(Fruit1, Fruit2, MergeLocation, NextBallType);
    
    return AnimState->GetAnimTimerHandle();
}