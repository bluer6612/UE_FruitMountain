#include "FruitPhysicsInitializer.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicsEngine/PhysicsSettings.h"

// 정적 멤버 변수 초기화
FThrowPhysicsResult UFruitPhysicsInitializer::CachedResult;
FVector UFruitPhysicsInitializer::LastStartLocation = FVector::ZeroVector;
FVector UFruitPhysicsInitializer::LastTargetLocation = FVector::ZeroVector;
float UFruitPhysicsInitializer::LastThrowAngle = 0.0f;
float UFruitPhysicsInitializer::LastBallMass = 0.0f;
float UFruitPhysicsInitializer::CacheTimeout = 0.0f;

// 캐시 확인 함수
bool UFruitPhysicsInitializer::CheckCachedResult(const FPhysicsInitData& InitData, FThrowPhysicsResult& OutResult)
{
    // 현재 시간 가져오기
    float CurrentTime = InitData.World ? InitData.World->GetTimeSeconds() : 0.0f;
    
    // 입력 파라미터가 이전과 거의 같고, 캐시가 너무 오래되지 않았으면 캐시된 결과 반환
    if (CachedResult.bSuccess && 
        (LastStartLocation - InitData.StartLocation).Size() < 1.0f &&
        (LastTargetLocation - InitData.TargetLocation).Size() < 1.0f &&
        FMath::Abs(LastThrowAngle - InitData.ThrowAngle) < 0.1f &&
        FMath::Abs(LastBallMass - InitData.BallMass) < 0.1f &&
        (CurrentTime - CacheTimeout) < 0.1f) // 0.1초 이내 캐시만 사용
    {
        OutResult = CachedResult;
        return true;
    }
    
    return false;
}

// 물리 초기화 통합 함수
FPhysicsBaseResult UFruitPhysicsInitializer::InitializePhysics(const FPhysicsInitData& InitData)
{
    FPhysicsBaseResult Result;
    
    // 각 단계 순차적으로 실행
    InitializeAngles(InitData, Result);
    FindPlateInfo(InitData, Result);
    CalculateDirectionAndDistance(InitData, Result);
    CalculateAdjustedTarget(InitData, Result);
    CalculateLaunchDirection(InitData, Result);
    
    return Result;
}

// 각도 초기화 함수
void UFruitPhysicsInitializer::InitializeAngles(const FPhysicsInitData& InitData, FPhysicsBaseResult& Result)
{
    // 각도 값 클램핑
    Result.UseAngle = FMath::Clamp(InitData.ThrowAngle, UFruitPhysicsHelper::MinThrowAngle, UFruitPhysicsHelper::MaxThrowAngle);
    Result.ThrowAngleRad = FMath::DegreesToRadians(Result.UseAngle);
    
    // 중력 값 가져오기
    Result.Gravity = FMath::Abs(GetDefault<UPhysicsSettings>()->DefaultGravityZ);
}

// FindPlateInfo 함수 수정
void UFruitPhysicsInitializer::FindPlateInfo(const FPhysicsInitData& InitData, FPhysicsBaseResult& Result)
{
    // 접시 위치는 TargetLocation으로 전달받음
    Result.PlateCenter = InitData.TargetLocation;
    Result.PlateTopHeight = 0.f;
    
    // 접시 높이 정보만 계산
    if (InitData.World)
    {
        TArray<AActor*> PlateActors;
        UGameplayStatics::GetAllActorsWithTag(InitData.World, FName("Plate"), PlateActors);
        
        if (PlateActors.Num() > 0)
        {
            FVector PlateOrigin;
            FVector PlateExtent;
            PlateActors[0]->GetActorBounds(false, PlateOrigin, PlateExtent);
            
            Result.PlateTopHeight = PlateOrigin.Z + PlateExtent.Z + 5.0f;
        }
    }
}

// 방향 벡터 및 거리 계산 함수
void UFruitPhysicsInitializer::CalculateDirectionAndDistance(const FPhysicsInitData& InitData, FPhysicsBaseResult& Result)
{
    // 접시 중심에서 플레이어까지의 방향 벡터 계산 (Z 성분은 무시)
    FVector DirectionToPlate = Result.PlateCenter - InitData.StartLocation;
    
    // 중요: 수평 거리는 항상 일관되게 계산 (카메라 회전과 무관하게)
    // 실제 XY 평면상의 거리를 사용
    Result.HorizontalDistance = FVector(DirectionToPlate.X, DirectionToPlate.Y, 0.0f).Size();
    
    // 중요: 높이 차이도 일관되게 유지
    Result.HeightDifference = Result.PlateTopHeight - InitData.StartLocation.Z;
    
    // 정규화된 방향 (XY 평면에서만)
    DirectionToPlate.Z = 0.0f;
    if (!DirectionToPlate.IsNearlyZero())
    {
        DirectionToPlate.Normalize();
    }
    else
    {
        // 접시와 플레이어가 같은 위치에 있는 경우를 대비한 안전장치
        DirectionToPlate = FVector(1.0f, 0.0f, 0.0f);
    }
    
    Result.DirectionToTarget = DirectionToPlate;
    
    // 디버그 로깅 추가
    UE_LOG(LogTemp, Verbose, TEXT("방향 계산: 거리=%.1f, 높이차=%.1f, 방향=%s"),
           Result.HorizontalDistance, Result.HeightDifference, *Result.DirectionToTarget.ToString());
}

// 조정된 타겟 위치 계산 함수
void UFruitPhysicsInitializer::CalculateAdjustedTarget(const FPhysicsInitData& InitData, FPhysicsBaseResult& Result)
{
    // 기본 거리 비율 설정
    Result.DistanceRatio = 0.15f;
    
    // 접시 높이에 따른 미세 조정
    if (Result.PlateTopHeight > 30.0f) {
        float HeightFactor = (Result.PlateTopHeight - 30.0f) / 20.0f;
        Result.DistanceRatio *= (1.0f - 0.02f * HeightFactor);
    }
    
    // 타겟 위치 계산
    float AdjustedDistance = Result.HorizontalDistance * Result.DistanceRatio;
    Result.AdjustedTarget = InitData.StartLocation + Result.DirectionToTarget * AdjustedDistance;
    Result.AdjustedTarget.Z = Result.PlateTopHeight;
    
    // 디버그 로깅
    UE_LOG(LogTemp, Warning, TEXT("조정된 타겟: %s (거리비율: %.3f, 거리: %.1f)"),
        *Result.AdjustedTarget.ToString(), Result.DistanceRatio, AdjustedDistance);
}

// 발사 방향 계산 함수
void UFruitPhysicsInitializer::CalculateLaunchDirection(const FPhysicsInitData& InitData, FPhysicsBaseResult& Result)
{
    // 수평 방향 계산
    FVector HorizontalDir = FVector(Result.DirectionToTarget.X, Result.DirectionToTarget.Y, 0.0f).GetSafeNormal();
    
    // 각도에 따른 높이 계수 계산
    Result.HeightFactor = FMath::GetMappedRangeValueClamped(
        FVector2D(UFruitPhysicsHelper::MinThrowAngle, 50.0f),
        FVector2D(0.25f, 1.25f),
        Result.UseAngle
    );
    
    // 수직/수평 성분 계산
    Result.VerticalMultiplier = FMath::Sin(Result.ThrowAngleRad) * Result.HeightFactor;
    Result.HorizontalMultiplier = FMath::Cos(Result.ThrowAngleRad);
    
    // 최종 발사 방향 계산
    Result.LaunchDirection = FVector(
        HorizontalDir.X * Result.HorizontalMultiplier,
        HorizontalDir.Y * Result.HorizontalMultiplier,
        Result.VerticalMultiplier
    ).GetSafeNormal();
    
    // 디버그 로깅
    UE_LOG(LogTemp, Warning, TEXT("발사 각도: %.1f°, 높이계수: %.2f, 수직성분: %.2f, 수평성분: %.2f"),
        Result.UseAngle, Result.HeightFactor, Result.VerticalMultiplier, Result.HorizontalMultiplier);
}

void UFruitPhysicsInitializer::UpdateCachedResult(const FPhysicsInitData& InitData, const FThrowPhysicsResult& Result, float CurrentTime)
{
    LastStartLocation = InitData.StartLocation;
    LastTargetLocation = InitData.TargetLocation;
    LastThrowAngle = InitData.ThrowAngle;
    LastBallMass = InitData.BallMass;
    CachedResult = Result;
    CacheTimeout = CurrentTime;
}