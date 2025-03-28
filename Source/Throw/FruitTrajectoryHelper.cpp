#include "FruitTrajectoryHelper.h"
#include "FruitPlayerController.h"
#include "FruitThrowHelper.h"
#include "FruitPhysicsHelper.h"
#include "FruitSpawnHelper.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "DrawDebugHelpers.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Actors/FruitBall.h" // FruitBall 헤더 포함

// 궤적 계산 함수 - 물리 헬퍼와 동일한 방식 사용
TArray<FVector> UFruitTrajectoryHelper::CalculateTrajectoryPoints(AFruitPlayerController* Controller, const FVector& StartLocation, const FVector& TargetLocation, float BallMass)
{
    TArray<FVector> TrajectoryPoints;
    
    if (!Controller)
        return TrajectoryPoints;
    
    // 던지기 파라미터 계산 - 물리 헬퍼 사용
    float AdjustedForce;
    FVector LaunchDirection;
    UFruitPhysicsHelper::CalculateThrowParameters(Controller, StartLocation, TargetLocation, AdjustedForce, LaunchDirection, BallMass);
    
    // 초기 속도 계산 - 힘/질량 = 속도 (F = m*a 공식 활용)
    // 질량이 0이 아닌지 확인
    if (BallMass <= 0.0f)
        BallMass = 1.0f;
    
    FVector InitialVelocity = LaunchDirection * (AdjustedForce / BallMass);
    
    // 중력 가속도 - 물리 헬퍼와 동일한 값 사용
    float GravityZ = -980.0f;
    FVector Gravity = FVector(0, 0, GravityZ);
    
    // 시작점 추가
    TrajectoryPoints.Add(StartLocation);
    
    // 시간 간격으로 위치 계산 - 더 촘촘한 포인트 생성
    const float TimeStep = 0.02f;
    const float MaxTime = 5.0f;
    
    // 시간에 따른 포인트 계산 - 단순화된 코드
    for (float Time = TimeStep; Time < MaxTime; Time += TimeStep)
    {
        // 포물선 운동 방정식: P = P0 + V0*t + 0.5*a*t^2
        FVector Position = StartLocation + InitialVelocity * Time + 0.5f * Gravity * Time * Time;
        
        // 포인트 추가
        TrajectoryPoints.Add(Position);
        
        // 바닥에 닿았는지 확인
        if (Position.Z < 0.0f)
            break;
            
        // 목표 지점 근처인지 확인
        float DistanceToTarget = FVector::Dist(Position, TargetLocation);
        if (DistanceToTarget < 10.0f)
            break;
    }
    
    return TrajectoryPoints;
}

// 포물선 궤적 그리기 함수 - 단순화된 버전
void UFruitTrajectoryHelper::DrawTrajectoryPath(UWorld* World, const TArray<FVector>& TrajectoryPoints, const FVector& TargetLocation, bool bPersistent, int32 TrajectoryID)
{
    if (!World || TrajectoryPoints.Num() < 2)
        return;
    
    // 항상 영구적으로 표시
    float Duration = -1.0f; // 무기한 지속
    bool PersistentFlag = true; // 항상 영구적
    
    // 기존 궤적 비우기 (ID별로 비우기)
    FlushPersistentDebugLines(World);
    DrawDebugLine(World, FVector::ZeroVector, FVector::ZeroVector, FColor::Black, false, 0.0f, TrajectoryID, 0.0f); // 임시 라인 생성 후 삭제
    
    // 선명한 하늘색 계열로 색상 설정
    FColor LineColor = FColor(80, 150, 255); // 파란색 계열 선
    FColor SphereColor = FColor(135, 206, 235); // 하늘색 구
    float LineThickness = 2.5f; // 선 두께
    
    // 표시할 구의 개수 증가
    const int32 MaxSpheresCount = 30; // 여러 개의 구 세그먼트
    int32 SphereInterval = FMath::Max(1, TrajectoryPoints.Num() / MaxSpheresCount);
    
    // 구 크기 감소
    const float UniformSphereSize = 1.5f; // 작은 구
    
    // 1. 먼저 선으로 모든 포인트 연결 (배경)
    for (int32 i = 0; i < TrajectoryPoints.Num() - 1; i++)
    {
        // 두 점이 유효한지 확인
        if (!TrajectoryPoints[i].IsZero() && !TrajectoryPoints[i+1].IsZero())
        {
            // 선 그리기
            DrawDebugLine(World, TrajectoryPoints[i], TrajectoryPoints[i + 1], LineColor, PersistentFlag, Duration, TrajectoryID, LineThickness);
        }
    }
    
    // 2. 궤적 포인트에 작고 균일한 크기의 하늘색 구 세그먼트 그리기
    for (int32 i = 0; i < TrajectoryPoints.Num(); i += SphereInterval)
    {
        // 모든 지점에 작고 동일한 크기의 하늘색 구 그리기
        DrawDebugSphere(World, TrajectoryPoints[i], UniformSphereSize, 6, SphereColor, PersistentFlag, Duration, TrajectoryID, 1.0f);
    }
    
    // 목표 지점만 작은 표시
    DrawDebugSphere(World, TargetLocation, UniformSphereSize, 6, SphereColor, PersistentFlag, Duration, TrajectoryID, 1.0f);
}

// 궤적 업데이트 함수 - 개선된 버전
void UFruitTrajectoryHelper::UpdateTrajectoryPath(AFruitPlayerController* Controller, const FVector& StartLocation, const FVector& TargetLocation, bool bPersistent, int32 CustomTrajectoryID)
{
    if (!Controller || !Controller->GetWorld())
        return;
    
    UWorld* World = Controller->GetWorld();
    
    // 고정 ID 사용
    const int32 TrajectoryID = (CustomTrajectoryID != 0) ? CustomTrajectoryID : 9999;
    
    // 공의 질량 계산
    float BallMass = UFruitSpawnHelper::CalculateBallMass(Controller->CurrentBallType);
    
    // 시작점 조정 - 공의 중심 위치 사용
    FVector AdjustedStartLocation = StartLocation;
    
    // 공이 있으면 공의 실제 중심 위치 계산
    if (Controller->PreviewBall)
    {
        // 공의 크기 및 반지름 계산
        float BallScale = Controller->PreviewBall->GetActorScale3D().X;
        float BallRadius = BallScale * 50.0f; // 더 정확한 반지름 계산 (50cm 기준 구)
        
        // 공의 물리 바운드 반지름 가져오기 (가능한 경우)
        UStaticMeshComponent* MeshComp = Cast<UStaticMeshComponent>(Controller->PreviewBall->GetComponentByClass(UStaticMeshComponent::StaticClass()));
        if (MeshComp && MeshComp->GetStaticMesh())
        {
            // 메시에서 직접 바운드 반지름 계산
            FBoxSphereBounds Bounds = MeshComp->GetStaticMesh()->GetBounds();
            BallRadius = Bounds.SphereRadius * BallScale * 100.0f; // cm 단위로 변환
        }
        
        // 중심 보정치 적용 (Z축 방향으로만 보정)
        FVector UpVector = FVector(0, 0, BallRadius);
        AdjustedStartLocation = Controller->PreviewBall->GetActorLocation() + UpVector;
        
        UE_LOG(LogTemp, Log, TEXT("공 반지름 계산: %f, 보정된 시작 위치: %s"), 
            BallRadius, *AdjustedStartLocation.ToString());
    }
    
    // 접시 위치 정확히 찾기
    FVector PreciseTargetLocation = TargetLocation;
    
    // 접시가 태그로 설정되어 있는지 확인
    TArray<AActor*> PlateActors;
    UGameplayStatics::GetAllActorsWithTag(World, FName("Plate"), PlateActors);
    if (PlateActors.Num() > 0)
    {
        // 접시 위치 조정 - 접시 표면 위치 계산
        AActor* PlateActor = PlateActors[0];
        FVector PlateCenter = PlateActor->GetActorLocation();
        
        // 접시의 메시 컴포넌트 찾기
        UStaticMeshComponent* PlateMesh = Cast<UStaticMeshComponent>(PlateActor->GetComponentByClass(UStaticMeshComponent::StaticClass()));
        if (PlateMesh)
        {
            // 접시의 Z 방향 크기 가져오기
            FBoxSphereBounds PlateBounds = PlateMesh->Bounds;
            float PlateHeight = PlateBounds.BoxExtent.Z * 2.0f; // 높이 계산
            
            // 접시 표면 위치 계산 (접시 위 약간 띄움)
            PlateCenter.Z += PlateHeight * 0.5f + 5.0f;
        }
        else
        {
            // 메시 정보를 찾을 수 없는 경우 기본값 사용
            PlateCenter.Z += 10.0f;
        }
        
        PreciseTargetLocation = PlateCenter;
        
        // 컨트롤러에 접시 위치 업데이트
        if (Controller->GetClass()->FindPropertyByName(TEXT("PlateLocation")))
        {
            Controller->PlateLocation = PlateCenter;
        }
    }
    
    // 궤적 계산 - 인자 한 줄로
    TArray<FVector> NewTrajectoryPoints = CalculateTrajectoryPoints(Controller, AdjustedStartLocation, PreciseTargetLocation, BallMass);
    
    // 궤적 포인트가 충분한지 확인
    if (NewTrajectoryPoints.Num() < 2)
    {
        UE_LOG(LogTemp, Warning, TEXT("궤적 포인트가 충분하지 않음: %d"), NewTrajectoryPoints.Num());
        return;
    }
    
    // 각도 및 위치 로그
    UE_LOG(LogTemp, Log, TEXT("궤적 그리기: 각도=%f, 포인트=%d개, 시작=%s, 목표=%s"), 
        Controller->ThrowAngle, 
        NewTrajectoryPoints.Num(), 
        *NewTrajectoryPoints[0].ToString(), 
        *PreciseTargetLocation.ToString());
    
    // 새 궤적 그리기 - 인자 한 줄로
    DrawTrajectoryPath(World, NewTrajectoryPoints, PreciseTargetLocation, true, TrajectoryID);
}

// 하위 호환성을 위한 영구 궤적 그리기 함수 - 인자 한 줄로
void UFruitTrajectoryHelper::DrawPersistentTrajectoryPath(AFruitPlayerController* Controller, const FVector& StartLocation, const FVector& TargetLocation)
{
    // UpdateTrajectoryPath 함수 호출 - 인자 한 줄로
    UpdateTrajectoryPath(Controller, StartLocation, TargetLocation, true);
}