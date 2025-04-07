#include "FruitBall.h"
#include "Components/StaticMeshComponent.h"
#include "Gameplay/Create/FruitMergeHelper.h"

AFruitBall::AFruitBall()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // 기본값 설정
    BallType = 1;
    bIsBeingMerged = false;
    
    // 메시 컴포넌트 생성 및 루트로 설정
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FruitBallMesh"));
    RootComponent = MeshComponent;
    
    // 물리 시뮬레이션 활성화
    MeshComponent->SetSimulatePhysics(true);
    MeshComponent->SetCollisionProfileName(TEXT("PhysicsActor"));

    // 기본 공 메시로 언리얼 기본 20면체 메시 사용
    static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Engine/BasicShapes/Sphere"));
    if (MeshAsset.Succeeded())
    {
        MeshComponent->SetStaticMesh(MeshAsset.Object);
    }

    // 충돌 이벤트 등록
    MeshComponent->OnComponentHit.AddDynamic(this, &AFruitBall::OnBallHit);
}

// 공 크기 계산 함수 구현 - 이미 언리얼 스케일로 반환
float AFruitBall::CalculateBallSize(int32 BallType)
{
    // 과일 레벨에 따른 크기 증가 (UE 단위로 직접 반환)
    // 레벨 1: 15cm, 레벨 2: 16.5cm, ...
    return BaseBallSize + ((BallType - 1) * BaseBallSize * 0.1f);
}

// 공 질량 계산 함수 구현
float AFruitBall::CalculateBallMass(int32 BallType)
{
    // 과일 레벨에 따른 질량 증가 (예시 공식)
    // 레벨 1: 10kg, 레벨 2: 15kg, 레벨 3: 21kg, ...
    return 10.0f * FMath::Pow(1.05f, BallType - 1);
}

void AFruitBall::OnBallHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    // 이미 병합 중이면 무시
    if (bIsBeingMerged) return;
    
    // 충돌한 상대방이 과일인지 확인
    AFruitBall* OtherFruit = Cast<AFruitBall>(OtherActor);
    if (OtherFruit && !OtherFruit->bIsBeingMerged)
    {
        TryMergeWithOtherFruit(OtherFruit);
    }
}

bool AFruitBall::TryMergeWithOtherFruit(AFruitBall* OtherFruit)
{
    // 두 과일의 레벨이 같은지 확인
    if (BallType == OtherFruit->BallType)
    {
        // 병합 중 표시하여 중복 처리 방지
        bIsBeingMerged = true;
        OtherFruit->bIsBeingMerged = true;
        
        // 충돌 위치 계산 (두 과일의 중간점)
        FVector MergeLocation = (GetActorLocation() + OtherFruit->GetActorLocation()) * 0.5f;
        
        // 병합 함수 호출
        UFruitMergeHelper::MergeFruits(this, OtherFruit, MergeLocation);
        
        // 디버그 정보 표시
        DisplayDebugInfo();
        
        return true;
    }
    
    return false;
}

void AFruitBall::DisplayDebugInfo()
{
    if (GEngine)
    {
        // GetName() 대신 안전한 방법으로 이름 가져오기
        FString ActorName = GetNameSafe(this);
        FString DebugMsg = FString::Printf(TEXT("과일 ID: %s, 레벨: %d"), *ActorName, BallType);
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, DebugMsg);
    }
}