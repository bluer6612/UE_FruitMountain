#include "FruitBall.h"
#include "Components/StaticMeshComponent.h"
#include "Gameplay/Fruit/FruitCollisionHelper.h"
#include "Gameplay/Fruit/FruitMergeHelper.h"
#include "Engine/StaticMesh.h"
#include "Gameplay/Controller/FruitPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Interface/HUD/FruitHUD.h"

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

    // 생성자에서는 기본 메시(타입 1)를 로드 (나중에 UpdateFruitMesh에서 업데이트)
    static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMeshAsset(TEXT("/Game/Fruit/Meshes/Fruit1"));
    if (DefaultMeshAsset.Succeeded())
    {
        MeshComponent->SetStaticMesh(DefaultMeshAsset.Object);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("기본 과일 메시(Fruit1)를 찾을 수 없습니다!"));
    }

    // 물리적 감쇠 설정
    MeshComponent->SetLinearDamping(0.0f);
    MeshComponent->SetAngularDamping(0.0f);
}

void AFruitBall::BeginPlay()
{
    Super::BeginPlay();
    
    if (MeshComponent)
    {
        // 물리 시뮬레이션 확인
        MeshComponent->SetSimulatePhysics(true);
        
        // 모든 채널에 대해 충돌 설정
        MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        MeshComponent->SetCollisionObjectType(ECC_PhysicsBody);
        
        // 기본 PhysicsActor 프로필 사용
        MeshComponent->SetCollisionProfileName(TEXT("PhysicsActor"));
        
        // 충돌 응답 수동 설정
        MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
        
        // 충돌 이벤트 활성화
        MeshComponent->SetNotifyRigidBodyCollision(true);
        
        // 과일 메시 업데이트 (BeginPlay에서 현재 BallType으로 업데이트)
        UpdateFruitMesh(BallType);
    }
}

// 공 크기 계산 함수 구현 - 이미 언리얼 스케일로 반환
float AFruitBall::CalculateBallSize(int32 BallType)
{
    // 과일 레벨에 따른 크기 선형 상수 증가 (UE 단위로 직접 반환)
    // 레벨 1: 15cm, 레벨 2: 16.5cm, ...
    return BaseBallSize + ((BallType - 1) * BaseBallSize * 0.2f);
}

// 공 질량 계산 함수 구현
float AFruitBall::CalculateBallMass(int32 BallType)
{
    // 과일 레벨에 따른 질량 지수 증가 (예시 공식)
    // 레벨 1: 100kg, 레벨 2: 102.5kg, ...
    return DensityFactor * FMath::Pow(1.025f, BallType - 1);
}

// Tick 함수 수정/추가
void AFruitBall::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // 미리보기 공이나 병합 중인 과일은 체크하지 않음
    if (bIsPreviewBall || bIsBeingMerged) return;
    
    // 모든 과일에 대해 추락 확인 (충돌 여부 상관없이)
    float CurrentZ = GetActorLocation().Z;
    
    // 접시보다 약간이라도 아래로 내려가면 게임 오버
    if (CurrentZ < FallThreshold)
    {
        UE_LOG(LogTemp, Warning, TEXT("과일이 접시 바깥으로 떨어짐: %s (Z=%f)"), 
               *GetName(), CurrentZ);
               
        // 게임오버 처리
        APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        AFruitPlayerController* FruitController = Cast<AFruitPlayerController>(PC);
        
        if (FruitController)
        {
            FruitController->GameOver();
            
            // 게임오버 발생 후 더 이상 체크하지 않도록 Tick 비활성화
            SetActorTickEnabled(false);
        }
    }
}

// OnBallHit 함수 수정: 충돌 발생 시 플래그 설정
void AFruitBall::OnBallHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, 
                         UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    // 충돌 경험 설정
    bHasCollided = true;
    
    // 접시와의 충돌인지 확인
    if (OtherActor && (OtherActor->ActorHasTag("Plate")))
    {
        // 기존 코드 유지
        StabilizeOnPlate(HitComponent);
        return;
    }

    // 다른 과일과의 충돌 처리는 기존대로 유지
    AFruitBall* OtherFruit = Cast<AFruitBall>(OtherActor);
    if (OtherFruit)
    {
        UFruitCollisionHelper::HandleFruitCollision(this, OtherFruit, Hit);
    }
}

void AFruitBall::StabilizeOnPlate(UPrimitiveComponent* HitComponent)
{
    if (!HitComponent || !HitComponent->IsSimulatingPhysics())
        return;
    
    // 즉시 감쇠를 매우 높게 설정하여 빠르게 에너지 손실
    HitComponent->SetAngularDamping(10.0f);
    HitComponent->SetLinearDamping(10.0f);

    // 부자연스러워서 제거
    // 수직 속도를 즉시 감소
    //FVector CurrentVel = HitComponent->GetPhysicsLinearVelocity();
    //CurrentVel.Z *= 0.2f;  // Z 속도를 80% 감소
    //HitComponent->SetPhysicsLinearVelocity(CurrentVel);

    // 각속도 감소
    //FVector AngVel = HitComponent->GetPhysicsAngularVelocityInDegrees();
    //HitComponent->SetPhysicsAngularVelocityInDegrees(AngVel * 0.2f);
    
    // 잠시 후에 완전히 안정화 - 약한 참조 사용
    GetWorld()->GetTimerManager().SetTimer(StabilizeTimerHandle, 
        [WeakThis=TWeakObjectPtr<AFruitBall>(this), WeakMeshComp=TWeakObjectPtr<UPrimitiveComponent>(HitComponent)]() 
        {
            // 약한 포인터로 유효성 검사 (이미 소멸된 객체에 안전하게 접근)
            if (WeakThis.IsValid() && WeakMeshComp.IsValid() && WeakMeshComp->IsSimulatingPhysics())
            {
                // 수직 속도를 0으로 설정 (부자연스러워서 제거)
                //WeakMeshComp->SetPhysicsLinearVelocity(FVector::ZeroVector);
                // 각속도는 기울임과 관련되서 0 처리 제외
                //WeakMeshComp->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
                
                // 일반 감쇠 값 복원
                WeakMeshComp->SetAngularDamping(2.0f);
                WeakMeshComp->SetLinearDamping(2.0f);
            }
        }, 
        1.0f, false);
}

// 과일 타입에 맞는 메시 업데이트
void AFruitBall::UpdateFruitMesh(int32 NewBallType)
{
    if (!MeshComponent) return;
    
    // 메시 경로 생성: /Game/Fruit/Meshes/Fruit + BallType
    FString MeshPath = FString::Printf(TEXT("/Game/Fruit/Meshes/Fruit%d"), NewBallType);
    
    // 새 메시 로드
    UStaticMesh* NewMesh = LoadObject<UStaticMesh>(nullptr, *MeshPath);
    
    if (NewMesh)
    {
        // 새 메시 설정
        MeshComponent->SetStaticMesh(NewMesh);
        UE_LOG(LogTemp, Verbose, TEXT("과일 메시 업데이트: %s (타입: %d)"), *MeshPath, NewBallType);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("메시를 찾을 수 없음: %s"), *MeshPath);
        
        // fallback 메시 - 기본 과일1 시도
        UStaticMesh* FallbackMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Fruit/Meshes/Fruit1"));
        if (FallbackMesh)
        {
            MeshComponent->SetStaticMesh(FallbackMesh);
            UE_LOG(LogTemp, Warning, TEXT("대체 메시 사용: Fruit1"));
        }
    }
}

// BallType을 설정하고 메시를 업데이트하는 함수
void AFruitBall::SetBallType(int32 NewBallType)
{
    // 타입 변경 시에만 메시 업데이트
    if (BallType != NewBallType)
    {
        BallType = NewBallType;
        UpdateFruitMesh(BallType);
    }
}

void AFruitBall::DisplayDebugInfo()
{
    if (GEngine)
    {
        // GetName() 대신 안전한 방법으로 이름 가져오기
        FString ActorName = GetNameSafe(this);
        FString DebugMsg = FString::Printf(TEXT("과일 ID: %s, 레벨: %d"), *ActorName, GetBallType());
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, DebugMsg);
    }
}