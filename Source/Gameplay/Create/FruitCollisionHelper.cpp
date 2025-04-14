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

    // 이미 등록되었는지 확인
    if (Fruit->IsCollisionHandlerRegistered())
    {
        UE_LOG(LogTemp, Log, TEXT("과일 충돌 핸들러 이미 등록됨: %s"), *Fruit->GetName());
        return;
    }

    // 혹시 모를 기존 바인딩 제거 (안전 조치)
    Fruit->GetMeshComponent()->OnComponentHit.RemoveDynamic(Fruit, &AFruitBall::OnBallHit);
    
    // 충돌 이벤트에 연결
    Fruit->GetMeshComponent()->OnComponentHit.AddDynamic(Fruit, &AFruitBall::OnBallHit);
    
    // 등록 상태 설정
    Fruit->SetCollisionHandlerRegistered(true);
    
    UE_LOG(LogTemp, Log, TEXT("과일 충돌 핸들러 등록 완료: %s"), *Fruit->GetName());
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