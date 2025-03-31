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

// 포물선 궤적 계산 함수 - 완전히 재작성
TArray<FVector> UFruitTrajectoryHelper::CalculateTrajectoryPoints(AFruitPlayerController* Controller, const FVector& StartLocation, const FVector& TargetLocation, float BallMass)
{
    TArray<FVector> TrajectoryPoints;
    
    if (!Controller)
        return TrajectoryPoints;
    
    // 1. 기본 파라미터 설정
    float UseAngle = FMath::Clamp(Controller->ThrowAngle, 15.0f, 60.0f);
    float ThrowAngleRad = FMath::DegreesToRadians(UseAngle);
    float Gravity = 980.0f; // 중력 가속도 (양수로 설정)
    
    // 2. 수평 거리와 높이 차이 계산
    FVector HorizontalDelta = TargetLocation - StartLocation;
    float HorizontalDistance = FVector(HorizontalDelta.X, HorizontalDelta.Y, 0.0f).Size();
    float HeightDifference = TargetLocation.Z - StartLocation.Z;
    
    // 3. 초기 속도 계산 (단순 물리 공식 사용)
    // v₀² = (g * d²) / (2 * cos²θ * (d * tanθ - h))
    float Numerator = Gravity * HorizontalDistance * HorizontalDistance;
    float Denominator = 2.0f * FMath::Cos(ThrowAngleRad) * FMath::Cos(ThrowAngleRad) * 
                       (HorizontalDistance * FMath::Tan(ThrowAngleRad) - HeightDifference);
    
    // 분모가 0이 되지 않도록 보호
    if (FMath::Abs(Denominator) < 0.001f)
        Denominator = 0.001f;
    
    float InitialSpeedSquared = Numerator / Denominator;
    float InitialSpeed;
    
    // 속도가 음수가 나오면 포물선이 도달할 수 없는 것이므로 기본값 사용
    if (InitialSpeedSquared <= 0.0f)
    {
        InitialSpeed = 300.0f; // 기본 속도 사용
        UE_LOG(LogTemp, Warning, TEXT("계산된 초기 속도가 음수입니다. 기본값 사용: %f"), InitialSpeed);
    }
    else
    {
        InitialSpeed = FMath::Sqrt(InitialSpeedSquared);
        
        // 안전 범위 제한
        InitialSpeed = FMath::Clamp(InitialSpeed, 100.0f, 500.0f);
    }
    
    // 각도에 따른 초기 속도 조정 (각도가 높을수록 속도 감소)
    if (UseAngle > 45.0f)
    {
        float AngleRatio = FMath::GetMappedRangeValueClamped(
            FVector2D(45.0f, 60.0f),
            FVector2D(1.0f, 0.7f),
            UseAngle
        );
        InitialSpeed *= AngleRatio;
    }
    
    // 4. 발사 방향 계산
    FVector HorizontalDir = FVector(HorizontalDelta.X, HorizontalDelta.Y, 0.0f).GetSafeNormal();
    FVector LaunchDirection = FVector(
        HorizontalDir.X * FMath::Cos(ThrowAngleRad),
        HorizontalDir.Y * FMath::Cos(ThrowAngleRad),
        FMath::Sin(ThrowAngleRad)
    ).GetSafeNormal();
    
    // 5. 초기 속도 벡터 계산
    FVector InitialVelocity = LaunchDirection * InitialSpeed;
    
    // 6. 시간에 따른 포물선 궤적 계산
    const float TimeStep = 0.05f; // 더 큰 시간 간격으로 포인트 수 줄이기
    const float MaxTime = 5.0f;
    FVector Gravity3D = FVector(0, 0, -Gravity);
    
    // 시작점 추가
    TrajectoryPoints.Add(StartLocation);
    
    // 시간에 따른 위치 계산 (포물선 방정식)
    for (float Time = TimeStep; Time < MaxTime; Time += TimeStep)
    {
        FVector Position = StartLocation + InitialVelocity * Time + 0.5f * Gravity3D * Time * Time;
        TrajectoryPoints.Add(Position);
        
        // 지면에 닿았거나 목표 근처에 도달했는지 확인
        if (Position.Z <= 0.0f || FVector::Dist(Position, TargetLocation) < 10.0f)
            break;
    }
    
    // 7. 로그 출력
    UE_LOG(LogTemp, Log, TEXT("새 포물선 계산: 거리=%.1f, 높이차=%.1f, 각도=%.1f, 속도=%.1f, 방향=%s, 포인트=%d개"),
        HorizontalDistance, HeightDifference, UseAngle, InitialSpeed, *LaunchDirection.ToString(), TrajectoryPoints.Num());
    
    return TrajectoryPoints;
}

// 궤적 그리기 함수 - 단순화 및 명확하게 개선
void UFruitTrajectoryHelper::DrawTrajectoryPath(UWorld* World, const TArray<FVector>& TrajectoryPoints, const FVector& TargetLocation, bool bPersistent, int32 TrajectoryID)
{
    if (!World || TrajectoryPoints.Num() < 2)
        return;
    
    // 기존 궤적 비우기
    FlushPersistentDebugLines(World);
    
    // 하늘색 + 투명도
    FColor LineColor = FColor(135, 206, 235, 100);
    FColor MarkerColor = FColor(135, 206, 235, 180);
    
    // 모든 포인트를 연결하는 선 그리기 (포물선 형태)
    for (int32 i = 0; i < TrajectoryPoints.Num() - 1; i++)
    {
        DrawDebugLine(
            World,
            TrajectoryPoints[i],
            TrajectoryPoints[i + 1],
            LineColor,
            true,  // 영구적
            -1.0f, // 무기한
            TrajectoryID,
            0.5f   // 선 두께
        );
    }
    
    // 포물선 상의 주요 지점에만 마커 표시
    const int32 MarkerCount = 5; // 마커 개수 줄이기
    for (int32 i = 0; i < MarkerCount; i++)
    {
        int32 Index = (i * (TrajectoryPoints.Num() - 1)) / (MarkerCount - 1);
        if (Index < TrajectoryPoints.Num())
        {
            DrawDebugBox(
                World,
                TrajectoryPoints[Index],
                FVector(0.4f), // 작은 크기
                FQuat::Identity,
                MarkerColor,
                true,
                -1.0f,
                TrajectoryID
            );
        }
    }
}

// 기존 UpdateTrajectoryPath 함수를 사용하되 몇 가지 수정
void UFruitTrajectoryHelper::UpdateTrajectoryPath(AFruitPlayerController* Controller, const FVector& StartLocation, const FVector& TargetLocation, bool bPersistent, int32 CustomTrajectoryID)
{
    if (!Controller || !Controller->GetWorld())
        return;
    
    UWorld* World = Controller->GetWorld();
    const int32 TrajectoryID = (CustomTrajectoryID != 0) ? CustomTrajectoryID : 9999;
    
    // 기존 궤적 비우기
    FlushPersistentDebugLines(World);
    
    // 현재 각도 가져오기
    float UseAngle = FMath::Clamp(Controller->ThrowAngle, 15.0f, 60.0f);
    
    // 시작점과 종료점
    FVector Start = StartLocation;
    FVector End = TargetLocation;
    
    // 수평 거리
    FVector HorizontalDelta = End - Start;
    float HorizontalDistance = FVector(HorizontalDelta.X, HorizontalDelta.Y, 0.0f).Size();
    
    // 각도에 따른 높이 비율 계산 (각도가 높을수록 더 높은 포물선)
    float PeakHeightRatio = FMath::GetMappedRangeValueClamped(
        FVector2D(15.0f, 60.0f),  // 각도 범위
        FVector2D(0.2f, 0.8f),    // 높이 비율 범위 (더 낮게 조정)
        UseAngle                  // 현재 각도
    );
    
    // 정점 높이 = 수평 거리 * 높이 비율
    float PeakHeight = HorizontalDistance * PeakHeightRatio;
    
    // 정점 위치 (시작점과 종료점 사이의 중간, 계산된 높이만큼 위로)
    FVector Peak = Start + HorizontalDelta * 0.5f;
    Peak.Z = FMath::Max(Start.Z, End.Z) + PeakHeight;
    
    // 색상 - 반투명 하늘색
    FColor LineColor = FColor(135, 206, 235, 120);  // 더 투명하게
    FColor MarkerColor = FColor(135, 206, 235, 160); // 더 투명하게
    
    // 포물선 세그먼트 수 - 부드러운 곡선을 위해
    const int32 CurveSegments = 15;
    
    // 베지어 곡선으로 포물선 그리기
    for (int32 i = 0; i < CurveSegments; i++)
    {
        float t1 = (float)i / CurveSegments;
        float t2 = (float)(i + 1) / CurveSegments;
        
        // 2차 베지어 곡선 계산 - 완벽한 2차 포물선 형태
        FVector Point1 = FMath::Pow(1.0f - t1, 2) * Start + 
                         2 * t1 * (1.0f - t1) * Peak + 
                         t1 * t1 * End;
                         
        FVector Point2 = FMath::Pow(1.0f - t2, 2) * Start + 
                         2 * t2 * (1.0f - t2) * Peak + 
                         t2 * t2 * End;
        
        // 선 그리기 - 더 얇게
        DrawDebugLine(
            World,
            Point1,
            Point2,
            LineColor,
            true,
            -1.0f,
            TrajectoryID,
            0.5f  // 더 얇은 선
        );
    }
    
    // 마커는 적게, 더 작게
    const int32 MarkerCount = 4;
    
    for (int32 i = 0; i < MarkerCount; i++)
    {
        float t = (float)i / (MarkerCount - 1);
        
        // 포물선 상의 위치 계산
        FVector Point = FMath::Pow(1.0f - t, 2) * Start + 
                        2 * t * (1.0f - t) * Peak + 
                        t * t * End;
        
        // 작은 큐브로 표시
        DrawDebugBox(
            World,
            Point,
            FVector(0.3f),  // 더 작은 크기
            FQuat::Identity,
            MarkerColor,
            true,
            -1.0f,
            TrajectoryID
        );
    }
    
    // 디버그 로그
    UE_LOG(LogTemp, Log, TEXT("개선된 베지어 궤적: 각도=%f, 거리=%f, 높이=%f"), 
        UseAngle, HorizontalDistance, PeakHeight);
}

// 하위 호환성을 위한 영구 궤적 그리기 함수 - 인자 한 줄로
void UFruitTrajectoryHelper::DrawPersistentTrajectoryPath(AFruitPlayerController* Controller, const FVector& StartLocation, const FVector& TargetLocation)
{
    // UpdateTrajectoryPath 함수 호출 - 인자 한 줄로
    UpdateTrajectoryPath(Controller, StartLocation, TargetLocation, true);
}