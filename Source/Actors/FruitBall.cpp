#include "FruitBall.h"
#include "Components/StaticMeshComponent.h"
#include "Gameplay/Fruit/FruitCollisionHelper.h"
#include "Gameplay/Fruit/FruitMergeHelper.h"
#include "Engine/StaticMesh.h"
#include "Gameplay/Controller/FruitPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Interface/HUD/FruitHUD.h"
#include "System/Camera/CameraOrbitFunctionLibrary.h"
#include "Gameplay/Physics/FruitTrajectoryHelper.h"

AFruitBall::AFruitBall()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // 기본값 설정
    BallType = 1;
    bIsBeingMerged = false;
    bSlowMotionActive = false;
    
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

// Tick 함수 수정 - 떨어짐 감지 시 카메라 이동 및 슬로우 모션 추가
void AFruitBall::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // 미리보기 공이나 병합 중인 과일은 체크하지 않음
    if (bIsPreviewBall || bIsBeingMerged) return;
    
    // 충돌 경험이 있는 과일만 추락 체크
    if (bHasCollided)
    {
        // 현재 위치 확인
        float CurrentZ = GetActorLocation().Z;
        
        // 접시보다 약간이라도 아래로 내려가면 게임 오버
        if (CurrentZ < FallThreshold)
        {
            UE_LOG(LogTemp, Warning, TEXT("충돌 경험 있는 과일이 접시 바깥으로 떨어짐: %s (Z=%f)"), 
                *GetName(), CurrentZ);
                            
            // Tick 비활성화
            SetActorTickEnabled(false);
            
            // 이미 슬로우 모션 처리 중인지 확인
            if (!bSlowMotionActive)
            {
                bSlowMotionActive = true;
                
                // 슬로우 모션 효과 적용
                UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.3f); // 20% 속도로 감속해 슬로우 모션
                
                // 1. 과일 자체의 물리 설정 변경
                MeshComponent->SetLinearDamping(30.0f);
                MeshComponent->SetAngularDamping(30.0f);
                
                // 2. 중력 영향 감소 (떨어지는 속도 감소)
                MeshComponent->SetEnableGravity(false); // 중력 비활성화
                
                // 기존 속도의 방향을 유지하면서 속도 감소
                FVector CurrentVelocity = MeshComponent->GetPhysicsLinearVelocity();
                MeshComponent->SetPhysicsLinearVelocity(CurrentVelocity * 0.2f);
                
                // 수동으로 약한 낙하 속도 적용 (중력 없이 아래로 천천히 떨어짐)
                FVector SlowFallVector = FVector(0, 0, -10.0f);
                MeshComponent->AddForce(SlowFallVector, NAME_None, true);
                
                // 3. 카메라를 과일 쪽으로 이동
                APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
                AFruitPlayerController* FruitController = Cast<AFruitPlayerController>(PC);

                if (FruitController)
                {
                    // 미리보기 공 제거
                    if (FruitController->PreviewBall)
                    {
                        FruitController->PreviewBall->Destroy();
                        FruitController->PreviewBall = nullptr;
                    }
                    
                    // 궤적 표시 제거 - 실제 구현된 방식으로 호출
                    UFruitTrajectoryHelper::ResetTrajectorySystem();
                    
                    // 컨트롤러 입력 정지
                    FruitController->DisableInput(PC);
                    FruitController->bIsThrowingInProgress = false; // 던지기 상태 해제
                    
                    // 카메라 이동
                    UCameraOrbitFunctionLibrary::MoveViewToFallingFruit(FruitController, GetActorLocation(), FRotator::ZeroRotator);
                    
                    // 약간의 딜레이 후 실제 게임 오버 처리
                    GetWorld()->GetTimerManager().SetTimer(
                        GameOverTimerHandle,
                        [this, FruitController]()
                        {
                            // 게임 오버 처리
                            FruitController->GameOver();
                            
                            // 시간 다시 정상화
                            UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);
                        },
                        5.0f * UGameplayStatics::GetGlobalTimeDilation(GetWorld()), // 슬로우 모션 상태에서 5초
                        false
                    );
                }
            }
        }
    }
}

void AFruitBall::OnBallHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, 
                       UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    UFruitCollisionHelper::HandleBallHit(this, HitComponent, OtherActor, OtherComp, NormalImpulse, Hit);
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