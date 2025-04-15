#include "FruitCollisionHelper.h"
#include "Actors/FruitBall.h"
#include "FruitMergeHelper.h"
#include "Components/PrimitiveComponent.h"

void UFruitCollisionHelper::RegisterCollisionHandlers(AFruitBall* Fruit)
{
    // 임시 디버그/테스트 목적으로 핸들러 등록 완전히 비활성화
    UE_LOG(LogTemp, Warning, TEXT("[테스트] 충돌 핸들러 등록 비활성화: %s"), 
        Fruit ? *Fruit->GetName() : TEXT("유효하지 않은 과일"));
    
    return; // 핸들러 등록 안함 - 일시적으로 비활성화
}

void UFruitCollisionHelper::HandleFruitCollision(AFruitBall* FruitA, AFruitBall* FruitB, const FHitResult& Hit)
{
    if (!FruitA || !FruitB)
    {
        UE_LOG(LogTemp, Error, TEXT("FruitCollisionHelper: 유효하지 않은 과일 참조"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("과일 충돌 처리: %s와 %s"), 
           *FruitA->GetName(), *FruitB->GetName());
    
    // 두 과일의 타입 비교
    int32 TypeA = FruitA->GetBallType();
    int32 TypeB = FruitB->GetBallType();
    
    UE_LOG(LogTemp, Warning, TEXT("과일 타입 비교: A=%d, B=%d"), TypeA, TypeB);
    
    // 병합 시도
    bool bMerged = UFruitMergeHelper::TryMergeFruits(FruitA, FruitB, Hit.ImpactPoint);
    
    if (bMerged)
    {
        UE_LOG(LogTemp, Warning, TEXT("과일 병합 성공"));
    }
}