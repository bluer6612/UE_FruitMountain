#include "FruitSpawnHelper.h"
#include "Gameplay/Merging/FruitCollisionHelper.h"
#include "Gameplay/Controller/FruitPlayerController.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "Actors/PlateActor.h"
#include "Actors/FruitBall.h"

// SpawnBall 함수 수정 - 기존 코드 유지
AActor* UFruitSpawnHelper::SpawnBall(AFruitPlayerController* Controller, const FVector& Location, int32 BallType, bool bEnablePhysics)
{
    if (!Controller || !Controller->FruitBallClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("SpawnBall: Controller 또는 FruitBallClass가 유효하지 않습니다."));
        return nullptr;
    }

    // 공 속성 계산 - 크기 및 질량 (FruitBall 클래스 기반)
    float BallSize = AFruitBall::CalculateBallSize(BallType) / 100.0f; // 언리얼 액터 스케일 반환 (100으로 나눔)
    float BallMass = AFruitBall::CalculateBallMass(BallType);

    // 공 액터 스폰
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    SpawnParams.Owner = Controller;
    
    AActor* SpawnedBall = Controller->GetWorld()->SpawnActor<AActor>(
        Controller->FruitBallClass, Location, FRotator::ZeroRotator, SpawnParams);

    if (SpawnedBall)
    {
        // 크기 설정 - 모든 축에 동일한 스케일 적용
        SpawnedBall->SetActorScale3D(FVector(BallSize));
        
        // BallType 설정 및 미리보기 플래그 설정
        AFruitBall* FruitBall = Cast<AFruitBall>(SpawnedBall);
        if (FruitBall)
        {
            // SetBallType 함수를 사용하여 타입 설정 및 메시 업데이트
            FruitBall->SetBallType(BallType);
            
            // 중요: 미리보기 여부 명시적 설정 (물리가 활성화되지 않으면 미리보기 공)
            FruitBall->bIsPreviewBall = !bEnablePhysics;
                
            // 여기서 직접 충돌 핸들러 등록 (미리보기 공이 아닐 때만)
            if (!FruitBall->bIsPreviewBall)
            {
                UFruitCollisionHelper::RegisterCollisionHandlers(FruitBall);
            }
            else
            {
                FruitBall->DisplayDebugInfo();
            }
            
            // 질량 설정 - 실제 과일일 경우만
            if (!FruitBall->bIsPreviewBall)
            {
                UStaticMeshComponent* MeshComp = FruitBall->GetMeshComponent();
                if (MeshComp)
                {
                    float Mass = AFruitBall::CalculateBallMass(BallType);
                    MeshComp->SetMassOverrideInKg(NAME_None, Mass);
                }
            }
        }
        
        // 물리 활성화 설정
        UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(SpawnedBall->GetRootComponent());
        if (RootPrimitive)
        {
            RootPrimitive->SetSimulatePhysics(bEnablePhysics);
            RootPrimitive->SetEnableGravity(bEnablePhysics);
        }
    }
    
    return SpawnedBall;
}