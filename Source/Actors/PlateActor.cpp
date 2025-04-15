#include "PlateActor.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

APlateActor::APlateActor()
{
    PrimaryActorTick.bCanEverTick = false;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));

    // 테이블 에셋 로드 및 설정
    static ConstructorHelpers::FObjectFinder<UStaticMesh> TableAsset(TEXT("/Game/Asset/Table"));
    if (!TableAsset.Succeeded())
    {
        UE_LOG(LogTemp, Warning, TEXT("TableAsset not found!"));
        return;
    }

    // 테이블 메시 생성 및 설정
    UStaticMeshComponent* TableMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TableMesh"));
    TableMesh->SetupAttachment(RootComponent);
    TableMesh->SetStaticMesh(TableAsset.Object);
    
    // 테이블 위치, 크기, 충돌 설정
    TableMesh->SetWorldScale3D(FVector(10.0f, 10.0f, 1.0f));
    TableMesh->SetWorldLocation(FVector(0.0f, 0.0f, -10.0f));
    TableMesh->SetCollisionProfileName(TEXT("BlockAll"));
    
    // 컴포넌트에 태그 추가
    TableMesh->ComponentTags.AddUnique(FName("Object"));

    // 접시 에셋 로드 및 설정
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
    PlateMesh->SetWorldLocation(PlateLocation);
    PlateMesh->SetCollisionProfileName(TEXT("BlockAll"));
    
    // 컴포넌트에 태그 추가
    PlateMesh->ComponentTags.AddUnique(FName("Object"));

    // 액터에 태그 추가
    Tags.Add(FName("Plate"));
}