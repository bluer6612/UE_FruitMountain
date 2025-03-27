#include "PlateActor.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

APlateActor::APlateActor()
{
    PrimaryActorTick.bCanEverTick = false;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderAsset(TEXT("/Engine/BasicShapes/Cylinder"));
    if (!CylinderAsset.Succeeded())
        return;

    // 바닥 (원형 얕은 판)
    UStaticMeshComponent* Bottom = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Bottom"));
    Bottom->SetupAttachment(RootComponent);
    Bottom->SetStaticMesh(CylinderAsset.Object);
    Bottom->SetRelativeScale3D(FVector(4.f, 4.f, 0.2f));
    Bottom->SetRelativeRotation(FRotator(90.f, 0.f, 0.f));
    Bottom->SetRelativeLocation(FVector(0.f, 0.f, -10.f));
    Bottom->SetCollisionProfileName(TEXT("BlockAll"));

    // 원형 U자 테두리를 여러 세그먼트로 생성
    const int32 NumSegments = 16; // 세그먼트 개수
    const float Radius = 200.f;   // 접시 반지름
    const float WallHeight = 50.f;
    const float GapAngle = 60.f;  // 열려 있는 각도(손잡이 부분)

    for (int32 i = 0; i < NumSegments; ++i)
    {
        // i번째 세그먼트의 중심 각도
        float Angle = 360.f * (float)i / NumSegments;

        // U자 모양을 만들기 위해 GapAngle 범위만큼 생략
        // 예: 0~60도 범위를 비움 (각도 비교는 상황에 맞게 조정)
        if (Angle >= 0.f && Angle < GapAngle)
            continue;

        // 위치 및 회전 계산
        float Rad = FMath::DegreesToRadians(Angle);
        float X = FMath::Cos(Rad) * Radius;
        float Y = FMath::Sin(Rad) * Radius;
        float Yaw = Angle;

        // 메시 생성
        FName SegmentName = *FString::Printf(TEXT("WallSegment_%d"), i);
        UStaticMeshComponent* WallSegment = CreateDefaultSubobject<UStaticMeshComponent>(SegmentName);
        WallSegment->SetupAttachment(RootComponent);
        WallSegment->SetStaticMesh(CylinderAsset.Object);
        // 세그먼트 두께/높이 조정
        WallSegment->SetRelativeScale3D(FVector(5.f, 0.1f, WallHeight / 100.f));
        WallSegment->SetRelativeRotation(FRotator(0.f, Yaw, 0.f));
        WallSegment->SetRelativeLocation(FVector(X, Y, WallHeight * 0.5f));
        WallSegment->SetCollisionProfileName(TEXT("BlockAll"));
    }

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