// RldPlayerController.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "TimerManager.h"

#include "Common/Input/CmnInputTypes.h"
#include "RldPlayerController.generated.h"

class UEnhancedInputComponent;
class UEnhancedInputLocalPlayerSubsystem;
class UCmnInputRouter;
class UCmnInputConfig;
class UCmnSettingsSubsystem;
class URldDebugOptionsWidget;
class ARldPlayerCharacter;
struct FInputRuntimeSettings;

/**
 * RogueLikeDungeon用PlayerController
 * 入力処理と入力変換を行う
 */
UCLASS()
class ROGUELIKEDUNGEON_API ARldPlayerController : public APlayerController
{
    GENERATED_BODY()

private:

    /** デバッグコマンド入力種別 */
    enum class ERldDebugCommandInputType : uint8
    {
        None,
        Keyboard,
        Gamepad
    };

public:

    /** PlayerControllerを初期化する */
    ARldPlayerController();

    // ----- AActor -----

    virtual void PostInitializeComponents() override;
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // ----- AController -----

    virtual void SetupInputComponent() override;

public:

    /** 共通入力モードを切り替える */
    UFUNCTION(BlueprintCallable, Category = "Rld|Input")
    void SetCommonInputMode(ECmnInputMode Mode);

    /** 入力ルーターを取得する */
    UFUNCTION(BlueprintCallable, Category = "Rld|Input")
    UCmnInputRouter* GetInputRouter() const;

private:

    // ----- 設定通知 -----

    /** 入力設定変更通知を受け取る */
    void HandleInputSettingsChanged(const FInputRuntimeSettings& NewSettings);

private:

    // ----- ゲーム操作の入力 -----

    /** 移動入力開始時の処理を行う */
    void OnMoveStarted(const FInputActionValue& Value);

    /** 移動入力の生値ログを出力する */
    void OnMoveTriggered(const FInputActionValue& Value);

    /** 足踏み入力開始時の処理を行う */
    void OnStepInPlaceModifierStarted(const FInputActionValue& Value);

    /** 足踏み入力終了時の処理を行う */
    void OnStepInPlaceModifierCompleted(const FInputActionValue& Value);

    /** 通常攻撃入力を処理する */
    void OnAttackStarted(const FInputActionValue& Value);

    /** インタラクト入力を処理する */
    void OnInteractStarted(const FInputActionValue& Value);

    /** メニュー入力を処理する */
    void OnMenuStarted(const FInputActionValue& Value);

    /** カメラ視点入力を処理する */
    void OnCameraLookTriggered(const FInputActionValue& Value);

    /** カメラズーム入力を処理する */
    void OnCameraZoomTriggered(const FInputActionValue& Value);

private:

    // ----- UI操作の入力 -----

    /** UI方向入力を処理する */
    void OnUIDirectionTriggered(const FInputActionValue& Value);

    /** UI決定入力を処理する */
    void OnUIConfirmStarted(const FInputActionValue& Value);

    /** UIクローズ入力を処理する */
    void OnUICloseStarted(const FInputActionValue& Value);

    /** UIスクロール入力を処理する */
    void OnUIScrollTriggered(const FInputActionValue& Value);

private:

    // ----- デバッグUI入力 -----

    /** デバッグコマンド開始入力を処理する */
    void OnDebugCommandPrefixStarted(const FInputActionValue& Value);

    /** キーボード用デバッグコマンド第1入力を処理する */
    void OnDebugCommandKeyboard1Started(const FInputActionValue& Value);

    /** キーボード用デバッグコマンド第2入力を処理する */
    void OnDebugCommandKeyboard2Started(const FInputActionValue& Value);

    /** キーボード用デバッグコマンド第3入力を処理する */
    void OnDebugCommandKeyboard3Started(const FInputActionValue& Value);

    /** キーボード用デバッグコマンド第4入力を処理する */
    void OnDebugCommandKeyboard4Started(const FInputActionValue& Value);

    /** キーボード用デバッグコマンド第5入力を処理する */
    void OnDebugCommandKeyboard5Started(const FInputActionValue& Value);

    /** ゲームパッド用デバッグコマンド第1入力を処理する */
    void OnDebugCommandGamepad1Started(const FInputActionValue& Value);

    /** ゲームパッド用デバッグコマンド第2入力を処理する */
    void OnDebugCommandGamepad2Started(const FInputActionValue& Value);

private:

    // ----- ゲーム固有入力変換 -----

    /**
     * 入力軸をカメラ基準のグリッド8方向へ変換する
     *
     * @param Axis 入力軸
     * @param OutDirection 変換後のグリッド方向
     * @return 方向確定できた場合はtrue
     */
    bool TryConvertMoveAxisToGridDir(const FVector2D& Axis, FIntPoint& OutDirection) const;

    /**
     * 入力軸をカメラ基準のワールド平面方向へ変換する
     *
     * @param Axis 入力軸
     * @param OutWorldDirection 変換後のワールド平面方向
     * @return 有効な方向が得られた場合はtrue
     */
    bool TryConvertInputAxisToCameraRelativeWorldDir(
        const FVector2D& Axis,
        FVector& OutWorldDirection
    ) const;

    /**
     * ワールド平面方向をグリッド8方向へ変換する
     *
     * @param WorldDirection ワールド平面方向
     * @param OutGridDirection 変換後のグリッド方向
     * @return 方向確定できた場合はtrue
     */
    bool TryConvertWorldDirToGridDir(
        const FVector& WorldDirection,
        FIntPoint& OutGridDirection
    ) const;

private:

    // ----- 左スティック専用処理 -----

    /**
     * 左スティック入力かどうかを判定する
     *
     * @param Axis 入力軸
     * @return 左スティック入力とみなせる場合はtrue
     */
    bool IsLikelyLeftStickInput(const FVector2D& Axis) const;

    /**
     * 左スティックの移動入力を処理する
     *
     * @param Axis 入力軸
     */
    void HandleLeftStickMoveTriggered(const FVector2D& Axis);

private:

    // ----- キャラクター取得 -----

    /**
     * 現在操作中のプレイヤーキャラクターを取得する
     *
     * @return プレイヤーキャラクター
     */
    ARldPlayerCharacter* GetRldPlayerCharacter() const;

private:

    // ----- 左スティックの状態 -----

    // 左スティック入力確定済みフラグ
    bool bLeftStickMoveConsumed = false;

private:

    // ----- 足踏み入力状態 -----

    // 足踏み入力中フラグ
    bool bStepInPlaceModifierPressed = false;

private:

    // ----- デバッグ補助 -----

    /** 現在の移動入力元をデバッグ文字列で取得する */
    FString BuildMoveInputSourceDebugText() const;

private:

    /**
     * 確定した移動方向を処理する
     *
     * @param Direction 移動方向
     * @param InputSourceText デバッグ表示用の入力元文字列
     * @param Axis デバッグ表示用の入力軸
     */
    void ProcessResolvedMoveDirection(
        const FIntPoint& Direction,
        const FString& InputSourceText,
        const FVector2D& Axis
    );

private:

    // ----- 移動リピート制御 -----

    /**
     * 押しっぱなし移動のリピートを開始する
     *
     * @param Direction 移動方向
     * @param InputSourceText デバッグ表示用の入力元文字列
     * @param Axis デバッグ表示用の入力軸
     */
    void StartMoveRepeat(
        const FIntPoint& Direction,
        const FString& InputSourceText,
        const FVector2D& Axis
    );

    /** 押しっぱなし移動のリピートを停止する */
    void StopMoveRepeat();

    /** 押しっぱなし移動の初回遅延処理を行う */
    void BeginMoveRepeat();

    /** 押しっぱなし移動の定期実行を行う */
    void TickMoveRepeat();

    /** 押しっぱなし移動を継続すべきか判定する */
    bool ShouldContinueMoveRepeat() const;

private:

    // ----- 移動リピート状態 -----

    // 初回遅延Timer
    FTimerHandle MoveRepeatStartTimerHandle;

    // 継続リピートTimer
    FTimerHandle MoveRepeatTickTimerHandle;

    // 現在のリピート方向
    FIntPoint RepeatingMoveDirection = FIntPoint::ZeroValue;

    // 現在のリピート入力元
    FString RepeatingMoveInputSourceText;

    // 現在のリピート入力軸
    FVector2D RepeatingMoveAxis = FVector2D::ZeroVector;

    // 押しっぱなし移動の初回遅延秒数
    float MoveRepeatInitialDelay = 0.25f;

    // 押しっぱなし移動の継続間隔秒数
    float MoveRepeatInterval = 0.10f;

    // 押しっぱなし移動中フラグ
    bool bMoveRepeatActive = false;

private:

    // ----- デバッグUI表示 -----

    /** キーボード用デバッグコマンドの1入力分を処理する */
    void HandleKeyboardDebugCommandCharacter(int32 commandIndex);

    /** ゲームパッド用デバッグコマンドの1入力分を処理する */
    void HandleGamepadDebugCommandInput(bool bIsGamepad1Input);

    /** デバッグコマンド入力状態を初期化する */
    void ResetDebugCommandState();

    /** デバッグコマンド入力受付中か判定する */
    bool IsHiddenDebugCommandInputActive() const;

    /** デバッグコマンド入力がタイムアウトしているか判定する */
    bool IsDebugCommandTimedOut() const;

    /** デバッグオプションWidgetを表示する */
    void OpenDebugOptionsWidget();

    /** 表示中のデバッグオプションWidgetを取得する */
    URldDebugOptionsWidget* GetActiveDebugOptionsWidget() const;

private:

    // ----- デバッグUI設定 -----

    // デバッグオプションWidgetクラス
    UPROPERTY(EditDefaultsOnly, Category = "Rld|Debug")
    TSubclassOf<URldDebugOptionsWidget> DebugOptionsWidgetClass;

    // 表示中のデバッグオプションWidget
    UPROPERTY(Transient)
    TObjectPtr<URldDebugOptionsWidget> ActiveDebugOptionsWidget = nullptr;

    // デバッグコマンド開始入力の必要回数
    UPROPERTY(EditDefaultsOnly, Category = "Rld|Debug", meta = (ClampMin = "1"))
    int32 debugCommandPrefixRequiredCount = 3;

    // デバッグコマンド入力待ち時間
    UPROPERTY(EditDefaultsOnly, Category = "Rld|Debug", meta = (ClampMin = "0.1"))
    float hiddenDebugCommandTimeout = 5.0f;

private:

    // ----- デバッグUI入力状態 -----

    // デバッグコマンド開始入力回数
    int32 debugCommandPrefixCount = 0;

    // デバッグコマンド文字入力Index
    int32 debugCommandIndex = 0;

    // デバッグコマンド入力種別
    ERldDebugCommandInputType debugCommandInputType = ERldDebugCommandInputType::None;

    // 最後にデバッグコマンド入力を受け取った時刻
    double lastHiddenDebugInputTime = 0.0;

private:

    // ----- 内部処理 -----

    /** 入力設定をロードして入力ルーターへ適用する */
    void LoadAndApplyInputConfig();

    /** EnhancedInputComponentへ差し替える */
    void EnsureEnhancedInputComponent();

private:

    // ----- 入力設定DataAsset -----

    // 入力設定DataAsset参照(エディタ設定用)
    UPROPERTY(EditDefaultsOnly, Category = "Rld|Input")
    TSoftObjectPtr<UCmnInputConfig> InputConfigAsset;

    // 同期ロード済み入力設定キャッシュ
    UPROPERTY(Transient)
    TObjectPtr<UCmnInputConfig> LoadedInputConfig = nullptr;

    // 入力処理・IMC制御を行うルーター
    UPROPERTY(VisibleAnywhere, Category = "Rld|Input")
    TObjectPtr<UCmnInputRouter> InputRouter = nullptr;

private:

    // ----- 設定ローカルキャッシュ -----

    bool bInvertCameraX = false;
    bool bInvertCameraY = false;

    // 設定Subsystem参照
    UCmnSettingsSubsystem* SettingsSubsystem = nullptr;
};
