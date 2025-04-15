#include "FruitCollisionHelper.h"
#include "Actors/FruitBall.h"
#include "FruitMergeHelper.h"
#include "Components/PrimitiveComponent.h"

void UFruitCollisionHelper::RegisterCollisionHandlers(AFruitBall* Fruit)
{
    if (!Fruit || !Fruit->GetMeshComponent())
    {
        UE_LOG(LogTemp, Error, TEXT("FruitCollisionHelper: 유효하지 않은 과일 또는 메시 컴포넌트"));
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

void UFruitCollisionHelper::HandleFruitCollision(AFruitBall* FruitA, AFruitBall* FruitB, const FHitResult& Hit)
{
    if (!FruitA || !FruitB)
    {
        UE_LOG(LogTemp, Error, TEXT("FruitCollisionHelper: 유효하지 않은 과일 참조"));
        return;
    }
    
    // 두 과일의 타입 비교
    int32 TypeA = FruitA->GetBallType();
    int32 TypeB = FruitB->GetBallType();
    
    // 병합 시도
    UFruitMergeHelper::TryMergeFruits(FruitA, FruitB, Hit.ImpactPoint);