#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "FruitThrowHelper.h"
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
    
    // 통합 물리 계산 함수 (모든 물리 계산의 핵심)
    UFUNCTION(BlueprintCallable, Category = "Physics")
    static FThrowPhysicsResult CalculateThrowPhysics(UWorld* World, const FVector& StartLocation, const FVector& TargetLocation, float ThrowAngle, float BallMass);

private:
    // 내부 헬퍼 함수 - 발사 방향 계산
    static FVector CalculateLaunchDirection(const FVector& StartLocation, const FVector& TargetLocation, float ThrowAngleRad);
};