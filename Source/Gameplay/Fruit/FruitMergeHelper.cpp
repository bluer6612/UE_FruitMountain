#include "FruitMergeHelper.h"
#include "Actors/FruitBall.h"
#include "FruitSpawnHelper.h"
#include "Kismet/GameplayStatics.h"
#include "Gameplay/Controller/FruitPlayerController.h"
#include "FruitUtilityHelper.h"

void UFruitMergeHelper::TryMergeFruits(AFruitBall* FruitA, AFruitBall* FruitB, const FVector& CollisionPoint)
{
    if (!FruitA || !FruitB) {
        UE_LOG(LogTemp, Error, TEXT("TryMergeFruits: 과일 참조가 유효하지 않음"));
        return;
    }
    
    // 미리보기 공 체크 추가 - 둘 중 하나라도 미리보기 공이면 병합하지 않음
    if (FruitA->IsPreviewBall() || FruitB->IsPreviewBall()) {
        UE_LOG(LogTemp, Verbose, TEXT("미리보기 공과의 충돌 무시"));
        return;
    }
    
    // 두 과일의 타입 가져오기
    int32 TypeA = FruitA->GetBallType();
    int32 TypeB = FruitB->GetBallType();
    
    // 타입이 서로 다르면 병합하지 않음
    if (TypeA != TypeB) {
        return; 
    }
    
    // 이미 병합 중인 과일이면 무시
    if (FruitA->IsMerging() || FruitB->IsMerging()) {
        UE_LOG(LogTemp, Warning, TEXT("이미 병합중인 과일이 있음"));
        return;
    }
    
    // 두 과일 모두 병합 상태로 설정
    FruitA->SetMerging(true);
    FruitB->SetMerging(true);
    
    // 병합 처리 수행
    MergeFruits(FruitA, FruitB, CollisionPoint);
}

void UFruitMergeHelper::MergeFruits(AFruitBall* FruitA, AFruitBall* FruitB, const FVector& MergeLocation)
{
    if (!FruitA || !FruitB) {
        UE_LOG(LogTemp, Error, TEXT("MergeFruits: 과일 참조가 유효하지 않음"));
        return;
    }
    
    // 두 과일의 타입 가져오기
    int32 TypeA = FruitA->GetBallType();
    int32 TypeB = FruitB->GetBallType();
    
    UWorld* World = FruitA->GetWorld();
    if (!World) return;
    
    // 마지막 레벨 체크
    if (TypeA >= AFruitBall::MaxBallType)
    {
        UE_LOG(LogTemp, Warning, TEXT("병합 완료: 최대 레벨 과일 병합"));
        UFruitUtilityHelper::AddScore(World, TypeA);
        UFruitUtilityHelper::PlayMergeEffect(World, MergeLocation, TypeA);
        
        FruitA->Destroy();
        FruitB->Destroy();
        return;
    }
    
    // 다음 레벨의 과일 생성
    int32 NextType = TypeA + 1;
    
    // 이펙트 및 점수 처리
    UFruitUtilityHelper::PlayMergeEffect(World, MergeLocation, TypeA);
    UFruitUtilityHelper::AddScore(World, NextType);
    
    // 병합 위치 주변 과일들의 속도 감소 (폭발적 충돌 방지)
    UFruitUtilityHelper::StabilizeFruits(World);
    
    // 새 과일 생성 전에 기존 과일의 회전값 저장
    FRotator ExistingRotation = FruitA->GetActorRotation();
    
    // 새 과일 생성
    AFruitPlayerController* Controller = Cast<AFruitPlayerController>(UGameplayStatics::GetPlayerController(World, 0));
    if (Controller)
    {
        // 정확히 병합 위치에 생성
        AActor* SpawnedActor = UFruitSpawnHelper::SpawnBall(Controller, MergeLocation, NextType, true);
        AFruitBall* NewFruit = Cast<AFruitBall>(SpawnedActor);
        
        // 생성된 과일에 자연스러운 움직임 적용
        if (NewFruit && NewFruit->GetMeshComponent())
        {
            // 기존 과일의 회전각 적용
            NewFruit->SetActorRotation(ExistingRotation);
            
            // 새 과일 물리 속성 설정
            UFruitUtilityHelper::StabilizeFruits(World, NewFruit, 8.0f, true);
        }
        
        UE_LOG(LogTemp, Warning, TEXT("새 과일 생성 완료: 레벨=%d, 위치=%s"), 
               NextType, *MergeLocation.ToString());
    }
    
    // 기존 과일들 제거
    FruitA->Destroy();
    FruitB->Destroy();
}