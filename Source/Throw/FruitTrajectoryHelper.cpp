#include "FruitTrajectoryHelper.h"
#include "FruitPlayerController.h"
#include "FruitThrowHelper.h"
#include "FruitSpawnHelper.h"
#include "Physics/FruitPhysicsHelper.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "DrawDebugHelpers.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Actors/FruitBall.h"

// 포물선 궤적 계산 함수 - 최적화 버전
TArray<FVector> UFruitTrajectoryHelper::CalculateTrajectoryPoints(AFruitPlayerController* Controller, const FVector& StartLocation, const FVector& TargetLocation, float BallMass)
{
    TArray<FVector> TrajectoryPoints;
    
    if (!Controller)
        return TrajectoryPoints;
    
    // 1. FruitPhysicsHelper에서 필요한 던지기 파라미터 가져오기
    float UseAngle = Controller->ThrowAngle;
    float MinAngle, MaxAngle;
    UFruitPhysicsHelper::GetThrowAngleRange(MinAngle, MaxAngle);
    UseAngle = FMath::Clamp(UseAngle, MinAngle, MaxAngle);
    
    // 2. FruitPhysicsHelper에서 초기 속도 계산 함수 호출
    FVector LaunchVelocity;
    
    // 던지기 각도에 따라 속도 조정
    bool bSuccess = UFruitPhysicsHelper::CalculateThrowVelocity(StartLocation, TargetLocation, UseAngle, BallMass, LaunchVelocity);
    
    if (!bSuccess)
    {
        // 계산 실패 시 기본값 사용
        UE_LOG(LogTemp, Warning, TEXT("물리 계산에 실패하여 기본 속도 사용"));
        
        // 기본 방향과 속도 설정
        float ThrowAngleRad = FMath::DegreesToRadians(UseAngle);
        FVector HorizontalDelta = TargetLocation - StartLocation;
        FVector HorizontalDir = FVector(HorizontalDelta.X, HorizontalDelta.Y, 0.0f).GetSafeNormal();
        
        FVector LaunchDirection = FVector(HorizontalDir.X * FMath::Cos(ThrowAngleRad), HorizontalDir.Y * FMath::Cos(ThrowAngleRad), FMath::Sin(ThrowAngleRad)).GetSafeNormal();
        
        LaunchVelocity = LaunchDirection * 300.0f; // 기본 속도
    }
    
    // 3. 시간에 따른 포물선 궤적 계산
    float Gravity = 980.0f; // 중력 가속도
    const float TimeStep = 0.05f;
    const float MaxTime = 5.0f;
    FVector Gravity3D = FVector(0, 0, -Gravity);
    
    // 시작점 추가
    TrajectoryPoints.Add(StartLocation);
    
    // 시간에 따른 위치 계산 (포물선 방정식)
    for (float Time = TimeStep; Time < MaxTime; Time += TimeStep)
    {
        FVector Position = StartLocation + LaunchVelocity * Time + 0.5f * Gravity3D * Time * Time;
        TrajectoryPoints.Add(Position);
        
        // 지면에 닿았거나 목표 근처에 도달했는지 확인
        if (Position.Z <= 0.0f || FVector::Dist(Position, TargetLocation) < 10.0f)
            break;
    }
    
    // 로그 출력
    UE_LOG(LogTemp, Log, TEXT("최적화된 포물선 계산: 각도=%.1f, 속도=%.1f, 방향=%s, 포인트=%d개"),
        UseAngle, LaunchVelocity.Size(), *LaunchVelocity.GetSafeNormal().ToString(), TrajectoryPoints.Num());
    
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
        DrawDebugLine(World, TrajectoryPoints[i], TrajectoryPoints[i + 1], LineColor, true, -1.0f, TrajectoryID, 0.5f);
    }
    
    // 포물선 상의 주요 지점에만 마커 표시
    const int32 MarkerCount = 5; // 마커 개수 줄이기
    for (int32 i = 0; i < MarkerCount; i++)
    {
        int32 Index = (i * (TrajectoryPoints.Num() - 1)) / (MarkerCount - 1);
        if (Index < TrajectoryPoints.Num())
        {
            DrawDebugBox(World, TrajectoryPoints[Index], FVector(0.4f), FQuat::Identity, MarkerColor, true, -1.0f, TrajectoryID);
        }
    }
}

// UpdateTrajectoryPath 함수 수정 부분
void UFruitTrajectoryHelper::UpdateTrajectoryPath(AFruitPlayerController* Controller, const FVector& StartLocation, const FVector& TargetLocation, bool bPersistent, int32 CustomTrajectoryID)
{
    if (!Controller || !Controller->GetWorld())
        return;
    
    UWorld* World = Controller->GetWorld();
    const int32 TrajectoryID = (CustomTrajectoryID != 0) ? CustomTrajectoryID : 9999;
    
    // 기존 궤적 비우기
    FlushPersistentDebugLines(World);
    
    // 현재 각도 가져오기
    float MinAngle, MaxAngle;
    UFruitPhysicsHelper::GetThrowAngleRange(MinAngle, MaxAngle);
    float UseAngle = FMath::Clamp(Controller->ThrowAngle, MinAngle, MaxAngle);
    
    // 시작점과 종료점
    FVector Start = StartLocation;
    FVector End = TargetLocation;
    
    // 접시 높이 정보 가져오기 - 접시 액터 찾기
    float PlateTopHeight = End.Z;
    TArray<AActor*> PlateActors;
    UGameplayStatics::GetAllActorsWithTag(World, FName("Plate"), PlateActors);
    
    if (PlateActors.Num() > 0)
    {
        // 접시 액터의 충돌 정보 활용
        AActor* PlateActor = PlateActors[0];
        FVector PlateOrigin = PlateActor->GetActorLocation();
        FVector PlateExtent;
        PlateActor->GetActorBounds(false, PlateOrigin, PlateExtent);
        
        // 접시 상부 높이 계산 (Z축 중심에서 Z축 범위의 절반을 더함)
        PlateTopHeight = PlateOrigin.Z + PlateExtent.Z;
        
        // 약간의 오프셋 추가 (공이 접시 위에 안정적으로 위치하도록)
        PlateTopHeight += 5.0f;
    }
    else
    {
        // 접시를 찾지 못한 경우 기본값으로 조금 더 높게 설정
        PlateTopHeight = FMath::Max(End.Z, 20.0f);
    }
    
    // 종료점 높이를 접시 상부 높이로 조정
    End.Z = PlateTopHeight;
    
    // 수평 거리
    FVector HorizontalDelta = End - Start;
    float HorizontalDistance = FVector(HorizontalDelta.X, HorizontalDelta.Y, 0.0f).Size();
    
    // 각도에 따른 높이 비율 계산
    float PeakHeightRatio = FMath::GetMappedRangeValueClamped(FVector2D(MinAngle, MaxAngle), FVector2D(0.2f, 0.8f), UseAngle);
    
    // 정점 높이 = 수평 거리 * 높이 비율
    float PeakHeight = HorizontalDistance * PeakHeightRatio;
    
    // 정점 위치 (시작점과 종료점 사이의 중간, 계산된 높이만큼 위로)
    FVector Peak = Start + HorizontalDelta * 0.5f;
    Peak.Z = FMath::Max(Start.Z, End.Z) + PeakHeight;
    
    // 색상 - 더 선명한 하늘색
    FColor LineColor = FColor(135, 206, 235, 180);
    FColor MarkerColor = FColor(135, 206, 235, 220);
    
    // 포물선 세그먼트 수 - 부드러운 곡선을 위해
    const int32 CurveSegments = 18;
    
    // 베지어 곡선으로 포물선 그리기
    for (int32 i = 0; i < CurveSegments; i++)
    {
        float t1 = (float)i / CurveSegments;
        float t2 = (float)(i + 1) / CurveSegments;
        
        // 2차 베지어 곡선 계산 - 완벽한 2차 포물선 형태
        FVector Point1 = FMath::Pow(1.0f - t1, 2) * Start + 2 * t1 * (1.0f - t1) * Peak + t1 * t1 * End;
        FVector Point2 = FMath::Pow(1.0f - t2, 2) * Start + 2 * t2 * (1.0f - t2) * Peak + t2 * t2 * End;
        
        // 선 그리기 - 한 줄로 변경
        DrawDebugLine(World, Point1, Point2, LineColor, true, -1.0f, TrajectoryID, 1.2f);
    }
    
    // 마커는 시작, 중간, 끝 지점만
    const int32 MarkerCount = 3;
    
    for (int32 i = 0; i < MarkerCount; i++)
    {
        float t = (float)i / (MarkerCount - 1);
        
        // 포물선 상의 위치 계산
        FVector Point = FMath::Pow(1.0f - t, 2) * Start + 2 * t * (1.0f - t) * Peak + t * t * End;
        
        // 큐브로 표시 - 한 줄로 변경
        DrawDebugBox(World, Point, FVector(0.8f), FQuat::Identity, MarkerColor, true, -1.0f, TrajectoryID);
    }
    
    // 디버그 로그 수정
    UE_LOG(LogTemp, Log, TEXT("접시 상부 높이=%f로 궤적 계산: 각도=%f, 거리=%f, 높이=%f"), PlateTopHeight, UseAngle, HorizontalDistance, PeakHeight);
}

// 하위 호환성을 위한 영구 궤적 그리기 함수 - 인자 한 줄로
void UFruitTrajectoryHelper::DrawPersistentTrajectoryPath(AFruitPlayerController* Controller, const FVector& StartLocation, const FVector& TargetLocation)
{
    // UpdateTrajectoryPath 함수 호출 - 인자 한 줄로
    UpdateTrajectoryPath(Controller, StartLocation, TargetLocation, true);
}