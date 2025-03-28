#include "FruitPhysicsHelper.h"
#include "FruitPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Actor.h"
#include "DrawDebugHelpers.h"

// 정적 변수 초기화
float UFruitPhysicsHelper::GlobalThrowAngle = 20.0f;

// 전역 던지기 각도 설정 함수 구현
void UFruitPhysicsHelper::SetGlobalThrowAngle(float Angle)
{
    GlobalThrowAngle = Angle;
}

// 현재 전역 던지기 각도 반환 함수
float UFruitPhysicsHelper::GetGlobalThrowAngle()
{
    return GlobalThrowAngle;
}

// 던지기 파라미터 계산 함수에서 각도 제한 수정
void UFruitPhysicsHelper::CalculateThrowParameters(AFruitPlayerController* Controller, const FVector& StartLocation, const FVector& TargetLocation, float& OutAdjustedForce, FVector& OutLaunchDirection, float BallMass)
{
    // 1. 목표 지점까지의 벡터 계산
    FVector DirectionToTarget = TargetLocation - StartLocation;
    float HorizontalDistance = FVector(DirectionToTarget.X, DirectionToTarget.Y, 0.0f).Size();
    float HeightDifference = DirectionToTarget.Z;
    
    // 2. 던지기 각도 설정 (컨트롤러의 ThrowAngle 직접 사용)
    float UseAngle = FMath::Clamp(Controller->ThrowAngle, 30.0f, 90.0f);  // 각도 범위 보장
    float ThrowAngleRadians = FMath::DegreesToRadians(UseAngle);
    
    // 중력 및 삼각함수 계산
    float Gravity = 980.0f;
    float CosTheta = FMath::Cos(ThrowAngleRadians);
    float TanTheta = FMath::Tan(ThrowAngleRadians);
    
    // 분모 계산 및 유효성 검사
    float Denominator = 2.0f * (CosTheta * CosTheta) * (HorizontalDistance * TanTheta + HeightDifference);
    
    // 분모가 매우 작거나 음수가 되는 경우 처리 (높은 각도 문제)
    if (Denominator <= 1.0f)
    {
        // 대체 계산: 높은 각도에서도 작동하는 공식 사용
        // v₀ = sqrt(g * d / sin(2θ)) 공식 적용 (최대 거리 계산법)
        float Sin2Theta = FMath::Sin(2 * ThrowAngleRadians);
        if (Sin2Theta > 0.1f)  // 0에 가까우면 계산 안정성 위해 제한
        {
            float InitialVelocity = FMath::Sqrt((Gravity * HorizontalDistance) / Sin2Theta);
            OutAdjustedForce = BallMass * InitialVelocity;
        }
        else
        {
            // 안전 기본값
            OutAdjustedForce = 8000.0f;
        }
    }
    else
    {
        // 원래 공식 계산
        float InitialVelocity = FMath::Sqrt((Gravity * HorizontalDistance * HorizontalDistance) / Denominator);
        OutAdjustedForce = BallMass * InitialVelocity;
    }
    
    // 힘 범위 제한 - 높은 각도에 맞게 상한선 증가
    OutAdjustedForce = FMath::Clamp(OutAdjustedForce, 3000.0f, 18000.0f);
    
    // 발사 방향 벡터 계산
    FVector HorizontalDir = FVector(DirectionToTarget.X, DirectionToTarget.Y, 0.0f).GetSafeNormal();
    OutLaunchDirection = FVector(
        HorizontalDir.X,
        HorizontalDir.Y,
        FMath::Tan(ThrowAngleRadians)
    ).GetSafeNormal();
    
    UE_LOG(LogTemp, Warning, TEXT("던지기 계산: 거리=%.1f, 높이차=%.1f, 각도=%.1f도, 힘=%.1f, 질량=%.1f"),
        HorizontalDistance, HeightDifference, UseAngle, OutAdjustedForce, BallMass);
}