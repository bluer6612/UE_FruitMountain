#include "FruitSpawnHelper.h"
#include "Gameplay/Controller/FruitPlayerController.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "Actors/PlateActor.h"
#include "Actors/FruitBall.h"

// 크기 계산 함수 - FruitBall 클래스 함수 사용
float UFruitSpawnHelper::CalculateBallSize(int32 BallType)
{
    // FruitBall 클래스의 함수 사용 - 월드 스케일로 계산됨
    float WorldSizeCM = AFruitBall::CalculateBallSize(BallType);
    
    // 언리얼 액터 스케일 반환 (100으로 나눔)
    return WorldSizeCM / 100.0f;
}

// 질량 계산 함수 - FruitBall 클래스 함수 사용
float UFruitSpawnHelper::CalculateBallMass(int32 BallType)
{
    // 직접 FruitBall 클래스의 함수 호출
    return AFruitBall::CalculateBallMass(BallType);
}

// SpawnBall 함수 수정 - 기존 코드 유지
AActor* UFruitSpawnHelper::SpawnBall(AFruitPlayerController* Controller, const FVector& Location, int32 BallType, bool bEnablePhysics)
{
    if (!Controller || !Controller->FruitBallClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("SpawnBall: Controller 또는 FruitBallClass가 유효하지 않습니다."));
        return nullptr;
    }

    // 공 속성 계산 - 크기 및 질량 (FruitBall 클래스 기반)
    float BallSize = CalculateBallSize(BallType); // 이미 액터 스케일로 변환됨 (1/100)
    float BallMass = CalculateBallMass(BallType);

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
        
        // BallType 설정
        AFruitBall* FruitBall = Cast<AFruitBall>(SpawnedBall);
        if (FruitBall)
        {
            FruitBall->BallType = BallType;
        }
            
        FruitBall->DisplayDebugInfo();
        
        // 디버그 로그로 크기 확인
        UE_LOG(LogTemp, Verbose, TEXT("공 생성: 타입=%d, 크기(스케일)=%f, 실제 크기(cm)=%f"),
            BallType, BallSize, BallSize * 100.0f);
        
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

// 접시 가장자리 위치 계산 함수 구현 - 순수 접시 반지름만 계산
FVector UFruitSpawnHelper::CalculatePlateEdgeSpawnPosition(UWorld* World, float CameraAngle)
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
            
            // 테이블과 접시를 포함한 전체 바운딩 박스 (높이 계산용)
            FBox TotalBounds = PlateActorRef->GetComponentsBoundingBox();
            
            // 순수 접시 메시만 찾아서 반지름 계산
            UStaticMeshComponent* PlateMeshComp = nullptr;
            TArray<UStaticMeshComponent*> MeshComponents;
            PlateActorRef->GetComponents<UStaticMeshComponent>(MeshComponents);
            
            // 접시 컴포넌트 찾기 ("PlateMesh"라는 이름으로 가정)
            for (UStaticMeshComponent* MeshComp : MeshComponents)
            {
                if (MeshComp && MeshComp->GetName().Contains("Plate"))
                {
                    PlateMeshComp = MeshComp;
                    break;
                }
            }
            
            // 접시 컴포넌트를 찾은 경우 그 바운딩 박스로 반지름 계산
            if (PlateMeshComp)
            {
                FBox PlateBounds = PlateMeshComp->Bounds.GetBox();
                FVector PlateSize = PlateBounds.GetSize();
                
                // 접시만의 반지름 계산 (X, Y 중 큰 값의 절반)
                PlateRadius = FMath::Max(PlateSize.X, PlateSize.Y) * 0.45f; // 약간 여유를 두고 0.45배
                
                UE_LOG(LogTemp, Verbose, TEXT("순수 접시 반경: %.1f (X=%.1f, Y=%.1f)"),
                    PlateRadius, PlateSize.X, PlateSize.Y);
            }
            else
            {
                // 접시 컴포넌트를 못 찾았다면 전체 바운드에서 예상 반지름 계산
                // 테이블을 제외한 접시 크기를 추정
                FVector Bounds = TotalBounds.GetSize();
                PlateRadius = FMath::Min(Bounds.X, Bounds.Y) * 0.4f; // 더 작은 값 사용 (테이블이 더 큰 경향)
                
                UE_LOG(LogTemp, Warning, TEXT("접시 메시를 찾을 수 없어 추정 반경 사용: %.1f"), PlateRadius);
            }
            
            // 카메라 방향 벡터 계산
            float RadianAngle = FMath::DegreesToRadians(CameraAngle);
            FVector CameraDirection;
            CameraDirection.X = FMath::Cos(RadianAngle);
            CameraDirection.Y = FMath::Sin(RadianAngle);
            CameraDirection.Z = 0.0f;
            CameraDirection.Normalize();
            
            // 카메라 방향의 반대쪽 접시 가장자리 지점 계산 (카메라에서 가장 먼 곳)
            FVector EdgePoint = PlateCenter + CameraDirection * PlateRadius;
            
            // 높이 조정 - 전체 구조물(테이블+접시) 위로, 공 크기를 고려한 오프셋 적용
            float BallTypeOffset = 10.0f; // 추가 여유 높이
            EdgePoint.Z = TotalBounds.Max.Z + BallTypeOffset;
            
            // 디버그 로그
            UE_LOG(LogTemp, Verbose, TEXT("공 스폰 위치 계산: 접시 중심=(%.1f, %.1f, %.1f), 순수 접시 반경=%.1f, 최종 높이=%.1f"),
                PlateCenter.X, PlateCenter.Y, PlateCenter.Z, PlateRadius, EdgePoint.Z);
                
            return EdgePoint;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("접시 액터를 찾을 수 없습니다."));
    return FVector::ZeroVector;
}