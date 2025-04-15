#include "FruitBall.h"
#include "Components/StaticMeshComponent.h"
#include "Gameplay/Fruit/FruitCollisionHelper.h"
#include "Gameplay/Fruit/FruitMergeHelper.h"

AFruitBall::AFruitBall()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // 기본값 설정
    BallType = 1;
    bIsBeingMerged = false;
    
    // 메시 컴포넌트 생성 및 루트로 설정
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FruitBallMesh"));
    RootComponent = MeshComponent;
    
    // 물리 시뮬레이션 활성화
    MeshComponent->SetSimulatePhysics(true);
    MeshComponent->SetCollisionProfileName(TEXT("PhysicsActor"));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Game/Asset/Fruit/Fruit2"));
    if (MeshAsset.Succeeded())
    {
        MeshComponent->SetStaticMesh(MeshAsset.Object);
    }

    // 물리적 감쇠 설정
    MeshComponent->SetLinearDamping(0.0f);
    MeshComponent->SetAngularDamping(0.0f);
}

void AFruitBall::BeginPlay()
{
    Super::BeginPlay();
    
    if (MeshComponent)
    {
        // 물리 시뮬레이션 확인
        MeshComponent->SetSimulatePhysics(true);
        
        // 모든 채널에 대해 충돌 설정
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComponent->SetCollisionObjectType(ECC_PhysicsBody);
        
        // 기본 PhysicsActor 프로필 사용
        MeshComponent->SetCollisionProfileName(TEXT("PhysicsActor"));
        
        // 충돌 응답 수동 설정
        MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
        
        // 충돌 이벤트 활성화
        MeshComponent->SetNotifyRigidBodyCollision(true);
    }
}

// 공 크기 계산 함수 구현 - 이미 언리얼 스케일로 반환
float AFruitBall::CalculateBallSize(int32 BallType)
{
    // 과일 레벨에 따른 크기 선형 상수 증가 (UE 단위로 직접 반환)
    // 레벨 1: 15cm, 레벨 2: 16.5cm, ...
    return BaseBallSize + ((BallType - 1) * BaseBallSize * 0.2f);
}

// 공 질량 계산 함수 구현
float AFruitBall::CalculateBallMass(int32 BallType)
{
    // 과일 레벨에 따른 질량 지수 증가 (예시 공식)
    // 레벨 1: 100kg, 레벨 2: 102.5kg, ...
    return DensityFactor * FMath::Pow(1.025f, BallType - 1);
}

// 델리게이트 타겟으로 사용
void AFruitBall::OnBallHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, 
                         UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    // 접시 또는 테이블과의 충돌은 무시
    if (OtherActor && (OtherComp->ComponentHasTag("Object")))
    {
        return;
    }

    // 충돌한 액터가 과일인지 확인
    AFruitBall* OtherFruit = Cast<AFruitBall>(OtherActor);
    if (OtherFruit)
    {
        // 충돌 처리는 CollisionHelper로 위임
        UFruitCollisionHelper::HandleFruitCollision(this, OtherFruit, Hit);
    }
}

void AFruitBall::DisplayDebugInfo()
{
    if (GEngine)
    {
        // GetName() 대신 안전한 방법으로 이름 가져오기
        FString ActorName = GetNameSafe(this);
        FString DebugMsg = FString::Printf(TEXT("과일 ID: %s, 레벨: %d"), *ActorName, BallType);
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, DebugMsg);
    }
}