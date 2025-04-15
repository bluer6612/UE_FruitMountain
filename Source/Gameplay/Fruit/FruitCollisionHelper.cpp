#include "FruitCollisionHelper.h"
#include "Actors/FruitBall.h"
#include "FruitMergeHelper.h"
#include "Components/PrimitiveComponent.h"

// 정적 배열 초기화
TArray<TWeakObjectPtr<AFruitBall>> UFruitCollisionHelper::RegisteredFruits;

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
    UE_LOG(LogTemp, Log, TEXT("과일 충돌 핸들러 등록 완료: %s"), *Fruit->GetName());
    
    // 등록된 과일 배열에 추가
    RegisteredFruits.Add(Fruit);
}

void UFruitCollisionHelper::ResetCollisionSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("충돌 시스템 초기화 시작 - 등록된 과일: %d개"), RegisteredFruits.Num());
    
    // 모든 등록된 핸들러 정리
    for (auto& FruitPtr : RegisteredFruits)
    {
        if (FruitPtr.IsValid())
        {
            AFruitBall* Fruit = FruitPtr.Get();
            if (Fruit && Fruit->GetMeshComponent())
            {
                Fruit->GetMeshComponent()->OnComponentHit.RemoveAll(Fruit);
                UE_LOG(LogTemp, Warning, TEXT("과일 충돌 핸들러 정리: %s"), *Fruit->GetName());
            }
        }
    }
    
    // 배열 비우기
    RegisteredFruits.Empty();
    
    // 추가 정리 작업이 필요하면 여기에 추가
    
    UE_LOG(LogTemp, Warning, TEXT("충돌 시스템 초기화 완료"));
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