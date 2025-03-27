#include "PlateActor.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

APlateActor::APlateActor()
{
    PrimaryActorTick.bCanEverTick = true;

    // 루트
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));

    // 바닥 메시
    BottomMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BottomMesh"));
    BottomMesh->SetupAttachment(RootComponent);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeAsset.Succeeded())
    {
        BottomMesh->SetStaticMesh(CubeAsset.Object);
        // 바닥은 4배 확대, 폭을 넓게
        BottomMesh->SetRelativeScale3D(FVector(4.f, 4.f, 0.1f));
        BottomMesh->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
    }

    // 왼쪽 벽
    LeftWallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftWallMesh"));
    LeftWallMesh->SetupAttachment(RootComponent);
    if (CubeAsset.Succeeded())
    {
        LeftWallMesh->SetStaticMesh(CubeAsset.Object);
        // 벽도 4배 확대, 높이를 크게. 살짝 좌측으로 이동.
        LeftWallMesh->SetRelativeScale3D(FVector(4.f, 0.2f, 1.f));
        LeftWallMesh->SetRelativeLocation(FVector(0.f, -190.f, 50.f)); 
    }

    // 오른쪽 벽
    RightWallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightWallMesh"));
    RightWallMesh->SetupAttachment(RootComponent);
    if (CubeAsset.Succeeded())
    {
        RightWallMesh->SetStaticMesh(CubeAsset.Object);
        // 벽도 4배 확대, 높이를 크게. 살짝 우측으로 이동.
        RightWallMesh->SetRelativeScale3D(FVector(4.f, 0.2f, 1.f));
        RightWallMesh->SetRelativeLocation(FVector(0.f, 190.f, 50.f));
    }
}

void APlateActor::BeginPlay()
{
    Super::BeginPlay();
}

void APlateActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}