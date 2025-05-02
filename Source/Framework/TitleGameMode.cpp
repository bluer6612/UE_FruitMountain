#include "TitleGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Interface/UI/TitleLevel/MainMenu/MainMenuWidget.h"
#include "UObject/ConstructorHelpers.h"
#include "Interface/HUD/FruitHUD.h"
#include "Actors/PlateActor.h"
#include "Gameplay/Merging/Core/FruitMergeHelper.h"
#include "System/Camera/CameraOrbitFunctionLibrary.h"

ATitleGameMode::ATitleGameMode()
{
    HUDClass = AFruitHUD::StaticClass();

    // 블루프린트 클래스 참조 - 정확한 경로 지정
    static ConstructorHelpers::FClassFinder<UMainMenuWidget> MainMenuWidgetClassFinder(TEXT("/Game/UI/TitleLevel/BP_MainMenuWidget"));
    if (MainMenuWidgetClassFinder.Succeeded())
    {
        MainMenuWidgetClass = MainMenuWidgetClassFinder.Class;
        UE_LOG(LogTemp, Warning, TEXT("BP_MainMenuWidget 클래스 로드 성공"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("BP_MainMenuWidget 클래스를 찾을 수 없음!"));
    }

    // Plate 기본 클래스 지정
    PlateClass = APlateActor::StaticClass();
}

void ATitleGameMode::BeginPlay()
{
    Super::BeginPlay();

    FString MapName = GetWorld()->GetMapName();
    if (!MapName.Contains(TEXT("TitleLevel")))
    {
        UE_LOG(LogTemp, Warning, TEXT("TitleGameMode: 현재 맵이 TitleLevel이 아니므로 MainMenuWidget을 생성하지 않습니다. (MapName=%s)"), *MapName);
        return;
    }

    UWorld* World = GetWorld();
    if (World && MainMenuWidgetClass)
    {
        MainMenuWidget = CreateWidget<UMainMenuWidget>(World, MainMenuWidgetClass);
        if (MainMenuWidget)
        {
            MainMenuWidget->AddToViewport();

            // HUD에게 MainMenuWidget 인스턴스 전달
            if (AFruitHUD* FruitHUD = Cast<AFruitHUD>(World->GetFirstPlayerController()->GetHUD()))
            {
                UE_LOG(LogTemp, Warning, TEXT("TitleGameMode: SetMainMenuWidget 호출, MainMenuWidget=%p"), MainMenuWidget);
                FruitHUD->SetMainMenuWidget(MainMenuWidget);
            }

            // 입력 비활성화
            if (APlayerController* PC = World->GetFirstPlayerController())
            {
                PC->SetIgnoreMoveInput(true);
                PC->SetIgnoreLookInput(true);
            }
        }
    }

    // 과일 메시 사전 로드
    UFruitMergeHelper::PreloadAllFruitMeshes(GetWorld());
}

void ATitleGameMode::StartPlay()
{
    Super::StartPlay();

    // Plate 액터가 있는지 확인, 없으면 생성
    TArray<AActor*> PlateActors;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Plate"), PlateActors);
    if (PlateActors.Num() == 0)
    {
        if (PlateClass)
        {
            FActorSpawnParameters SpawnParams;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            FVector PlateLocation = FVector::ZeroVector;
            FRotator PlateRotation = FRotator::ZeroRotator;
            AActor* NewPlate = GetWorld()->SpawnActor<AActor>(PlateClass, PlateLocation, PlateRotation, SpawnParams);
            if (NewPlate)
            {
                NewPlate->Tags.Add(FName("Plate"));
                UE_LOG(LogTemp, Log, TEXT("ATitleGameMode 접시 액터가 생성되었습니다."));

                // --- 카메라 오빗 위치 조정: 접시가 화면 오른쪽에 보이도록 ---
                APawn* Pawn = GetWorld()->GetFirstPlayerController() ? GetWorld()->GetFirstPlayerController()->GetPawn() : nullptr;
                if (Pawn)
                {
                    float OrbitAngle = 30.0f;
                    float OrbitRadius = 150.0f;
                    UCameraOrbitFunctionLibrary::UpdateCameraOrbit(Pawn, NewPlate->GetActorLocation(), OrbitAngle, OrbitRadius);
                    UE_LOG(LogTemp, Warning, TEXT("Pawn - 접시 위치: %s"), *NewPlate->GetActorLocation().ToString());
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("Pawn을 찾을 수 없습니다."));
                }
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("ATitleGameMode 접시 액터 생성에 실패했습니다."));
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("ATitleGameMode PlateClass가 설정되어 있지 않습니다."));
        }
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("ATitleGameMode 이미 접시 액터가 존재합니다."));
    }
}