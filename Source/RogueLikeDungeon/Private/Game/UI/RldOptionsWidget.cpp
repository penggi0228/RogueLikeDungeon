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
    else
    {
        UE_LOG(
            LogRldOptions,
            Warning,
            TEXT("NativeOnInitialized: Widget=%s CheckBoxInvertXが未設定です"),
            *GetNameSafe(this)
        );
    }

    // カメラ上下反転チェックボックス取得時はイベント登録
    if (CheckBoxInvertY)
    {
        CheckBoxInvertY->OnCheckStateChanged.AddDynamic(this, &URldOptionsWidget::OnInvertYChanged);
    }
    else
    {
        UE_LOG(
            LogRldOptions,
            Warning,
            TEXT("NativeOnInitialized: Widget=%s CheckBoxInvertYが未設定です"),
            *GetNameSafe(this)
        );
    }

    // 適用ボタン取得時はイベント登録
    if (ButtonApply)
    {
        ButtonApply->OnClicked.AddDynamic(this, &URldOptionsWidget::OnApplyClicked);
    }
    else
    {
        UE_LOG(
            LogRldOptions,
            Warning,
            TEXT("NativeOnInitialized: Widget=%s ButtonApplyが未設定です"),
            *GetNameSafe(this)
        );
    }

    // キャンセルボタン取得時はイベント登録
    if (ButtonCancel)
    {
        ButtonCancel->OnClicked.AddDynamic(this, &URldOptionsWidget::OnCancelClicked);
    }
    else
    {
        UE_LOG(
            LogRldOptions,
            Warning,
            TEXT("NativeOnInitialized: Widget=%s ButtonCancelが未設定です"),
            *GetNameSafe(this)
        );
    }

    // 閉じるボタン取得時はイベント登録
    if (ButtonClose)
    {
        ButtonClose->OnClicked.AddDynamic(this, &URldOptionsWidget::OnCloseClicked);
    }
    else
    {
        UE_LOG(
            LogRldOptions,
            Warning,
            TEXT("NativeOnInitialized: Widget=%s ButtonCloseが未設定です"),
            *GetNameSafe(this)
        );
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
    UGameInstance* gameInstance = GetGameInstance();

    // GameInstance未取得時はSubsystemを取得しない
    if (!gameInstance)
    {
        return nullptr;
    }

    return gameInstance->GetSubsystem<UCmnSettingsSubsystem>();
}

/** 現在の設定をUIへ反映する */
void URldOptionsWidget::RefreshUIFromSettings()
{
    UCmnSettingsSubsystem* settings = GetSettingsSubsystem();

    // SettingsSubsystem未取得時は反映しない
    if (!settings)
    {
        UE_LOG(
            LogRldOptions,
            Warning,
            TEXT("RefreshUIFromSettings: Widget=%s SettingsSubsystem未取得のためUIへ反映しません"),
            *GetNameSafe(this)
        );

        return;
    }

    // カメラ左右反転チェックボックス取得時は設定値を反映
    if (CheckBoxInvertX)
    {
        CheckBoxInvertX->SetIsChecked(settings->GetInvertCameraX());
    }

    // カメラ上下反転チェックボックス取得時は設定値を反映
    if (CheckBoxInvertY)
    {
        CheckBoxInvertY->SetIsChecked(settings->GetInvertCameraY());
    }

    UE_LOG(
        LogRldOptions,
        Verbose,
        TEXT("RefreshUIFromSettings: Widget=%s UI反映完了 左右反転=%s 上下反転=%s"),
        *GetNameSafe(this),
        settings->GetInvertCameraX() ? TEXT("有効") : TEXT("無効"),
        settings->GetInvertCameraY() ? TEXT("有効") : TEXT("無効")
    );
}

/** カメラ左右反転チェック変更時の処理を行う */
void URldOptionsWidget::OnInvertXChanged(bool bIsChecked)
{
    UCmnSettingsSubsystem* settings = GetSettingsSubsystem();

    // SettingsSubsystem取得時は左右反転設定を更新
    if (settings)
    {
        settings->SetInvertCameraX(bIsChecked);
    }
    else
    {
        UE_LOG(
            LogRldOptions,
            Warning,
            TEXT("OnInvertXChanged: Widget=%s SettingsSubsystem未取得のため左右反転設定を更新できません 左右反転=%s"),
            *GetNameSafe(this),
            bIsChecked ? TEXT("有効") : TEXT("無効")
        );
    }

    UE_LOG(
        LogRldOptions,
        Log,
        TEXT("OnInvertXChanged: Widget=%s カメラ左右反転を変更しました 左右反転=%s"),
        *GetNameSafe(this),
        bIsChecked ? TEXT("有効") : TEXT("無効")
    );

    // 適用ボタンの状態を更新
    UpdateApplyButtonState();
}

/** カメラ上下反転チェック変更時の処理を行う */
void URldOptionsWidget::OnInvertYChanged(bool bIsChecked)
{
    UCmnSettingsSubsystem* settings = GetSettingsSubsystem();

    // SettingsSubsystem取得時は上下反転設定を更新
    if (settings)
    {
        settings->SetInvertCameraY(bIsChecked);
    }
    else
    {
        UE_LOG(
            LogRldOptions,
            Warning,
            TEXT("OnInvertYChanged: Widget=%s SettingsSubsystem未取得のため上下反転設定を更新できません 上下反転=%s"),
            *GetNameSafe(this),
            bIsChecked ? TEXT("有効") : TEXT("無効")
        );
    }

    UE_LOG(
        LogRldOptions,
        Log,
        TEXT("OnInvertYChanged: Widget=%s カメラ上下反転を変更しました 上下反転=%s"),
        *GetNameSafe(this),
        bIsChecked ? TEXT("有効") : TEXT("無効")
    );

    // 適用ボタンの状態を更新
    UpdateApplyButtonState();
}

/** 適用ボタンクリック時の処理を行う */
void URldOptionsWidget::OnApplyClicked()
{
    UCmnSettingsSubsystem* settings = GetSettingsSubsystem();

    // SettingsSubsystem未取得時は保存しない
    if (!settings)
    {
        UE_LOG(
            LogRldOptions,
            Warning,
            TEXT("OnApplyClicked: Widget=%s SettingsSubsystem未取得のため入力オプションを保存できません"),
            *GetNameSafe(this)
        );

        return;
    }

    const bool bSaved = settings->ApplyAndSaveInputOptions();

    // 保存失敗時は警告を出す
    if (!bSaved)
    {
        UE_LOG(
            LogRldOptions,
            Warning,
            TEXT("OnApplyClicked: Widget=%s 入力オプションの保存に失敗しました"),
            *GetNameSafe(this)
        );

        return;
    }

    UE_LOG(
        LogRldOptions,
        Log,
        TEXT("OnApplyClicked: Widget=%s 入力オプションを保存しました"),
        *GetNameSafe(this)
    );

    // 適用後の状態を反映
    UpdateApplyButtonState();
}

/** キャンセルボタンクリック時の処理を行う */
void URldOptionsWidget::OnCancelClicked()
{
    UCmnSettingsSubsystem* settings = GetSettingsSubsystem();

    // SettingsSubsystem未取得時は再読み込みしない
    if (!settings)
    {
        UE_LOG(
            LogRldOptions,
            Warning,
            TEXT("OnCancelClicked: Widget=%s SettingsSubsystem未取得のため入力オプションを再読み込みできません"),
            *GetNameSafe(this)
        );

        return;
    }

    settings->ReloadInputOptions();

    // 保存済みの設定値をUIへ再反映
    RefreshUIFromSettings();

    // 適用ボタンの状態を更新
    UpdateApplyButtonState();

    UE_LOG(
        LogRldOptions,
        Log,
        TEXT("OnCancelClicked: Widget=%s 入力オプションを保存済み状態へ戻しました"),
        *GetNameSafe(this)
    );
}

/** 閉じるボタンクリック時の処理を行う */
void URldOptionsWidget::OnCloseClicked()
{
    UCmnSettingsSubsystem* settings = GetSettingsSubsystem();

    // SettingsSubsystem未取得時はそのまま閉じる
    if (!settings)
    {
        UE_LOG(
            LogRldOptions,
            Warning,
            TEXT("OnCloseClicked: Widget=%s SettingsSubsystem未取得のため未保存状態を確認せず閉じます"),
            *GetNameSafe(this)
        );

        RemoveFromParent();
        return;
    }

    // 未保存変更がない場合はそのまま閉じる
    if (!settings->HasUnsavedChanges())
    {
        RemoveFromParent();

        UE_LOG(
            LogRldOptions,
            Log,
            TEXT("OnCloseClicked: Widget=%s オプション画面を閉じました"),
            *GetNameSafe(this)
        );

        return;
    }

    // 未保存変更がある場合は確認処理を行う想定
    UE_LOG(
        LogRldOptions,
        Warning,
        TEXT("OnCloseClicked: Widget=%s 未保存変更があるため閉じません"),
        *GetNameSafe(this)
    );
}

/** 適用ボタンの状態を更新する */
void URldOptionsWidget::UpdateApplyButtonState()
{
    // 適用ボタン未取得時は更新しない
    if (!ButtonApply)
    {
        return;
    }

    UCmnSettingsSubsystem* settings = GetSettingsSubsystem();

    // SettingsSubsystem未取得時は適用ボタンを無効にする
    if (!settings)
    {
        ButtonApply->SetIsEnabled(false);
        return;
    }

    const bool bHasUnsavedChanges = settings->HasUnsavedChanges();

    ButtonApply->SetIsEnabled(bHasUnsavedChanges);

    UE_LOG(
        LogRldOptions,
        Verbose,
        TEXT("UpdateApplyButtonState: Widget=%s 適用ボタン状態=%s"),
        *GetNameSafe(this),
        bHasUnsavedChanges ? TEXT("有効") : TEXT("無効")
    );
}
