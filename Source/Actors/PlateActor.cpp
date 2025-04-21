#include "PlateActor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

// 정적 멤버 변수 초기화
FVector APlateActor::LastSpawnPos = FVector::ZeroVector;
float APlateActor::LastAngle = -999.0f;

APlateActor::APlateActor()
{
    PrimaryActorTick.bCanEverTick = false;
    
    // 루트 컴포넌트 생성
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    
    // 접시 메시 컴포넌트 생성
    PlateMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlateMesh"));
    PlateMeshComponent->SetupAttachment(RootComponent);
    
    // 기본 접시 반경 설정 (실제 계산은 BeginPlay에서)
    PlateRadius = 300.0f;
    
    // 이 액터에 "Plate" 태그 추가
    Tags.Add("Plate");
}

void APlateActor::BeginPlay()
{
    Super::BeginPlay();
    
    // 접시 반경 계산
    CalculatePlateRadius();
}

void APlateActor::CalculatePlateRadius()
{
    if (PlateMeshComponent)
    {
        FBox PlateBounds = PlateMeshComponent->Bounds.GetBox();
        FVector PlateSize = PlateBounds.GetSize();
        
        // 접시만의 반지름 계산 (X, Y 중 큰 값의 절반)
        PlateRadius = FMath::Max(PlateSize.X, PlateSize.Y) * 0.475f; // 약간 여유를 두고 0.475배
        
        UE_LOG(LogTemp, Verbose, TEXT("접시 반경 계산됨: %.1f (X=%.1f, Y=%.1f)"),
            PlateRadius, PlateSize.X, PlateSize.Y);
    }
}

// 통합된 함수 구현 
FVector APlateActor::CalculatePlateEdge(UWorld* World, float CameraAngle, APlateActor* PlateInstance)
{
    // 1. 액터 인스턴스 확보
    APlateActor* PlateActor = PlateInstance;
    
    // 인스턴스가 없으면 World에서 찾기
    if (!PlateActor && World)
    {
        TArray<AActor*> PlateActors;
        UGameplayStatics::GetAllActorsWithTag(World, FName("Plate"), PlateActors);
        
        if (PlateActors.Num() > 0)
        {
            PlateActor = Cast<APlateActor>(PlateActors[0]);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("접시 액터를 찾을 수 없습니다."));
            return FVector::ZeroVector;
        }
    }
    
    // 유효한 인스턴스가 없으면 에러
    if (!PlateActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("유효한 접시 인스턴스가 없습니다."));
        return FVector::ZeroVector;
    }
    
    // 2. 이제 실제 계산 수행
    // 접시 중심 위치 가져오기
    FVector PlateCenter = PlateActor->GetActorLocation();
    
    // 전체 바운딩 박스 (높이 계산용)
    FBox TotalBounds = PlateActor->GetComponentsBoundingBox();
    
    // 카메라 방향 벡터 계산
    float RadianAngle = FMath::DegreesToRadians(CameraAngle);
    FVector CameraDirection;
    CameraDirection.X = FMath::Cos(RadianAngle);
    CameraDirection.Y = FMath::Sin(RadianAngle);
    CameraDirection.Z = 0.0f;
    CameraDirection.Normalize();
    
    // 카메라 방향의 반대쪽 접시 가장자리 지점 계산 (카메라에서 가장 먼 곳)
    FVector EdgePoint = PlateCenter + CameraDirection * PlateActor->PlateRadius;
    
    // 높이 조정 - 전체 구조물(테이블+접시) 위로, 공 크기를 고려한 오프셋 적용
    float BallTypeOffset = 7.5f; // 추가 여유 높이
    EdgePoint.Z = TotalBounds.Max.Z + BallTypeOffset;
    
    // 계산된 위치 저장 (카메라 회전 테스트용으로 지워도 됨)
    LastSpawnPos = EdgePoint;
    LastAngle = CameraAngle;
    
    return EdgePoint;
}