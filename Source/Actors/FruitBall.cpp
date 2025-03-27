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
        // 기본 크기 조정 (필요 시 수정)
        MeshComponent->SetRelativeScale3D(FVector(0.5f));
    }
}

void AFruitBall::BeginPlay()
{
    Super::BeginPlay();
}

void AFruitBall::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}