#include "FruitThrowHelper.h"
#include "FruitTrajectoryHelper.h"
#include "FruitPhysicsHelper.h"
#include "Gameplay/Controller/FruitPlayerController.h"
#include "Gameplay/Create/FruitSpawnHelper.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "Camera/CameraComponent.h"
#include "Actors/PlateActor.h"
#include "Actors/FruitBall.h"

// ThrowFruit 함수 - FruitPhysicsHelper 활용
void UFruitThrowHelper::ThrowFruit(AFruitPlayerController* Controller)
{
    if (!Controller)
    {
        UE_LOG(LogTemp, Warning, TEXT("Controller가 유효하지 않습니다."));
        return;
    }
    
    // 미리보기 공이 있는지 확인하고 필요시 제거
    if (Controller->PreviewBall)
    {
        Controller->PreviewBall->Destroy();
        Controller->PreviewBall = nullptr;
    }
    
    // 공 스폰 위치 계산
    FVector SpawnLocation = UFruitSpawnHelper::CalculatePlateEdgeSpawnPosition(Controller->GetWorld(), Controller->CameraOrbitAngle);
    
    if (SpawnLocation == FVector::ZeroVector)
    {
        UE_LOG(LogTemp, Warning, TEXT("유효한 스폰 위치를 계산할 수 없습니다."));
        return;
    }
    
    // 공 스폰 후 물리 적용 - 즉시 표시되도록 설정
    AActor* SpawnedBall = UFruitSpawnHelper::SpawnBall(Controller, SpawnLocation, Controller->CurrentBallType, true);
    
    // 공 던지기 - 물리 헬퍼 활용하여 힘 적용
    if (SpawnedBall)
    {
        UStaticMeshComponent* MeshComp = Cast<UStaticMeshComponent>(
            SpawnedBall->GetComponentByClass(UStaticMeshComponent::StaticClass()));
            
        if (MeshComp)
        {
            // 물리 시뮬레이션 활성화 확인
            if (!MeshComp->IsSimulatingPhysics())
            {
                MeshComp->SetSimulatePhysics(true);
            }
            
            // 접시 위치 찾기
            FVector PlateCenter = FVector::ZeroVector;
            TArray<AActor*> PlateActors;
            UGameplayStatics::GetAllActorsWithTag(Controller->GetWorld(), FName("Plate"), PlateActors);
            if (PlateActors.Num() > 0)
            {
                PlateCenter = PlateActors[0]->GetActorLocation();
                
                // 접시 약간 위를 목표로 설정
                PlateCenter.Z += 10.0f;
            }
            
            // 물리 시뮬레이션 활성화 상태에서 질량 확인
            float ActualMass = MeshComp->GetMass();
            
            // 통합 물리 계산 사용
            FThrowPhysicsResult PhysicsResult = UFruitPhysicsHelper::CalculateThrowPhysics(
                Controller->GetWorld(), SpawnLocation, PlateCenter, Controller->ThrowAngle, ActualMass);
            
            // 물리적으로 더 정확한 초기화 방법
            MeshComp->SetPhysicsLinearVelocity(FVector::ZeroVector, false); // 현재 속도 초기화
            MeshComp->SetWorldLocation(SpawnLocation, false, nullptr, ETeleportType::TeleportPhysics);
            MeshComp->AddImpulse(PhysicsResult.LaunchDirection * PhysicsResult.InitialSpeed * ActualMass);
            
            UE_LOG(LogTemp, Warning, TEXT("공 던지기: 직접 속도 설정=(%s), 크기=%.1f"),
                *PhysicsResult.LaunchDirection.ToString(), PhysicsResult.InitialSpeed);
        }
    }
    
    // 다음 공 타입 랜덤 설정
    Controller->CurrentBallType = FMath::RandRange(1, AFruitBall::MaxBallType);
    
    // 약간의 딜레이 후 새 미리보기 공 업데이트
    FTimerHandle UpdatePreviewTimerHandle;
    Controller->GetWorld()->GetTimerManager().SetTimer(
        UpdatePreviewTimerHandle, 
        [Controller]() 
        { 
            Controller->UpdatePreviewBall(); 
        }, 
        Controller->BallThrowDelay, 
        false
    );
}

// UpdatePreviewBall 함수 수정 - 물리 헬퍼 사용
void UFruitThrowHelper::UpdatePreviewBall(AFruitPlayerController* Controller)
{
    if (!Controller)
    {
        UE_LOG(LogTemp, Warning, TEXT("UpdatePreviewBall: Controller가 유효하지 않습니다."));
        return;
    }

    // 던지기 중이면 미리보기 공을 생성하지 않음
    if (Controller->bIsThrowingInProgress)
    {
        UE_LOG(LogTemp, Verbose, TEXT("던지기 중이므로 미리보기 공 업데이트 건너뜀"));
        return;
    }

    // 공통 함수를 사용하여 위치 계산
    FVector PreviewLocation = UFruitSpawnHelper::CalculatePlateEdgeSpawnPosition(Controller->GetWorld(), Controller->CameraOrbitAngle);
    
    if (PreviewLocation == FVector::ZeroVector)
    {
        UE_LOG(LogTemp, Error, TEXT("미리보기 실패: 유효한 위치를 계산할 수 없습니다!"));
        return;
    }
    
    // 미리보기 공이 없는 경우에만 새로 생성
    if (!Controller->PreviewBall)
    {
        // 기존 미리보기 공 제거 (혹시 무효한 참조가 있을 경우를 대비)
        if (Controller->PreviewBall)
        {
            Controller->PreviewBall->Destroy();
            Controller->PreviewBall = nullptr;
        }
        
        // 새 미리보기 공 생성
        Controller->PreviewBall = UFruitSpawnHelper::SpawnBall(Controller, PreviewLocation, Controller->CurrentBallType, false);
    }
    else
    {
        // 기존 공의 위치만 업데이트
        Controller->PreviewBall->SetActorLocation(PreviewLocation);
        
        // 크기도 업데이트 - 공통 함수 사용
        float BallSize = UFruitSpawnHelper::CalculateBallSize(Controller->CurrentBallType);
        Controller->PreviewBall->SetActorScale3D(FVector(BallSize));
        
        // 질량 업데이트 - 공통 함수 사용
        UStaticMeshComponent* MeshComp = Cast<UStaticMeshComponent>(
            Controller->PreviewBall->GetComponentByClass(UStaticMeshComponent::StaticClass()));
            
        if (MeshComp)
        {
            float BallMass = UFruitSpawnHelper::CalculateBallMass(Controller->CurrentBallType);
            MeshComp->SetMassOverrideInKg(NAME_None, BallMass);
        }
    }
    
    // 접시 위치 찾기
    FVector PlateCenter = FVector::ZeroVector;
    TArray<AActor*> PlateActors;
    UGameplayStatics::GetAllActorsWithTag(Controller->GetWorld(), FName("Plate"), PlateActors);
    
    if (PlateActors.Num() > 0)
    {
        PlateCenter = PlateActors[0]->GetActorLocation();
        
        // 예상 경로 업데이트 (물리 헬퍼를 직접 사용하지 않고 래퍼 함수 호출)
        UFruitTrajectoryHelper::UpdateTrajectoryPath(Controller, PreviewLocation, PlateCenter);
    }
}