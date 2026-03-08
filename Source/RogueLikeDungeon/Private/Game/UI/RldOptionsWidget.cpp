// RldOptionsWidget.cpp

#include "Game/UI/RldOptionsWidget.h"

#include "Components/CheckBox.h"
#include "Components/Button.h"
#include "Engine/GameInstance.h"
#include "Common/Settings/CmnSettingsSubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogRldOptions, Log, All);

/**
 * ゲーム内オプション画面ロジック部分
 * 初期化時のイベントBind
 */
void URldOptionsWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    /* カメラ左右反転 */
    if (CheckBoxInvertX)
    {
        CheckBoxInvertX->OnCheckStateChanged.AddDynamic(this, &URldOptionsWidget::OnInvertXChanged);
    }

    /* カメラ上下反転 */
    if (CheckBoxInvertY)
    {
        CheckBoxInvertY->OnCheckStateChanged.AddDynamic(this, &URldOptionsWidget::OnInvertYChanged);
    }

    /* 適用ボタン */
    if (ButtonApply)
    {
        ButtonApply->OnClicked.AddDynamic(this, &URldOptionsWidget::OnApplyClicked);
    }

    /* キャンセルボタン */
    if (ButtonCancel)
    {
        ButtonCancel->OnClicked.AddDynamic(this, &URldOptionsWidget::OnCancelClicked);
    }

    /* 閉じるボタン */
    if (ButtonClose)
    {
        ButtonClose->OnClicked.AddDynamic(this, &URldOptionsWidget::OnCloseClicked);
    }
}


/**
 * ゲーム内オプション画面ロジック部分のコンストラクタ
 * 現在の設定をUIへ反映
 * 適用ボタンの状態更新
 */
void URldOptionsWidget::NativeConstruct()
{
    Super::NativeConstruct();

    RefreshUIFromSettings();    // Subsystemの現在値をUIへ反映
    UpdateApplyButtonState();  //  適用ボタンの状態更新
}

/**
 * Subsystem取得
 */
UCmnSettingsSubsystem* URldOptionsWidget::GetSettingsSubsystem() const
{
    if (!GetGameInstance())
    {
        return nullptr;
    }

    return GetGameInstance()->GetSubsystem<UCmnSettingsSubsystem>();
}

/**
 * Subsystemの現在値をUIへ反映する
 */
void URldOptionsWidget::RefreshUIFromSettings()
{
    UCmnSettingsSubsystem* Settings = GetSettingsSubsystem();
    if (!Settings)
    {
        return;
    }

    /* カメラ左右反転 */
    if (CheckBoxInvertX)
    {
        CheckBoxInvertX->SetIsChecked(Settings->GetInvertCameraX());
    }

    /* カメラ上下反転 */
    if (CheckBoxInvertY)
    {
        CheckBoxInvertY->SetIsChecked(Settings->GetInvertCameraY());
    }
}

/**
 * 左右反転チェック変更
 */
void URldOptionsWidget::OnInvertXChanged(bool bIsChecked)
{
    if (UCmnSettingsSubsystem* Settings = GetSettingsSubsystem())
    {
        Settings->SetInvertCameraX(bIsChecked);
    }

    UE_LOG(LogRldOptions, Log, TEXT("InvertX Changed: %d"), bIsChecked ? 1 : 0);

    // 適用ボタンの状態更新
    UpdateApplyButtonState();
}

/**
 * 上下反転チェック変更
 */
void URldOptionsWidget::OnInvertYChanged(bool bIsChecked)
{
    if (UCmnSettingsSubsystem* Settings = GetSettingsSubsystem())
    {
        Settings->SetInvertCameraY(bIsChecked);
    }

    UE_LOG(LogRldOptions, Log, TEXT("InvertY Changed: %d"), bIsChecked ? 1 : 0);

    // 適用ボタンの状態更新
    UpdateApplyButtonState();
}

/**
 * 適用ボタンクリック
 */
void URldOptionsWidget::OnApplyClicked()
{
    if (UCmnSettingsSubsystem* Settings = GetSettingsSubsystem())
    {
        Settings->ApplyAndSaveInputOptions();
    }

    UE_LOG(LogRldOptions, Log, TEXT("Apply Clicked"));
}

/**
 * キャンセルボタンクリック
 * ・保存せず、保存済み状態に戻す
 */
void URldOptionsWidget::OnCancelClicked()
{
    if (UCmnSettingsSubsystem* Settings = GetSettingsSubsystem())
    {
        Settings->ReloadInputOptions();
    }

    RefreshUIFromSettings();

    UE_LOG(LogRldOptions, Log, TEXT("キャンセルボタンがクリックされました"));
}


/**
 * 閉じるボタンクリック
 */
void URldOptionsWidget::OnCloseClicked()
{
    UCmnSettingsSubsystem* Settings = GetSettingsSubsystem();
    if (!Settings)
    {
        RemoveFromParent();
        return;
    }

    if (!Settings->HasUnsavedChanges())
    {
        RemoveFromParent();
        return;
    }
    
    // 未保存あり → 簡易警告
    //UE_LOG(LogRldOptions, Warning, TEXT("変更が保存されていません"));

    // ★実務ではここで確認ダイアログを出す
}

/**
 * 適用ボタンの状態更新
 */
void URldOptionsWidget::UpdateApplyButtonState()
{
    if (!ButtonApply)
    {
        return;
    }

    if (UCmnSettingsSubsystem* Settings = GetSettingsSubsystem())
    {
        ButtonApply->SetIsEnabled(Settings->HasUnsavedChanges());
    }
}
