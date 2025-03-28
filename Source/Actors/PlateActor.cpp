#include "PlateActor.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

APlateActor::APlateActor()
{
    PrimaryActorTick.bCanEverTick = false;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> PlateAsset(TEXT("/Game/Asset/Plate1"));
    if (!PlateAsset.Succeeded())
    {
        UE_LOG(LogTemp, Warning, TEXT("PlateAsset not found!"));
        return;
    }

    UStaticMeshComponent* PlateMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlateMesh"));
    PlateMesh->SetupAttachment(RootComponent);
    PlateMesh->SetStaticMesh(PlateAsset.Object);

    // 원판을 얇게, 25.f 위에 배치
    PlateMesh->SetRelativeScale3D(FVector(20.f, 20.f, 0.1f));
    PlateMesh->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
    PlateMesh->SetRelativeLocation(FVector(0.f, 0.f, 25.f));

    PlateMesh->SetCollisionProfileName(TEXT("BlockAll"));
    Tags.Add(FName("Plate"));
}

void APlateActor::BeginPlay()
{
    Super::BeginPlay();
}

void APlateActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}