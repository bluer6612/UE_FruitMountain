#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "FruitPhysicsHelper.h"
#include "FruitPhysicsInitializer.generated.h"

// 초기 물리 계산에 필요한 입력 데이터 구조체
USTRUCT()
struct FPhysicsInitData
{
    GENERATED_BODY()
    
    UWorld* World;
    FVector StartLocation;
    FVector TargetLocation;
    float ThrowAngle;
    float BallMass;
    
    FPhysicsInitData() : 
        World(nullptr),
        StartLocation(FVector::ZeroVector),
        TargetLocation(FVector::ZeroVector),
        ThrowAngle(30.0f),
        BallMass(30.0f) {}
        
    FPhysicsInitData(UWorld* InWorld, const FVector& InStart, const FVector& InTarget, float InAngle, float InMass) :
        World(InWorld),
        StartLocation(InStart),
        TargetLocation(InTarget),
        ThrowAngle(InAngle),
        BallMass(InMass) {}
};

// 물리 계산 중간 결과 데이터 구조체
USTRUCT()
struct FPhysicsBaseResult
{
    GENERATED_BODY()
    
    float UseAngle;
    float ThrowAngleRad;
    FVector PlateCenter;
    float PlateTopHeight;
    FVector DirectionToTarget;
    float HorizontalDistance;
    float HeightDifference;
    float Gravity;
    float DistanceRatio;
    FVector AdjustedTarget;
    float HeightFactor;
    FVector LaunchDirection;
    float VerticalMultiplier;
    float HorizontalMultiplier;
    
    FPhysicsBaseResult() :
        UseAngle(30.0f),
        ThrowAngleRad(FMath::DegreesToRadians(30.0f)),
        PlateCenter(FVector::ZeroVector),
        PlateTopHeight(20.0f),
        DirectionToTarget(FVector::ForwardVector),
        HorizontalDistance(0.0f),
        HeightDifference(0.0f),
        Gravity(980.0f),
        DistanceRatio(0.15f),
        AdjustedTarget(FVector::ZeroVector),
        HeightFactor(1.0f),
        LaunchDirection(FVector::ForwardVector),
        VerticalMultiplier(0.5f),
        HorizontalMultiplier(0.866f) {}
};

// 물리 초기화 헬퍼 클래스
UCLASS()
class UE_FRUITMOUNTAIN_API UFruitPhysicsInitializer : public UObject
{
    GENERATED_BODY()
    
public:
    // 캐시 확인 함수
    static bool CheckCachedResult(
        const FPhysicsInitData& InitData, 
        FThrowPhysicsResult& OutResult);
    
    // 물리 초기화 통합 함수 - 섹션 0~6 모두 처리
    static FPhysicsBaseResult InitializePhysics(
        const FPhysicsInitData& InitData);
    
private:
    // 개별 단계 함수들
    static void InitializeAngles(
        const FPhysicsInitData& InitData, 
        FPhysicsBaseResult& Result);
        
    static void FindPlateInfo(
        const FPhysicsInitData& InitData, 
        FPhysicsBaseResult& Result);
        
    static void CalculateDirectionAndDistance(
        const FPhysicsInitData& InitData, 
        FPhysicsBaseResult& Result);
        
    static void CalculateAdjustedTarget(
        const FPhysicsInitData& InitData, 
        FPhysicsBaseResult& Result);
        
    static void CalculateLaunchDirection(
        const FPhysicsInitData& InitData, 
        FPhysicsBaseResult& Result);
        
    // 캐시 관련 정적 변수
    static FThrowPhysicsResult CachedResult;
    static FVector LastStartLocation;
    static FVector LastTargetLocation;
    static float LastThrowAngle;
    static float LastBallMass;
    static float CacheTimeout;
};