#include "FruitSpawnHelper.h"
#include "FruitPlayerController.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "Actors/PlateActor.h"

// FruitBall 클래스 전방 선언 - 전체 헤더 포함 대신
class AFruitBall;

// 크기 계산 함수 - 임시 구현으로 대체
float UFruitSpawnHelper::CalculateBallSize(int32 BallType)
{
    // 첫 번째 공(BallType 1)의 기본 크기를 15.0f로 설정
    float BaseSize = 15.0f;
    
    // BallType 1부터 시작하므로, (BallType - 1)이 0부터 시작
    // 각 타입마다 크기 10% 증가 (1.0, 1.1, 1.2, 1.3, ...)
    float GrowthFactor = 1.0f + 0.1f * (BallType - 1);
    
    // 최종 공 크기 계산
    float BallSize = BaseSize * GrowthFactor;
    
    // 언리얼에서의 액터 스케일은 100배 축소된 값을 사용하므로 100으로 나눔
    // 예: 15.0f의 실제 크기 → 0.15f의 액터 스케일
    return BallSize / 100.0f;
}

// 질량 계산 함수 - 임시 구현으로 대체
float UFruitSpawnHelper::CalculateBallMass(int32 BallType)
{
    // 크기를 월드 스케일로 가져옴 (스케일 값 * 100)
    float BallSize = CalculateBallSize(BallType) * 100.0f;
    
    // 크기에 비례하여 질량 계산 - 체적 기반 (r^3에 비례)
    // 작은 공(15cm)은 약 1kg, 큰 공은 더 무겁게
    float BallRadius = BallSize / 2.0f; // 반지름 (cm)
    float BallVolume = (4.0f/3.0f) * PI * FMath::Pow(BallRadius, 3); // 구의 체적 (cm^3)
    
    // 밀도 계수 (그램/cm^3) - 약 0.3으로 설정하여 15cm 공이 약 1kg이 되도록 함
    float DensityFactor = 0.3f;
    
    // 체적 * 밀도 = 질량 (그램)
    // 킬로그램으로 변환 (1000으로 나눔)
    float BallMass = (BallVolume * DensityFactor) / 1000.0f;
    
    // 최소 질량 보장
    return FMath::Max(BallMass, 0.5f);
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