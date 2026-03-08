// CmnSettingsSubsystem.h

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CmnSettingsSubsystem.generated.h"

class UCmnInputOptionsSave;

/**
 * 入力ランタイム設定
 * SaveGameの値とUI側で編集した値を分離するためのランタイム用コンテナ
 * 保存済みの値と未保存の変更値をstruct単位で差分比較する
 */
USTRUCT(BlueprintType)
struct FInputRuntimeSettings
{
    GENERATED_BODY()

public:

    /** カメラ左右反転(現在の編集中値) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    bool bInvertCameraX = false;

    /** カメラ上下反転(現在の編集中値) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    bool bInvertCameraY = false;
    
public:

    /**
     * 設定差分判定用
     * SavedSettingsとRuntimeSettingsの比較に使用する。
     */
    bool operator==(const FInputRuntimeSettings& Other) const
    {
        return bInvertCameraX == Other.bInvertCameraX
            && bInvertCameraY == Other.bInvertCameraY;
    }

    /**
     * 設定差分判定用
     * 差分があるかどうかを簡潔に判定するために使用する。
     */
    bool operator!=(const FInputRuntimeSettings& Other) const
    {
        return !(*this == Other);
    }
};


/** 入力設定変更通知(C++専用) */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInputSettingsChanged, const FInputRuntimeSettings&);

/**
 * 共通設定管理サブシステム
 * SaveGameとRuntime値を管理し、未保存変更を検知
 * PlayerConrtoller側は設定を管理せず、必要な値を通知で受け取るだけにする
 */
UCLASS()
class ROGUELIKEDUNGEON_API UCmnSettingsSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // ----- UGameInstanceSubsystem -----

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ----- 通知デリゲート -----

    /**
     * 入力設定変更通知デリゲートを取得する
     * 外部クラスはこのデリゲートへ登録して変更通知を受け取る
     */
    FOnInputSettingsChanged& GetOnInputSettingsChanged()
    {
        return OnInputSettingsChanged;
    }


    // ----- Getter -----

    /** カメラ左右反転を取得 */
    UFUNCTION(BlueprintPure, Category = "Common|Settings|Input|Camera")
    bool GetInvertCameraX() const
    {
        return RuntimeSettings.bInvertCameraX;
    }

    /** カメラ上下反転を取得 */
    UFUNCTION(BlueprintPure, Category = "Common|Settings|Input|Camera")
    bool GetInvertCameraY() const
    {
        return RuntimeSettings.bInvertCameraY;
    }

    /**
     * 現在の入力設定を一括取得
     * C++側で設定をまとめて参照したい場合に使用
     */
    const FInputRuntimeSettings& GetCurrentSettings() const
    {
        return RuntimeSettings;
    }

    /** 保存済みの値との差分があるかどうかを返す */
    UFUNCTION(BlueprintPure, Category = "Common|Settings")
    bool HasUnsavedChanges() const;

public:

    // ----- Setter -----

    /** カメラ左右反転を更新(未保存) */
    UFUNCTION(BlueprintCallable, Category = "Common|Settings|Input|Camera")
    void SetInvertCameraX(bool bValue);

    /** カメラ上下反転を更新(未保存) */
    UFUNCTION(BlueprintCallable, Category = "Common|Settings|Input|Camera")
    void SetInvertCameraY(bool bValue);


    // ----- 保存操作 -----

    /**
     * 現在値をSaveGameへ保存(適用)する
     * Runtime値をキャッシュへ書き戻した後、SaveGameへ保存
     * 保存成功時のみ保存済み基準値を更新する
     */
    UFUNCTION(BlueprintCallable, Category = "Common|Settings")
    bool ApplyAndSaveInputOptions();

    /**
     * 未保存の変更を破棄してSaveGameを読み直す(= 元に戻す)
     * 保存済み状態へ戻したい場合に使用
     */
    UFUNCTION(BlueprintCallable, Category = "Common|Settings")
    void ReloadInputOptions();

private:

    // ----- Save & Load: 入力オプション -----

    void LoadInputOptionsInternal(); // 入力オプションSaveGameのロード / 生成
    void ReadCacheToRuntime();       // Save → Runtimeへ反映
    void WriteRuntimeToCache();      // Runtime → Saveへ書き戻し
    bool SaveInputOptions();         // 実際の保存処理
    void UpdateDirtyFlag();          // 差分判定


private:

    // ----- Runtime値 / 保存済み値 -----

    /** Runtime値 */
    FInputRuntimeSettings RuntimeSettings;

    /** 保存済み値 */
    FInputRuntimeSettings SavedSettings;

    /** SavedSettingsとRuntimeSettingsとの差分フラグ */
    bool bHasUnsavedChanges = false;

private:

    // ----- SaveGameキャッシュ -----

    /** 入力オプションSaveGameキャッシュ */
    UPROPERTY(Transient)
    TObjectPtr<UCmnInputOptionsSave> CachedInputOptions = nullptr;

private:

    // ----- SaveGameスロット設定 -----

    /** 入力オプションSaveGameのスロット名 */
    UPROPERTY(EditDefaultsOnly, Category = "Common|Settings|Save", meta = (AllowPrivateAccess = "true"))
    FString InputOptionsSlotName = TEXT("InputOptions");

    /** 入力オプションSaveGameのユーザーIndex */
    UPROPERTY(EditDefaultsOnly, Category = "Common|Settings|Save", meta = (AllowPrivateAccess = "true"))
    int32 InputOptionsUserIndex = 0;

private:

    // ----- 通知デリゲート -----

    /** 入力設定変更時に発火 */
    FOnInputSettingsChanged OnInputSettingsChanged;

};
