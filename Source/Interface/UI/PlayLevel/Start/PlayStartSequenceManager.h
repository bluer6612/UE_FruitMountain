#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Components/Image.h"
#include "Interface/UI/Core/UIWidgetRenderer.h"
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
    void StartSequence(UObject* InWorldContextObject);
    
    // 시퀀스 완료 이벤트
    UPROPERTY(BlueprintAssignable, Category = "Game Sequence")
    FOnSequenceCompleted OnSequenceCompleted;

    // 싱글톤 인스턴스 접근자
    UFUNCTION(BlueprintCallable, Category = "Game Sequence")
    static UPlayStartSequenceManager* GetInstance();

    // 싱글톤 인스턴스 생성
    UFUNCTION(BlueprintCallable, Category = "Game Sequence", meta = (WorldContext = "WorldContextObject"))
    static UPlayStartSequenceManager* CreateInstance(UObject* WorldContextObject);
    
    // 외부 위젯 렌더러 설정 
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
    
    // 시퀀스 타이머 핸들
    FTimerHandle SequenceTimerHandle;
    
    // 애니메이션 진행 상태
    int32 CurrentPhase;
    float ElapsedTime;
    float PhaseDuration;
    
    // 리소스 경로
    FString ReadyTexturePath;
    FString StartTexturePath;
    
    // 애니메이션 관련 값
    float MaxReadyScaleFactor;
    float MaxStartScaleFactor;
    
    // 월드 객체 레퍼런스
    UObject* WorldContextObject;
    
    // 위젯 생성
    void CreateSequenceWidgets();
    
    // 이미지 로드 및 설정을 위한 헬퍼 함수
    void LoadAndSetupImage(UImage*& ImageWidget, const FString& TexturePath, bool bVisible, float InitialScale);
    
    // 타이머 업데이트
    UFUNCTION()
    void UpdateSequence();
};