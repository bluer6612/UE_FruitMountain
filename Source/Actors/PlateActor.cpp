#include "PlateActor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

APlateActor::APlateActor()
{
    PrimaryActorTick.bCanEverTick = false;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // 테이블 에셋 로드 및 설정
    static ConstructorHelpers::FObjectFinder<UStaticMesh> TableAsset(TEXT("/Game/Asset/Table"));
    if (TableAsset.Succeeded())
    {
        UStaticMeshComponent* TableMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TableMesh"));
        TableMesh->SetupAttachment(RootComponent);
        TableMesh->SetStaticMesh(TableAsset.Object);
        TableMesh->SetWorldScale3D(FVector(10.0f, 10.0f, 1.0f));
        TableMesh->SetWorldLocation(FVector(0.0f, 0.0f, -10.0f));
        TableMesh->SetCollisionProfileName(TEXT("BlockAll"));
        TableMesh->ComponentTags.AddUnique(FName("Object"));
    }

    // 접시 에셋 로드 및 설정
    static ConstructorHelpers::FObjectFinder<UStaticMesh> PlateAsset(TEXT("/Game/Asset/Plate1"));
    if (PlateAsset.Succeeded())
    {
        PlateMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlateMesh"));
        PlateMeshComponent->SetupAttachment(RootComponent);
        PlateMeshComponent->SetStaticMesh(PlateAsset.Object);
        PlateMeshComponent->SetWorldScale3D(PlateScale);
        PlateMeshComponent->SetWorldLocation(PlateLocation);
        PlateMeshComponent->SetCollisionProfileName(TEXT("BlockAll"));
        PlateMeshComponent->ComponentTags.AddUnique(FName("Object"));
    }
    
    // 액터에 태그 추가
    Tags.Add(FName("Plate"));
}

void APlateActor::BeginPlay()
{
    Super::BeginPlay();
    
    // 접시 반경 계산
    CalculatePlateRadius();
}

void APlateActor::CalculatePlateRadius()
{
    // 멤버 변수 직접 사용
    if (PlateMeshComponent)
    {
        FBox PlateBounds = PlateMeshComponent->Bounds.GetBox();
        FVector PlateSize = PlateBounds.GetSize();
        
        // 접시만의 반지름 계산 (X, Y 중 큰 값의 절반)
        PlateRadius = FMath::Max(PlateSize.X, PlateSize.Y) * 0.475f; // 약간 여유를 두고 0.475배
        
        UE_LOG(LogTemp, Warning, TEXT("접시 반경 계산됨: %.1f (X=%.1f, Y=%.1f)"),
            PlateRadius, PlateSize.X, PlateSize.Y);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("접시 메시 컴포넌트가 유효하지 않음"));
    }
}