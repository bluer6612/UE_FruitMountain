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

void APlateActor::BeginPlay()
{
    Super::BeginPlay();
    
    // 접시 반경 계산
    CalculatePlateRadius();
}

void APlateActor::CalculatePlateRadius()
{
    UStaticMeshComponent* PlateMeshComponent = FindPlateMeshComponent();
    
    if (PlateMeshComponent)
    {
        FBox PlateBounds = PlateMeshComponent->Bounds.GetBox();
        FVector PlateSize = PlateBounds.GetSize();
        
        // 접시만의 반지름 계산 (X, Y 중 큰 값의 절반)
        PlateRadius = FMath::Max(PlateSize.X, PlateSize.Y) * 0.475f; // 약간 여유를 두고 0.475배
        
        UE_LOG(LogTemp, Verbose, TEXT("접시 반경 계산됨: %.1f (X=%.1f, Y=%.1f)"),
            PlateRadius, PlateSize.X, PlateSize.Y);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("접시 메시 컴포넌트를 찾을 수 없음"));
    }
}

// 태그로 접시 메시 컴포넌트 찾기
UStaticMeshComponent* APlateActor::FindPlateMeshComponent() const
{
    TArray<UActorComponent*> Components;
    GetComponents(UStaticMeshComponent::StaticClass(), Components);
    
    for (UActorComponent* Component : Components)
    {
        UStaticMeshComponent* MeshComp = Cast<UStaticMeshComponent>(Component);
        if (MeshComp && MeshComp->ComponentHasTag(FName("PlateMesh")))
        {
            return MeshComp;
        }
    }
    
    return nullptr;
}