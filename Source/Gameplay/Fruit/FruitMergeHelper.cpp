#include "FruitMergeHelper.h"
#include "Actors/FruitBall.h"
#include "FruitSpawnHelper.h"
#include "Kismet/GameplayStatics.h"
#include "Gameplay/Controller/FruitPlayerController.h"
#include "Framework/UE_FruitMountainGameMode.h"
#include "Components/StaticMeshComponent.h"
#include "Gameplay/Fruit/ScoreManagerComponent.h"

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
        AddScore(World, TypeA); // World 인자 추가
        PlayMergeEffect(World, MergeLocation, TypeA);
        
        FruitA->Destroy();
        FruitB->Destroy();
        return;
    }
    
    // 다음 레벨의 과일 생성
    int32 NextType = TypeA + 1;
    
    // 이펙트 및 점수 처리
    PlayMergeEffect(World, MergeLocation, TypeA);
    AddScore(World, NextType); // World 인자 추가
    
    // 병합 위치 주변 과일들의 속도 감소 (폭발적 충돌 방지)
    StabilizeFruits(World);
    
    // 새 과일 생성 전에 기존 과일의 회전값 저장
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
            MeshComp->SetAngularDamping(8.0f);
            
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
        
        FVector StabilizingForce = ToCenterXY.GetSafeNormal() * CurrentSpeed * 1.5f;
        MeshComp->AddForce(StabilizingForce, NAME_None, true); // 질량 고려하여 적용

        // 일시적으로 감쇠 증가
        MeshComp->SetLinearDamping(20.0f);
        MeshComp->SetAngularDamping(20.0f);
        
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

// 점수 추가 함수 수정 - ScoreManagerComponent를 사용하도록
void UFruitMergeHelper::AddScore(UWorld* World, int32 BallType)
{
    if (!World) return;
    
    // 게임모드에서 ScoreManagerComponent 찾기 (또는 생성)
    AUE_FruitMountainGameMode* GameMode = Cast<AUE_FruitMountainGameMode>(UGameplayStatics::GetGameMode(World));
    if (!GameMode) 
    {
        UE_LOG(LogTemp, Error, TEXT("AddScore: 게임모드를 찾을 수 없음"));
        return;
    }
    
    // 게임모드에서 ScoreManagerComponent 가져오기
    UScoreManagerComponent* ScoreManager = GameMode->FindComponentByClass<UScoreManagerComponent>();
    
    // 없으면 생성
    if (!ScoreManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("AddScore: ScoreManagerComponent가 없어 새로 생성합니다."));
        ScoreManager = NewObject<UScoreManagerComponent>(GameMode, UScoreManagerComponent::StaticClass());
        ScoreManager->RegisterComponent();
    }
    
    // 점수 추가 로직을 ScoreManagerComponent에 위임
    ScoreManager->AddScore(BallType);
}

// ResetCombo 함수도 ScoreManagerComponent 사용으로 수정
void UFruitMergeHelper::ResetCombo(UWorld* World)
{
    if (!World) return;
    
    AUE_FruitMountainGameMode* GameMode = Cast<AUE_FruitMountainGameMode>(UGameplayStatics::GetGameMode(World));
    if (!GameMode) return;
    
    UScoreManagerComponent* ScoreManager = GameMode->FindComponentByClass<UScoreManagerComponent>();
    if (ScoreManager)
    {
        ScoreManager->ResetCombo();
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

void UFruitMergeHelper::PreloadAllFruitMeshes(UWorld* World)
{
    UE_LOG(LogTemp, Display, TEXT("게임 에셋 사전 로드 시작..."));
    
    // 1. 모든 과일 메시 미리 로드 (최대 레벨까지)
    for (int32 i = 1; i <= AFruitBall::MaxBallType; i++)
    {
        // 메시 경로 - 게임의 실제 경로와 일치하게 수정
        FString MeshPath = FString::Printf(TEXT("/Game/Fruit/Meshes/Fruit%d.Fruit%d"), i, i);
        
        // 동기적 로딩 사용
        UStaticMesh* FruitMesh = LoadObject<UStaticMesh>(nullptr, *MeshPath);
        
        if (FruitMesh)
        {
            // 메시가 완전히 로드되도록 보장
            FruitMesh->ConditionalPostLoad();
            UE_LOG(LogTemp, Warning, TEXT("과일 메시 #%d 사전 로드 완료: %s"), i, *MeshPath);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("과일 메시 #%d 로드 실패: %s"), i, *MeshPath);
        }
    }
    
    // 2. 파티클 효과 미리 로드
    TSubclassOf<AActor> PreloadParticleClass = LoadClass<AActor>(nullptr, TEXT("/Game/Particle/02_Blueprints/BP_Particle_Burst_Lvl_1.BP_Particle_Burst_Lvl_1_C"));
    if (PreloadParticleClass && World)
    {
        FVector HiddenLocation = FVector(0, 0, -10000);
        AActor* PreloadActor = World->SpawnActor<AActor>(PreloadParticleClass, HiddenLocation, FRotator::ZeroRotator);
        if (PreloadActor)
        {
            PreloadActor->SetActorHiddenInGame(true);
            PreloadActor->SetActorTickEnabled(false);
            
            FTimerHandle DestroyHandle;
            World->GetTimerManager().SetTimer(DestroyHandle, [PreloadActor]() {
                if(IsValid(PreloadActor)) PreloadActor->Destroy();
            }, 0.1f, false);
        }
        
        UE_LOG(LogTemp, Display, TEXT("병합 이펙트 파티클 미리 로드 완료"));
    }
    
    // 3. 사운드도 미리 로드
    USoundBase* PreloadSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/Sounds/S_FruitMerge"));
}