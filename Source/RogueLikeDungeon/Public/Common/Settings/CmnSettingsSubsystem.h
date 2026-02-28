// CmnSettingsSubsystem.h

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CmnSettingsSubsystem.generated.h"

class UCmnInputOptionsSave;

/**
 * 共通ユーザー設定サブシステム
 * ・SaveGameのロード/キャッシュ/保存を一元管理
 * ・UIはこのSubsystemを呼ぶだけでよい
 * ・ゲーム側(PC等)は現在値を参照するだけでよい
 */
UCLASS()
class ROGUELIKEDUNGEON_API UCmnSettingsSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // ----- UGameInstanceSubsystem -----
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

public:
    // ----- Input Options (カメラ操作反転など) -----

    /** 入力オプションがロード済みか */
    UFUNCTION(BlueprintCallable, Category = "Common|Settings|Input")
    bool IsInputOptionsLoaded() const { return (CachedInputOptions != nullptr); }

    /** カメラ左右反転(現在値) */
    UFUNCTION(BlueprintCallable, Category = "Common|Settings|Input|Camera")
    bool GetInvertCameraX() const { return bInvertCameraX; }

    /** カメラ上下反転(現在値) */
    UFUNCTION(BlueprintCallable, Category = "Common|Settings|Input|Camera")
    bool GetInvertCameraY() const { return bInvertCameraY; }

    /** カメラ左右反転(現在値)を更新(未保存) */
    UFUNCTION(BlueprintCallable, Category = "Common|Settings|Input|Camera")
    void SetInvertCameraX(bool bInvert);

    /** カメラ上下反転(現在値)を更新(未保存) */
    UFUNCTION(BlueprintCallable, Category = "Common|Settings|Input|Camera")
    void SetInvertCameraY(bool bInvert);

    /**
     * 保存済み状態に戻す(破棄/戻す)
     * ・SaveGameを読み直して現在値へ反映する
     */
    UFUNCTION(BlueprintCallable, Category = "Common|Settings|Input")
    void ReloadInputOptions();

    /**
     * 現在値をSaveGameへ保存する(適用)
     * ・SaveGameが無ければ作成して保存する
     */
    UFUNCTION(BlueprintCallable, Category = "Common|Settings|Input")
    void ApplyAndSaveInputOptions();

    /** 未保存変更があるかどうか */
    UFUNCTION(BlueprintCallable, Category = "Common|Settings")
    bool HasUnsavedChanges() const { return bHasUnsavedChanges; }

private:
    /** 入力オプションSaveGameをロード/作成してキャッシュする */
    void LoadInputOptionsInternal();

    /** 現在値をキャッシュへ書き戻す */
    void WriteRuntimeToCache();

    /** キャッシュを現在値へ反映する */
    void ReadCacheToRuntime();

    /** 保存済み状態との差分があるか */
    bool bHasUnsavedChanges = false;

private:
    // ----- Save slot settings -----

    /** 入力オプションSaveGameのスロット名 */
    UPROPERTY(EditDefaultsOnly, Category = "Common|Settings|Save", meta = (AllowPrivateAccess = "true"))
    FString InputOptionsSlotName = TEXT("InputOptions");

    /** 入力オプションSaveGameのユーザーIndex */
    UPROPERTY(EditDefaultsOnly, Category = "Common|Settings|Save", meta = (AllowPrivateAccess = "true"))
    int32 InputOptionsUserIndex = 0;

private:
    // ----- Cached SaveGame -----

    /** 入力オプションSaveGameキャッシュ */
    UPROPERTY(Transient)
    TObjectPtr<UCmnInputOptionsSave> CachedInputOptions = nullptr;

private:
    // ----- Runtime values (UIが触るのはこれ) -----

    /** カメラ左右反転(ランタイム値) */
    bool bInvertCameraX = false;

    /** カメラ上下反転(ランタイム値) */
    bool bInvertCameraY = false;

    /** カメラ感度 */
    float CameraSensitivity = 1.0f;
};