// RldOptionsWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RldOptionsWidget.generated.h"

class UCheckBox;
class UButton;
class UCmnSettingsSubsystem;

/**
 * ゲーム内オプション画面のロジック部分
 */
UCLASS()
class ROGUELIKEDUNGEON_API URldOptionsWidget : public UUserWidget
{
    GENERATED_BODY()

protected:

    // ----- UUserWidget -----

    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;

private:

    // ===== BindWidget（Blueprint側に必ず同名で配置すること）=====

    /** カメラ左右反転チェック */
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UCheckBox> CheckBoxInvertX;

    /** カメラ上下反転チェック */
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UCheckBox> CheckBoxInvertY;

    /** 適用ボタン */
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> ButtonApply;

    /** キャンセルボタン */
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> ButtonCancel;

    /** 閉じるボタン */
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> ButtonClose;

private:

    // ===== Event Handlers =====

    /** カメラ左右反転チェック ON/OFFイベント */
    UFUNCTION()
    void OnInvertXChanged(bool bIsChecked);

    /** カメラ上下反転チェック ON/OFFイベント */
    UFUNCTION()
    void OnInvertYChanged(bool bIsChecked);

    /** 適用ボタンクリックイベント */
    UFUNCTION()
    void OnApplyClicked();

    /** キャンセルボタンクリックイベント */
    UFUNCTION()
    void OnCancelClicked();

    /** 閉じるボタンクリックイベント */
    UFUNCTION()
    void OnCloseClicked();

private:

    /** Subsystem取得 */
    UCmnSettingsSubsystem* GetSettingsSubsystem() const;

    /** 現在の設定をUIへ反映 */
    void RefreshUIFromSettings();

    /** 適用ボタンの状態更新 */
    void UpdateApplyButtonState();
};