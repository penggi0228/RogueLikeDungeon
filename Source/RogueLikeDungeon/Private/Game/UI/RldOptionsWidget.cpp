// RldOptionsWidget.cpp

#include "Game/UI/RldOptionsWidget.h"

#include "Components/CheckBox.h"
#include "Components/Button.h"
#include "Engine/GameInstance.h"
#include "Common/Settings/CmnSettingsSubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogRldOptions, Log, All);

/** 初期化時のイベント登録を行う */
void URldOptionsWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    // カメラ左右反転チェックボックス取得時はイベント登録
    if (CheckBoxInvertX)
    {
        CheckBoxInvertX->OnCheckStateChanged.AddDynamic(this, &URldOptionsWidget::OnInvertXChanged);
    }

    // カメラ上下反転チェックボックス取得時はイベント登録
    if (CheckBoxInvertY)
    {
        CheckBoxInvertY->OnCheckStateChanged.AddDynamic(this, &URldOptionsWidget::OnInvertYChanged);
    }

    // 適用ボタン取得時はイベント登録
    if (ButtonApply)
    {
        ButtonApply->OnClicked.AddDynamic(this, &URldOptionsWidget::OnApplyClicked);
    }

    // キャンセルボタン取得時はイベント登録
    if (ButtonCancel)
    {
        ButtonCancel->OnClicked.AddDynamic(this, &URldOptionsWidget::OnCancelClicked);
    }

    // 閉じるボタン取得時はイベント登録
    if (ButtonClose)
    {
        ButtonClose->OnClicked.AddDynamic(this, &URldOptionsWidget::OnCloseClicked);
    }
}

/** 構築時に設定値とボタン状態を反映する */
void URldOptionsWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 現在の設定をUIへ反映
    RefreshUIFromSettings();

    // 適用ボタンの状態を更新
    UpdateApplyButtonState();
}

/** SettingsSubsystemを取得する */
UCmnSettingsSubsystem* URldOptionsWidget::GetSettingsSubsystem() const
{
    if (!GetGameInstance())
    {
        return nullptr;
    }

    return GetGameInstance()->GetSubsystem<UCmnSettingsSubsystem>();
}

/** 現在の設定をUIへ反映する */
void URldOptionsWidget::RefreshUIFromSettings()
{
    UCmnSettingsSubsystem* Settings = GetSettingsSubsystem();

    // SettingsSubsystem未取得時は反映しない
    if (!Settings)
    {
        return;
    }

    // カメラ左右反転チェックボックス取得時は設定値を反映
    if (CheckBoxInvertX)
    {
        CheckBoxInvertX->SetIsChecked(Settings->GetInvertCameraX());
    }

    // カメラ上下反転チェックボックス取得時は設定値を反映
    if (CheckBoxInvertY)
    {
        CheckBoxInvertY->SetIsChecked(Settings->GetInvertCameraY());
    }
}

/** カメラ左右反転チェック変更時の処理を行う */
void URldOptionsWidget::OnInvertXChanged(bool bIsChecked)
{
    if (UCmnSettingsSubsystem* Settings = GetSettingsSubsystem())
    {
        Settings->SetInvertCameraX(bIsChecked);
    }

    UE_LOG(LogRldOptions, Log, TEXT("OnInvertXChanged: 値=%d"), bIsChecked ? 1 : 0);

    // 適用ボタンの状態を更新
    UpdateApplyButtonState();
}

/** カメラ上下反転チェック変更時の処理を行う */
void URldOptionsWidget::OnInvertYChanged(bool bIsChecked)
{
    if (UCmnSettingsSubsystem* Settings = GetSettingsSubsystem())
    {
        Settings->SetInvertCameraY(bIsChecked);
    }

    UE_LOG(LogRldOptions, Log, TEXT("OnInvertYChanged: 値=%d"), bIsChecked ? 1 : 0);

    // 適用ボタンの状態を更新
    UpdateApplyButtonState();
}

/** 適用ボタンクリック時の処理を行う */
void URldOptionsWidget::OnApplyClicked()
{
    if (UCmnSettingsSubsystem* Settings = GetSettingsSubsystem())
    {
        Settings->ApplyAndSaveInputOptions();
    }

    UE_LOG(LogRldOptions, Log, TEXT("OnApplyClicked: 適用ボタン押下"));

    // 適用後の状態を反映
    UpdateApplyButtonState();
}

/** キャンセルボタンクリック時の処理を行う */
void URldOptionsWidget::OnCancelClicked()
{
    if (UCmnSettingsSubsystem* Settings = GetSettingsSubsystem())
    {
        Settings->ReloadInputOptions();
    }

    // 保存済みの設定値をUIへ再反映
    RefreshUIFromSettings();

    // 適用ボタンの状態を更新
    UpdateApplyButtonState();

    UE_LOG(LogRldOptions, Log, TEXT("OnCancelClicked: キャンセルボタン押下"));
}

/** 閉じるボタンクリック時の処理を行う */
void URldOptionsWidget::OnCloseClicked()
{
    UCmnSettingsSubsystem* Settings = GetSettingsSubsystem();

    // SettingsSubsystem未取得時はそのまま閉じる
    if (!Settings)
    {
        RemoveFromParent();
        return;
    }

    // 未保存変更がない場合はそのまま閉じる
    if (!Settings->HasUnsavedChanges())
    {
        RemoveFromParent();
        return;
    }

    // 未保存変更がある場合は確認処理を行う想定
    // UE_LOG(LogRldOptions, Warning, TEXT("OnCloseClicked: 変更が保存されていません"));
}

/** 適用ボタンの状態を更新する */
void URldOptionsWidget::UpdateApplyButtonState()
{
    // 適用ボタン未取得時は更新しない
    if (!ButtonApply)
    {
        return;
    }

    if (UCmnSettingsSubsystem* Settings = GetSettingsSubsystem())
    {
        ButtonApply->SetIsEnabled(Settings->HasUnsavedChanges());
    }
}
