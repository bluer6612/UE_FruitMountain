#include "FruitTrajectoryHelper.h"
#include "FruitPlayerController.h"
#include "FruitThrowHelper.h"
#include "FruitPhysicsHelper.h" // 새 헬퍼 클래스 포함
#include "FruitSpawnHelper.h" // 공의 질량 계산을 위한 헬퍼 클래스 포함
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "DrawDebugHelpers.h"
#include "PhysicsEngine/PhysicsSettings.h"

// 궤적 계산 함수 - FruitPhysicsHelper 활용
TArray<FVector> UFruitTrajectoryHelper::CalculateTrajectoryPoints(
    AFruitPlayerController* Controller,
    const FVector& StartLocation,
    const FVector& TargetLocation,
    float BallMass)
{
    TArray<FVector> TrajectoryPoints;
    
    if (!Controller)
        return TrajectoryPoints;
    
    // 정확한 목표 지점 사용
    FVector PreciseTargetLocation = TargetLocation;
    
    // FruitPhysicsHelper 함수 호출하여 던지기 파라미터 계산
    float AdjustedForce;
    FVector LaunchDirection;
    UFruitPhysicsHelper::CalculateThrowParameters(Controller, StartLocation, PreciseTargetLocation, AdjustedForce, LaunchDirection, BallMass);
        
    // 초기 속도 계산 - 약간 더 강한 힘 적용 (1.05배)
    FVector InitialVelocity = LaunchDirection * (AdjustedForce * 1.05f / BallMass);
    
    // 중력 가속도
    float GravityZ = -980.0f;
    FVector Gravity = FVector(0, 0, GravityZ);
    
    // 시작점 추가
    TrajectoryPoints.Add(StartLocation);
    
    // 시간 간격으로 위치 계산
    const float TimeStep = 0.05f; // 더 촘촘하게 포인트 생성
    const float MaxTime = 5.0f;
    bool bHitTarget = false;
    
    for (float Time = TimeStep; Time < MaxTime; Time += TimeStep)
    {
        // 포물선 운동 방정식: P = P0 + V0*t + 0.5*a*t^2
        FVector Position = StartLocation + InitialVelocity * Time + 0.5f * Gravity * Time * Time;
        
        // 포인트 추가
        TrajectoryPoints.Add(Position);
        
        // 바닥에 닿았는지 확인
        if (Position.Z < 0.0f)
            break;
            
        // 목표 지점 근처인지 확인 - 더 작은 값으로 정확도 향상
        float DistanceToTarget = FVector::Dist(Position, PreciseTargetLocation);
        if (DistanceToTarget < 10.0f)
        {
            bHitTarget = true;
            break;
        }
    }
    
    // 목표 지점에 닿지 않았다면, 마지막 포인트를 목표 지점까지 보간
    if (!bHitTarget && TrajectoryPoints.Num() > 0)
    {
        // 마지막 포인트와 목표 지점 사이에 추가 포인트 생성
        FVector LastPoint = TrajectoryPoints.Last();
        
        // 5개의 추가 보간 포인트 생성
        for (int32 i = 1; i <= 5; i++)
        {
            float Alpha = (float)i / 6.0f;
            FVector InterpPoint = FMath::Lerp(LastPoint, PreciseTargetLocation, Alpha);
            TrajectoryPoints.Add(InterpPoint);
        }
        
        // 마지막에 정확한 목표 지점 추가
        TrajectoryPoints.Add(PreciseTargetLocation);
    }
    
    return TrajectoryPoints;
}

// 확실하게 보이는 궤적 그리기 함수 - 점 제거하고 라인만 그리도록 수정
void UFruitTrajectoryHelper::DrawTrajectoryPath(UWorld* World, const TArray<FVector>& TrajectoryPoints, const FVector& TargetLocation, bool bPersistent, int32 TrajectoryID)
{
    if (!World || TrajectoryPoints.Num() < 2)
        return;
    
    // 항상 영구적으로 표시
    float Duration = -1.0f; // 무기한 지속
    bool PersistentFlag = true; // 항상 영구적
    
    // 색상 설정 - 더 선명한 색상 사용
    FColor LineColor = FColor::Blue;  // 밝은 파란색
    
    // 선 두께 - 더 두껍게 설정하여 가시성 향상
    float LineThickness = 2.0f;  // 더 두꺼운 선
    
    // 새 궤적 그리기 - 라인만 그림
    for (int32 i = 0; i < TrajectoryPoints.Num() - 1; i++)
    {
        // 두 점이 유효한지 확인
        if (!TrajectoryPoints[i].IsZero() && !TrajectoryPoints[i+1].IsZero())
        {
            // 점들을 선으로 연결 (인자 한 줄로)
            DrawDebugLine(World, TrajectoryPoints[i], TrajectoryPoints[i + 1], LineColor, PersistentFlag, Duration, TrajectoryID, LineThickness);
        }
    }
}

// 궤적 업데이트 함수 개선 - 정확한 접시 위치 사용
void UFruitTrajectoryHelper::UpdateTrajectoryPath(AFruitPlayerController* Controller, const FVector& StartLocation, const FVector& TargetLocation, bool bPersistent, int32 CustomTrajectoryID)
{
    if (!Controller || !Controller->GetWorld())
        return;
    
    UWorld* World = Controller->GetWorld();
    
    // 이전의 모든 디버그 라인을 강제로 제거
    FlushPersistentDebugLines(World);
    
    // 고정 ID 사용 - 다른 디버그 라인과 충돌 방지
    const int32 TrajectoryID = (CustomTrajectoryID != 0) ? CustomTrajectoryID : 9999;
    
    // 공의 질량 계산
    float BallMass = UFruitSpawnHelper::CalculateBallMass(Controller->CurrentBallType);
    
    // 시작점 조정
    FVector AdjustedStartLocation = StartLocation;
    
    // 접시 위치 정확히 찾기
    FVector PreciseTargetLocation = TargetLocation;
    
    // 접시가 태그로 설정되어 있는지 확인
    TArray<AActor*> PlateActors;
    UGameplayStatics::GetAllActorsWithTag(World, FName("Plate"), PlateActors);
    if (PlateActors.Num() > 0)
    {
        // 첫 번째 접시 액터 사용
        AActor* PlateActor = PlateActors[0];
        
        // 접시의 정확한 위치 (중앙 + 약간 위로)
        FVector PlateCenter = PlateActor->GetActorLocation();
        
        // 접시 위에 도달하도록 약간 위로 조정 (10 유닛)
        PlateCenter.Z += 10.0f;
        
        // 정확한 목표 위치 사용
        PreciseTargetLocation = PlateCenter;
        
        // 컨트롤러에 접시 위치 업데이트
        Controller->PlateLocation = PlateCenter;
    }
    
    // 공이 있으면 공의 실제 중심 위치 계산
    if (Controller->PreviewBall)
    {
        // 공의 크기 가져오기
        float BallSize = Controller->PreviewBall->GetActorScale3D().X;
        
        // 정확한 반지름 계산 (BallSize * 50.0f / 2)
        float BallRadius = BallSize * 25.0f; // 공의 실제 반지름
        
        // 중심 보정치 적용 - 정확한 반지름 적용
        FVector UpVector = FVector(0, 0, BallRadius);
        AdjustedStartLocation = Controller->PreviewBall->GetActorLocation() + UpVector;
    }
    
    // 궤적 계산 (인자 한 줄로)
    TArray<FVector> NewTrajectoryPoints = CalculateTrajectoryPoints(Controller, AdjustedStartLocation, PreciseTargetLocation, BallMass);
    
    // 궤적 포인트가 충분한지 확인
    if (NewTrajectoryPoints.Num() < 2)
        return;
    
    // 새 궤적 그리기 (항상 영구적으로) - 라인만 그림
    DrawTrajectoryPath(World, NewTrajectoryPoints, PreciseTargetLocation, true, TrajectoryID);
}

// 이전 DrawPersistentTrajectoryPath 함수를 새 함수 호출로 대체
void UFruitTrajectoryHelper::DrawPersistentTrajectoryPath(AFruitPlayerController* Controller, const FVector& StartLocation, const FVector& TargetLocation)
{
    // 통합된 함수 호출 - 영구적 궤적을 위한 ID 0 사용
    UpdateTrajectoryPath(Controller, StartLocation, TargetLocation, true, 0);
}