#include "FruitPhysicsHelper.h"
#include "Gameplay/Controller/FruitPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Actor.h"
#include "DrawDebugHelpers.h"

// const 정적 변수 초기화
const float UFruitPhysicsHelper::MinThrowAngle = 5.0f;
const float UFruitPhysicsHelper::MaxThrowAngle = 60.0f;

// 각도 범위 가져오기
void UFruitPhysicsHelper::GetThrowAngleRange(float& OutMinAngle, float& OutMaxAngle)
{
    OutMinAngle = MinThrowAngle;
    OutMaxAngle = MaxThrowAngle;
}

// 각도 제한 확인 함수
bool UFruitPhysicsHelper::IsAngleInValidRange(float Angle)
{
    return (Angle >= MinThrowAngle && Angle <= MaxThrowAngle);
}

// 기본 물리 계산 헬퍼 함수 (내부용)
bool UFruitPhysicsHelper::CalculateInitialSpeed(const FVector& StartLocation, const FVector& TargetLocation, float ThrowAngle, float& OutInitialSpeed)
{
    // 1. 기본 파라미터 설정
    float ThrowAngleRad = FMath::DegreesToRadians(ThrowAngle);
    float Gravity = 980.0f; // 중력 가속도 (양수로 설정)
    
    // 2. 수평 거리와 높이 차이 계산
    FVector HorizontalDelta = TargetLocation - StartLocation;
    float HorizontalDistance = FVector(HorizontalDelta.X, HorizontalDelta.Y, 0.0f).Size();
    float HeightDifference = TargetLocation.Z - StartLocation.Z;
    
    // 3. 초기 속도 계산 (단순 물리 공식 사용)
    float Numerator = Gravity * HorizontalDistance * HorizontalDistance;
    float Denominator = 2.0f * FMath::Cos(ThrowAngleRad) * FMath::Cos(ThrowAngleRad) * 
                       (HorizontalDistance * FMath::Tan(ThrowAngleRad) - HeightDifference);
    
    // 분모가 0이 되지 않도록 보호
    if (FMath::Abs(Denominator) < 0.001f)
    {
        Denominator = 0.001f;
    }
    
    float InitialSpeedSquared = Numerator / Denominator;
    
    // 속도가 음수가 나오면 포물선이 도달할 수 없는 것이므로 실패 반환
    if (InitialSpeedSquared <= 0.0f)
    {
        return false;
    }
    
    OutInitialSpeed = FMath::Sqrt(InitialSpeedSquared);
    
    // 안전 범위 제한
    OutInitialSpeed = FMath::Clamp(OutInitialSpeed, 100.0f, 500.0f);
    
    // 각도에 따른 초기 속도 조정 (각도가 높을수록 속도 감소)
    if (ThrowAngle > 45.0f)
    {
        float AngleRatio = FMath::GetMappedRangeValueClamped(FVector2D(45.0f, 60.0f), FVector2D(1.0f, 0.7f), ThrowAngle);
        OutInitialSpeed *= AngleRatio;
    }
    
    return true;
}

// 발사 방향 계산 함수 분리 (내부용)
FVector UFruitPhysicsHelper::CalculateLaunchDirection(const FVector& StartLocation, const FVector& TargetLocation, float ThrowAngleRad)
{
    FVector DirectionToTarget = TargetLocation - StartLocation;
    FVector HorizontalDir = FVector(DirectionToTarget.X, DirectionToTarget.Y, 0.0f).GetSafeNormal();
    
    // ThrowAngleRadians를 ThrowAngleRad로 수정
    return FVector(HorizontalDir.X * FMath::Cos(ThrowAngleRad), HorizontalDir.Y * FMath::Cos(ThrowAngleRad), FMath::Sin(ThrowAngleRad)).GetSafeNormal();
}

// 던지기 속도 계산 함수 - 통합 코드 사용
bool UFruitPhysicsHelper::CalculateThrowVelocity(const FVector& StartLocation, const FVector& TargetLocation, float ThrowAngle, float BallMass, FVector& OutLaunchVelocity)
{
    // 물리 계산 결과 사용
    FThrowPhysicsResult PhysicsResult = CalculateThrowPhysics(nullptr, StartLocation, TargetLocation, ThrowAngle, BallMass);
    
    if (PhysicsResult.bSuccess)
    {
        OutLaunchVelocity = PhysicsResult.LaunchVelocity;
        return true;
    }
    
    // 실패 시 기본값
    float ThrowAngleRad = FMath::DegreesToRadians(ThrowAngle);
    FVector LaunchDirection = CalculateLaunchDirection(StartLocation, TargetLocation, ThrowAngleRad);
    OutLaunchVelocity = LaunchDirection * 200.0f;
    
    return false;
}

// 던지기 파라미터 계산 함수 - 단일 오류 수정
void UFruitPhysicsHelper::CalculateThrowParameters(AFruitPlayerController* Controller, const FVector& StartLocation, const FVector& TargetLocation, float& OutAdjustedForce, FVector& OutLaunchDirection, float BallMass)
{
    if (!Controller) return;
    
    float UseAngle = FMath::Clamp(Controller->ThrowAngle, MinThrowAngle, MaxThrowAngle);
    
    // 물리 계산 결과 사용
    FThrowPhysicsResult PhysicsResult = CalculateThrowPhysics(
        Controller->GetWorld(), StartLocation, TargetLocation, UseAngle, BallMass);
    
    // 결과 반환
    OutLaunchDirection = PhysicsResult.LaunchDirection;
    OutAdjustedForce = PhysicsResult.AdjustedForce;
    
    UE_LOG(LogTemp, Warning, TEXT("던지기 파라미터: 각도=%.1f, 속도=%.1f, 힘=%.1f, 질량=%.1f"),
        UseAngle, PhysicsResult.InitialSpeed, OutAdjustedForce, BallMass);
}

// 물리 헬퍼 클래스에 새 함수 구현 추가
FVector UFruitPhysicsHelper::CalculateAdjustedTargetLocation(UWorld* World, const FVector& StartLocation, const FVector& TargetLocation, float ThrowAngle, FVector& OutPlateCenter, float& OutPlateTopHeight)
{
    // 기본 물리 계산 결과 가져오기
    FThrowPhysicsResult PhysicsResult = CalculateThrowPhysics(World, StartLocation, TargetLocation, ThrowAngle, 30.0f);
    
    // 접시 정보 찾기 (플레이트 액터 검색)
    if (World)
    {
        TArray<AActor*> PlateActors;
        UGameplayStatics::GetAllActorsWithTag(World, FName("Plate"), PlateActors);
        
        if (PlateActors.Num() > 0)
        {
            AActor* PlateActor = PlateActors[0];
            FVector PlateOrigin;
            FVector PlateExtent;
            PlateActor->GetActorBounds(false, PlateOrigin, PlateExtent);
            
            OutPlateCenter = PlateOrigin;
            OutPlateTopHeight = PlateOrigin.Z + PlateExtent.Z + 5.0f;
        }
        else
        {
            OutPlateCenter = TargetLocation;
            OutPlateTopHeight = TargetLocation.Z;
        }
    }
    else
    {
        OutPlateCenter = TargetLocation;
        OutPlateTopHeight = TargetLocation.Z;
    }
    
    return PhysicsResult.AdjustedTarget;
}

float UFruitPhysicsHelper::CalculateTrajectoryPeakHeight(float HorizontalDistance, float ThrowAngle, float MinAngle, float MaxAngle)
{
    // 각도에 따른 높이 비율 계산 (각도가 높을수록 더 높게)
    float PeakHeightRatio = FMath::GetMappedRangeValueClamped(
        FVector2D(MinAngle, MaxAngle), FVector2D(0.15f, 0.9f), ThrowAngle
    );
    
    // 정점 높이 = 수평 거리 * 높이 비율
    return HorizontalDistance * PeakHeightRatio;
}

// CalculateTrajectoryPoints 함수 수정
TArray<FVector> UFruitPhysicsHelper::CalculateTrajectoryPoints(UWorld* World, const FVector& StartLocation, const FVector& TargetLocation, float ThrowAngle, float BallMass)
{
    TArray<FVector> TrajectoryPoints;
    
    if (!World)
        return TrajectoryPoints;
    
    // 물리 계산 결과 사용
    FThrowPhysicsResult PhysicsResult = CalculateThrowPhysics(
        World, StartLocation, TargetLocation, ThrowAngle, BallMass);
    
    // 계산된 속도로 포물선 궤적 생성
    float Gravity = 980.0f;
    const float TimeStep = 0.05f;
    const float MaxTime = 5.0f;
    FVector Gravity3D = FVector(0, 0, -Gravity);
    
    // 시작점 추가
    TrajectoryPoints.Add(StartLocation);
    
    // 시간에 따른 위치 계산
    for (float Time = TimeStep; Time < MaxTime; Time += TimeStep)
    {
        FVector Position = StartLocation + PhysicsResult.LaunchVelocity * Time + 0.5f * Gravity3D * Time * Time;
        TrajectoryPoints.Add(Position);
        
        // 지면에 닿았거나 목표 근처에 도달했는지 확인
        if (Position.Z <= 0.0f || FVector::Dist(Position, PhysicsResult.AdjustedTarget) < 10.0f)
            break;
    }
    
    return TrajectoryPoints;
}

// CalculateBezierPoints 함수를 FruitTrajectoryHelper에서 이동
TArray<FVector> UFruitPhysicsHelper::CalculateBezierPoints(const FVector& Start, const FVector& End, float PeakHeight, int32 PointCount)
{
    TArray<FVector> Points;
    Points.Reserve(PointCount);
    
    // 정점 위치 계산
    FVector HorizontalDelta = End - Start;
    FVector Peak = Start + HorizontalDelta * 0.5f;
    Peak.Z = FMath::Max(Start.Z, End.Z) + PeakHeight;
    
    // 베지어 곡선 포인트 생성
    for (int32 i = 0; i < PointCount; i++)
    {
        float t = (float)i / (PointCount - 1);
        FVector Point = FMath::Pow(1.0f - t, 2) * Start + 2 * t * (1.0f - t) * Peak + t * t * End;
        Points.Add(Point);
    }
    
    return Points;
}

// 통합 물리 계산 함수 구현
FThrowPhysicsResult UFruitPhysicsHelper::CalculateThrowPhysics(
    UWorld* World, 
    const FVector& StartLocation, 
    const FVector& TargetLocation, 
    float ThrowAngle, 
    float BallMass)
{
    FThrowPhysicsResult Result;
    
    // 1. 각도 범위 가져오기 & 조정
    float MinAngle, MaxAngle;
    GetThrowAngleRange(MinAngle, MaxAngle);
    float UseAngle = FMath::Clamp(ThrowAngle, MinAngle, MaxAngle);
    float ThrowAngleRad = FMath::DegreesToRadians(UseAngle);
    
    // 2. 접시 정보 찾기
    FVector PlateCenter = TargetLocation;
    float PlateTopHeight = 20.0f;
    
    if (World)
    {
        TArray<AActor*> PlateActors;
        UGameplayStatics::GetAllActorsWithTag(World, FName("Plate"), PlateActors);
        
        if (PlateActors.Num() > 0)
        {
            AActor* PlateActor = PlateActors[0];
            FVector PlateOrigin;
            FVector PlateExtent;
            PlateActor->GetActorBounds(false, PlateOrigin, PlateExtent);
            
            PlateCenter = PlateOrigin;
            PlateTopHeight = PlateOrigin.Z + PlateExtent.Z + 5.0f;
        }
    }
    
    // 3. 방향 벡터 계산
    FVector DirectionToTarget = PlateCenter - StartLocation;
    float HorizontalDistance = FVector(DirectionToTarget.X, DirectionToTarget.Y, 0.0f).Size();
    float HeightDifference = PlateTopHeight - StartLocation.Z;
    DirectionToTarget.Z = 0.0f;
    DirectionToTarget.Normalize();
    
    // 4. 각도에 따른 비율 계산
    float OptimalAngle = (MinAngle + MaxAngle) * 0.5f;
    float AngleDeviation = FMath::Abs(UseAngle - OptimalAngle) / ((MaxAngle - MinAngle) * 0.5f);
    float DistanceRatio = FMath::Clamp(1.0f - AngleDeviation * 0.7f, 0.3f, 1.0f);
    
    // 5. 조정된 타겟 위치 계산
    float AdjustedDistance = HorizontalDistance * DistanceRatio;
    Result.AdjustedTarget = StartLocation + DirectionToTarget * AdjustedDistance;
    Result.AdjustedTarget.Z = PlateTopHeight;
    
    // 6. 발사 방향 계산
    FVector HorizontalDir = FVector(DirectionToTarget.X, DirectionToTarget.Y, 0.0f).GetSafeNormal();
    Result.LaunchDirection = FVector(
        HorizontalDir.X * FMath::Cos(ThrowAngleRad),
        HorizontalDir.Y * FMath::Cos(ThrowAngleRad),
        FMath::Sin(ThrowAngleRad)
    ).GetSafeNormal();
    
    // 7. 초기 속도 계산 (포물선 공식)
    float Gravity = 980.0f; // 중력 가속도
    float AdjustedHorizontalDistance = AdjustedDistance;
    float AdjustedHeightDifference = Result.AdjustedTarget.Z - StartLocation.Z;
    
    // 포물선 방정식 사용 - v²cos²(θ) = g*x²/(2*(x*tan(θ) - h))
    float InitialSpeedSquared = (Gravity * AdjustedHorizontalDistance * AdjustedHorizontalDistance) / 
                               (2.0f * FMath::Cos(ThrowAngleRad) * FMath::Cos(ThrowAngleRad) * 
                               (AdjustedHorizontalDistance * FMath::Tan(ThrowAngleRad) - AdjustedHeightDifference));
    
    // 음수 체크 및 안전장치
    if (InitialSpeedSquared <= 0.0f)
    {
        // 각도에 따른 기본 속도 사용
        if (UseAngle <= 45.0f)
        {
            Result.InitialSpeed = 200.0f;
        }
        else
        {
            float AngleRatio = FMath::GetMappedRangeValueClamped(FVector2D(45.0f, 60.0f), FVector2D(1.0f, 0.7f), UseAngle);
            Result.InitialSpeed = 200.0f * AngleRatio;
        }
    }
    else
    {
        Result.InitialSpeed = FMath::Sqrt(InitialSpeedSquared);
        Result.InitialSpeed = FMath::Clamp(Result.InitialSpeed, 100.0f, 500.0f);
        
        // 각도에 따른 초기 속도 조정
        if (UseAngle > 45.0f)
        {
            float AngleRatio = FMath::GetMappedRangeValueClamped(FVector2D(45.0f, 60.0f), FVector2D(1.0f, 0.7f), UseAngle);
            Result.InitialSpeed *= AngleRatio;
        }
    }
    
    // 8. 발사 속도 벡터 계산
    Result.LaunchVelocity = Result.LaunchDirection * Result.InitialSpeed;
    
    // 9. 적용할 힘 계산 (질량 고려)
    float ForceAdjustment = 0.6f;
    Result.AdjustedForce = BallMass * Result.InitialSpeed * ForceAdjustment;
    
    // 10. 힘 범위 제한
    float MinForce = 1500.0f;
    float MaxForce = 7500.0f;
    float AdjustedMinForce = MinForce * (BallMass / 30.0f);
    float AdjustedMaxForce = MaxForce * (BallMass / 30.0f);
    
    Result.AdjustedForce = FMath::Clamp(Result.AdjustedForce, AdjustedMinForce, AdjustedMaxForce);
    
    // 11. 궤적 최고점 높이 계산
    float PeakHeightRatio = FMath::GetMappedRangeValueClamped(
        FVector2D(MinAngle, MaxAngle), FVector2D(0.15f, 0.9f), UseAngle);
    Result.PeakHeight = HorizontalDistance * PeakHeightRatio;
    
    // 12. 계산 성공 표시
    Result.bSuccess = true;
    
    UE_LOG(LogTemp, Log, TEXT("물리 계산: 각도=%.1f°, 속도=%.1f, 힘=%.1f, 질량=%.1f"),
        UseAngle, Result.InitialSpeed, Result.AdjustedForce, BallMass);
    
    return Result;
}