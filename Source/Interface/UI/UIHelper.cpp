#include "UIHelper.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "Components/Widget.h"

void UUIHelper::SetAnchorForSlot(UCanvasPanelSlot* CanvasSlot, EWidgetAnchor Anchor, float PaddingX, float PaddingY)
{
    if (!CanvasSlot) return;
    
    // 앵커 타입에 따라 위치와 정렬 설정
    switch (Anchor)
    {
        case EWidgetAnchor::TopLeft:
            CanvasSlot->SetAnchors(FAnchors(0.0f, 0.0f, 0.0f, 0.0f));
            CanvasSlot->SetPosition(FVector2D(PaddingX, PaddingY));
            CanvasSlot->SetAlignment(FVector2D(0.0f, 0.0f));
            break;
            
        case EWidgetAnchor::TopRight:
            CanvasSlot->SetAnchors(FAnchors(1.0f, 0.0f, 1.0f, 0.0f));
            CanvasSlot->SetPosition(FVector2D(-PaddingX, PaddingY));
            CanvasSlot->SetAlignment(FVector2D(1.0f, 0.0f));
            break;
            
        case EWidgetAnchor::MiddleLeft:
            CanvasSlot->SetAnchors(FAnchors(0.0f, 0.5f, 0.0f, 0.5f));
            CanvasSlot->SetPosition(FVector2D(PaddingX, 0.0f));
            CanvasSlot->SetAlignment(FVector2D(0.0f, 0.5f));
            break;
            
        case EWidgetAnchor::MiddleRight:
            CanvasSlot->SetAnchors(FAnchors(1.0f, 0.5f, 1.0f, 0.5f));
            CanvasSlot->SetPosition(FVector2D(-PaddingX, 0.0f));
            CanvasSlot->SetAlignment(FVector2D(1.0f, 0.5f));
            break;
            
        case EWidgetAnchor::BottomLeft:
            CanvasSlot->SetAnchors(FAnchors(0.0f, 1.0f, 0.0f, 1.0f));
            CanvasSlot->SetPosition(FVector2D(PaddingX, -PaddingY));
            CanvasSlot->SetAlignment(FVector2D(0.0f, 1.0f));
            break;
            
        case EWidgetAnchor::BottomRight:
            CanvasSlot->SetAnchors(FAnchors(1.0f, 1.0f, 1.0f, 1.0f));
            CanvasSlot->SetPosition(FVector2D(-PaddingX, -PaddingY));
            CanvasSlot->SetAlignment(FVector2D(1.0f, 1.0f));
            break;
            
        case EWidgetAnchor::Center:
            CanvasSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
            CanvasSlot->SetPosition(FVector2D(0.0f, 0.0f));
            CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
            break;
    }
}

UTexture2D* UUIHelper::LoadAndApplyTexture(UImage* ImageWidget, const FString& TexturePath)
{
    if (!ImageWidget) return nullptr;
    
    // 텍스처 로드
    UTexture2D* LoadedTexture = LoadObject<UTexture2D>(nullptr, *TexturePath);
    if (!LoadedTexture)
    {
        // 실패 시 StaticLoadObject 시도
        LoadedTexture = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, *TexturePath));
    }

    if (LoadedTexture)
    {
        // 브러시 설정 
        FSlateBrush Brush;
        Brush.SetResourceObject(LoadedTexture);
        Brush.DrawAs = ESlateBrushDrawType::Image;
        
        // 원본 텍스처 크기 사용
        const int32 TexWidth = LoadedTexture->GetSizeX();
        const int32 TexHeight = LoadedTexture->GetSizeY();
        
        // 실제 텍스처 크기 적용
        Brush.ImageSize = FVector2D(TexWidth, TexHeight);
        
        // 이미지 속성 설정
        ImageWidget->SetBrush(Brush);
        ImageWidget->SetColorAndOpacity(FLinearColor::White);
        ImageWidget->SetRenderOpacity(1.0f);
        ImageWidget->SetVisibility(ESlateVisibility::Visible);
        
        UE_LOG(LogTemp, Log, TEXT("텍스처 로드 성공! 경로: %s, 실제 크기: %dx%d"), 
               *TexturePath, TexWidth, TexHeight);
    }
    else
    {
        // 텍스처 로드 실패 시 빨간색 박스 표시
        FSlateBrush DefaultBrush;
        DefaultBrush.TintColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);
        DefaultBrush.DrawAs = ESlateBrushDrawType::Box;
        
        ImageWidget->SetBrush(DefaultBrush);
        ImageWidget->SetVisibility(ESlateVisibility::Visible);
        
        UE_LOG(LogTemp, Error, TEXT("텍스처 로드 실패! 경로: %s"), *TexturePath);
    }
    
    return LoadedTexture;
}

void UUIHelper::SetupTextBlockStyle(UTextBlock* TextBlock, FLinearColor Color, int32 FontSize, bool bWithShadow, ESlateVisibility DefaultVisibility)
{
    if (!TextBlock)
        return;
    
    // 색상 설정
    TextBlock->SetColorAndOpacity(Color);
    
    // 폰트 설정
    TextBlock->SetFont(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), FontSize));
    
    // 그림자 설정
    if (bWithShadow)
    {
        TextBlock->SetShadowOffset(FVector2D(2.0f, 2.0f));
        TextBlock->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.5f));
    }
    
    // 기본 텍스트 및 가시성
    TextBlock->SetText(FText::FromString(TEXT("")));
    TextBlock->SetVisibility(DefaultVisibility);
}

UCanvasPanelSlot* UUIHelper::SetScoreDisplayPosition(UTextBlock* TextBlock, float PosX, float PosY, float Width, float Height, bool bRightAlign)
{
    if (!TextBlock)
        return nullptr;
    
    UCanvasPanelSlot* TextSlot = Cast<UCanvasPanelSlot>(TextBlock->Slot);
    if (TextSlot)
    {
        // 앵커 설정 (기본적으로 우측 앵커)
        TextSlot->SetAnchors(FAnchors(bRightAlign ? 1.0f : 0.0f, 0.0f, bRightAlign ? 1.0f : 0.0f, 0.0f));
        
        // 정렬 설정
        TextSlot->SetAlignment(FVector2D(bRightAlign ? 1.0f : 0.0f, 0.5f));
        
        // 위치 및 크기 설정
        TextSlot->SetPosition(FVector2D(PosX, PosY));
        TextSlot->SetSize(FVector2D(Width, Height));
    }
    
    return TextSlot;
}