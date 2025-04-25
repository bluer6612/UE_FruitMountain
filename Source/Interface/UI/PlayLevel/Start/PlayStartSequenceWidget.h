#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "TimerManager.h"
#include "PlayStartSequenceWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSequenceFinished);

/**
 * 게임 시작 애니메이션 시퀀스를 처리하는 위젯 클래스
 * Ready -> Start 순서로 표시되며 각각 크기 변환 및 페이드 효과를 가짐
 */
UCLASS()
class UE_FRUITMOUNTAIN_API UPlayStartSequenceWidget : public UUserWidget
{
    GENERATED_BODY()
    
public:
    UPlayStartSequenceWidget(const FObjectInitializer& ObjectInitializer);
    
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
    
    // 애니메이션 시퀀스 시작
    UFUNCTION(BlueprintCallable, Category = "Game Sequence")
    void StartSequence();
    
    // 애니메이션 시퀀스 완료 이벤트
    UPROPERTY(BlueprintAssignable, Category = "Game Sequence")
    FOnSequenceFinished OnSequenceFinished;
    
protected:
    // UI 이미지 컴포넌트
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UImage* ReadyImage;
    
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UImage* StartImage;
    
    // 리소스 경로
    UPROPERTY(EditDefaultsOnly, Category = "Resources")
    FString ReadyTexturePath = TEXT("/Game/UI/PlayLevel/UI_Play_Ready");
    
    UPROPERTY(EditDefaultsOnly, Category = "Resources")
    FString StartTexturePath = TEXT("/Game/UI/PlayLevel/UI_Play_Start");
    
    // 애니메이션 단계
    enum class ESequenceState : uint8
    {
        Inactive,
        ReadyShrinking,   // Ready 이미지 크기 줄이기
        ReadyFadingOut,   // Ready 이미지 페이드 아웃
        StartGrowing,     // Start 이미지 크기 키우기
        StartFadingOut,   // Start 이미지 페이드 아웃
        Completed
    };
    
    // 현재 애니메이션 상태
    ESequenceState CurrentState;
    
    // 애니메이션 타이머
    float AnimationTimer;
    
    // 애니메이션 설정
    UPROPERTY(EditDefaultsOnly, Category = "Animation")
    float ReadyShrinkTime = 1.5f;
    
    UPROPERTY(EditDefaultsOnly, Category = "Animation")
    float ReadyFadeOutTime = 0.5f;
    
    UPROPERTY(EditDefaultsOnly, Category = "Animation")
    float StartGrowTime = 1.5f;
    
    UPROPERTY(EditDefaultsOnly, Category = "Animation")
    float StartFadeOutTime = 0.5f;
    
    UPROPERTY(EditDefaultsOnly, Category = "Animation")
    float MaxScaleFactor = 1.5f;
    
    // 애니메이션 상태 전환
    void SetSequenceState(ESequenceState NewState);
    
    // 텍스처 로드
    UFUNCTION()
    void LoadTextures();
    
    // 정적 인스턴스 및 생성 함수
    static UPlayStartSequenceWidget* Instance;
    
public:
    // 싱글톤 접근자
    static UPlayStartSequenceWidget* GetInstance();
    
    // 위젯 생성 함수
    UFUNCTION(BlueprintCallable, Category = "UI")
    static UPlayStartSequenceWidget* CreatePlayStartSequence(UObject* WorldContextObject);
};