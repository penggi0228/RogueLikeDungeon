// CmnSettingsSubsystem.cpp

#include "Common/Settings/CmnSettingsSubsystem.h"

#include "Common/Input/CmnInputOptionsSave.h"
#include "Common/Save/CmnSaveGameLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(LogCmnSettings, Log, All);

// ----- UGameInstanceSubsystem -----

/** 設定管理サブシステムを初期化する */
void UCmnSettingsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // 入力オプションSaveGameのロードとキャッシュ
    LoadInputOptionsInternal();

    // キャッシュ値をRuntimeへ反映
    ReadCacheToRuntime();

    // 現在値を保存済み値として保持
    SavedSettings = RuntimeSettings;

    // 差分状態の更新
    UpdateDirtyFlag();

    UE_LOG(
        LogCmnSettings,
        Log,
        TEXT("Initialize: Subsystem=%s 設定管理サブシステム初期化完了 左右反転=%s 上下反転=%s 未保存変更あり=%s"),
        *GetNameSafe(this),
        RuntimeSettings.bInvertCameraX ? TEXT("有効") : TEXT("無効"),
        RuntimeSettings.bInvertCameraY ? TEXT("有効") : TEXT("無効"),
        bHasUnsavedChanges ? TEXT("true") : TEXT("false")
    );
}

/** 設定管理サブシステムを終了する */
void UCmnSettingsSubsystem::Deinitialize()
{
    Super::Deinitialize();
}

// ----- Getter -----

/** 未保存変更の有無を返す */
bool UCmnSettingsSubsystem::HasUnsavedChanges() const
{
    return bHasUnsavedChanges;
}

// ----- Setter -----

/** カメラ左右反転設定を更新する */
void UCmnSettingsSubsystem::SetInvertCameraX(bool bValue)
{
    // 値が同一の場合は更新しない
    if (RuntimeSettings.bInvertCameraX == bValue)
    {
        return;
    }

    // Runtime値を更新
    RuntimeSettings.bInvertCameraX = bValue;

    // 差分状態の更新
    UpdateDirtyFlag();

    // 変更通知
    OnInputSettingsChanged.Broadcast(RuntimeSettings);

    UE_LOG(
        LogCmnSettings,
        Log,
        TEXT("SetInvertCameraX: Subsystem=%s カメラ左右反転設定を更新しました 左右反転=%s 未保存変更あり=%s"),
        *GetNameSafe(this),
        bValue ? TEXT("有効") : TEXT("無効"),
        bHasUnsavedChanges ? TEXT("true") : TEXT("false")
    );
}

/** カメラ上下反転設定を更新する */
void UCmnSettingsSubsystem::SetInvertCameraY(bool bValue)
{
    // 値が同一の場合は更新しない
    if (RuntimeSettings.bInvertCameraY == bValue)
    {
        return;
    }

    // Runtime値を更新
    RuntimeSettings.bInvertCameraY = bValue;

    // 差分状態の更新
    UpdateDirtyFlag();

    // 変更通知
    OnInputSettingsChanged.Broadcast(RuntimeSettings);

    UE_LOG(
        LogCmnSettings,
        Log,
        TEXT("SetInvertCameraY: Subsystem=%s カメラ上下反転設定を更新しました 上下反転=%s 未保存変更あり=%s"),
        *GetNameSafe(this),
        bValue ? TEXT("有効") : TEXT("無効"),
        bHasUnsavedChanges ? TEXT("true") : TEXT("false")
    );
}

// ----- 保存操作 -----

/** 入力オプションを保存する */
bool UCmnSettingsSubsystem::ApplyAndSaveInputOptions()
{
    // Runtime値をSaveGameキャッシュへ反映
    WriteRuntimeToCache();

    // SaveGameへ保存
    const bool bSaved = SaveInputOptions();

    // 保存失敗時は処理終了
    if (!bSaved)
    {
        return false;
    }

    // 保存済み値を更新
    SavedSettings = RuntimeSettings;

    // 差分状態の更新
    UpdateDirtyFlag();

    // 変更通知
    OnInputSettingsChanged.Broadcast(RuntimeSettings);

    UE_LOG(
        LogCmnSettings,
        Log,
        TEXT("ApplyAndSaveInputOptions: Subsystem=%s 入力オプション保存完了 左右反転=%s 上下反転=%s 未保存変更あり=%s"),
        *GetNameSafe(this),
        RuntimeSettings.bInvertCameraX ? TEXT("有効") : TEXT("無効"),
        RuntimeSettings.bInvertCameraY ? TEXT("有効") : TEXT("無効"),
        bHasUnsavedChanges ? TEXT("true") : TEXT("false")
    );

    return true;
}

/** 入力オプションを再読み込みする */
void UCmnSettingsSubsystem::ReloadInputOptions()
{
    // SaveGameを再ロード
    LoadInputOptionsInternal();

    // キャッシュ値をRuntimeへ反映
    ReadCacheToRuntime();

    // 保存済み値を更新
    SavedSettings = RuntimeSettings;

    // 差分状態の更新
    UpdateDirtyFlag();

    // 変更通知
    OnInputSettingsChanged.Broadcast(RuntimeSettings);

    UE_LOG(
        LogCmnSettings,
        Log,
        TEXT("ReloadInputOptions: Subsystem=%s 入力オプション再読み込み完了 左右反転=%s 上下反転=%s 未保存変更あり=%s"),
        *GetNameSafe(this),
        RuntimeSettings.bInvertCameraX ? TEXT("有効") : TEXT("無効"),
        RuntimeSettings.bInvertCameraY ? TEXT("有効") : TEXT("無効"),
        bHasUnsavedChanges ? TEXT("true") : TEXT("false")
    );
}

// ----- Save & Load: 入力オプション -----

/** 入力オプションSaveGameをロードまたは新規作成する */
void UCmnSettingsSubsystem::LoadInputOptionsInternal()
{
    CachedInputOptions = Cast<UCmnInputOptionsSave>(
        UCmnSaveGameLibrary::LoadOrCreate(
            UCmnInputOptionsSave::StaticClass(),
            InputOptionsSlotName,
            InputOptionsUserIndex
        )
    );

    // SaveGame取得に失敗した場合は警告ログ出力
    if (!CachedInputOptions)
    {
        UE_LOG(
            LogCmnSettings,
            Warning,
            TEXT("LoadInputOptionsInternal: Subsystem=%s 入力設定SaveGameの取得に失敗しました SlotName=%s UserIndex=%d"),
            *GetNameSafe(this),
            *InputOptionsSlotName,
            InputOptionsUserIndex
        );

        return;
    }

    UE_LOG(
        LogCmnSettings,
        Verbose,
        TEXT("LoadInputOptionsInternal: Subsystem=%s 入力設定SaveGameを取得しました SlotName=%s UserIndex=%d SaveGame=%s"),
        *GetNameSafe(this),
        *InputOptionsSlotName,
        InputOptionsUserIndex,
        *GetNameSafe(CachedInputOptions)
    );
}

/** SaveGameキャッシュ値をRuntimeへ反映する */
void UCmnSettingsSubsystem::ReadCacheToRuntime()
{
    // キャッシュ未取得の場合は処理しない
    if (!CachedInputOptions)
    {
        UE_LOG(
            LogCmnSettings,
            Warning,
            TEXT("ReadCacheToRuntime: Subsystem=%s CachedInputOptionsがnullのためRuntimeへ反映しません"),
            *GetNameSafe(this)
        );

        return;
    }

    RuntimeSettings.bInvertCameraX = CachedInputOptions->bInvertCameraX;
    RuntimeSettings.bInvertCameraY = CachedInputOptions->bInvertCameraY;
}

/** Runtime値をSaveGameキャッシュへ書き戻す */
void UCmnSettingsSubsystem::WriteRuntimeToCache()
{
    // キャッシュ未取得の場合は処理しない
    if (!CachedInputOptions)
    {
        UE_LOG(
            LogCmnSettings,
            Warning,
            TEXT("WriteRuntimeToCache: Subsystem=%s CachedInputOptionsがnullのためSaveGameキャッシュへ書き戻しません"),
            *GetNameSafe(this)
        );

        return;
    }

    CachedInputOptions->bInvertCameraX = RuntimeSettings.bInvertCameraX;
    CachedInputOptions->bInvertCameraY = RuntimeSettings.bInvertCameraY;
}

/** 入力オプションを保存する */
bool UCmnSettingsSubsystem::SaveInputOptions()
{
    // キャッシュ未取得の場合は保存失敗
    if (!CachedInputOptions)
    {
        UE_LOG(
            LogCmnSettings,
            Warning,
            TEXT("SaveInputOptions: Subsystem=%s CachedInputOptionsがnullのため保存できません"),
            *GetNameSafe(this)
        );

        return false;
    }

    const bool bResult = UCmnSaveGameLibrary::Save(
        CachedInputOptions,
        InputOptionsSlotName,
        InputOptionsUserIndex
    );

    // 保存失敗時はエラーログ出力
    if (!bResult)
    {
        UE_LOG(
            LogCmnSettings,
            Error,
            TEXT("SaveInputOptions: Subsystem=%s 入力設定の保存に失敗しました SlotName=%s UserIndex=%d"),
            *GetNameSafe(this),
            *InputOptionsSlotName,
            InputOptionsUserIndex
        );
    }

    return bResult;
}

/** 未保存変更の差分状態を更新する */
void UCmnSettingsSubsystem::UpdateDirtyFlag()
{
    bHasUnsavedChanges = (RuntimeSettings != SavedSettings);
}
