#include "PlateActor.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

APlateActor::APlateActor()
{
    PrimaryActorTick.bCanEverTick = false;

    // 기본 메시 컴포넌트를 생성 및 설정 (예: 원통형 메시 사용)
    UStaticMeshComponent* MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlateMesh"));
    RootComponent = MeshComp;

    // 기본 메시(원통)를 찾아 설정 (엔진 기본 제공 메시 사용, 필요에 따라 경로 수정)
    static ConstructorHelpers::FObjectFinder<UStaticMesh> PlateMeshObj(TEXT("/Engine/BasicShapes/Cylinder"));
    if (PlateMeshObj.Succeeded())
    {
        MeshComp->SetStaticMesh(PlateMeshObj.Object);
        // 메시 크기를 조정하여 접시 모양으로 만듦 (예: X, Y 방향으로 크게, Z 방향으로 얇게)
        MeshComp->SetRelativeScale3D(FVector(2.f, 2.f, 0.2f));
    }
}

void APlateActor::BeginPlay()
{
    Super::BeginPlay();
    // 추가 초기화 코드 (필요 시)
}