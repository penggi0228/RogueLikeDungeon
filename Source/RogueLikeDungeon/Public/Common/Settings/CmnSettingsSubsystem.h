// CmnSettingsSubsystem.h

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CmnSettingsSubsystem.generated.h"

class UCmnInputOptionsSave;

/**
 * 入力ランタイム設定
 * 入力設定の現在値を保持する
 */
USTRUCT(BlueprintType)
struct FInputRuntimeSettings
{
    GENERATED_BODY()

public:

    // カメラ左右反転設定
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    bool bInvertCameraX = false;

    // カメラ上下反転設定
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    bool bInvertCameraY = false;

public:

    /** 入力設定を比較する */
    bool operator==(const FInputRuntimeSettings& Other) const
    {
        return bInvertCameraX == Other.bInvertCameraX
            && bInvertCameraY == Other.bInvertCameraY;
    }

    /** 入力設定の差分を判定する */
    bool operator!=(const FInputRuntimeSettings& Other) const
    {
        return !(*this == Other);
    }
};

/** 入力設定変更通知デリゲート */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInputSettingsChanged, const FInputRuntimeSettings&);

/**
 * 共通設定管理サブシステム
 * 設定値の読み込みと保存を管理する
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

    /** カメラ左右反転設定を取得する */
    UFUNCTION(BlueprintPure, Category = "Common|Settings|Input|Camera")
    bool GetInvertCameraX() const
    {
        return RuntimeSettings.bInvertCameraX;
    }

    /** カメラ上下反転設定を取得する */
    UFUNCTION(BlueprintPure, Category = "Common|Settings|Input|Camera")
    bool GetInvertCameraY() const
    {
        return RuntimeSettings.bInvertCameraY;
    }

    /**
     * 現在の入力設定を取得する
     *
     * @return 現在の入力設定
     */
    const FInputRuntimeSettings& GetCurrentSettings() const
    {
        return RuntimeSettings;
    }

    /** 未保存の変更があるか判定する */
    UFUNCTION(BlueprintPure, Category = "Common|Settings")
    bool HasUnsavedChanges() const;

public:

    // ----- Setter -----

    /** カメラ左右反転設定を更新する */
    UFUNCTION(BlueprintCallable, Category = "Common|Settings|Input|Camera")
    void SetInvertCameraX(bool bValue);

    /** カメラ上下反転設定を更新する */
    UFUNCTION(BlueprintCallable, Category = "Common|Settings|Input|Camera")
    void SetInvertCameraY(bool bValue);

    // ----- 保存操作 -----

    /**
     * 入力オプションを保存する
     *
     * @return 保存成功ならtrue
     */
    UFUNCTION(BlueprintCallable, Category = "Common|Settings")
    bool ApplyAndSaveInputOptions();

    /** 入力オプションを再読み込みする */
    UFUNCTION(BlueprintCallable, Category = "Common|Settings")
    void ReloadInputOptions();

private:

    // ----- Save & Load: 入力オプション -----

    void LoadInputOptionsInternal(); // SaveGameをロードまたは新規作成してキャッシュ
    void ReadCacheToRuntime();       // SaveGameのキャッシュ値をRuntimeへ反映
    void WriteRuntimeToCache();      // Runtime値をSaveGameのキャッシュへ書き戻す
    bool SaveInputOptions();         // SaveGameのキャッシュをスロットへ保存
    void UpdateDirtyFlag();          // Runtimeと保存済み値の差分を判定

private:

    // ----- Runtime値 / 保存済み値 -----

    // 現在の入力設定
    FInputRuntimeSettings RuntimeSettings;

    // 保存済みの入力設定
    FInputRuntimeSettings SavedSettings;

    // 未保存変更の有無
    bool bHasUnsavedChanges = false;

private:

    // ----- SaveGameキャッシュ -----

    // 入力オプションSaveGameキャッシュ
    UPROPERTY(Transient)
    TObjectPtr<UCmnInputOptionsSave> CachedInputOptions = nullptr;

private:

    // ----- SaveGameスロット設定 -----

    // 入力オプションSaveGameのスロット名
    UPROPERTY(EditDefaultsOnly, Category = "Common|Settings|Save", meta = (AllowPrivateAccess = "true"))
    FString InputOptionsSlotName = TEXT("InputOptions");

    // 入力オプションSaveGameのユーザーIndex
    UPROPERTY(EditDefaultsOnly, Category = "Common|Settings|Save", meta = (AllowPrivateAccess = "true"))
    int32 InputOptionsUserIndex = 0;

private:

    // ----- 通知デリゲート -----

    // 入力設定変更通知
    FOnInputSettingsChanged OnInputSettingsChanged;
};
