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

// 궤적 계산 함수 - 부드러운 포물선 곡선 생성
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
        
    // 초기 속도 계산 - 질량과 무관하게 일정한 속도로 궤적 시뮬레이션
    float StandardMass = 10.0f; // 표준 질량 정의
    FVector InitialVelocity = LaunchDirection * (AdjustedForce / StandardMass); // 표준 질량으로 나누어 질량 효과 제거
    
    // 중력 가속도
    float GravityZ = -980.0f;
    FVector Gravity = FVector(0, 0, GravityZ);
    
    // 시작점 추가
    TrajectoryPoints.Add(StartLocation);
    
    // 시간 간격으로 위치 계산 - 더 촘촘한 포인트 생성
    const float TimeStep = 0.02f; // 0.05f에서 0.02f로 줄여 더 촘촘하게 포인트 생성
    const float MaxTime = 5.0f;
    bool bHitTarget = false;
    
    // 곡선을 계산하기 위한 삼차 베지어 곡선 제어점
    TArray<FVector> ControlPoints;
    ControlPoints.Add(StartLocation); // 시작점
    
    // 시간에 따른 포인트 계산
    for (float Time = TimeStep; Time < MaxTime; Time += TimeStep)
    {
        // 포물선 운동 방정식: P = P0 + V0*t + 0.5*a*t^2
        FVector Position = StartLocation + InitialVelocity * Time + 0.5f * Gravity * Time * Time;
        
        // 제어점 배열에 추가
        ControlPoints.Add(Position);
        
        // 바닥에 닿았는지 확인
        if (Position.Z < 0.0f)
            break;
            
        // 목표 지점 근처인지 확인
        float DistanceToTarget = FVector::Dist(Position, PreciseTargetLocation);
        if (DistanceToTarget < 10.0f)
        {
            bHitTarget = true;
            ControlPoints.Add(PreciseTargetLocation); // 목표점 추가
            break;
        }
    }
    
    // 목표 지점에 닿지 않았다면, 마지막 포인트를 목표 지점까지 보간
    if (!bHitTarget && ControlPoints.Num() > 1)
    {
        // 마지막 포인트와 목표 지점 사이에 추가 보간 포인트 생성
        FVector LastPoint = ControlPoints.Last();
        
        // 부드러운 보간을 위한 추가 포인트
        for (int32 i = 1; i <= 5; i++)
        {
            float Alpha = (float)i / 6.0f;
            FVector InterpPoint = FMath::Lerp(LastPoint, PreciseTargetLocation, Alpha);
            ControlPoints.Add(InterpPoint);
        }
        
        // 마지막에 정확한 목표 지점 추가
        ControlPoints.Add(PreciseTargetLocation);
    }
    
    // 제어점에서 스플라인 곡선 생성 - 더 부드러운 곡선을 위한 최종 포인트 생성
    const int32 CurveSegments = FMath::Max(ControlPoints.Num() * 3, 50); // 충분한 세그먼트 생성
    
    // 베지어 곡선 방식으로 부드럽게 보간된 포인트 생성
    for (int32 i = 0; i <= CurveSegments; i++)
    {
        float Alpha = (float)i / (float)CurveSegments;
        
        // 부드러운 곡선을 위한 보간 포인트 계산
        FVector InterpolatedPoint;
        
        // 시작 및 끝 부분은 직접 제어점 사용
        if (i == 0)
        {
            InterpolatedPoint = ControlPoints[0];
        }
        else if (i == CurveSegments)
        {
            InterpolatedPoint = ControlPoints.Last();
        }
        else
        {
            // 인접한 제어점 간의 선형 보간 (더 복잡한 베지어 또는 스플라인 보간으로 대체 가능)
            float PointPosition = Alpha * (ControlPoints.Num() - 1);
            int32 PointIndex = FMath::FloorToInt(PointPosition);
            float LocalAlpha = PointPosition - PointIndex;
            
            if (PointIndex < ControlPoints.Num() - 1)
            {
                InterpolatedPoint = FMath::Lerp(ControlPoints[PointIndex], ControlPoints[PointIndex + 1], LocalAlpha);
            }
            else
            {
                InterpolatedPoint = ControlPoints.Last();
            }
        }
        
        TrajectoryPoints.Add(InterpolatedPoint);
    }
    
    return TrajectoryPoints;
}

// 부드러운 포물선 궤적 그리기 함수
void UFruitTrajectoryHelper::DrawTrajectoryPath(UWorld* World, const TArray<FVector>& TrajectoryPoints, const FVector& TargetLocation, bool bPersistent, int32 TrajectoryID)
{
    if (!World || TrajectoryPoints.Num() < 2)
        return;
    
    // 항상 영구적으로 표시
    float Duration = -1.0f; // 무기한 지속
    bool PersistentFlag = true; // 항상 영구적
    
    // 색상 설정 - 더 선명한 색상 사용
    FColor LineColor = FColor::Blue;  // 밝은 파란색
    
    // 선 두께 - 부드러운 곡선에 적합한 두께
    float LineThickness = 2.5f;  // 더 두꺼운 선
    
    // 부드러운 곡선 그리기 - 모든 포인트를 선으로 연결
    for (int32 i = 0; i < TrajectoryPoints.Num() - 1; i++)
    {
        // 두 점이 유효한지 확인
        if (!TrajectoryPoints[i].IsZero() && !TrajectoryPoints[i+1].IsZero())
        {
            // 더 작은 간격으로 선 그리기 - 부드러운 곡선 효과
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
        
        // 컨트롤러에 접시 위치 업데이트 (만약 컨트롤러에 해당 속성이 있다면)
        if (Controller->GetClass()->FindPropertyByName(TEXT("PlateLocation")))
        {
            Controller->PlateLocation = PlateCenter;
        }
    }
    
    // 궤적 계산 (인자 한 줄로)
    TArray<FVector> NewTrajectoryPoints = CalculateTrajectoryPoints(Controller, AdjustedStartLocation, PreciseTargetLocation, BallMass);
    
    // 궤적 포인트가 충분한지 확인
    if (NewTrajectoryPoints.Num() < 2)
        return;
    
    // 새 궤적 그리기 (항상 영구적으로) - 라인만 그림
    DrawTrajectoryPath(World, NewTrajectoryPoints, PreciseTargetLocation, true, TrajectoryID);
}

// 하위 호환성을 위한 영구 궤적 그리기 함수 구현
void UFruitTrajectoryHelper::DrawPersistentTrajectoryPath(AFruitPlayerController* Controller, const FVector& StartLocation, const FVector& TargetLocation)
{
    // 단순히 UpdateTrajectoryPath 함수를 호출하고 영구적 파라미터를 true로 설정
    UpdateTrajectoryPath(Controller, StartLocation, TargetLocation, true);
}