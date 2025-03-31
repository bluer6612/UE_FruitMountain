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
    float UseAngle = FMath::Clamp(Controller->ThrowAngle, 15.0f, 60.0f);  // 각도 범위 보장
    float ThrowAngleRadians = FMath::DegreesToRadians(UseAngle);
    
    // 중력 및 삼각함수 계산
    float Gravity = 980.0f;
    float CosTheta = FMath::Cos(ThrowAngleRadians);
    float TanTheta = FMath::Tan(ThrowAngleRadians);
    
    // 각도에 따른 기본 속도값 설정 (질량 무관)
    float BaseVelocity;
    
    // 각도에 따라 속도 조정 (각도가 높을수록 속도 감소)
    if (UseAngle <= 45.0f)
    {
        // 45도 이하: 강한 투척
        BaseVelocity = 300.0f;
    }
    else
    {
        // 45도 초과: 각도가 높을수록 속도 감소
        float AngleRatio = FMath::GetMappedRangeValueClamped(
            FVector2D(45.0f, 60.0f),
            FVector2D(1.0f, 0.7f),
            UseAngle
        );
        BaseVelocity = 300.0f * AngleRatio;
    }
    
    // 발사 방향 벡터 계산 (각도에 따라)
    FVector HorizontalDir = FVector(DirectionToTarget.X, DirectionToTarget.Y, 0.0f).GetSafeNormal();
    OutLaunchDirection = FVector(
        HorizontalDir.X * CosTheta,
        HorizontalDir.Y * CosTheta,
        FMath::Sin(ThrowAngleRadians)
    ).GetSafeNormal();
    
    // 힘 = 질량 * 고정 속도
    OutAdjustedForce = BallMass * BaseVelocity;
    
    // 힘 범위 제한 (안전장치)
    OutAdjustedForce = FMath::Clamp(OutAdjustedForce, 3000.0f, 15000.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("던지기 계산: 거리=%.1f, 높이차=%.1f, 각도=%.1f도, 기본속도=%.1f, 힘=%.1f, 질량=%.1f"),
        HorizontalDistance, HeightDifference, UseAngle, BaseVelocity, OutAdjustedForce, BallMass);
}