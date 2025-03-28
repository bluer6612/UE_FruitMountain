#include "FruitPhysicsHelper.h"
#include "FruitPlayerController.h"

// 던지기 파라미터 계산 함수 - 질량에 따른 보정으로 모든 공이 접시 중앙에 도달하도록 함
void UFruitPhysicsHelper::CalculateThrowParameters(
    AFruitPlayerController* Controller, 
    const FVector& StartLocation, 
    const FVector& TargetLocation, 
    float& OutAdjustedForce, 
    FVector& OutLaunchDirection,
    float BallMass)
{
    // 기본값 설정
    OutAdjustedForce = 500.0f;
    OutLaunchDirection = FVector::ForwardVector;
    
    if (!Controller)
        return;
    
    // 시작점과 목표점 사이의 방향과 거리 계산
    FVector Direction = TargetLocation - StartLocation;
    float Distance = Direction.Size();
    
    // 목표 지점이 너무 가까우면 기본값 반환
    if (Distance < 50.0f)
        return;
    
    // 방향 벡터 정규화
    Direction.Normalize();

    // --- 투사체 운동 공식 기반 계산 ---
    
    // 중력 상수
    const float GravityZ = -980.0f;
    
    // Controller의 ThrowAngle 값 사용 (20~70도 범위 내에서 제한)
    float OptimalAngle = FMath::Clamp(Controller->ThrowAngle, 20.0f, 70.0f);
    
    // 각도를 라디안으로 변환
    float AngleRad = FMath::DegreesToRadians(OptimalAngle);
    
    // 수평 방향 계산
    FVector HorizontalDir = Direction;
    HorizontalDir.Z = 0.0f;
    HorizontalDir.Normalize();
    
    // Z 방향 조정 - 컨트롤러의 던지기 각도 적용
    float ZComponent = FMath::Sin(AngleRad);
    float HorizontalComponent = FMath::Cos(AngleRad);
    
    // 최종 방향 벡터 구성
    OutLaunchDirection = HorizontalDir * HorizontalComponent + FVector(0, 0, 1) * ZComponent;
    OutLaunchDirection.Normalize();
    
    // --- 질량 기반 힘 계산 (질량이 높을수록 더 적은 힘 적용) ---
    
    // 기본 힘 계산
    float BaseForce = 6000.0f; // 기본 힘
    float DistanceFactor = FMath::Clamp(Distance / 300.0f, 0.8f, 3.0f);
    
    // 중요: 질량 역수 계수 적용 - 질량이 높을수록 힘 감소
    // 표준 질량을 10으로 가정하고, 그것에 맞게 힘 조정
    float StandardMass = 10.0f;
    float MassRatio = StandardMass / BallMass; // 질량이 클수록 작은 값이 됨
    
    // 최종 힘 계산 (질량이 높을수록 힘 감소)
    // 질량이 10배 증가하면 힘은 1/10로 감소
    OutAdjustedForce = BaseForce * DistanceFactor * MassRatio;
    
    // 추가 제한: 최소/최대 힘 설정
    float MinForce = 200.0f;
    float MaxForce = 2000.0f;
    OutAdjustedForce = FMath::Clamp(OutAdjustedForce, MinForce, MaxForce);
    
    // 디버그 로그
    UE_LOG(LogTemp, Warning, TEXT("던지기 계산: 거리=%f, 각도=%f도, 힘=%f, 질량=%f, 질량비율=%f"), 
        Distance, OptimalAngle, OutAdjustedForce, BallMass, MassRatio);
}