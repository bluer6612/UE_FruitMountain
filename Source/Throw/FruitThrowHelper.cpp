#include "FruitThrowHelper.h"
#include "FruitPlayerController.h"
#include "FruitTrajectoryHelper.h"
#include "FruitPhysicsHelper.h"
#include "FruitSpawnHelper.h" // 새로운 헬퍼 클래스 추가
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "Camera/CameraComponent.h"
#include "Actors/PlateActor.h"

// 공 던지기 함수 수정
void UFruitThrowHelper::ThrowFruit(AFruitPlayerController* Controller)
{
    if (!Controller)
    {
        UE_LOG(LogTemp, Warning, TEXT("Controller가 유효하지 않습니다."));
        return;
    }
    
    // 미리보기 공이 있으면 먼저 제거
    if (Controller->PreviewBall)
    {
        Controller->PreviewBall->Destroy();
        Controller->PreviewBall = nullptr;
    }
    
    // 공통 함수 사용하여 스폰 위치 계산 - 새 클래스의 함수 호출
    FVector SpawnLocation = UFruitSpawnHelper::CalculatePlateEdgeSpawnPosition(
        Controller->GetWorld(), 50.f, Controller->CameraOrbitAngle);
    
    if (SpawnLocation == FVector::ZeroVector)
    {
        UE_LOG(LogTemp, Warning, TEXT("유효한 스폰 위치를 계산할 수 없습니다."));
        return;
    }
    
    // 공 스폰 후 물리 적용 - 새 클래스의 함수 호출
    AActor* SpawnedBall = UFruitSpawnHelper::SpawnBall(Controller, SpawnLocation, Controller->CurrentBallType, true);
    
    // 공 던지기 - 질량 처리 부분
    if (SpawnedBall)
    {
        UStaticMeshComponent* MeshComp = Cast<UStaticMeshComponent>(
            SpawnedBall->GetComponentByClass(UStaticMeshComponent::StaticClass()));
            
        if (MeshComp)
        {
            // 접시 위치 찾기
            FVector PlateCenter = FVector::ZeroVector;
            TArray<AActor*> PlateActors;
            UGameplayStatics::GetAllActorsWithTag(Controller->GetWorld(), FName("Plate"), PlateActors);
            if (PlateActors.Num() > 0)
            {
                PlateCenter = PlateActors[0]->GetActorLocation();
            }
            
            // 공의 질량 정보 가져오기 - 이미 SpawnBall에서 10.0f로 고정됨
            float BallMass = 10.0f; // 고정값 사용
            
            // 공통 함수 호출하여 던지기 파라미터 계산 (질량 정보 전달)
            float AdjustedForce;
            FVector LaunchDirection;
            UFruitPhysicsHelper::CalculateThrowParameters(
                Controller,
                SpawnLocation,
                PlateCenter,
                AdjustedForce,
                LaunchDirection,
                BallMass);
                
            // 최종 힘 적용 (물리 환경에 맞게 보정)
            float PhysicsCalibrationFactor = 20.0f;
            MeshComp->AddImpulse(LaunchDirection * (AdjustedForce * PhysicsCalibrationFactor));
            
            UE_LOG(LogTemp, Warning, TEXT("공 던지기: 최종 힘=%f, 질량=%f"),
                AdjustedForce * PhysicsCalibrationFactor, BallMass);
        }
    }
    
    // 다음 공 타입 랜덤 설정
    Controller->CurrentBallType = FMath::RandRange(1, 11);
    
    // 약간의 딜레이 후 새 미리보기 공 업데이트
    FTimerHandle UpdatePreviewTimerHandle;
    Controller->GetWorld()->GetTimerManager().SetTimer(
        UpdatePreviewTimerHandle,
        [Controller]()
        {
            Controller->UpdatePreviewBall();
        },
        0.5f,
        false
    );
}

// 미리보기 공 업데이트 함수 수정
void UFruitThrowHelper::UpdatePreviewBall(AFruitPlayerController* Controller)
{
    if (!Controller)
    {
        UE_LOG(LogTemp, Warning, TEXT("UpdatePreviewBall: Controller가 유효하지 않습니다."));
        return;
    }

    // 공통 함수를 사용하여 위치 계산 - 새 클래스의 함수 호출
    FVector PreviewLocation = UFruitSpawnHelper::CalculatePlateEdgeSpawnPosition(
        Controller->GetWorld(), 50.f, Controller->CameraOrbitAngle);
    
    if (PreviewLocation == FVector::ZeroVector)
    {
        UE_LOG(LogTemp, Error, TEXT("미리보기 실패: 유효한 위치를 계산할 수 없습니다!"));
        return;
    }
    
    // 미리보기 공이 없는 경우에만 새로 생성 - 새 클래스의 함수 호출
    if (!Controller->PreviewBall)
    {
        // 공 생성 (물리 비활성화)
        Controller->PreviewBall = UFruitSpawnHelper::SpawnBall(Controller, PreviewLocation, Controller->CurrentBallType, false);
        UE_LOG(LogTemp, Warning, TEXT("미리보기 공 새로 생성 - 위치: %s"), *PreviewLocation.ToString());
    }
    else
    {
        // 기존 공의 위치만 업데이트
        Controller->PreviewBall->SetActorLocation(PreviewLocation);
        
        // 필요한 경우 크기도 업데이트
        float BaseSize = 0.5f;
        float ScaleFactor = 1.f + 0.1f * (Controller->CurrentBallType - 1);
        float BallSize = BaseSize * ScaleFactor;
        Controller->PreviewBall->SetActorScale3D(FVector(BallSize));
        
        // 질량 처리 코드 추가
        UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Controller->PreviewBall->GetComponentByClass(UPrimitiveComponent::StaticClass()));
        if (PrimComp)
        {
            UStaticMeshComponent* MeshComp = Cast<UStaticMeshComponent>(PrimComp);
            if (MeshComp)
            {
                const float FixedMass = 10.0f; // 모든 공에 동일한 질량 적용
                MeshComp->SetMassOverrideInKg(NAME_None, FixedMass);
            }
        }
        
        UE_LOG(LogTemp, Warning, TEXT("미리보기 공 위치 업데이트 - 위치: %s"), *PreviewLocation.ToString());
    }
    
    if (Controller->PreviewBall)
    {
        // 접시 위치 찾기
        FVector PlateCenter = FVector::ZeroVector;
        TArray<AActor*> PlateActors;
        UGameplayStatics::GetAllActorsWithTag(Controller->GetWorld(), FName("Plate"), PlateActors);
        if (PlateActors.Num() > 0)
        {
            PlateCenter = PlateActors[0]->GetActorLocation();
        }
        
        // 예상 경로 업데이트
        UFruitTrajectoryHelper::UpdateTrajectoryPath(Controller, PreviewLocation, PlateCenter);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("미리보기 공 생성에 실패했습니다!"));
    }
}