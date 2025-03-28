#include "FruitThrowHelper.h"
#include "FruitPlayerController.h"
#include "FruitTrajectoryHelper.h" // 새 헬퍼 클래스 include
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "Camera/CameraComponent.h"
#include "Actors/PlateActor.h"

// 공통 함수로 볼 생성 로직 통합
AActor* UFruitThrowHelper::SpawnBall(AFruitPlayerController* Controller, const FVector& Location, int32 BallType, bool bEnablePhysics)
{
    if (!Controller || !Controller->FruitBallClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("SpawnBall: Controller 또는 FruitBallClass가 유효하지 않습니다."));
        return nullptr;
    }

    // 공 액터 스폰
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    SpawnParams.Owner = Controller;
    
    AActor* SpawnedBall = Controller->GetWorld()->SpawnActor<AActor>(
        Controller->FruitBallClass, Location, FRotator::ZeroRotator, SpawnParams);
        
    if (SpawnedBall)
    {
        // 크기 설정 - 모든 공에 동일한 계산식 적용
        float BaseSize = 0.5f;
        float ScaleFactor = 1.f + 0.1f * (BallType - 1);
        float BallSize = BaseSize * ScaleFactor;
        SpawnedBall->SetActorScale3D(FVector(BallSize));
        
        // 물리 컴포넌트 설정
        UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(
            SpawnedBall->GetComponentByClass(UPrimitiveComponent::StaticClass()));
            
        if (PrimComp)
        {
            if (bEnablePhysics)
            {
                // 물리 시뮬레이션 활성화
                PrimComp->SetSimulatePhysics(true);
                PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); // 콜리전 설정 명시적으로 추가
                PrimComp->SetCollisionProfileName(TEXT("PhysicsActor"));
            }
            else
            {
                // 물리 시뮬레이션 비활성화 (미리보기용)
                PrimComp->SetSimulatePhysics(false);
                PrimComp->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 미리보기시 콜리전 비활성화
                
                // 다른 액터에 부착
                SpawnedBall->AttachToComponent(
                    Controller->GetPawn()->GetRootComponent(),
                    FAttachmentTransformRules::KeepWorldTransform
                );
            }
        }
        
        UE_LOG(LogTemp, Log, TEXT("공 스폰 성공 - 타입: %d, 크기: %f, 물리: %s"), 
            BallType, BallSize, bEnablePhysics ? TEXT("활성화") : TEXT("비활성화"));
    }
    
    return SpawnedBall;
}

// 접시 가장자리 계산 함수 수정 - 카메라 방향 기준 가장 가까운 접시 가장자리에 공 생성
FVector UFruitThrowHelper::CalculatePlateEdgeSpawnPosition(UWorld* World, float HeightOffset, float CameraAngle)
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
            
            UE_LOG(LogTemp, Warning, TEXT("접시 가장자리 공 생성: 중심=%s, 크기=%s, 생성=%s, 카메라각도=%f"),
                *PlateCenter.ToString(), *Bounds.ToString(), *EdgePoint.ToString(), CameraAngle);
                
            return EdgePoint;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("접시 액터를 찾을 수 없습니다."));
    return FVector::ZeroVector;
}

// 공 던지기 함수 수정
void UFruitThrowHelper::ThrowFruit(AFruitPlayerController* Controller)
{
    if (!Controller)
    {
        UE_LOG(LogTemp, Warning, TEXT("Controller가 유효하지 않습니다."));
        return;
    }
    
    // 공통 함수 사용하여 스폰 위치 계산 (카메라 각도 전달)
    FVector SpawnLocation = CalculatePlateEdgeSpawnPosition(
        Controller->GetWorld(), 50.f, Controller->CameraOrbitAngle);
    
    if (SpawnLocation == FVector::ZeroVector)
    {
        UE_LOG(LogTemp, Warning, TEXT("유효한 스폰 위치를 계산할 수 없습니다."));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("공 생성 위치: %s"), *SpawnLocation.ToString());
    
    // 공 스폰 후 물리 적용
    AActor* SpawnedBall = SpawnBall(Controller, SpawnLocation, Controller->CurrentBallType, true);
    
    // 공 던지기 - 접시 중심을 향해 힘 적용
    if (SpawnedBall)
    {
        UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(SpawnedBall->GetComponentByClass(UPrimitiveComponent::StaticClass()));
        if (PrimComp)
        {
            // 접시 위치 찾기
            FVector PlateCenter = FVector::ZeroVector;
            TArray<AActor*> PlateActors;
            UGameplayStatics::GetAllActorsWithTag(Controller->GetWorld(), FName("Plate"), PlateActors);
            if (PlateActors.Num() > 0)
            {
                PlateCenter = PlateActors[0]->GetActorLocation();
            }
            
            // 접시 중심을 향하는 벡터
            FVector ThrowDirection = PlateCenter - SpawnLocation;
            ThrowDirection.Normalize();
            
            // 포물선 궤적을 위해 약간 위쪽으로 힘 적용
            ThrowDirection.Z += 0.6f;
            ThrowDirection.Normalize();
            
            // 던지는 힘 적용
            float ThrowForce = 700.0f;
            PrimComp->AddImpulse(ThrowDirection * ThrowForce);
            
            UE_LOG(LogTemp, Warning, TEXT("공 던지기: 방향=%s, 힘=%f"), 
                *ThrowDirection.ToString(), ThrowForce);
        }
    }
}