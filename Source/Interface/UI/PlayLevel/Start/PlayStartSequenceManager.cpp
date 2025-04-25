#include "PlayStartSequenceManager.h"
#include "Interface/UI/Core/UIWidgetUtility.h"
#include "Engine/Engine.h"
#include "Components/CanvasPanelSlot.h"
#include "Kismet/GameplayStatics.h"

// 정적 인스턴스 초기화
UPlayStartSequenceManager* UPlayStartSequenceManager::Instance = nullptr;

UPlayStartSequenceManager::UPlayStartSequenceManager()
    : CurrentPhase(0)
    , ElapsedTime(0.0f)
    , PhaseDuration(0.0f)
    , ReadyTexturePath(TEXT("/Game/UI/PlayLevel/UI_Play_Ready"))
    , StartTexturePath(TEXT("/Game/UI/PlayLevel/UI_Play_Start"))
    , MaxReadyScaleFactor(2.0f)
    , MaxStartScaleFactor(1.75f)
    , WorldContextObject(nullptr)
{
}

void UPlayStartSequenceManager::StartSequence(UObject* InWorldContextObject)
{
    // 월드 컨텍스트 검증
    if (!InWorldContextObject)
    {
        UE_LOG(LogTemp, Error, TEXT("PlayStartSequence: 유효하지 않은 WorldContextObject"));
        return;
    }
    WorldContextObject = InWorldContextObject;
    
    // 이미 실행 중인 시퀀스가 있다면 정리
    if (WidgetRenderer)
    {
        APlayerController* PC = UUIWidgetUtility::GetValidPlayerController(WorldContextObject);
        if (PC)
        {
            PC->GetWorldTimerManager().ClearTimer(SequenceTimerHandle);
        }
    }
    
    // 위젯 생성
    CreateSequenceWidgets();
    
    // 시퀀스 초기화 - Ready 이미지 준비
    if (ReadyImage)
    {
        ReadyImage->SetRenderScale(FVector2D(MaxReadyScaleFactor, MaxReadyScaleFactor));
        ReadyImage->SetRenderOpacity(1.0f);
        ReadyImage->SetVisibility(ESlateVisibility::HitTestInvisible);
    }
    
    if (StartImage)
    {
        StartImage->SetRenderScale(FVector2D(1.1f, 1.1f));
        StartImage->SetRenderOpacity(1.0f);
        StartImage->SetVisibility(ESlateVisibility::Hidden);
    }
    
    // 시퀀스 시작 - 첫 번째 단계로 설정
    CurrentPhase = 0; // Ready 축소 단계
    ElapsedTime = 0.0f;
    PhaseDuration = 0.5f; // Ready 축소 시간 (0.5초)
    
    // 타이머 설정 - 더 빠른 업데이트 주기
    APlayerController* PC = UUIWidgetUtility::GetValidPlayerController(WorldContextObject);
    if (PC)
    {
        PC->GetWorldTimerManager().SetTimer(SequenceTimerHandle, 
                                           this, 
                                           &UPlayStartSequenceManager::UpdateSequence, 
                                           0.005f,  // 매우 작은 간격으로 업데이트
                                           true);
    }
    
    //UE_LOG(LogTemp, Display, TEXT("PlayStartSequence: 시작 시퀀스 시작됨"));
}

void UPlayStartSequenceManager::CreateSequenceWidgets()
{
    // UIWidgetRenderer 생성
    WidgetRenderer = UUIWidgetRenderer::CreateDisplayWidget(WorldContextObject);
    if (!WidgetRenderer)
    {
        UE_LOG(LogTemp, Error, TEXT("PlayStartSequence: UIWidgetRenderer 생성 실패"));
        return;
    }
    
    // Z-Order 설정
    if (WidgetRenderer && WidgetRenderer->IsInViewport())
    {
        WidgetRenderer->RemoveFromParent();
        WidgetRenderer->AddToViewport(9999);
        UE_LOG(LogTemp, Warning, TEXT("PlayStartSequence: Z-Order를 9999로 설정"));
    }

    // 이미지 생성 및 설정을 헬퍼 함수로 간소화
    LoadAndSetupImage(ReadyImage, ReadyTexturePath, true, MaxReadyScaleFactor);  // Ready는 보이게
    LoadAndSetupImage(StartImage, StartTexturePath, false, 1.0f);  // Start는 안 보이게, 원래 크기
    
    //UE_LOG(LogTemp, Display, TEXT("PlayStartSequence: 위젯 생성 완료"));
}

void UPlayStartSequenceManager::LoadAndSetupImage(UImage*& ImageWidget, const FString& TexturePath, bool bVisible, float InitialScale)
{
    // 이미지 위젯 생성
    ImageWidget = nullptr;
    WidgetRenderer->RenderUIImage(ImageWidget, 
                                EWidgetAnchor::Center, 
                                TexturePath, 
                                FVector2D(807, 230),
                                0.0f, 0.0f);
                                
    if (ImageWidget)
    {
        // 스케일 설정
        ImageWidget->SetRenderScale(FVector2D(InitialScale, InitialScale));
        
        // 투명도 설정
        ImageWidget->SetRenderOpacity(1.0f);
        
        // 가시성 설정
        ImageWidget->SetVisibility(bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("이미지 위젯 생성 실패: %s"), *TexturePath);
    }
}

void UPlayStartSequenceManager::UpdateSequence()
{
    APlayerController* PC = UUIWidgetUtility::GetValidPlayerController(WorldContextObject);
    if (!PC) return;
    
    float DeltaTime = PC->GetWorld()->GetDeltaSeconds();
    ElapsedTime += DeltaTime;
    
    // 현재 단계에 따른 처리
    switch (CurrentPhase)
    {
        case 0: // Ready 축소 단계
            if (ReadyImage)
            {
                float Progress = FMath::Clamp(ElapsedTime / PhaseDuration, 0.0f, 1.0f);
                float CurrentScale = MaxReadyScaleFactor - ((MaxReadyScaleFactor - 1.0f) * Progress);
                ReadyImage->SetRenderScale(FVector2D(CurrentScale, CurrentScale));
            }
            break;
            
        case 1: // Ready 유지 단계
            // 특별한 처리 없음 (타이머만 증가)
            break;
            
        case 2: // Start 확대 단계 - 0.25초로 단축
            if (StartImage)
            {
                float Progress = FMath::Clamp(ElapsedTime / PhaseDuration, 0.0f, 1.0f);
                float CurrentScale = 1.0f + ((MaxStartScaleFactor - 1.0f) * Progress);
                StartImage->SetRenderScale(FVector2D(CurrentScale, CurrentScale));
                
                // 0.5배 시점에 페이드아웃 시작 (0.125초부터)
                if (ElapsedTime >= (PhaseDuration * 0.5f))
                {
                    float FadeProgress = (ElapsedTime - (PhaseDuration * 0.5f)) / (PhaseDuration * 0.5f);
                    FadeProgress = FMath::Clamp(FadeProgress, 0.0f, 1.0f);
                    float CurrentOpacity = 1.0f - (0.65f * FadeProgress); // 65% 투명도까지 (0.35 남음)
                    StartImage->SetRenderOpacity(CurrentOpacity);
                }
            }
            break;
            
        case 3: // 마지막 페이드아웃 단계 (0.5초부터 2초까지 완전히 사라짐)
            if (StartImage)
            {
                float Progress = FMath::Clamp(ElapsedTime / PhaseDuration, 0.0f, 1.0f);
                float CurrentOpacity = 0.35f - (0.35f * Progress); // 50%에서 0%로
                StartImage->SetRenderOpacity(CurrentOpacity);
            }
            break;
            
        case 4: // 완료 단계
            // 아무것도 안함
            break;
    }
    
    // 단계 완료 확인 및 다음 단계로 진행
    if (ElapsedTime >= PhaseDuration && CurrentPhase < 4)
    {
        // 다음 단계로 진행
        CurrentPhase++;
        ElapsedTime = 0.0f;
        
        // 단계별 처리
        switch (CurrentPhase)
        {
            case 1: // Ready 유지 단계
                PhaseDuration = 1.25f;
                break;
                
            case 2: // Start 확대 단계
                PhaseDuration = 0.25f;
                // Ready 이미지 숨기기
                if (ReadyImage) ReadyImage->SetVisibility(ESlateVisibility::Hidden);
                // Start 이미지 표시
                if (StartImage)
                {
                    StartImage->SetVisibility(ESlateVisibility::HitTestInvisible);
                    StartImage->SetRenderOpacity(1.0f);
                    StartImage->SetRenderScale(FVector2D(1.0f, 1.0f));
                }
                break;
                
            case 3: // 마지막 페이드아웃
                PhaseDuration = 1.5f;
                OnSequenceCompleted.Broadcast();
                break;
                
            case 4: // 완료 단계
                // 타이머 중지
                if (PC) PC->GetWorldTimerManager().ClearTimer(SequenceTimerHandle);
                // 위젯 숨기기
                if (ReadyImage) ReadyImage->SetVisibility(ESlateVisibility::Hidden);
                if (StartImage) StartImage->SetVisibility(ESlateVisibility::Hidden);
                //UE_LOG(LogTemp, Display, TEXT("PlayStartSequence: 시퀀스 완료"));
                break;
        }
    }
}

UPlayStartSequenceManager* UPlayStartSequenceManager::GetInstance()
{
    return Instance;
}

UPlayStartSequenceManager* UPlayStartSequenceManager::CreateInstance(UObject* WorldContextObject)
{
    if (!Instance)
    {
        Instance = NewObject<UPlayStartSequenceManager>();
        Instance->AddToRoot(); // 가비지 컬렉션 방지
    }
    
    return Instance;
}

void UPlayStartSequenceManager::SetExistingWidgetRenderer(UUIWidgetRenderer* ExistingRenderer)
{
    if (ExistingRenderer && IsValid(ExistingRenderer))
    {
        // 기존 위젯 정리
        if (WidgetRenderer && WidgetRenderer != ExistingRenderer)
        {
            WidgetRenderer->RemoveFromParent();
        }
        
        WidgetRenderer = ExistingRenderer;
        //UE_LOG(LogTemp, Warning, TEXT("PlayStartSequence: 외부에서 제공된 UIWidgetRenderer 사용"));
    }
}