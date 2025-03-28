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

    PlateMesh->SetWorldScale3D(PlateScale);
    PlateMesh->SetWorldRotation(PlateRotation);
    PlateMesh->SetWorldLocation(PlateLocation);

    PlateMesh->SetCollisionProfileName(TEXT("BlockAll"));
    Tags.Add(FName("Plate"));
}