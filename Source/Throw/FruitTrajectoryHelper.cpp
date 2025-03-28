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

// 궤적 생성 및 그리기를 위한 공통 함수
TArray<FVector> UFruitTrajectoryHelper::CalculateTrajectoryPoints(
    AFruitPlayerController* Controller,
    const FVector& StartLocation,
    const FVector& TargetLocation,
    float BallMass)
{
    TArray<FVector> TrajectoryPoints;
    
    if (!Controller)
        return TrajectoryPoints;
    
    // 공통 함수 호출하여 던지기 파라미터 계산 (인자 한 줄로)
    float AdjustedForce;
    FVector LaunchDirection;
    UFruitPhysicsHelper::CalculateThrowParameters(Controller, StartLocation, TargetLocation, AdjustedForce, LaunchDirection, BallMass);
        
    // 초기 속도 계산
    FVector InitialVelocity = LaunchDirection * (AdjustedForce / BallMass);
    
    // 중력 가속도
    float GravityZ = -980.0f;
    FVector Gravity = FVector(0, 0, GravityZ);
    
    // 시작점 추가
    TrajectoryPoints.Add(StartLocation);
    
    // 시간 간격으로 위치 계산
    const float TimeStep = 0.05f; // 더 촘촘하게 포인트 생성
    const float MaxTime = 5.0f;
    
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
        if (DistanceToTarget < 20.0f)
            break;
    }
    
    return TrajectoryPoints;
}

// 확실하게 보이는 궤적 그리기 함수 (인자 한 줄 작성으로 수정)
void UFruitTrajectoryHelper::DrawTrajectoryPath(UWorld* World, const TArray<FVector>& TrajectoryPoints, const FVector& TargetLocation, bool bPersistent, int32 TrajectoryID)
{
    if (!World || TrajectoryPoints.Num() < 2)
    {
        UE_LOG(LogTemp, Error, TEXT("유효하지 않은 파라미터로 궤적 그리기가 실패했습니다."));
        return;
    }
    
    // 항상 영구적으로 표시
    float Duration = -1.0f; // 무기한 지속
    bool PersistentFlag = true; // 항상 영구적
    
    // 색상 설정 - 더 선명한 색상 사용
    FColor LineColor = FColor::Blue;         // 밝은 파란색
    FColor PointColor = FColor::Cyan;        // 청록색 점
    FColor EndColor = FColor::Green;         // 밝은 초록색 끝점
    
    // 선 두께 및 점 크기 - 더 크게 설정
    float LineThickness = 1.5f;  // 더 두꺼운 선
    float PointSize = 5.0f;      // 더 큰 점
    float EndPointSize = 5.0f;  // 더 큰 끝점
    
    // 점 간격 - 더 촘촘하게
    int32 PointInterval = 5;
    
    UE_LOG(LogTemp, Warning, TEXT("궤적 그리기 시작: %d개 포인트"), TrajectoryPoints.Num());
    
    // 새 궤적 그리기
    for (int32 i = 0; i < TrajectoryPoints.Num() - 1; i++)
    {
        // 두 점이 유효한지 확인
        if (!TrajectoryPoints[i].IsZero() && !TrajectoryPoints[i+1].IsZero())
        {
            // 점들을 선으로 연결 (인자 한 줄로)
            DrawDebugLine(World, TrajectoryPoints[i], TrajectoryPoints[i + 1], LineColor, PersistentFlag, Duration, TrajectoryID, LineThickness);
            
            // 디버그 로그 (라인 생성 확인)
            if (i % 10 == 0)
            {
                UE_LOG(LogTemp, Verbose, TEXT("  - 라인 그리기 [%d]: %s -> %s"), 
                    i, *TrajectoryPoints[i].ToString(), *TrajectoryPoints[i+1].ToString());
            }
            
            // 일정 간격으로 구체 그리기 (인자 한 줄로)
            if (i % PointInterval == 0)
            {
                DrawDebugSphere(World, TrajectoryPoints[i], PointSize, 8, PointColor, PersistentFlag, Duration, TrajectoryID, 1.0f);
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("  - 유효하지 않은 궤적 포인트: [%d]=%s, [%d]=%s"), 
                i, *TrajectoryPoints[i].ToString(), i+1, *TrajectoryPoints[i+1].ToString());
        }
    }
    
    // 마지막 지점은 큰 구체로 표시 (인자 한 줄로)
    if (TrajectoryPoints.Num() > 1 && !TrajectoryPoints.Last().IsZero())
    {
        DrawDebugSphere(World, TrajectoryPoints.Last(), EndPointSize, 12, EndColor, PersistentFlag, Duration, TrajectoryID, 1.0f);
    }
}

// 통합된 궤적 그리기 함수 - 공통 로직을 하나로 합침
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
        
        // 디버그 시각화 - 디버깅 모드에서만 수행
        if (Controller->bDebugMode)
        {
            // 디버그 - 공 중심 위치 로깅
            UE_LOG(LogTemp, Warning, TEXT("미리보기 공 위치: %s, 보정된 위치: %s (공 크기: %f, 공 반지름: %f)"), 
                *Controller->PreviewBall->GetActorLocation().ToString(), 
                *AdjustedStartLocation.ToString(),
                BallSize,
                BallRadius);
            
            // 디버그 - 공의 실제 위치와 보정된 위치를 시각적으로 표시
            DrawDebugSphere(World, Controller->PreviewBall->GetActorLocation(), 5.0f, 8, FColor::White, true, -1.0f, 0, 1.0f);
            DrawDebugSphere(World, AdjustedStartLocation, 5.0f, 8, FColor::Yellow, true, -1.0f, 0, 1.0f);
            DrawDebugLine(World, Controller->PreviewBall->GetActorLocation(), AdjustedStartLocation, FColor::Cyan, true, -1.0f, 0, 1.0f);
        }
    }
    else
    {
        UE_LOG(LogTemp, Verbose, TEXT("미리보기 공이 없습니다!"));
    }
    
    // 궤적 계산 (인자 한 줄로)
    TArray<FVector> NewTrajectoryPoints = CalculateTrajectoryPoints(Controller, AdjustedStartLocation, TargetLocation, BallMass);
    
    // 궤적 포인트가 충분한지 확인
    if (NewTrajectoryPoints.Num() < 2)
    {
        UE_LOG(LogTemp, Error, TEXT("궤적 계산 실패: 포인트 개수 = %d"), NewTrajectoryPoints.Num());
        return;
    }
    
    // 새 궤적 그리기 (항상 영구적으로)
    DrawTrajectoryPath(World, NewTrajectoryPoints, TargetLocation, true, TrajectoryID);
    
    // 시작점과 목표점 표시 (인자 한 줄로) - 디버깅 모드에서만 수행
    if (Controller->bDebugMode)
    {
        DrawDebugSphere(World, AdjustedStartLocation, 10.0f, 8, FColor::Yellow, true, -1.0f, 0, 1.0f);
        DrawDebugSphere(World, TargetLocation, 15.0f, 8, FColor::Red, true, -1.0f, 0, 1.0f);
    }
}

// 이전 DrawPersistentTrajectoryPath 함수를 새 함수 호출로 대체
void UFruitTrajectoryHelper::DrawPersistentTrajectoryPath(AFruitPlayerController* Controller, const FVector& StartLocation, const FVector& TargetLocation)
{
    // 통합된 함수 호출 - 영구적 궤적을 위한 ID 0 사용
    UpdateTrajectoryPath(Controller, StartLocation, TargetLocation, true, 0);
}