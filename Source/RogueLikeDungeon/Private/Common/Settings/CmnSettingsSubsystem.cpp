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
        UE_LOG(LogCmnSettings, Warning,
            TEXT("LoadInputOptionsInternal: 入力設定SaveGameの取得に失敗"));
    }
}

/** SaveGameキャッシュ値をRuntimeへ反映する */
void UCmnSettingsSubsystem::ReadCacheToRuntime()
{
    // キャッシュ未取得の場合は処理しない
    if (!CachedInputOptions)
    {
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
        UE_LOG(LogCmnSettings, Warning,
            TEXT("SaveInputOptions: CachedInputOptionsがnull"));
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
        UE_LOG(LogCmnSettings, Error,
            TEXT("SaveInputOptions: 入力設定の保存に失敗"));
    }

    return bResult;
}

/** 未保存変更の差分状態を更新する */
void UCmnSettingsSubsystem::UpdateDirtyFlag()
{
    bHasUnsavedChanges = (RuntimeSettings != SavedSettings);
}
