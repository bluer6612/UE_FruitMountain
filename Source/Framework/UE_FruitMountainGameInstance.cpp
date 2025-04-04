#include "UE_FruitMountainGameInstance.h"

#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "TimerManager.h"
#include "Interface/UI/SimpleTextureWidget.h"

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
    USimpleTextureWidget::CreateSimpleUI(GetWorld());
}

void UUE_FruitMountainGameInstance::ShowFruitUIWidget()
{
    // GetWorld()를 사용해 WorldContextObject 전달
    USimpleTextureWidget::CreateSimpleUI(GetWorld());
    UE_LOG(LogTemp, Warning, TEXT("ShowFruitUIWidget: SimpleTextureWidget 생성 완료"));
}