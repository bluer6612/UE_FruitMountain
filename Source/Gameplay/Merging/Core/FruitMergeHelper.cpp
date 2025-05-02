#include "FruitMergeHelper.h"
#include "Actors/FruitBall.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMesh.h"
#include "MergeController.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"

void UFruitMergeHelper::RegisterCollisionHandlers(AFruitBall* Fruit)
{
    if (!Fruit || !Fruit->GetMeshComponent())
    {
        UE_LOG(LogTemp, Error, TEXT("UFruitMergeHelper: 유효하지 않은 과일 또는 메시 컴포넌트"));
        return;
    }

    // 미리보기 과일 체크 (추가 안전장치)
    if (Fruit->bIsPreviewBall)
    {
        return;
    }

    // 충돌 이벤트에 연결
    Fruit->GetMeshComponent()->OnComponentHit.AddDynamic(Fruit, &AFruitBall::OnBallHit);
    //UE_LOG(LogTemp, Log, TEXT("과일 충돌 핸들러 등록 완료: %s"), *Fruit->GetName());
}

// 모든 메시 사전 로드
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
    if (World)
    {
        TSubclassOf<AActor> PreloadParticleClass = LoadClass<AActor>(nullptr, TEXT("/Game/Particle/02_Blueprints/BP_Particle_Burst_Lvl_1.BP_Particle_Burst_Lvl_1_C"));
        if (PreloadParticleClass)
        {
            // 보이지 않는 위치에 미리 인스턴스 생성 후 즉시 제거 (렌더링 캐시 준비)
            FVector HiddenLocation = FVector(0, 0, -10000);
            AActor* PreloadActor = World->SpawnActor<AActor>(PreloadParticleClass, HiddenLocation, FRotator::ZeroRotator);
            if (PreloadActor)
            {
                PreloadActor->SetActorHiddenInGame(true);
                PreloadActor->Destroy();
            }
        }
    }
    
    // 3. 사운드 미리 로드
    USoundBase* PreloadSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/Sounds/S_FruitMerge"));
    if (PreloadSound)
    {
        UE_LOG(LogTemp, Display, TEXT("병합 사운드 미리 로드 완료"));
    }
    
    UE_LOG(LogTemp, Display, TEXT("게임 에셋 사전 로드 완료"));
}

void UFruitMergeHelper::SpawnPreviewFruitsOnPlate(UWorld* World)
{
    if (!World) return;

    // PlateActor 찾기
    TArray<AActor*> Plates;
    UGameplayStatics::GetAllActorsWithTag(World, FName("Plate"), Plates);
    if (Plates.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("SpawnPreviewFruitsOnPlate: PlateActor를 찾을 수 없습니다."));
        return;
    }
    AActor* Plate = Plates[0];
    FVector PlateLoc = Plate->GetActorLocation();

    // 1~10까지 랜덤 순서 배열 생성
    TArray<int32> BallTypes;
    for (int32 i = 1; i <= 10; ++i)
        BallTypes.Add(i);
    for (int32 i = 0; i < BallTypes.Num(); ++i)
    {
        int32 SwapIdx = FMath::RandRange(i, BallTypes.Num() - 1);
        BallTypes.Swap(i, SwapIdx);
    }

    // 타이머로 0.25초 간격으로 하나씩 스폰
    struct FPreviewFruitSpawnData
    {
        UWorld* World;
        FVector PlateLoc;
        TArray<int32> BallTypes;
        int32 Index;
        FTimerHandle TimerHandle;
    };
    FPreviewFruitSpawnData* SpawnData = new FPreviewFruitSpawnData{ World, PlateLoc, BallTypes, 0 };

    auto SpawnFunc = [SpawnData]()
    {
        if (!SpawnData->World || SpawnData->Index >= SpawnData->BallTypes.Num())
        {
            if (SpawnData->World)
                SpawnData->World->GetTimerManager().ClearTimer(SpawnData->TimerHandle);
            delete SpawnData;
            return;
        }

        int32 BallType = SpawnData->BallTypes[SpawnData->Index];
        FVector SpawnLoc = SpawnData->PlateLoc + FVector(0, 0, 150.f); // 하늘 위에서 떨어뜨림
        FRotator SpawnRot = FRotator::ZeroRotator;

        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        AFruitBall* Fruit = SpawnData->World->SpawnActor<AFruitBall>(AFruitBall::StaticClass(), SpawnLoc, SpawnRot, Params);
        if (Fruit)
        {
            Fruit->SetBallType(BallType);
            Fruit->bIsPreviewBall = true;
            Fruit->SetActorEnableCollision(true);
            
            // 타입에 맞는 크기로 조절
            float BallSize = AFruitBall::CalculateBallSize(BallType); // cm 단위
            float PreviewScale = BallSize / 100.f; // 1m = 100cm, 원하는 월드 스케일에 맞게 조정
            Fruit->SetActorScale3D(FVector(PreviewScale));
        
        }

        SpawnData->Index++;
        if (SpawnData->Index >= SpawnData->BallTypes.Num())
        {
            if (SpawnData->World)
                SpawnData->World->GetTimerManager().ClearTimer(SpawnData->TimerHandle);
            delete SpawnData;
        }
    };

    World->GetTimerManager().SetTimer(
        SpawnData->TimerHandle,
        FTimerDelegate::CreateLambda(SpawnFunc),
        0.4f,
        true
    );
}

// 병합 이펙트 재생
void UFruitMergeHelper::PlayMergeEffect(UWorld* World, const FVector& Location, int32 BallType)
{
    if (!World || !IsValid(World))
    {
        return;
    }
    
    // 1. 시각적 효과 (블루프린트 액터)
    static TSubclassOf<AActor> MergeEffectClass = nullptr;
    if (!MergeEffectClass)
    {
        // 블루프린트 액터 클래스 로드
        MergeEffectClass = LoadClass<AActor>(nullptr, TEXT("/Game/Particle/02_Blueprints/BP_Particle_Burst_Lvl_1.BP_Particle_Burst_Lvl_1_C"));
        
        if (!MergeEffectClass)
        {
            UE_LOG(LogTemp, Error, TEXT("병합 이펙트를 로드할 수 없습니다"));
            return;
        }
    }
    
    // 2. 이펙트 스폰
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
    AActor* SpawnedEffect = World->SpawnActor<AActor>(MergeEffectClass, Location, FRotator::ZeroRotator, SpawnParams);
    if (!IsValid(SpawnedEffect))
    {
        return;
    }
    
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
    //static USoundBase* MergeSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/Sounds/S_FruitMerge"));
    
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
}