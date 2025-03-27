#include "FruitThrowHelper.h"
#include "FruitPlayerController.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"

// 공통 함수로 볼 생성 로직 통합
AActor* UFruitThrowHelper::SpawnBall(AFruitPlayerController* Controller, const FVector& Location, int32 BallType, bool bEnablePhysics)
{
    if (!Controller || !Controller->FruitBallClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("SpawnBall: Controller 또는 FruitBallClass가 유효하지 않습니다."));
        return nullptr;
    }

    // 공 액터 스폰
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    SpawnParams.Owner = Controller;
    
    AActor* SpawnedBall = Controller->GetWorld()->SpawnActor<AActor>(
        Controller->FruitBallClass, Location, FRotator::ZeroRotator, SpawnParams);
        
    if (SpawnedBall)
    {
        // 크기 설정 - 모든 공에 동일한 계산식 적용
        float BaseSize = 0.5f;
        float ScaleFactor = 1.f + 0.1f * (BallType - 1);
        float BallSize = BaseSize * ScaleFactor;
        SpawnedBall->SetActorScale3D(FVector(BallSize));
        
        // 물리 컴포넌트 설정
        UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(
            SpawnedBall->GetComponentByClass(UPrimitiveComponent::StaticClass()));
            
        if (PrimComp)
        {
            // 물리 활성화 여부 설정
            PrimComp->SetSimulatePhysics(bEnablePhysics);
            
            if (bEnablePhysics)
            {
                PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                PrimComp->SetCollisionProfileName(TEXT("PhysicsActor"));
            }
            else
            {
                PrimComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            }
        }
        
        UE_LOG(LogTemp, Log, TEXT("공 스폰 성공 - 타입: %d, 크기: %f, 물리: %s"), 
            BallType, BallSize, bEnablePhysics ? TEXT("활성화") : TEXT("비활성화"));
    }
    
    return SpawnedBall;
}

void UFruitThrowHelper::ThrowFruit(AFruitPlayerController* Controller)
{
    if (!Controller)
    {
        UE_LOG(LogTemp, Warning, TEXT("Controller가 유효하지 않습니다."));
        return;
    }

    // 접시 위치 찾기
    FVector PlateCenter = FVector::ZeroVector;
    float PlateRadius = 200.0f; // 기본 반경 값
    
    TArray<AActor*> PlateActors;
    UGameplayStatics::GetAllActorsWithTag(Controller->GetWorld(), FName("Plate"), PlateActors);
    if (PlateActors.Num() > 0)
    {
        PlateCenter = PlateActors[0]->GetActorLocation();
        
        // 접시 반경 추정
        FVector PlateScale = PlateActors[0]->GetActorScale3D();
        PlateRadius = FMath::Max(PlateScale.X, PlateScale.Y) * 10.0f;
    }
    
    // 미리보기 공이 존재하는지 확인
    if (Controller->PreviewBall)
    {
        UE_LOG(LogTemp, Log, TEXT("미리보기 공을 발사합니다."));
        
        // 물리 컴포넌트 가져오기
        UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(
            Controller->PreviewBall->GetComponentByClass(UPrimitiveComponent::StaticClass()));
            
        if (PrimComp)
        {
            // 현재 위치는 그대로 유지 (위치 변경 안함)
            FVector CurrentLocation = Controller->PreviewBall->GetActorLocation();
            
            // 부착 해제만 수행
            Controller->PreviewBall->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
            
            // 물리 시뮬레이션 활성화
            PrimComp->SetSimulatePhysics(true);
            PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            PrimComp->SetCollisionProfileName(TEXT("PhysicsActor"));
            
            // 포물선 발사를 위한 방향 계산
            // 접시 중심을 향하되 위쪽 방향 성분 추가
            FVector ToPlateDirection = (PlateCenter - CurrentLocation).GetSafeNormal();
            ToPlateDirection.Z += 0.5f; // 위쪽 성분 추가 (포물선을 위해)
            ToPlateDirection.Normalize();
            
            // 발사
            PrimComp->AddImpulse(ToPlateDirection * Controller->ThrowForce, NAME_None, true);
            
            UE_LOG(LogTemp, Log, TEXT("공 발사 - 위치: %s, 방향: %s, 힘: %f"), 
                *CurrentLocation.ToString(),
                *ToPlateDirection.ToString(),
                Controller->ThrowForce);
            
            // 발사된 공은 더 이상 미리보기 공이 아님
            Controller->PreviewBall = nullptr;
            
            // 다음 미리보기 공 준비
            Controller->CurrentBallType = FMath::RandRange(1, 11);
            UpdatePreviewBall(Controller);
        }
    }
    else
    {
        // 미리보기 공이 없는 경우 (fallback)
        if (Controller->FruitBallClass)
        {
            // 플레이어 위치 획득
            APawn* PlayerPawn = Controller->GetPawn();
            if (!PlayerPawn)
            {
                UE_LOG(LogTemp, Warning, TEXT("PlayerPawn이 유효하지 않습니다."));
                return;
            }
            
            // 플레이어 방향 계산
            FVector PlayerLocation = PlayerPawn->GetActorLocation();
            FVector PlayerDirection = PlayerPawn->GetActorForwardVector();
            
            // 스폰 위치 계산
            FVector SpawnLocation = PlayerLocation + PlayerDirection * 300.f;
            SpawnLocation.Z = PlateCenter.Z; // 접시와 같은 높이
            
            // 공 생성
            AActor* SpawnedBall = SpawnBall(Controller, SpawnLocation, Controller->CurrentBallType, true);
            if (SpawnedBall)
            {
                // 물리 컴포넌트 가져오기
                UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(
                    SpawnedBall->GetComponentByClass(UPrimitiveComponent::StaticClass()));
                    
                if (PrimComp)
                {
                    // 포물선 발사를 위한 방향 계산
                    FVector ToPlateDirection = (PlateCenter - SpawnLocation).GetSafeNormal();
                    ToPlateDirection.Z += 0.5f; // 위쪽 성분 추가 (포물선을 위해)
                    ToPlateDirection.Normalize();
                    
                    // 발사
                    PrimComp->AddImpulse(ToPlateDirection * Controller->ThrowForce, NAME_None, true);
                    
                    UE_LOG(LogTemp, Log, TEXT("공 발사 - 위치: %s, 방향: %s, 힘: %f"), 
                        *SpawnLocation.ToString(),
                        *ToPlateDirection.ToString(),
                        Controller->ThrowForce);
                }
                
                // 다음 미리보기 공 준비
                Controller->CurrentBallType = FMath::RandRange(1, 11);
                UpdatePreviewBall(Controller);
            }
        }
    }
}

// 미리보기 공 업데이트 함수 수정
void UFruitThrowHelper::UpdatePreviewBall(AFruitPlayerController* Controller)
{
    if (!Controller)
    {
        UE_LOG(LogTemp, Warning, TEXT("UpdatePreviewBall: Controller가 유효하지 않습니다."));
        return;
    }

    // 이전 미리보기 공 제거
    if (Controller->PreviewBall)
    {
        Controller->PreviewBall->Destroy();
        Controller->PreviewBall = nullptr;
    }
    
    // 접시 위치 및 크기 정보 가져오기
    FVector PlateCenter = FVector::ZeroVector;
    float PlateRadius = 200.0f; // 기본 반경 값 (실제 접시 크기에 맞게 조정 필요)
    
    TArray<AActor*> PlateActors;
    UGameplayStatics::GetAllActorsWithTag(Controller->GetWorld(), FName("Plate"), PlateActors);
    if (PlateActors.Num() > 0)
    {
        PlateCenter = PlateActors[0]->GetActorLocation();
        
        // 접시 반경은 스케일 정보를 기반으로 추정
        FVector PlateScale = PlateActors[0]->GetActorScale3D();
        PlateRadius = FMath::Max(PlateScale.X, PlateScale.Y) * 10.0f; // 스케일에 비례하는 반경
    }
    
    // 플레이어 정보 가져오기
    APawn* PlayerPawn = Controller->GetPawn();
    if (!PlayerPawn)
    {
        UE_LOG(LogTemp, Error, TEXT("미리보기 실패: PlayerPawn이 NULL입니다!"));
        return;
    }
    
    // 플레이어에서 접시 방향 계산
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    FVector PlayerToPlate = (PlateCenter - PlayerLocation);
    PlayerToPlate.Z = 0; // 수평 방향만 고려
    PlayerToPlate.Normalize();
    
    // 접시 가장자리 계산
    FVector PlateEdge = PlateCenter - PlayerToPlate * PlateRadius;
    
    // 접시 가장자리에서 50유닛 더 뒤로 (플레이어 쪽으로)
    FVector PreviewLocation = PlateEdge - PlayerToPlate * 50.0f;
    
    // 미리보기 공 높이는 접시와 같게 설정
    PreviewLocation.Z = PlateCenter.Z;
    
    // 공통 함수 호출로 공 생성 (물리 비활성화)
    Controller->PreviewBall = SpawnBall(Controller, PreviewLocation, Controller->CurrentBallType, false);
    
    if (Controller->PreviewBall)
    {
        UE_LOG(LogTemp, Warning, TEXT("미리보기 공 생성 성공 - 위치: %s, 접시 중앙: %s"), 
            *PreviewLocation.ToString(), *PlateCenter.ToString());
    }
}