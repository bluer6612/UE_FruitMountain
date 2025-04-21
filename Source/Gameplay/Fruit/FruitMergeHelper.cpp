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
            StabilizeFruits(World, NewFruit, 8.0f, true);
        }
        
        UE_LOG(LogTemp, Warning, TEXT("새 과일 생성 완료: 레벨=%d, 위치=%s"), 
               NextType, *MergeLocation.ToString());
    }
    
    // 기존 과일들 제거
    FruitA->Destroy();
    FruitB->Destroy();
}

// 통합된 과일 안정화 함수 - 월드 전체 또는 단일 과일 모두 처리 가능
void UFruitMergeHelper::StabilizeFruits(UWorld* World, AFruitBall* SingleFruit, float DampingMultiplier, bool bIsNewFruit)
{
    if (!World) return;
    
    // 1. 단일 과일만 처리하는 경우
    if (SingleFruit)
    {
        // 과일 안정화 로직 적용
        StabilizeSingleFruit(SingleFruit, DampingMultiplier, bIsNewFruit);
        return;
    }
    
    // 2. 월드의 모든 과일 처리
    TArray<AActor*> FoundFruits;
    UGameplayStatics::GetAllActorsOfClass(World, AFruitBall::StaticClass(), FoundFruits);
    
    for (AActor* Actor : FoundFruits)
    {
        AFruitBall* Fruit = Cast<AFruitBall>(Actor);
        if (!Fruit || !Fruit->GetMeshComponent()) continue;
        
        // 미리보기 공이나 이미 병합 중인 과일 제외
        if (Fruit->IsPreviewBall() || Fruit->IsMerging()) continue;
        
        // 각 과일 안정화 처리
        StabilizeSingleFruit(Fruit, DampingMultiplier, false);
    }
}

// 실제 안정화 작업을 수행하는 내부 헬퍼 함수
void UFruitMergeHelper::StabilizeSingleFruit(AFruitBall* Fruit, float InitialDampingMultiplier, bool bIsNewFruit)
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
        
        if (!MergeEffectClass)
        {
            UE_LOG(LogTemp, Error, TEXT("병합 이펙트 클래스를 로드할 수 없습니다"));
            return;
        }
    }
    
    // 2. 이펙트 스폰
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
    AActor* SpawnedEffect = World->SpawnActor<AActor>(MergeEffectClass, Location, FRotator::ZeroRotator, SpawnParams);
    if (SpawnedEffect)
    {
        // 과일 타입에 따라 이펙트 스케일 조정
        float EffectScale = 1.0f + (BallType * 0.025f);
        SpawnedEffect->SetActorScale3D(FVector(EffectScale, EffectScale, EffectScale));
        
        // 자동 삭제
        SpawnedEffect->SetLifeSpan(1.5f);
    }
    
    // 3. 사운드 효과 재생
    static USoundBase* MergeSound = nullptr;
    if (!MergeSound)
    {
        MergeSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/Sounds/S_FruitMerge"));
    }
    
    if (MergeSound)
    {
        // 과일 크기에 따라 볼륨과 피치 조정
        float VolumeMultiplier = FMath::Min(1.5f, 0.7f + (BallType * 0.1f));
        float PitchMultiplier = FMath::Max(0.7f, 1.1f - (BallType * 0.05f)); // 큰 과일은 낮은 소리
        
        UGameplayStatics::PlaySoundAtLocation(
            World, 
            MergeSound, 
            Location, 
            VolumeMultiplier, 
            PitchMultiplier
        );
    }
    
    // 4. 카메라 효과 (과일 크기에 따라 강도 조절)
    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if (PC && BallType >= 3) // 일정 크기 이상에서만 카메라 흔들림
    {
        // 과일 크기에 따라 흔들림 강도 증가
        float ShakeScale = FMath::Min(1.0f, 0.2f + ((BallType - 3) * 0.1f));
        PC->ClientStartCameraShake(UMatineeCameraShake::StaticClass(), ShakeScale);
    }
}