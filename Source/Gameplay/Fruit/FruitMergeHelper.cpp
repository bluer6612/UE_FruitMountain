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
            
            // 새 과일 물리 속성 설정
            StabilizeFruitPhysics(NewFruit, 8.0f, true);
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
    
    // 모든 과일에 감속 적용
    for (AActor* Actor : FoundFruits)
    {
        AFruitBall* Fruit = Cast<AFruitBall>(Actor);
        if (!Fruit || !Fruit->GetMeshComponent()) continue;
        
        // 미리보기 공이나 이미 병합 중인 과일 제외
        if (Fruit->IsPreviewBall() || Fruit->IsMerging()) continue;
        
        // 기존 과일 물리 속성 안정화
        StabilizeFruitPhysics(Fruit, 20.0f, false);
    }
}

// 과일 물리 속성 설정을 위한 통합 헬퍼 함수
void UFruitMergeHelper::StabilizeFruitPhysics(AFruitBall* Fruit, float InitialDampingMultiplier, bool bIsNewFruit)
{
    if (!Fruit || !Fruit->GetMeshComponent()) return;
    
    UStaticMeshComponent* MeshComp = Fruit->GetMeshComponent();
    UWorld* World = Fruit->GetWorld();
    if (!World) return;
    
    // 1. 크기 인자 계산
    int32 FruitType = Fruit->GetBallType();
    float SizeFactor = FMath::Min(2.0f, 0.5f + (FruitType * 0.2f));
    
    // 2. 현재 위치 기반 중앙 방향 힘 계산
    FVector ToCenterXY = FVector::ZeroVector - Fruit->GetActorLocation();
    ToCenterXY.Z = 0;
    
    float DistanceToCenter = ToCenterXY.Size();
    float PlateRadius = 100.0f;
    
    // 3. 새 과일 또는 기존 과일에 따라 다르게 처리
    if (bIsNewFruit)
    {
        // 새로 생성된 과일에 대한 특별 처리
        MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        
        // 업워드 힘 + 중심 힘 적용
        float UpwardForce = FMath::Max(-20.0f, -5.0f - (FruitType * 0.5f));
        
        // 중앙으로 향하는 힘 계산
        FVector CenteringForce = FVector::ZeroVector;
        if (DistanceToCenter > PlateRadius * 0.5f)
        {
            float CenteringStrength = FMath::Min(1.0f, DistanceToCenter / PlateRadius) * 10.0f;
            CenteringForce = ToCenterXY.GetSafeNormal() * CenteringStrength;
        }
        
        FVector FinalVelocity = FVector(CenteringForce.X, CenteringForce.Y, UpwardForce);
        MeshComp->SetPhysicsLinearVelocity(FinalVelocity);
        
        // 0.1초 후 충돌 재활성화
        FTimerHandle CollisionHandle;
        World->GetTimerManager().SetTimer(CollisionHandle, 
            [WeakMesh=TWeakObjectPtr<UPrimitiveComponent>(MeshComp)]() 
            {
                if (WeakMesh.IsValid())
                {
                    WeakMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                }
            }, 
            0.1f, false);
    }
    else
    {
        // 기존 과일 안정화
        FVector CurrentVel = MeshComp->GetPhysicsLinearVelocity();
        float ReductionFactor = 0.05f * SizeFactor;
        MeshComp->SetPhysicsLinearVelocity(CurrentVel * (1.0f - ReductionFactor));
        
        FVector AngVel = MeshComp->GetPhysicsAngularVelocityInDegrees();
        MeshComp->SetPhysicsAngularVelocityInDegrees(AngVel * (1.0f - ReductionFactor));
        
        // 중앙에서 멀리 있는 과일에 추가 힘 적용
        if (DistanceToCenter > PlateRadius * 0.4f)
        {
            float CenteringStrength = FMath::Min(1.0f, DistanceToCenter / PlateRadius) * 5.0f * SizeFactor;
            FVector StabilizingForce = ToCenterXY.GetSafeNormal() * CenteringStrength;
            MeshComp->AddForce(StabilizingForce, NAME_None, true);
        }
    }
    
    // 4. 공통: 감쇠 설정
    MeshComp->SetLinearDamping(InitialDampingMultiplier * SizeFactor);
    MeshComp->SetAngularDamping(InitialDampingMultiplier * SizeFactor);
    
    // 5. 감쇠 복원 타이머
    FTimerHandle DampingTimerHandle;
    World->GetTimerManager().SetTimer(DampingTimerHandle, 
        [WeakFruit=TWeakObjectPtr<AFruitBall>(Fruit), SizeFactor]() 
        {
            if (WeakFruit.IsValid() && WeakFruit->GetMeshComponent())
            {
                UStaticMeshComponent* MeshComp = WeakFruit->GetMeshComponent();
                MeshComp->SetLinearDamping(2.0f * SizeFactor);
                MeshComp->SetAngularDamping(2.0f * SizeFactor);
            }
        }, 
        0.5f, false);
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
        
    }
    
    // 3. 사운드도 미리 로드
    USoundBase* PreloadSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/Sounds/S_FruitMerge"));

    UE_LOG(LogTemp, Display, TEXT("메시 및 사운드 사전 로드 완료"));
}