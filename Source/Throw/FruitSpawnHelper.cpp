#include "FruitSpawnHelper.h"
#include "FruitPlayerController.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "Actors/PlateActor.h"

// 크기 계산 함수 구현
float UFruitSpawnHelper::CalculateBallSize(int32 BallType)
{
    float BaseSize = 0.5f;
    float ScaleFactor = 1.f + 0.1f * (BallType - 1);
    float BallSize = BaseSize * ScaleFactor;
    return BallSize;
}

// 질량 계산 함수 구현
float UFruitSpawnHelper::CalculateBallMass(int32 BallType)
{
    float BallSize = CalculateBallSize(BallType);
    float BallMass = BallSize * 20.0f; // 크기에 따라 질량 조정
    return BallMass;
}

// SpawnBall 함수 수정 - 물리 시뮬레이션 관련 수정
AActor* UFruitSpawnHelper::SpawnBall(AFruitPlayerController* Controller, const FVector& Location, int32 BallType, bool bEnablePhysics)
{
    if (!Controller || !Controller->FruitBallClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("SpawnBall: Controller 또는 FruitBallClass가 유효하지 않습니다."));
        return nullptr;
    }

    // 공 속성 계산 - 크기 및 질량
    float BallSize = CalculateBallSize(BallType);
    float BallMass = CalculateBallMass(BallType);

    // 공 액터 스폰
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    SpawnParams.Owner = Controller;
    
    AActor* SpawnedBall = Controller->GetWorld()->SpawnActor<AActor>(
        Controller->FruitBallClass, Location, FRotator::ZeroRotator, SpawnParams);
        
    if (SpawnedBall)
    {
        // 크기 설정
        SpawnedBall->SetActorScale3D(FVector(BallSize));
        
        // StaticMeshComponent 찾기
        UStaticMeshComponent* MeshComp = Cast<UStaticMeshComponent>(
            SpawnedBall->GetComponentByClass(UStaticMeshComponent::StaticClass()));
            
        if (MeshComp)
        {
            // 중요: 물리 시뮬레이션 활성화 후 질량 설정
            if (bEnablePhysics)
            {
                // 물리 시뮬레이션 활성화
                MeshComp->SetSimulatePhysics(true);
                MeshComp->SetEnableGravity(true);
                MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                MeshComp->SetCollisionProfileName(TEXT("PhysicsActor"));
                
                // 물리 시뮬레이션이 활성화된 후에만 질량 설정 가능
                MeshComp->SetMassOverrideInKg(NAME_None, BallMass);
                
                // UE_LOG(LogTemp, Warning, TEXT("던지는 공 생성: 타입=%d, 크기=%f, 질량=%f, 시뮬레이션=켜짐"),
                //     BallType, BallSize, BallMass);
            }
            else
            {
                // 미리보기용 공 설정 (물리 비활성화)
                MeshComp->SetSimulatePhysics(false);
                MeshComp->SetEnableGravity(false);
                MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                
                // 물리 시뮬레이션이 꺼져 있으면 질량을 설정할 수 없음
                // 대신 계산된 질량 값을 태그나 게임 데이터로 저장
                FString MassTag = FString::Printf(TEXT("BallMass:%f"), BallMass);
                SpawnedBall->Tags.Add(FName(*MassTag));
                
                // UE_LOG(LogTemp, Warning, TEXT("미리보기 공 생성: 타입=%d, 크기=%f, 계산된 질량=%f, 시뮬레이션=꺼짐"),
                //     BallType, BallSize, BallMass);
            }
        }
    }
    
    return SpawnedBall;
}

// 접시 가장자리 위치 계산 함수 구현
FVector UFruitSpawnHelper::CalculatePlateEdgeSpawnPosition(UWorld* World, float HeightOffset, float CameraAngle)
{
    // 접시 위치 확인
    FVector PlateCenter = FVector::ZeroVector;
    float PlateRadius = 0.0f;
    TArray<AActor*> PlateActors;
    UGameplayStatics::GetAllActorsWithTag(World, FName("Plate"), PlateActors);

    if (PlateActors.Num() > 0)
    {
        // 접시 정보 가져오기
        APlateActor* PlateActorRef = Cast<APlateActor>(PlateActors[0]);
        if (PlateActorRef)
        {
            PlateCenter = PlateActorRef->GetActorLocation();
            
            // 접시 반지름 계산
            FVector Bounds = PlateActorRef->GetComponentsBoundingBox().GetSize();
            PlateRadius = FMath::Max(Bounds.X, Bounds.Y) * 0.45f; // 가장자리에 더 가깝게 조정
            
            // 접시 높이 가져오기
            float PlateHeight = Bounds.Z;
            
            // 카메라 방향 벡터 계산
            float RadianAngle = FMath::DegreesToRadians(CameraAngle);
            FVector CameraDirection;
            CameraDirection.X = FMath::Cos(RadianAngle);
            CameraDirection.Y = FMath::Sin(RadianAngle);
            CameraDirection.Z = 0.0f;
            CameraDirection.Normalize();
            
            // 카메라 방향의 반대쪽 접시 가장자리 지점 계산 (카메라에서 가장 먼 곳)
            FVector EdgePoint = PlateCenter + CameraDirection * PlateRadius;
            
            // 높이 조정 - 접시 위로 HeightOffset만큼 더 높게
            EdgePoint.Z = PlateCenter.Z + PlateHeight + HeightOffset;
                
            return EdgePoint;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("접시 액터를 찾을 수 없습니다."));
    return FVector::ZeroVector;
}