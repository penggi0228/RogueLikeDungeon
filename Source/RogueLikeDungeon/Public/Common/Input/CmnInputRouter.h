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
 * 共通入力ルータ(Common層)
 * - 入力モード管理(Game / Menu / Dialog / Disabled)
 *   ※ モード変更時にIMC・InputMode・カーソル状態を切替
 * - UI方向入力の4方向化 + 保持リピート
 */
UCLASS(BlueprintType)
class ROGUELIKEDUNGEON_API UCmnInputRouter : public UObject
{
    GENERATED_BODY()

public:
    /** 初期化(PlayerController / EnhancedInput Subsystemを保持) */
    void Initialize(APlayerController* InPC, UEnhancedInputLocalPlayerSubsystem* InSubsys);

    /** DataAssetから参照(IMC/IA)とパラメータを取り込む(読み取り専用運用)*/
    void ApplyConfig(const UCmnInputConfig* Config);

    /** 入力モード切替(内部でIMCとInputModeを適用) */
    UFUNCTION(BlueprintCallable, Category = "Common|Input")
    void SetInputMode(ECmnInputMode NewMode);

    /** 現在の入力モード取得 */
    UFUNCTION(BlueprintCallable, Category = "Common|Input")
    ECmnInputMode GetInputMode() const { return CurrentMode; }

    /** モード判定(主にC++側で使用) */
    bool IsGameMode() const { return CurrentMode == ECmnInputMode::Game; }    // ゲームモード
    bool IsMenuMode() const { return CurrentMode == ECmnInputMode::Menu; }     // メニュー画面モード
    bool IsDialogMode() const { return CurrentMode == ECmnInputMode::Dialog; }   // ダイアログモード
    bool IsDisabled() const { return CurrentMode == ECmnInputMode::Disabled; }    // 操作不能

    /** ゲーム移動用：Axis2Dを4方向へ変換して返す(Game側で使用) */
    bool GetMoveDirFromAxis(const FVector2D& Axis, FIntPoint& OutDir) const;

    /** UI方向入力(Axis2D)を受け取り、4方向イベントへ変換(Menu/Dialogのみ有効) */
    void HandleUIDirectionAxis(const FVector2D& Axis);

    /** UIスクロール入力(Axis1D)を受け取り、スクロールイベントへ変換(Menu/Dialogのみ有効) */
    void HandleUIScrollAxis(float Amount);

    /** Timer駆動のためTick不要(互換用に残すが未使用)*/
    void Tick(float DeltaSeconds) { /* unused */ }

public:
    /** UI方向入力通知(Widget側でBind) */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUIDirection, FIntPoint, Direction);

    UPROPERTY(BlueprintAssignable, Category = "Common|Input")
    FOnUIDirection OnUIDirection;

    /** UIスクロール通知(Widget側でBind)(Amount: +で上/前、-で下/後) */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUIScroll, float, Amount);

    UPROPERTY(BlueprintAssignable, Category = "Common|Input")
    FOnUIScroll OnUIScroll;

    /** 入力モード変更通知(UI側の表示/フォーカス切替など) */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInputModeChanged, ECmnInputMode, NewMode);

    UPROPERTY(BlueprintAssignable, Category = "Common|Input")
    FOnInputModeChanged OnInputModeChanged;

public:
    // ----- Input Assets(Configから注入。通常はBPから書き換えない) -----

    /*** IMC ***/
    /** ゲーム操作用 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Common|Input")
    TObjectPtr<UInputMappingContext> IMC_Game = nullptr;

    /** UI操作用 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Common|Input")
    TObjectPtr<UInputMappingContext> IMC_UI = nullptr;

    /*** Input Action ***/
    /** ゲーム移動(Axis2D) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Common|Input")
    TObjectPtr<UInputAction> IA_Move = nullptr;

    /** カメラ視点(Axis2D) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Common|Input")
    TObjectPtr<UInputAction> IA_CameraLook = nullptr;

    /** カメラズーム(Axis1D) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Common|Input")
    TObjectPtr<UInputAction> IA_CameraZoom = nullptr;

    /** UI方向入力(Axis2D) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Common|Input")
    TObjectPtr<UInputAction> IA_UI_Direction = nullptr;

    /** UIスクロール(Axis1D) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Common|Input")
    TObjectPtr<UInputAction> IA_UI_Scroll = nullptr;

    // ----- Parameters(Configから注入。必要ならSetter経由で変更) -----

    /** UIリピート：初回遅延(秒) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Common|Input")
    float UIRepeatDelay = 0.25f;

    /** UIリピート：連続間隔(秒) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Common|Input")
    float UIRepeatInterval = 0.12f;

    /** 方向入力のデッドゾーン(0..1) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Common|Input")
    float DeadZone = 0.50f;

private:
    /** Axis2Dを4方向へスナップ(デッドゾーン考慮) */
    bool SnapToCardinal(const FVector2D& Axis, FIntPoint& OutDir) const;

    /** IMC適用(Gameは常時、UIは必要時のみ追加) */
    void ApplyMappingContexts(bool bEnableUI);

    /** UI入力を受けるモードかどうか(Menu/Dialog) */
    bool IsUIMode() const;

    /** UIリピート開始(方向確定後に呼ぶ) */
    void StartUIRepeat(const FIntPoint& Dir);

    /** UIリピート停止(スティック戻し/モード変更時) */
    void StopUIRepeat();

    /** 初回遅延満了：1回発火して連続へ移行 */
    UFUNCTION()
    void OnRepeatDelayElapsed();

    /** 連続リピート(一定間隔で発火) */
    UFUNCTION()
    void OnRepeatIntervalElapsed();

private:
    /** 所有PlayerController(World/Timer取得に使用) */
    UPROPERTY()
    TObjectPtr<APlayerController> PC;

    /** EnhancedInput Subsystem(IMC適用に使用) */
    UPROPERTY()
    TObjectPtr<UEnhancedInputLocalPlayerSubsystem> Subsys;

    /** 現在の入力モード */
    ECmnInputMode CurrentMode = ECmnInputMode::Disabled;

    /**
     * IMCが一度でも適用されたかどうかのフラグ
     * 初回はCurrentModeが同一でもIMCを適用するために使用
     */
    bool bAppliedAtLeastOnce = false;

    /** UIリピート状態 */
    bool bUIRepeating = false;

    /** リピート中の方向 */
    FIntPoint CurrentUIDir = FIntPoint::ZeroValue;

    /** Timerハンドル(遅延 / 連続) */
    FTimerHandle RepeatDelayHandle;
    FTimerHandle RepeatIntervalHandle;
};