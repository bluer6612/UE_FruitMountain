#include "PlateActor.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

APlateActor::APlateActor()
{
    PrimaryActorTick.bCanEverTick = false;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));

    // Bowl 형태 메시를 찾는다 (Starter Content나 프로젝트에 실제로 존재해야 함)
    // 예시 경로: "/Game/StarterContent/Props/SM_Bowl"
    static ConstructorHelpers::FObjectFinder<UStaticMesh> BowlAsset(TEXT("/Game/StarterContent/Props/SM_Bowl"));
    if (!BowlAsset.Succeeded())
    {
        UE_LOG(LogTemp, Warning, TEXT("BowlAsset not found! 경로를 확인하세요."));
        return;
    }

    // 그릇 메시
    UStaticMeshComponent* BowlMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BowlMesh"));
    BowlMesh->SetupAttachment(RootComponent);
    BowlMesh->SetStaticMesh(BowlAsset.Object);

    // 그릇 전체 스케일 조정 (너비, 너비, 높이)
    BowlMesh->SetRelativeScale3D(FVector(2.f, 2.f, 2.f));
    // 접시가 수평으로 놓이도록 회전값 조절 (필요 시 변경)
    BowlMesh->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
    // 중심 위치 조정
    BowlMesh->SetRelativeLocation(FVector(0.f, 0.f, 0.f));

    // 충돌 설정
    BowlMesh->SetCollisionProfileName(TEXT("BlockAll"));

    // 태그 추가(필요 시)
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