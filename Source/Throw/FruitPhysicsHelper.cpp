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
    // 목표 지점까지의 벡터 계산
    FVector DirectionToTarget = TargetLocation - StartLocation;
    float HorizontalDistance = FVector(DirectionToTarget.X, DirectionToTarget.Y, 0.0f).Size();
    float HeightDifference = DirectionToTarget.Z;
    
    // 던지기 각도 설정
    float UseAngle = FMath::Clamp(Controller->ThrowAngle, 15.0f, 60.0f);
    float ThrowAngleRadians = FMath::DegreesToRadians(UseAngle);
    
    // 삼각함수 계산
    float CosTheta = FMath::Cos(ThrowAngleRadians);
    float SinTheta = FMath::Sin(ThrowAngleRadians);
    
    // 1. 원하는 초기 속력 계산 (질량 독립적)
    float InitialSpeed;
    
    // 각도에 따른 속도 조정
    if (UseAngle <= 45.0f)
    {
        InitialSpeed = 300.0f;  // 기준 속도
    }
    else
    {
        // 45도 초과: 각도가 높을수록 속도 감소
        float AngleRatio = FMath::GetMappedRangeValueClamped(
            FVector2D(45.0f, 60.0f),
            FVector2D(1.0f, 0.5f),  // 더 강하게 감소 (0.7 -> 0.5)
            UseAngle
        );
        InitialSpeed = 300.0f * AngleRatio;
    }
    
    // 2. 방향 벡터 계산 (모든 질량에 동일)
    FVector HorizontalDir = FVector(DirectionToTarget.X, DirectionToTarget.Y, 0.0f).GetSafeNormal();
    OutLaunchDirection = FVector(
        HorizontalDir.X * CosTheta,
        HorizontalDir.Y * CosTheta,
        SinTheta
    ).GetSafeNormal();
    
    // 3. 힘 = 질량 * 속도
    // 동일한 가속도를 얻기 위해 질량에 비례하게 힘을 조정
    OutAdjustedForce = BallMass * InitialSpeed;
    
    // 4. 안전장치: 너무 작거나 큰 힘은 제한
    float MinForce = 3000.0f;
    float MaxForce = 15000.0f;
    
    // 질량에 관계없이 같은 궤적을 유지하기 위해 힘의 범위를 질량에 비례하게 조정
    float AdjustedMinForce = MinForce * (BallMass / 30.0f);  // 30을 기준 질량으로 설정
    float AdjustedMaxForce = MaxForce * (BallMass / 30.0f);
    
    // 안전장치 적용
    if (OutAdjustedForce < AdjustedMinForce)
    {
        OutAdjustedForce = AdjustedMinForce;
    }
    else if (OutAdjustedForce > AdjustedMaxForce)
    {
        OutAdjustedForce = AdjustedMaxForce;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("던지기 계산: 거리=%.1f, 높이차=%.1f, 각도=%.1f도, 초기속도=%.1f, 힘=%.1f, 질량=%.1f"),
        HorizontalDistance, HeightDifference, UseAngle, InitialSpeed, OutAdjustedForce, BallMass);
}