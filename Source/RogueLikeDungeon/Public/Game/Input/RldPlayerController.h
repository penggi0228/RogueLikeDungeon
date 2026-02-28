// RldPlayerController.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "Common/Input/CmnInputTypes.h"
#include "RldPlayerController.generated.h"

class UEnhancedInputComponent;
class UEnhancedInputLocalPlayerSubsystem;
class UCmnInputRouter;
class UCmnInputConfig;
class UCmnInputOptionsSave;

/**
 * ゲーム固有PlayerController
 * ・EnhancedInputのBindを担当
 * ・入力解釈はUCmnInputRouterに委譲
 * ・入力設定はDataAsset(UCmnInputConfig)から取得
 * ・入力オプション(反転など)はSaveGameで永続化
 *
 * ※UIリピートはRouter内部でTimer駆動(PlayerTick不要)
 */
UCLASS()
class ROGUELIKEDUNGEON_API ARldPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    ARldPlayerController();

    // ----- AActor -----
    virtual void PostInitializeComponents() override;
    virtual void BeginPlay() override;

    // ----- AController -----
    virtual void SetupInputComponent() override;

public:
    /** 共通入力モード切替(Game/Menu/Dialog/Disabled) */
    UFUNCTION(BlueprintCallable, Category = "Rld|Input")
    void SetCommonInputMode(ECmnInputMode Mode);

    /** Router取得(UI側から参照する場合に使用) */
    UFUNCTION(BlueprintCallable, Category = "Rld|Input")
    UCmnInputRouter* GetInputRouter() const { return InputRouter; }

    // ----- Camera Option (Invert) -----
    /**
     * カメラ左右反転を設定する(即時反映)
     * ・基本は「設定だけ」行い、保存はApply(適用)ボタンでまとめて行う想定
     * ・bSaveImmediately=true ならその場でSaveInputOptionsを呼ぶ
     */
    UFUNCTION(BlueprintCallable, Category = "Rld|Input|Camera")
    void SetInvertCameraX(bool bInvert, bool bSaveImmediately = false);

    /**
     * カメラ上下反転を設定する(即時反映)
     * ・基本は「設定だけ」行い、保存はApply(適用)ボタンでまとめて行う想定
     * ・bSaveImmediately=true ならその場でSaveInputOptionsを呼ぶ
     */
    UFUNCTION(BlueprintCallable, Category = "Rld|Input|Camera")
    void SetInvertCameraY(bool bInvert, bool bSaveImmediately = false);

    /** 現在の反転設定を取得 */
    UFUNCTION(BlueprintCallable, Category = "Rld|Input|Camera")
    bool GetInvertCameraX() const { return bInvertCameraX; }

    UFUNCTION(BlueprintCallable, Category = "Rld|Input|Camera")
    bool GetInvertCameraY() const { return bInvertCameraY; }

    /**
     * 現在の反転設定をSaveGameへ保存する(オプション画面の「適用」用)
     * ・内部でSaveGameの生成/ロードも保証する
     */
    UFUNCTION(BlueprintCallable, Category = "Rld|Input|Camera")
    void ApplyAndSaveInputOptions();

    /**
     * SaveGameから反転設定を読み直して反映する(オプション画面の「元に戻す」用)
     */
    UFUNCTION(BlueprintCallable, Category = "Rld|Input|Camera")
    void ReloadInputOptions();

private:
    // ----- Game Input -----

    /** ゲーム移動入力(Axis2D→4方向) */
    void OnMoveStarted(const FInputActionValue& Value);

    /** カメラ視点(Axis2D) */
    void OnCameraLookTriggered(const FInputActionValue& Value);

    /** カメラズーム(Axis1D) */
    void OnCameraZoomTriggered(const FInputActionValue& Value);

private:
    // ----- Camera Options (runtime state) -----

    /** カメラ左右反転 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rld|Input|Camera", meta = (AllowPrivateAccess = "true"))
    bool bInvertCameraX = false;

    /** カメラ上下反転 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rld|Input|Camera", meta = (AllowPrivateAccess = "true"))
    bool bInvertCameraY = false;

    // ----- UI Input -----

    /** UI方向入力(Axis2D→方向イベント) */
    void OnUIDirectionTriggered(const FInputActionValue& Value);

    /** UIスクロール(Axis1D→スクロールイベント) */
    void OnUIScrollTriggered(const FInputActionValue& Value);

private:
    /** InputConfigのロード&Routerへ適用(SetupInputComponent前に間に合わせる) */
    void LoadAndApplyInputConfig();

    /** EnhancedInputComponentが無い/違う型なら差し替える(InputComponentClassは使わない) */
    void EnsureEnhancedInputComponent();

private:
    // ----- Save: Input Options -----

    /** 入力オプション(反転など)をSaveGameからロードして反映する */
    void LoadInputOptions();

    /** 入力オプション(反転など)をSaveGameへ保存する */
    void SaveInputOptions();

private:
    /** 入力設定DataAsset(ソフト参照) */
    UPROPERTY(EditDefaultsOnly, Category = "Rld|Input")
    TSoftObjectPtr<UCmnInputConfig> InputConfigAsset;

    /** ロード済み入力設定(実体) */
    UPROPERTY(Transient)
    TObjectPtr<UCmnInputConfig> LoadedInputConfig = nullptr;

    /** 共通入力ルータ(ロジック担当) */
    UPROPERTY(VisibleAnywhere, Category = "Rld|Input")
    TObjectPtr<UCmnInputRouter> InputRouter = nullptr;

    private:
        /** 入力オプションSaveGameのキャッシュ */
        UPROPERTY(Transient)
        TObjectPtr<UCmnInputOptionsSave> CachedInputOptions = nullptr;

        /** 入力オプションSaveGameのスロット名 */
        UPROPERTY(EditDefaultsOnly, Category = "Rld|Input|Save", meta = (AllowPrivateAccess = "true"))
        FString InputOptionsSlotName = TEXT("InputOptions");

        /** 入力オプションSaveGameのユーザーIndex */
        UPROPERTY(EditDefaultsOnly, Category = "Rld|Input|Save", meta = (AllowPrivateAccess = "true"))
        int32 InputOptionsUserIndex = 0;
};