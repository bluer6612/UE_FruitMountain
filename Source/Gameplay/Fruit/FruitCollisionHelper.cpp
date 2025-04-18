#include "FruitCollisionHelper.h"
#include "Actors/FruitBall.h"
#include "FruitMergeHelper.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

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
}

void UFruitCollisionHelper::HandleBallHit(AFruitBall* Fruit, UPrimitiveComponent* HitComponent, AActor* OtherActor, 
                     UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (!Fruit) return;

    // 충돌 경험 설정
    Fruit->SetHasCollided(true);
    
    // 접시와의 충돌인지 확인
    if (OtherActor && (OtherActor->ActorHasTag("Plate")))
    {
        // 접시 안정화 처리
        StabilizeOnPlate(Fruit, HitComponent);
        return;
    }

    // 다른 과일과의 충돌 처리
    AFruitBall* OtherFruit = Cast<AFruitBall>(OtherActor);
    if (OtherFruit)
    {
        HandleFruitCollision(Fruit, OtherFruit, Hit);
    }
}

void UFruitCollisionHelper::StabilizeOnPlate(AFruitBall* Fruit, UPrimitiveComponent* HitComponent)
{
    if (!Fruit || !HitComponent || !HitComponent->IsSimulatingPhysics())
        return;
    
    UWorld* World = Fruit->GetWorld();
    if (!World) return;
    
    // 즉시 감쇠를 매우 높게 설정하여 빠르게 에너지 손실
    HitComponent->SetAngularDamping(10.0f);
    HitComponent->SetLinearDamping(10.0f);
    
    // 잠시 후에 완전히 안정화 - 약한 참조 사용
    FTimerHandle StabilizeTimerHandle;
    World->GetTimerManager().SetTimer(StabilizeTimerHandle, 
        [WeakFruit=TWeakObjectPtr<AFruitBall>(Fruit), WeakMeshComp=TWeakObjectPtr<UPrimitiveComponent>(HitComponent)]() 
        {
            // 약한 포인터로 유효성 검사 (이미 소멸된 객체에 안전하게 접근)
            if (WeakFruit.IsValid() && WeakMeshComp.IsValid() && WeakMeshComp->IsSimulatingPhysics())
            {
                // 일반 감쇠 값 복원
                WeakMeshComp->SetAngularDamping(2.0f);
                WeakMeshComp->SetLinearDamping(2.0f);
            }
        }, 
        1.0f, false);
}