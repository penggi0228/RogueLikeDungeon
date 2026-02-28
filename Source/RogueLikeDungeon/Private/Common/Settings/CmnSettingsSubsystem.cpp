// CmnSettingsSubsystem.cpp

#include "Common/Settings/CmnSettingsSubsystem.h"

#include "Common/Input/CmnInputOptionsSave.h"
#include "Common/Save/CmnSaveGameLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(LogCmnSettings, Log, All);

/**
 * Subsystem初期化
 * ・ゲーム起動中は常駐する想定なので、ここでロードしてキャッシュしておく
 */
void UCmnSettingsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    LoadInputOptionsInternal();

    UE_LOG(LogCmnSettings, Log, TEXT("Initialize: InputOptions Loaded=%d InvertX=%d InvertY=%d"),
        CachedInputOptions ? 1 : 0,
        bInvertCameraX ? 1 : 0,
        bInvertCameraY ? 1 : 0);
}

/**
 * Subsystem終了処理
 * ・基本は何もしない
 * ・必要ならオートセーブするが、勝手に保存したくないならやらない
 */
void UCmnSettingsSubsystem::Deinitialize()
{
    Super::Deinitialize();
}

/**
 * 入力オプションの左右反転を更新する(未保存)
 */
void UCmnSettingsSubsystem::SetInvertCameraX(bool bInvert)
{
    if (bInvertCameraX == bInvert)
    {
        return;
    }

    bInvertCameraX = bInvert;
    bHasUnsavedChanges = true;

    UE_LOG(LogCmnSettings, Log, TEXT("SetInvertCameraX: %d (Unsaved=1)"), bInvertCameraX ? 1 : 0);
}

/**
 * 入力オプションの上下反転を更新する(未保存)
 */
void UCmnSettingsSubsystem::SetInvertCameraY(bool bInvert)
{
    if (bInvertCameraY == bInvert)
    {
        return;
    }

    bInvertCameraY = bInvert;
    bHasUnsavedChanges = true;

    UE_LOG(LogCmnSettings, Log, TEXT("SetInvertCameraY: %d (Unsaved=1)"), bInvertCameraY ? 1 : 0);
}

/**
 * 保存済みの状態に戻す(破棄/戻す)
 */
void UCmnSettingsSubsystem::ReloadInputOptions()
{
    LoadInputOptionsInternal();
    bHasUnsavedChanges = false;
}

/**
 * 現在値をSaveGameへ保存する(適用)
 */
void UCmnSettingsSubsystem::ApplyAndSaveInputOptions()
{
    if (!CachedInputOptions)
    {
        LoadInputOptionsInternal();
    }

    if (!CachedInputOptions)
    {
        return;
    }

    WriteRuntimeToCache();

    const bool bOK = UCmnSaveGameLibrary::Save(CachedInputOptions, InputOptionsSlotName, InputOptionsUserIndex);

    if (bOK)
    {
        bHasUnsavedChanges = false;
    }

    UE_LOG(LogCmnSettings, Log, TEXT("ApplyAndSaveInputOptions: OK=%d"), bOK ? 1 : 0);
}

/**
 * SaveGameをロード/作成してキャッシュする
 * ・ロード成功→キャッシュ→ランタイム値へ反映
 * ・スロット無し→新規作成→初期値のままランタイムへ反映
 */
void UCmnSettingsSubsystem::LoadInputOptionsInternal()
{
    CachedInputOptions = Cast<UCmnInputOptionsSave>(
        UCmnSaveGameLibrary::LoadOrCreate(UCmnInputOptionsSave::StaticClass(), InputOptionsSlotName, InputOptionsUserIndex)
    );

    if (!CachedInputOptions)
    {
        UE_LOG(LogCmnSettings, Warning, TEXT("LoadInputOptionsInternal: Failed to load/create. Slot=%s UserIndex=%d"),
            *InputOptionsSlotName, InputOptionsUserIndex);
        return;
    }

    ReadCacheToRuntime();
}

/**
 * キャッシュ→ランタイムへ反映する
 */
void UCmnSettingsSubsystem::ReadCacheToRuntime()
{
    if (!CachedInputOptions)
    {
        return;
    }

    bInvertCameraX = CachedInputOptions->bInvertCameraX;
    bInvertCameraY = CachedInputOptions->bInvertCameraY;
    CameraSensitivity = CachedInputOptions->CameraSensitivity;
}

/**
 * ランタイム→キャッシュへ書き戻す
 */
void UCmnSettingsSubsystem::WriteRuntimeToCache()
{
    if (!CachedInputOptions)
    {
        return;
    }

    CachedInputOptions->bInvertCameraX = bInvertCameraX;
    CachedInputOptions->bInvertCameraY = bInvertCameraY;
    CachedInputOptions->CameraSensitivity = CameraSensitivity;
}