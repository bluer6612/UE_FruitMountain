#include "UE_FruitMountainGameInstance.h"
#include "Interface/UI/TextureDisplayWidget.h"

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
    UTextureDisplayWidget::CreateDisplayWidget(GetWorld());
}

void UUE_FruitMountainGameInstance::ShowFruitUIWidget()
{
    // GetWorld()를 사용해 WorldContextObject 전달
    UTextureDisplayWidget::CreateDisplayWidget(GetWorld());
}