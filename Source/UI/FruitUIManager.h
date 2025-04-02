#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "FruitUIWidget.h"
#include "FruitUIManager.generated.h"

/**
 * UI 위젯들을 관리하는 매니저 클래스
 */
UCLASS()
class UE_FRUITMOUNTAIN_API UFruitUIManager : public UObject
{
    GENERATED_BODY()
    
public:
    // 생성자
    UFruitUIManager();
    
    // 싱글톤 인스턴스 가져오기
    UFUNCTION(BlueprintCallable, Category = "UI")
    static UFruitUIManager* GetInstance();
    
    // 초기화 함수
    UFUNCTION(BlueprintCallable, Category = "UI")
    void Initialize(APlayerController* InController);
    
    // UI 위젯 생성 함수
    UFUNCTION(BlueprintCallable, Category = "UI")
    void CreateUIWidgets(TSubclassOf<UFruitUIWidget> InWidgetClass = nullptr);
    
    // UI 위젯 이미지 설정
    UFUNCTION(BlueprintCallable, Category = "UI")
    void SetWidgetImage(int32 WidgetIndex, UTexture2D* NewTexture);
    
    // UI 위젯 전체 표시/숨김
    UFUNCTION(BlueprintCallable, Category = "UI")
    void SetAllWidgetsVisibility(bool bVisible);
    
    // UI 위젯 특정 인덱스 표시/숨김
    UFUNCTION(BlueprintCallable, Category = "UI")
    void SetWidgetVisibility(int32 WidgetIndex, bool bVisible);
    
    // 기본 UI 이미지 로드 함수
    UFUNCTION(BlueprintCallable, Category = "UI")
    void LoadDefaultImages();
    
    // 특정 이미지 로드 함수
    UFUNCTION(BlueprintCallable, Category = "UI")
    UTexture2D* LoadUITexture(const FString& ImageName);
    
private:
    // 싱글톤 인스턴스
    static UFruitUIManager* Instance;
    
    // UI 위젯 배열
    UPROPERTY()
    TArray<UFruitUIWidget*> UIWidgets;
    
    // 플레이어 컨트롤러 참조
    UPROPERTY()
    APlayerController* PlayerController;
};