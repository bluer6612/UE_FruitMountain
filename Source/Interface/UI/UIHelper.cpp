#include "UIHelper.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"

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