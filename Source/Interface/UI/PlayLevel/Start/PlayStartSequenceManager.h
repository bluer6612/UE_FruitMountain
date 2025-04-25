#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Components/Image.h"
#include "Interface/UI/Core/UIWidgetRenderer.h"
#include "TimerManager.h"
#include "PlayStartSequenceManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSequenceCompleted);

/**
 * 게임 시작 애니메이션 시퀀스를 관리하는 클래스
 * UIWidgetRenderer를 활용하여 텍스처를 표시하고 애니메이션 처리
 */
UCLASS()
class UE_FRUITMOUNTAIN_API UPlayStartSequenceManager : public UObject
{
    GENERATED_BODY()

public:
    UPlayStartSequenceManager();
    
    // 시퀀스 시작
    UFUNCTION(BlueprintCallable, Category = "Game Sequence")
    void StartSequence(UObject* WorldContextObject);
    
    // 시퀀스 완료 이벤트
    UPROPERTY(BlueprintAssignable, Category = "Game Sequence")
    FOnSequenceCompleted OnSequenceCompleted;

    // 싱글톤 인스턴스 접근자
    UFUNCTION(BlueprintCallable, Category = "Game Sequence")
    static UPlayStartSequenceManager* GetInstance();

    // 싱글톤 인스턴스 생성
    UFUNCTION(BlueprintCallable, Category = "Game Sequence", meta = (WorldContext = "WorldContextObject"))
    static UPlayStartSequenceManager* CreateInstance(UObject* WorldContextObject);

    UFUNCTION(BlueprintCallable, Category = "Game Sequence")
    void SetExistingWidgetRenderer(UUIWidgetRenderer* ExistingRenderer);

private:
    // 싱글톤 인스턴스
    static UPlayStartSequenceManager* Instance;
    
    // UIWidgetRenderer 레퍼런스
    UPROPERTY()
    UUIWidgetRenderer* WidgetRenderer;
    
    // 이미지 레퍼런스
    UPROPERTY()
    UImage* ReadyImage;
    
    UPROPERTY()
    UImage* StartImage;
    
    // 시퀀스 단계
    enum class ESequenceStep : uint8
    {
        None,
        ReadyShrink,
        ReadyFadeOut,
        StartGrow,
        StartFadeOut,
        Complete
    };
    
    ESequenceStep CurrentStep;
    
    // 시퀀스 타이머 핸들
    FTimerHandle SequenceTimerHandle;
    
    // 애니메이션 진행 시간
    float ElapsedTime;
    float TotalDuration;
    
    // 리소스 경로
    FString ReadyTexturePath;
    FString StartTexturePath;
    
    // 애니메이션 관련 값
    float MaxScaleFactor;
    
    // 위젯 생성
    void CreateSequenceWidgets();
    
    // 애니메이션 단계 처리 함수
    void ProcessReadyShrink(float DeltaTime);
    void ProcessReadyFadeOut(float DeltaTime);
    void ProcessStartGrow(float DeltaTime);
    void ProcessStartFadeOut(float DeltaTime);
    
    // 타이머 업데이트
    UFUNCTION()
    void UpdateSequence();
    
    // 다음 단계로 진행
    void AdvanceToNextStep();
    
    // 시퀀스 초기화
    void InitializeSequence();
    
    // 시퀀스 완료
    void CompleteSequence();
    
    // 월드 객체 레퍼런스
    UObject* WorldContextObject;
};