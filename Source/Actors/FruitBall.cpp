#include "FruitBall.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

AFruitBall::AFruitBall()
{
    PrimaryActorTick.bCanEverTick = false;

    // 메시 컴포넌트 생성 및 루트로 설정
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FruitBallMesh"));
    RootComponent = MeshComponent;
    
    // 물리 시뮬레이션 활성화
    MeshComponent->SetSimulatePhysics(true);
    MeshComponent->SetCollisionProfileName(TEXT("PhysicsActor"));

    // 기본 공 메시로 엔진 기본 Sphere 메시 사용 (경로 필요에 따라 수정)
    static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Engine/BasicShapes/Sphere"));
    if (MeshAsset.Succeeded())
    {
        MeshComponent->SetStaticMesh(MeshAsset.Object);
        
        // 기본 크기 15.0f (월드 스케일) 적용
        // 언리얼에서는 100배 축소된 값 사용 - 15.0f → 0.15f
        float ActorScale = BaseBallSize / 100.0f;
        MeshComponent->SetRelativeScale3D(FVector(ActorScale));
    }
}

// 공 타입에 따른 크기 계산 함수 (정적 함수)
float AFruitBall::CalculateBallSize(int32 BallType, float BaseBallScale)
{
    // 각 타입마다 크기 10% 증가 (1.0, 1.1, 1.2, 1.3, ...)
    float GrowthFactor = 1.0f + 0.1f * (BallType - 1);
    
    // 최종 공 크기 계산 (월드 스케일)
    float BallSize = BaseBallScale * GrowthFactor;
    
    return BallSize;
}

// 공 타입과 크기에 따른 질량 계산 함수 (정적 함수)
float AFruitBall::CalculateBallMass(int32 BallType, float BaseBallScale)
{
    // 공 크기 계산 (월드 스케일)
    float BallSize = CalculateBallSize(BallType, BaseBallScale);
    
    // 크기에 비례하여 질량 계산 - 체적 기반 (r^3에 비례)
    float BallRadius = BallSize / 2.0f; // 반지름 (cm)
    float BallVolume = (4.0f/3.0f) * PI * FMath::Pow(BallRadius, 3); // 구의 체적 (cm^3)
    
    // 밀도 계수 (그램/cm^3) - 약 0.3으로 설정하여 15cm 공이 약 1kg이 되도록 함
    float DensityFactor = 0.3f;
    
    // 체적 * 밀도 = 질량 (그램)
    // 킬로그램으로 변환 (1000으로 나눔)
    float BallMass = (BallVolume * DensityFactor) / 1000.0f;
    
    // 최소 질량 보장
    return FMath::Max(BallMass, 0.5f);
}