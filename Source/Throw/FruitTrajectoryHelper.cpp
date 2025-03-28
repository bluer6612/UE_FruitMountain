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

// 궤적 계산 함수 - 인자 한 줄로 정리
TArray<FVector> UFruitTrajectoryHelper::CalculateTrajectoryPoints(AFruitPlayerController* Controller, const FVector& StartLocation, const FVector& TargetLocation, float BallMass)
{
    TArray<FVector> TrajectoryPoints;
    
    if (!Controller)
        return TrajectoryPoints;
    
    // 정확한 목표 지점 사용
    FVector PreciseTargetLocation = TargetLocation;
    
    // FruitPhysicsHelper 함수 호출하여 던지기 파라미터 계산 - 인자 한 줄로
    float AdjustedForce;
    FVector LaunchDirection;
    UFruitPhysicsHelper::CalculateThrowParameters(Controller, StartLocation, PreciseTargetLocation, AdjustedForce, LaunchDirection, BallMass);
        
    // 초기 속도 계산 - FruitBall 클래스의 표준 질량 사용
    // 중요: FruitBall의 밀도 계수와 일관되게 곱하여 같은 비율 유지
    float StandardMass = AFruitBall::DensityFactor * 50.0f / 0.3f; // 원래 표준 질량에 밀도 비율 적용
    FVector InitialVelocity = LaunchDirection * (AdjustedForce / StandardMass);
    
    // 중력 가속도
    float GravityZ = -980.0f;
    FVector Gravity = FVector(0, 0, GravityZ);
    
    // 시작점 추가
    TrajectoryPoints.Add(StartLocation);
    
    // 시간 간격으로 위치 계산 - 더 촘촘한 포인트 생성
    const float TimeStep = 0.02f;
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
            // 인접한 제어점 간의 선형 보간
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

// 포물선 궤적 그리기 함수 - 일정한 크기의 구 사용
void UFruitTrajectoryHelper::DrawTrajectoryPath(UWorld* World, const TArray<FVector>& TrajectoryPoints, const FVector& TargetLocation, bool bPersistent, int32 TrajectoryID)
{
    if (!World || TrajectoryPoints.Num() < 2)
        return;
    
    // 항상 영구적으로 표시
    float Duration = -1.0f; // 무기한 지속
    bool PersistentFlag = true; // 항상 영구적
    
    // 색상 및 두께 설정
    FColor LineColor = FColor::Blue;
    FColor SphereColor = FColor(0, 100, 255);
    float LineThickness = 2.0f;
    
    // 표시할 구의 개수 제한
    const int32 MaxSpheresCount = 20;
    int32 SphereInterval = FMath::Max(1, TrajectoryPoints.Num() / MaxSpheresCount);
    
    // 모든 구에 동일한 크기 적용
    const float UniformSphereSize = 2.0f;
    
    // 1. 궤적 포인트에 균일한 크기의 구 세그먼트 그리기
    for (int32 i = 0; i < TrajectoryPoints.Num(); i += SphereInterval)
    {
        // 모든 지점에 동일한 크기의 구 그리기
        DrawDebugSphere(World, TrajectoryPoints[i], UniformSphereSize, 8, SphereColor, PersistentFlag, Duration, 0, 1.0f);
    }
    
    // 2. 선으로 모든 포인트 연결
    for (int32 i = 0; i < TrajectoryPoints.Num() - 1; i++)
    {
        // 두 점이 유효한지 확인
        if (!TrajectoryPoints[i].IsZero() && !TrajectoryPoints[i+1].IsZero())
        {
            // 선 그리기
            DrawDebugLine(World, TrajectoryPoints[i], TrajectoryPoints[i + 1], LineColor, PersistentFlag, Duration, TrajectoryID, LineThickness);
        }
    }
}

// 궤적 업데이트 함수 - 인자 한 줄로 정리
void UFruitTrajectoryHelper::UpdateTrajectoryPath(AFruitPlayerController* Controller, const FVector& StartLocation, const FVector& TargetLocation, bool bPersistent, int32 CustomTrajectoryID)
{
    if (!Controller || !Controller->GetWorld())
        return;
    
    UWorld* World = Controller->GetWorld();
    
    // 이전의 모든 디버그 라인을 강제로 제거
    FlushPersistentDebugLines(World);
    
    // 고정 ID 사용
    const int32 TrajectoryID = (CustomTrajectoryID != 0) ? CustomTrajectoryID : 9999;
    
    // 공의 질량 계산
    float BallMass = UFruitSpawnHelper::CalculateBallMass(Controller->CurrentBallType);
    
    // 시작점 조정
    FVector AdjustedStartLocation = StartLocation;
    
    // 공이 있으면 공의 실제 중심 위치 계산
    if (Controller->PreviewBall)
    {
        // 공의 크기 및 반지름 계산
        float BallSize = Controller->PreviewBall->GetActorScale3D().X;
        float BallRadius = BallSize * 25.0f;
        
        // 중심 보정치 적용
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
        // 접시 위치 조정
        AActor* PlateActor = PlateActors[0];
        FVector PlateCenter = PlateActor->GetActorLocation();
        PlateCenter.Z += 10.0f;
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
        return;
    
    // 새 궤적 그리기 - 인자 한 줄로
    DrawTrajectoryPath(World, NewTrajectoryPoints, PreciseTargetLocation, true, TrajectoryID);
}

// 하위 호환성을 위한 영구 궤적 그리기 함수 - 인자 한 줄로
void UFruitTrajectoryHelper::DrawPersistentTrajectoryPath(AFruitPlayerController* Controller, const FVector& StartLocation, const FVector& TargetLocation)
{
    // UpdateTrajectoryPath 함수 호출 - 인자 한 줄로
    UpdateTrajectoryPath(Controller, StartLocation, TargetLocation, true);
}