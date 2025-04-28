#include "UE_FruitMountainGameInstance.h"
#include "Interface/UI/Core/UIWidgetRenderer.h"

UUE_FruitMountainGameInstance::UUE_FruitMountainGameInstance()
{
    // 생성자 코드
}

void UUE_FruitMountainGameInstance::Init()
{
    Super::Init();
    UE_LOG(LogTemp, Error, TEXT("GameInstance Init 호출됨"));
}

void UUE_FruitMountainGameInstance::CheckPersistentUI()
{
    // GetWorld()를 사용해 WorldContextObject 전달
    UUIWidgetRenderer::CreateDisplayWidget(GetWorld());
}

void UUE_FruitMountainGameInstance::ShowFruitUIWidget()
{
    UWorld* World = GetWorld();
    if (!World) return;

    FString MapName = World->GetMapName();

    if (MapName.Contains(TEXT("PlayLevel")))
    {
        UUIWidgetRenderer::CreateDisplayWidget(World);
    }
    // TitleLevel일 때는 Title UI만 생성 (여기선 아무것도 안함)
}