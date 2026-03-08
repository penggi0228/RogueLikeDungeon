// CmnSettingsSubsystem.cpp

#include "Common/Settings/CmnSettingsSubsystem.h"

#include "Common/Input/CmnInputOptionsSave.h"
#include "Common/Save/CmnSaveGameLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(LogCmnSettings, Log, All);

// ----- UGameInstanceSubsystem -----
/**
 * Subsystem初期化
 * ゲーム起動中は常駐する想定なので、初期化時にSaveGameを読み込み、
 * Runtime値と保存済み基準値を揃える
 */
void UCmnSettingsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    LoadInputOptionsInternal(); // SaveGameをロード / 生成してキャッシュ
    ReadCacheToRuntime();       // キャッシュ → ランタイムへ反映

    // 現在ロードされた値を保存済み基準値として記録
    SavedSettings = RuntimeSettings;
    UpdateDirtyFlag();
}

/**
 * Subsystem終了処理
 * 現時点では明示的な解放対象はないが、将来TimerやDelegate登録などを
 * 追加した場合の解放ポイントとして残しておく
 */
void UCmnSettingsSubsystem::Deinitialize()
{
    Super::Deinitialize();
}

// ----- Getter -----

/**
 * 未保存の変更があるかどうかを返す
 */
bool UCmnSettingsSubsystem::HasUnsavedChanges() const
{
    return bHasUnsavedChanges;
}

// ----- Setter -----

/**
 * 入力オプションの左右反転を更新する
 * 値が変化した場合のみ差分を更新し、登録側へ通知する
 */
void UCmnSettingsSubsystem::SetInvertCameraX(bool bValue)
{
    if (RuntimeSettings.bInvertCameraX == bValue)
        return;

    RuntimeSettings.bInvertCameraX = bValue;
    UpdateDirtyFlag();

    OnInputSettingsChanged.Broadcast(RuntimeSettings);
}

/**
 * 入力オプションの上下反転を更新する
 * 値が変化した場合のみ差分を更新し、登録側へ通知する
 */
void UCmnSettingsSubsystem::SetInvertCameraY(bool bValue)
{
    if (RuntimeSettings.bInvertCameraY == bValue)
        return;

    RuntimeSettings.bInvertCameraY = bValue;
    UpdateDirtyFlag();

    OnInputSettingsChanged.Broadcast(RuntimeSettings);
}

// ----- 保存操作 -----

/**
 * 現在の値をSaveGameへ保存(適用)する
 * 保存前にRuntime値をキャッシュへ反映し、
 * 保存成功時のみ保存済み基準値を更新する
 */
bool UCmnSettingsSubsystem::ApplyAndSaveInputOptions()
{
    WriteRuntimeToCache();

    const bool bSaved = SaveInputOptions();
    if (!bSaved)
    {
        return false;
    }

    SavedSettings = RuntimeSettings;
    UpdateDirtyFlag();

    OnInputSettingsChanged.Broadcast(RuntimeSettings);

    return true;
}

/**
 * 未保存の変更を破棄してSaveGameを再読み込みする
 * Runtime値を保存済み状態へ戻し、購読側へ再通知する
 */
void UCmnSettingsSubsystem::ReloadInputOptions()
{
    LoadInputOptionsInternal();
    ReadCacheToRuntime();

    SavedSettings = RuntimeSettings;
    UpdateDirtyFlag();

    OnInputSettingsChanged.Broadcast(RuntimeSettings);
}

// ----- Save & Load: 入力オプション -----

/**
 * SaveGameをロード / 生成してキャッシュする
 * Runtime値への反映はReadCacheToRuntime()で行う
 * スロットが存在しない場合は新規作成されたオブジェクトを保持する
 */
void UCmnSettingsSubsystem::LoadInputOptionsInternal()
{
    CachedInputOptions = Cast<UCmnInputOptionsSave>(
        UCmnSaveGameLibrary::LoadOrCreate(
            UCmnInputOptionsSave::StaticClass(),
            InputOptionsSlotName,
            InputOptionsUserIndex
        )
    );

    if (!CachedInputOptions)
    {
        UE_LOG(LogCmnSettings, Warning,
            TEXT("LoadInputOptionsInternal: 入力設定SaveGameのロード / 生成に失敗しました。"));
    }
}

/**
 * キャッシュからRuntime値へ反映する
 */
void UCmnSettingsSubsystem::ReadCacheToRuntime()
{
    if (!CachedInputOptions)
    {
        return;
    }

    RuntimeSettings.bInvertCameraX = CachedInputOptions->bInvertCameraX;
    RuntimeSettings.bInvertCameraY = CachedInputOptions->bInvertCameraY;
}

/**
 * Runtime値をキャッシュへ書き戻す
 * 実際の保存はSaveInputOptions()で行う
 */
void UCmnSettingsSubsystem::WriteRuntimeToCache()
{
    if (!CachedInputOptions)
    {
        return;
    }

    CachedInputOptions->bInvertCameraX = RuntimeSettings.bInvertCameraX;
    CachedInputOptions->bInvertCameraY = RuntimeSettings.bInvertCameraY;
}

/**
 * 入力オプション設定を保存する
 * @return 保存成功ならtrue、失敗ならfalse
 */
bool UCmnSettingsSubsystem::SaveInputOptions()
{
    if (!CachedInputOptions)
    {
        UE_LOG(LogCmnSettings, Warning,
            TEXT("SaveInputOptions: CachedInputOptionsがnullです。"));
        return false;
    }

    const bool bResult = UCmnSaveGameLibrary::Save(
        CachedInputOptions,
        InputOptionsSlotName,
        InputOptionsUserIndex
    );

    if (!bResult)
    {
        UE_LOG(LogCmnSettings, Error,
            TEXT("SaveInputOptions: 入力設定の保存に失敗しました。"));
    }

    return bResult;
}

/**
 * 保存済みの値と未保存の値の差分を判定する
 */
void UCmnSettingsSubsystem::UpdateDirtyFlag()
{
    bHasUnsavedChanges = (RuntimeSettings != SavedSettings);
}
