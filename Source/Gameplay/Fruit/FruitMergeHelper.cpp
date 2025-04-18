#include "FruitMergeHelper.h"
#include "Actors/FruitBall.h"
#include "FruitSpawnHelper.h"
#include "Kismet/GameplayStatics.h"
#include "Gameplay/Controller/FruitPlayerController.h"
#include "Framework/UE_FruitMountainGameMode.h"
#include "Components/StaticMeshComponent.h"

void UFruitMergeHelper::TryMergeFruits(AFruitBall* FruitA, AFruitBall* FruitB, const FVector& CollisionPoint)
{
    if (!FruitA || !FruitB) {
        UE_LOG(LogTemp, Error, TEXT("TryMergeFruits: 과일 참조가 유효하지 않음"));
        return;
    }
    
    // 미리보기 공 체크 추가 - 둘 중 하나라도 미리보기 공이면 병합하지 않음
    if (FruitA->IsPreviewBall() || FruitB->IsPreviewBall()) {
        UE_LOG(LogTemp, Verbose, TEXT("미리보기 공과의 충돌 무시"));
        return;
    }
    
    // 두 과일의 타입 가져오기
    int32 TypeA = FruitA->GetBallType();
    int32 TypeB = FruitB->GetBallType();
    
    // 타입이 서로 다르면 병합하지 않음
    if (TypeA != TypeB) {
        return; 
    }
    
    // 이미 병합 중인 과일이면 무시
    if (FruitA->IsMerging() || FruitB->IsMerging()) {
        UE_LOG(LogTemp, Warning, TEXT("이미 병합중인 과일이 있음"));
        return;
    }
    
    // 두 과일 모두 병합 상태로 설정
    FruitA->SetMerging(true);
    FruitB->SetMerging(true);
    
    // 병합 처리 수행
    MergeFruits(FruitA, FruitB, CollisionPoint);
}

// 연쇄는 2연쇄부터 시작해서 1.1배, 3연쇄 1.2배, 4연쇄 1.2배, 5연쇄 1.3배, 이렇게 2의 배수
// 1, 3, 6, 10, 15, 21, 28, 36, 45, 55, 66, 78, 91, 105, 120, 136, 153, 171, 190

void UFruitMergeHelper::MergeFruits(AFruitBall* FruitA, AFruitBall* FruitB, const FVector& MergeLocation)
{
    if (!FruitA || !FruitB) {
        UE_LOG(LogTemp, Error, TEXT("MergeFruits: 과일 참조가 유효하지 않음"));
        return;
    }
    
    // 두 과일의 타입 가져오기
    int32 TypeA = FruitA->GetBallType();
    int32 TypeB = FruitB->GetBallType();
    
    UWorld* World = FruitA->GetWorld();
    if (!World) return;
    
    // 마지막 레벨 체크
    if (TypeA >= AFruitBall::MaxBallType)
    {
        UE_LOG(LogTemp, Warning, TEXT("병합 완료: 최대 레벨 과일 병합"));
        AddScore(TypeA * 2);
        PlayMergeEffect(World, MergeLocation, TypeA);
        
        FruitA->Destroy();
        FruitB->Destroy();
        return;
    }
    
    // 다음 레벨의 과일 생성
    int32 NextType = TypeA + 1;
    
    // 이펙트 및 점수 처리
    PlayMergeEffect(World, MergeLocation, TypeA);
    AddScore(TypeA);
    
    // 병합 위치 주변 과일들의 속도 감소 (폭발적 충돌 방지)
    StabilizeFruits(World);
    
    // 새 과일 생성 전에 기존 과일의 회전값 저장
    // 두 과일 중 어떤 것이 접시에 있는 과일인지 판단 (여기서는 단순히 FruitA 사용)
    FRotator ExistingRotation = FruitA->GetActorRotation();
    
    // 새 과일 생성
    AFruitPlayerController* Controller = Cast<AFruitPlayerController>(UGameplayStatics::GetPlayerController(World, 0));
    if (Controller)
    {
        // 정확히 병합 위치에 생성
        AActor* SpawnedActor = UFruitSpawnHelper::SpawnBall(Controller, MergeLocation, NextType, true);
        AFruitBall* NewFruit = Cast<AFruitBall>(SpawnedActor);
        
        // 생성된 과일에 자연스러운 움직임 적용
        if (NewFruit && NewFruit->GetMeshComponent())
        {
            // 기존 과일의 회전각 적용
            NewFruit->SetActorRotation(ExistingRotation);
            
            UStaticMeshComponent* MeshComp = NewFruit->GetMeshComponent();
            
            // 물리 시뮬레이션 즉시 활성화 (멈추지 않음)
            MeshComp->SetSimulatePhysics(true);
            
            // 약간의 위쪽 방향 초기 속도 부여 (자연스러운 병합 효과)
            float UpwardForce = -5.0f - (NextType * 1.0f); // 레벨에 따라 증가
            MeshComp->SetPhysicsLinearVelocity(FVector(0, 0, UpwardForce));
            
            // 초기에는 댐핑을 높게 설정 (병합 직후 안정화 용도)
            MeshComp->SetLinearDamping(8.0f);
            MeshComp->SetAngularDamping(5.0f);
            
            // 접시에 닿는 시점에 StabilizeOnPlate가 댐핑을 관리할 것임
        }
        
        UE_LOG(LogTemp, Warning, TEXT("새 과일 생성 완료: 레벨=%d, 위치=%s"), 
               NextType, *MergeLocation.ToString());
    }
    
    // 기존 과일들 제거
    FruitA->Destroy();
    FruitB->Destroy();
}

// 모든 과일 안정화 함수
void UFruitMergeHelper::StabilizeFruits(UWorld* World)
{
    if (!World) return;
    
    // 현재 월드의 모든 과일 찾기
    TArray<AActor*> FoundFruits;
    UGameplayStatics::GetAllActorsOfClass(World, AFruitBall::StaticClass(), FoundFruits);
    
    // 거리 제한 없이 모든 과일에 감속 적용
    for (AActor* Actor : FoundFruits)
    {
        AFruitBall* Fruit = Cast<AFruitBall>(Actor);
        if (!Fruit || !Fruit->GetMeshComponent()) continue;
        
        // 미리보기 공이나 이미 병합 중인 과일 제외
        if (Fruit->IsPreviewBall() || Fruit->IsMerging()) continue;
        
        UStaticMeshComponent* MeshComp = Fruit->GetMeshComponent();
        
        // 현재 속도 감소 (모든 과일의 속도를 90% 감소)
        FVector CurrentVel = MeshComp->GetPhysicsLinearVelocity();
        MeshComp->SetPhysicsLinearVelocity(CurrentVel * 0.1f);
        
        // 회전 속도 감소
        FVector AngVel = MeshComp->GetPhysicsAngularVelocityInDegrees();
        MeshComp->SetPhysicsAngularVelocityInDegrees(AngVel * 0.1f);
        
        // 접시 중앙 방향으로 안정화 힘 추가 (횡방향만)
        float CurrentSpeed = CurrentVel.Size();
        FVector ToCenterXY = FVector::ZeroVector - Fruit->GetActorLocation();
        ToCenterXY.Z = 0; // Z 방향은 무시 (수직 안정화만)
        
        FVector StabilizingForce = ToCenterXY.GetSafeNormal() * CurrentSpeed * 0.5f;
        MeshComp->AddForce(StabilizingForce, NAME_None, true); // 질량 고려하여 적용

        // 일시적으로 감쇠 증가
        MeshComp->SetLinearDamping(10.0f);
        MeshComp->SetAngularDamping(10.0f);
        
        // 1초 후에 원래 감쇠 복원
        FTimerHandle DampingTimerHandle;
        World->GetTimerManager().SetTimer(DampingTimerHandle, 
            [WeakFruit=TWeakObjectPtr<AFruitBall>(Fruit)]() 
            {
                // 약한 포인터로 유효성 검사 (이미 소멸된 객체에 안전하게 접근)
                if (WeakFruit.IsValid() && WeakFruit->GetMeshComponent())
                {
                    WeakFruit->GetMeshComponent()->SetLinearDamping(2.0f);
                    WeakFruit->GetMeshComponent()->SetAngularDamping(2.0f);
                }
            }, 
            1.0f, false);
    }
}

void UFruitMergeHelper::AddScore(int32 BallType)
{
    // 레벨에 따른 점수 계산 (예: 레벨 * 10)
    int32 ScoreToAdd = BallType * 10;
    
    // 게임 모드나 UI를 통해 점수 업데이트
    UWorld* World = GEngine->GetWorld();
    if (World)
    {
        AGameModeBase* GameMode = World->GetAuthGameMode();
        AUE_FruitMountainGameMode* FruitGameMode = Cast<AUE_FruitMountainGameMode>(GameMode);
        if (FruitGameMode)
        {
            // 게임 모드에 점수 추가 함수 호출 (구현 필요)
            // FruitGameMode->AddScore(ScoreToAdd);
            
            // UI 업데이트
            UE_LOG(LogTemp, Warning, TEXT("과일 병합 점수 추가: %d (레벨 %d)"), ScoreToAdd, BallType);
        }
    }
}

void UFruitMergeHelper::PlayMergeEffect(UWorld* World, const FVector& Location, int32 BallType)
{
    if (!World) return;
    
    // 1. 시각적 효과 (블루프린트 액터)
    static TSubclassOf<AActor> MergeEffectClass = nullptr;
    if (!MergeEffectClass)
    {
        // 블루프린트 액터 클래스 로드
        MergeEffectClass = LoadClass<AActor>(nullptr, TEXT("/Game/Particle/02_Blueprints/BP_Particle_Burst_Lvl_1.BP_Particle_Burst_Lvl_1_C"));
    }
    
    if (MergeEffectClass)
    {
        // 블루프린트 액터 생성 (Z축으로 100 올림)
        FVector ElevatedLocation = Location + FVector(0, 0, 10.0f);
        
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        
        // 크기를 과일 레벨에 따라 조정
        float EffectScale = 1.0f;
        AActor* MergeEffect = World->SpawnActor<AActor>(
            MergeEffectClass, 
            ElevatedLocation,
            FRotator::ZeroRotator, 
            SpawnParams
        );
        
        if (MergeEffect)
        {
            MergeEffect->SetActorScale3D(FVector(EffectScale));
            
            // 2초 후에 효과 제거
            FTimerHandle DestroyTimerHandle;
            World->GetTimerManager().SetTimer(
                DestroyTimerHandle,
                FTimerDelegate::CreateLambda([MergeEffect]() {
                    if (IsValid(MergeEffect))
                    {
                        MergeEffect->Destroy();
                    }
                }),
                2.0f,
                false
            );
        }
    }
    
    // 2. 소리 효과
    static USoundBase* MergeSound = nullptr;
    if (!MergeSound)
    {
        // 사운드 에셋 로드 (게임에 맞는 경로로 수정 필요)
        MergeSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/Sounds/S_FruitMerge"));
    }
    
    if (MergeSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            World, MergeSound, Location, 
            1.0f + (BallType * 0.1f),  // 볼륨 (레벨이 높을수록 더 큰 소리)
            0.8f + (BallType * 0.05f)  // 피치 (레벨이 높을수록 더 높은 소리)
        );
    }
}

// FruitMergeHelper.cpp에 함수 구현 추가
void UFruitMergeHelper::PreloadAssets(UWorld* World)
{
    // 블루프린트 클래스 미리 로드
    TSubclassOf<AActor> PreloadClass = LoadClass<AActor>(nullptr, TEXT("/Game/Particle/02_Blueprints/BP_Particle_Burst_Lvl_1.BP_Particle_Burst_Lvl_1_C"));
    if (PreloadClass && World)
    {
        // 보이지 않는 위치에 미리 인스턴스 생성 후 즉시 제거 (렌더링 캐시 준비)
        FVector HiddenLocation = FVector(0, 0, -10000);
        AActor* PreloadActor = World->SpawnActor<AActor>(PreloadClass, HiddenLocation, FRotator::ZeroRotator);
        if (PreloadActor)
        {
            PreloadActor->SetActorHiddenInGame(true);
            PreloadActor->SetActorTickEnabled(false);
            
            // 1프레임 후 삭제 (렌더링 자원이 초기화되도록)
            FTimerHandle DestroyHandle;
            World->GetTimerManager().SetTimer(DestroyHandle, [PreloadActor]() {
                if(IsValid(PreloadActor)) PreloadActor->Destroy();
            }, 0.1f, false);
        }
        
        // 리소스가 이미 로딩됨을 알리는 로그
        UE_LOG(LogTemp, Display, TEXT("병합 이펙트 파티클 미리 로드 완료"));
    }
    
    // 사운드도 미리 로드
    USoundBase* PreloadSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/Sounds/S_FruitMerge"));
}