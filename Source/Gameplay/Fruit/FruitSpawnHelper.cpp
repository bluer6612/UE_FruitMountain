#include "FruitSpawnHelper.h"
#include "FruitCollisionHelper.h"
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
        
        // BallType 설정 및 미리보기 플래그 설정
        AFruitBall* FruitBall = Cast<AFruitBall>(SpawnedBall);
        if (FruitBall)
        {
            FruitBall->BallType = BallType;
            
            // 중요: 미리보기 여부 명시적 설정 (물리가 활성화되지 않으면 미리보기 공)
            FruitBall->bIsPreviewBall = !bEnablePhysics;
                
            // 여기서 직접 충돌 핸들러 등록 (미리보기 공이 아닐 때만)
            if (!FruitBall->bIsPreviewBall)
            {
                UFruitCollisionHelper::RegisterCollisionHandlers(FruitBall);
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

// 접시 가장자리 위치 계산 함수 구현 - 순수 접시 반지름만 계산
FVector UFruitSpawnHelper::CalculatePlateEdgeSpawnPosition(UWorld* World, float CameraAngle)
{
    // // 카메라 각도 테스트 모드 활성화 (일관성 확인용)
    static FVector LastSpawnPos = FVector::ZeroVector;
    static float LastAngle = -999.0f;
    
    // static bool bTestMode = true;
    // if (bTestMode) {
    //     if (LastSpawnPos != FVector::ZeroVector && FMath::Abs(LastAngle - CameraAngle) < 180.0f) {
    //         // 카메라 회전에도 동일한 위치 반환 (테스트용)
    //         // 일관된 계산을 위한 것, 실제 게임에서는 주석 처리 고려
    //         UE_LOG(LogTemp, Warning, TEXT("테스트 모드: 카메라 회전 무시, 이전 위치 재사용"));
    //         return LastSpawnPos;
    //     }
    // }
    
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
                PlateRadius = FMath::Max(PlateSize.X, PlateSize.Y) * 0.475f; // 약간 여유를 두고 0.475배
                
                UE_LOG(LogTemp, Verbose, TEXT("순수 접시 반경: %.1f (X=%.1f, Y=%.1f)"),
                    PlateRadius, PlateSize.X, PlateSize.Y);
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("접시 메시를 찾을 수 없음"));
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
            float BallTypeOffset = 7.5f; // 추가 여유 높이
            EdgePoint.Z = TotalBounds.Max.Z + BallTypeOffset;
            
            // 디버그 로그
            //UE_LOG(LogTemp, Verbose, TEXT("공 스폰 위치 계산: 접시 중심=(%.1f, %.1f, %.1f), 순수 접시 반경=%.1f, 최종 높이=%.1f"),
            //    PlateCenter.X, PlateCenter.Y, PlateCenter.Z, PlateRadius, EdgePoint.Z);
                
            FVector FinalPosition = EdgePoint;
            
            // 계산된 위치 저장 (카메라 회전 테스트용으로 지워도 됨)
            LastSpawnPos = FinalPosition;
            LastAngle = CameraAngle;
            
            return FinalPosition;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("접시 액터를 찾을 수 없습니다."));
    return FVector::ZeroVector;
}