// CmnInputRouter.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Common/Input/CmnInputTypes.h"
#include "TimerManager.h"

#include "CmnInputRouter.generated.h"

class APlayerController;
class UEnhancedInputLocalPlayerSubsystem;
class UInputMappingContext;
class UInputAction;
class UCmnInputConfig;

/**
 * 共通入力ルーター
 * 入力モード管理と入力変換処理を行う
 */
UCLASS(BlueprintType)
class ROGUELIKEDUNGEON_API UCmnInputRouter : public UObject
{
    GENERATED_BODY()

public:

    /** PlayerControllerとEnhancedInputSubsystemを設定する */
    void Initialize(APlayerController* InPC, UEnhancedInputLocalPlayerSubsystem* InSubsys);

    /** DataAssetから入力設定を適用する */
    void ApplyConfig(const UCmnInputConfig* Config);

    /** 入力モードを切り替える */
    UFUNCTION(BlueprintCallable, Category = "Common|Input")
    void SetInputMode(ECmnInputMode NewMode);

    /** 現在の入力モードを取得する */
    UFUNCTION(BlueprintCallable, Category = "Common|Input")
    ECmnInputMode GetInputMode() const
    {
        return CurrentMode;
    }

    // ----- モード判定 -----

    bool IsGameMode() const { return CurrentMode == ECmnInputMode::Game; }        // ゲームモード
    bool IsMenuMode() const { return CurrentMode == ECmnInputMode::Menu; }        // メニューモード
    bool IsDialogMode() const { return CurrentMode == ECmnInputMode::Dialog; }    // ダイアログモード
    bool IsDisabled() const { return CurrentMode == ECmnInputMode::Disabled; }    // 操作不能モード

    /** 移動入力を4方向へ変換して返す */
    bool GetMoveDirFromAxis(const FVector2D& Axis, FIntPoint& OutDir) const;

    /** UI方向入力を処理する */
    void HandleUIDirectionAxis(const FVector2D& Axis);

    /** UIスクロール入力を処理する */
    void HandleUIScrollAxis(float Amount);

    /** 互換用Tick */
    void Tick(float DeltaSeconds) { /* unused */ }

public:

    /** UI方向入力通知 */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUIDirection, FIntPoint, Direction);

    UPROPERTY(BlueprintAssignable, Category = "Common|Input")
    FOnUIDirection OnUIDirection;

    /** UIスクロール入力通知 */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUIScroll, float, Amount);

    UPROPERTY(BlueprintAssignable, Category = "Common|Input")
    FOnUIScroll OnUIScroll;

    /** 入力モード変更通知 */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInputModeChanged, ECmnInputMode, NewMode);

    UPROPERTY(BlueprintAssignable, Category = "Common|Input")
    FOnInputModeChanged OnInputModeChanged;

public:

    // ----- InputMappingContext -----

    // ゲーム操作用IMC
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Common|Input|IMC")
    TObjectPtr<UInputMappingContext> IMC_Game = nullptr;

    // UI操作用IMC
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Common|Input|IMC")
    TObjectPtr<UInputMappingContext> IMC_UI = nullptr;

public:

    // ----- Game InputAction -----

    // ゲーム移動用InputAction
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Common|Input|IA|Game")
    TObjectPtr<UInputAction> IA_Move = nullptr;

    // 待機用InputAction
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Common|Input|IA|Game")
    TObjectPtr<UInputAction> IA_Wait = nullptr;

    // 通常攻撃用InputAction
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Common|Input|IA|Game")
    TObjectPtr<UInputAction> IA_Attack = nullptr;

    // インタラクト用InputAction
    // 目の前のマスを調べる、宝箱を開けるなどの操作に使用
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Common|Input|IA|Game")
    TObjectPtr<UInputAction> IA_Interact = nullptr;

    // メニュー表示用InputAction
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Common|Input|IA|Game")
    TObjectPtr<UInputAction> IA_Menu = nullptr;

    // カメラ視点操作用InputAction
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Common|Input|IA|Game")
    TObjectPtr<UInputAction> IA_CameraLook = nullptr;

    // カメラズーム操作用InputAction
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Common|Input|IA|Game")
    TObjectPtr<UInputAction> IA_CameraZoom = nullptr;

public:

    // ----- UI InputAction -----

    // UI方向入力用InputAction
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Common|Input|IA|UI")
    TObjectPtr<UInputAction> IA_UI_Direction = nullptr;

    // UI決定用InputAction
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Common|Input|IA|UI")
    TObjectPtr<UInputAction> IA_UI_Confirm = nullptr;

    // UIクローズ用InputAction
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Common|Input|IA|UI")
    TObjectPtr<UInputAction> IA_UI_Close = nullptr;

    // UIスクロール用InputAction
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Common|Input|IA|UI")
    TObjectPtr<UInputAction> IA_UI_Scroll = nullptr;

public:

    // ----- Debug Command InputAction -----

    // デバッグコマンド開始用InputAction
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Common|Input|IA|Debug")
    TObjectPtr<UInputAction> IA_DebugCommandPrefix = nullptr;

    // キーボード用デバッグコマンド1InputAction
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Common|Input|IA|Debug|Keyboard")
    TObjectPtr<UInputAction> IA_DebugCommandKeyboard1 = nullptr;

    // キーボード用デバッグコマンド2InputAction
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Common|Input|IA|Debug|Keyboard")
    TObjectPtr<UInputAction> IA_DebugCommandKeyboard2 = nullptr;

    // キーボード用デバッグコマンド3InputAction
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Common|Input|IA|Debug|Keyboard")
    TObjectPtr<UInputAction> IA_DebugCommandKeyboard3 = nullptr;

    // キーボード用デバッグコマンド4InputAction
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Common|Input|IA|Debug|Keyboard")
    TObjectPtr<UInputAction> IA_DebugCommandKeyboard4 = nullptr;

    // キーボード用デバッグコマンド5InputAction
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Common|Input|IA|Debug|Keyboard")
    TObjectPtr<UInputAction> IA_DebugCommandKeyboard5 = nullptr;

    // ゲームパッド用デバッグコマンド1InputAction
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Common|Input|IA|Debug|Gamepad")
    TObjectPtr<UInputAction> IA_DebugCommandGamepad1 = nullptr;

    // ゲームパッド用デバッグコマンド2InputAction
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Common|Input|IA|Debug|Gamepad")
    TObjectPtr<UInputAction> IA_DebugCommandGamepad2 = nullptr;

public:

    // ----- Parameters -----

    // UIリピートの初回遅延時間
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Common|Input")
    float UIRepeatDelay = 0.25f;

    // UIリピートの間隔
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Common|Input")
    float UIRepeatInterval = 0.12f;

    // 方向入力のデッドゾーン
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Common|Input")
    float DeadZone = 0.50f;

private:

    /** Axis2D入力を4方向へ変換する */
    bool SnapToCardinal(const FVector2D& Axis, FIntPoint& OutDir) const;

    /** InputMappingContextを適用する */
    void ApplyMappingContexts(bool bEnableUI);

    /** UI入力を受け付けるモードか判定する */
    bool IsUIMode() const;

    /** UIリピートを開始する */
    void StartUIRepeat(const FIntPoint& Dir);

    /** UIリピートを停止する */
    void StopUIRepeat();

    /** UIリピート遅延完了時の処理を行う */
    UFUNCTION()
    void OnRepeatDelayElapsed();

    /** UIリピート間隔ごとの処理を行う */
    UFUNCTION()
    void OnRepeatIntervalElapsed();

private:

    // 所有PlayerController
    // WorldとTimerManagerの取得に使用
    UPROPERTY()
    TObjectPtr<APlayerController> PC;

    // EnhancedInputSubsystem
    // InputMappingContextの適用に使用
    UPROPERTY()
    TObjectPtr<UEnhancedInputLocalPlayerSubsystem> Subsys;

    // 現在の入力モード
    ECmnInputMode CurrentMode = ECmnInputMode::Disabled;

    // InputMappingContextの適用有無
    // 初回適用時に同一モードでも処理を通すために使用
    bool bAppliedAtLeastOnce = false;

    // UIリピート中かどうか
    bool bUIRepeating = false;

    // 現在のUI方向入力
    FIntPoint CurrentUIDir = FIntPoint::ZeroValue;

    // UIリピート用TimerHandle
    FTimerHandle RepeatDelayHandle;
    FTimerHandle RepeatIntervalHandle;
};
