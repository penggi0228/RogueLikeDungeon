// RldOptionsWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RldOptionsWidget.generated.h"

class UCheckBox;
class UButton;
class UCmnSettingsSubsystem;

/**
 * ゲーム内オプション画面のロジックを行うWidget
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

    // ----- BindWidget -----

    // カメラ左右反転チェックボックス
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UCheckBox> CheckBoxInvertX;

    // カメラ上下反転チェックボックス
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UCheckBox> CheckBoxInvertY;

    // 適用ボタン
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> ButtonApply;

    // キャンセルボタン
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> ButtonCancel;

    // 閉じるボタン
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> ButtonClose;

private:

    // ----- Event Handlers -----

    /** カメラ左右反転チェック変更時の処理を行う */
    UFUNCTION()
    void OnInvertXChanged(bool bIsChecked);

    /** カメラ上下反転チェック変更時の処理を行う */
    UFUNCTION()
    void OnInvertYChanged(bool bIsChecked);

    /** 適用ボタンクリック時の処理を行う */
    UFUNCTION()
    void OnApplyClicked();

    /** キャンセルボタンクリック時の処理を行う */
    UFUNCTION()
    void OnCancelClicked();

    /** 閉じるボタンクリック時の処理を行う */
    UFUNCTION()
    void OnCloseClicked();

private:

    /** SettingsSubsystemを取得する */
    UCmnSettingsSubsystem* GetSettingsSubsystem() const;

    /** 現在の設定をUIへ反映する */
    void RefreshUIFromSettings();

    /** 適用ボタンの状態を更新する */
    void UpdateApplyButtonState();
};
