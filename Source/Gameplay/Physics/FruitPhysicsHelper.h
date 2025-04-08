#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "FruitPhysicsHelper.generated.h"

class AFruitPlayerController;

// 물리 계산 결과를 담을 구조체 추가
USTRUCT(BlueprintType)
struct FThrowPhysicsResult
{
    GENERATED_BODY()
    
    // 조정된 타겟 위치
    UPROPERTY(BlueprintReadWrite, Category = "Physics")
    FVector AdjustedTarget;
    
    // 발사 속도 벡터
    UPROPERTY(BlueprintReadWrite, Category = "Physics")
    FVector LaunchVelocity;
    
    // 발사 방향 (정규화됨)
    UPROPERTY(BlueprintReadWrite, Category = "Physics")
    FVector LaunchDirection;
    
    // 초기 속력 (스칼라)
    UPROPERTY(BlueprintReadWrite, Category = "Physics")
    float InitialSpeed;
    
    // 적용할 힘 (질량 고려)
    UPROPERTY(BlueprintReadWrite, Category = "Physics")
    float AdjustedForce;
    
    // 궤적 최고점 높이
    UPROPERTY(BlueprintReadWrite, Category = "Physics")
    float PeakHeight;
    
    // 계산 성공 여부
    UPROPERTY(BlueprintReadWrite, Category = "Physics")
    bool bSuccess;
    
    FThrowPhysicsResult()
    {
        AdjustedTarget = FVector::ZeroVector;
        LaunchVelocity = FVector::ZeroVector;
        LaunchDirection = FVector::ForwardVector;
        InitialSpeed = 0.0f;
        AdjustedForce = 0.0f;
        PeakHeight = 0.0f;
        bSuccess = false;
    }
};

UCLASS()
class UFruitPhysicsHelper : public UObject
{
    GENERATED_BODY()

public:
    // const 정적 변수 (UPROPERTY 없이)
    static const float MinThrowAngle;
    static const float MaxThrowAngle;
    
    // 각도 제한 확인 함수
    UFUNCTION(BlueprintCallable, Category = "Physics")
    static bool IsAngleInValidRange(float Angle);
    
    // 각도 범위 가져오기
    UFUNCTION(BlueprintCallable, Category = "Physics")
    static void GetThrowAngleRange(float& OutMinAngle, float& OutMaxAngle);
    
    // 속도 벡터 계산 함수
    UFUNCTION(BlueprintCallable, Category = "Physics")
    static bool CalculateThrowVelocity(const FVector& StartLocation, const FVector& TargetLocation, float ThrowAngle, float BallMass, FVector& OutLaunchVelocity);
    
    // 던지기 파라미터 계산 함수
    UFUNCTION(BlueprintCallable, Category = "Physics")
    static void CalculateThrowParameters(AFruitPlayerController* Controller, const FVector& StartLocation, const FVector& TargetLocation, float& OutAdjustedForce, FVector& OutLaunchDirection, float BallMass);
    
    // 접시 정보를 고려한 조정된 타겟 위치 계산
    UFUNCTION(BlueprintCallable, Category = "Physics")
    static FVector CalculateAdjustedTargetLocation(UWorld* World, const FVector& StartLocation, const FVector& TargetLocation, float ThrowAngle, FVector& OutPlateCenter, float& OutPlateTopHeight);
        
    // 포물선 궤적의 피크 높이 계산
    UFUNCTION(BlueprintCallable, Category = "Physics")
    static float CalculateTrajectoryPeakHeight(float HorizontalDistance, float ThrowAngle, float MinAngle, float MaxAngle);
    
    // [NEW] 궤적 계산 함수 - 포물선 물리 (FruitTrajectoryHelper에서 이동)
    UFUNCTION(BlueprintCallable, Category = "Physics")
    static TArray<FVector> CalculateTrajectoryPoints(UWorld* World, const FVector& StartLocation, const FVector& TargetLocation, float ThrowAngle, float BallMass);
    
    // [NEW] 베지어 곡선 계산 함수 (FruitTrajectoryHelper에서 이동)
    UFUNCTION(BlueprintCallable, Category = "Physics")
    static TArray<FVector> CalculateBezierPoints(const FVector& Start, const FVector& End, float PeakHeight, int32 PointCount);
    
    // 통합 물리 계산 함수 (모든 물리 계산의 핵심)
    UFUNCTION(BlueprintCallable, Category = "Physics")
    static FThrowPhysicsResult CalculateThrowPhysics(UWorld* World, const FVector& StartLocation, const FVector& TargetLocation, float ThrowAngle, float BallMass);

private:
    // 내부 헬퍼 함수 - 초기 속도 계산
    static bool CalculateInitialSpeed(const FVector& StartLocation, const FVector& TargetLocation, float ThrowAngle, float& OutInitialSpeed);
    
    // 내부 헬퍼 함수 - 발사 방향 계산
    static FVector CalculateLaunchDirection(const FVector& StartLocation, const FVector& TargetLocation, float ThrowAngleRad);
};