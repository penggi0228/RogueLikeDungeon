// CmnInputRouter.cpp

#include "Common/Input/CmnInputRouter.h"

#include "Common/Input/CmnInputConfig.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogCmnInputRouter, Log, All);

/** PlayerControllerとEnhancedInputSubsystemを設定する */
void UCmnInputRouter::Initialize(APlayerController* InPC, UEnhancedInputLocalPlayerSubsystem* InSubsys)
{
    PC = InPC;
    Subsys = InSubsys;
}

/** DataAssetから入力設定を適用する */
void UCmnInputRouter::ApplyConfig(const UCmnInputConfig* Config)
{
    // Config未指定時は入力アセット参照をクリア
    if (!Config)
    {
        IMC_Game = nullptr;                                 // 通常操作用IMC
        IMC_UI = nullptr;                                      // UI操作用IMC

        IA_Move = nullptr;                                     // 移動
        IA_Wait = nullptr;                                      // 待機
        IA_Attack = nullptr;                                   // 通常攻撃
        IA_Interact = nullptr;                                 // インタラクト
        IA_Menu = nullptr;                                    // メニュー
        IA_CameraLook = nullptr;                          // カメラ視点操作
        IA_CameraZoom = nullptr;                         // カメラズーム操作

        IA_UI_Direction = nullptr;                          // UI方向入力
        IA_UI_Confirm = nullptr;                           // 決定
        IA_UI_Close = nullptr;                               // メニュー閉じる 
        IA_UI_Scroll = nullptr;                               // スクロール

        IA_DebugCommandPrefix = nullptr;            // デバッグコマンド開始キー
        IA_DebugCommandKeyboard1 = nullptr;     // キーボード用デバッグコマンド第1キー
        IA_DebugCommandKeyboard2 = nullptr;     // キーボード用デバッグコマンド第2キー
        IA_DebugCommandKeyboard3 = nullptr;     // キーボード用デバッグコマンド第3キー
        IA_DebugCommandKeyboard4 = nullptr;     // キーボード用デバッグコマンド第4キー
        IA_DebugCommandKeyboard5 = nullptr;     // キーボード用デバッグコマンド第5キー
        IA_DebugCommandGamepad1 = nullptr;     // ゲームパッド用デバッグコマンド第1キー
        IA_DebugCommandGamepad2 = nullptr;     // ゲームパッド用デバッグコマンド第2キー

        return;
    }

    // InputMappingContext参照を反映
    IMC_Game = Config->IMC_Game;
    IMC_UI = Config->IMC_UI;

    // ゲーム入力アセット参照を反映
    IA_Move = Config->IA_Move;
    IA_Wait = Config->IA_Wait;
    IA_Attack = Config->IA_Attack;
    IA_Interact = Config->IA_Interact;
    IA_Menu = Config->IA_Menu;
    IA_CameraLook = Config->IA_CameraLook;
    IA_CameraZoom = Config->IA_CameraZoom;

    // UI入力アセット参照を反映
    IA_UI_Direction = Config->IA_UI_Direction;
    IA_UI_Confirm = Config->IA_UI_Confirm;
    IA_UI_Close = Config->IA_UI_Close;
    IA_UI_Scroll = Config->IA_UI_Scroll;

    // デバッグ入力アセット参照を反映
    IA_DebugCommandPrefix = Config->IA_DebugCommandPrefix;
    IA_DebugCommandKeyboard1 = Config->IA_DebugCommandKeyboard1;
    IA_DebugCommandKeyboard2 = Config->IA_DebugCommandKeyboard2;
    IA_DebugCommandKeyboard3 = Config->IA_DebugCommandKeyboard3;
    IA_DebugCommandKeyboard4 = Config->IA_DebugCommandKeyboard4;
    IA_DebugCommandKeyboard5 = Config->IA_DebugCommandKeyboard5;
    IA_DebugCommandGamepad1 = Config->IA_DebugCommandGamepad1;
    IA_DebugCommandGamepad2 = Config->IA_DebugCommandGamepad2;

    // 入力設定値を反映
    UIRepeatDelay = Config->UIRepeatDelay;
    UIRepeatInterval = Config->UIRepeatInterval;
    DeadZone = Config->DeadZone;
}

/** UI入力を受け付けるモードか判定する */
bool UCmnInputRouter::IsUIMode() const
{
    return (CurrentMode == ECmnInputMode::Menu) || (CurrentMode == ECmnInputMode::Dialog);
}

/** 入力モードを切り替える */
void UCmnInputRouter::SetInputMode(ECmnInputMode NewMode)
{
    // 初回適用後に同一モードが指定された場合は何もしない
    if (bAppliedAtLeastOnce && CurrentMode == NewMode)
    {
        return;
    }

    // 入力モード更新とUIリピート停止
    bAppliedAtLeastOnce = true;
    CurrentMode = NewMode;
    StopUIRepeat();

    switch (CurrentMode)
    {
    case ECmnInputMode::Game:
        // ゲーム操作用の入力状態を適用
        ApplyMappingContexts(false);
        if (PC)
        {
            PC->SetInputMode(FInputModeGameOnly());
            PC->bShowMouseCursor = false;
        }
        break;

    case ECmnInputMode::Menu:
    case ECmnInputMode::Dialog:
        // UI操作用の入力状態を適用
        ApplyMappingContexts(true);
        if (PC)
        {
            FInputModeGameAndUI Mode;
            Mode.SetHideCursorDuringCapture(false);
            PC->SetInputMode(Mode);
            PC->bShowMouseCursor = true;
        }
        break;

    case ECmnInputMode::Disabled:
        // 操作不能用の入力状態を適用
        ApplyMappingContexts(false);
        if (PC)
        {
            PC->SetInputMode(FInputModeUIOnly());
            PC->bShowMouseCursor = true;
        }
        break;

    default:
        break;
    }

    OnInputModeChanged.Broadcast(CurrentMode);
}

/** IMCを適用する */
void UCmnInputRouter::ApplyMappingContexts(bool bEnableUI)
{
    if (!Subsys)
    {
        UE_LOG(LogCmnInputRouter, Warning, TEXT("ApplyMappingContexts: Subsys NULL"));
        return;
    }

    // 既存のIMCをクリア
    Subsys->ClearAllMappings();

    // 再適用前に現在のIMCを解除
    if (IMC_Game)
    {
        Subsys->RemoveMappingContext(IMC_Game);
    }

    if (IMC_UI)
    {
        Subsys->RemoveMappingContext(IMC_UI);
    }

    // ゲーム用IMCを適用
    // デバッグコマンドはゲーム中に受け付けるためGame側へ含める想定
    if (IMC_Game)
    {
        Subsys->AddMappingContext(IMC_Game, 0);
    }

    // 必要時のみUI用IMCを適用
    if (bEnableUI && IMC_UI)
    {
        Subsys->AddMappingContext(IMC_UI, 10);
    }

    UE_LOG(
        LogCmnInputRouter,
        Warning,
        TEXT("ApplyMappingContexts: GameIMC=%s UIIMC=%s EnableUI=%d"),
        IMC_Game ? *IMC_Game->GetName() : TEXT("NULL"),
        IMC_UI ? *IMC_UI->GetName() : TEXT("NULL"),
        bEnableUI ? 1 : 0
    );
}

/** Axis2D入力を4方向へ変換する */
bool UCmnInputRouter::SnapToCardinal(const FVector2D& Axis, FIntPoint& OutDir) const
{
    OutDir = FIntPoint::ZeroValue;

    // デッドゾーン未満の入力は無効
    if (Axis.SizeSquared() < DeadZone * DeadZone)
    {
        return false;
    }

    // 入力値の大きい軸を採用
    if (FMath::Abs(Axis.X) > FMath::Abs(Axis.Y))
    {
        OutDir = (Axis.X > 0.0f) ? FIntPoint(1, 0) : FIntPoint(-1, 0);
    }
    else
    {
        OutDir = (Axis.Y > 0.0f) ? FIntPoint(0, 1) : FIntPoint(0, -1);
    }

    return true;
}

/** UIリピートを開始する */
void UCmnInputRouter::StartUIRepeat(const FIntPoint& Dir)
{
    // リピート状態と現在方向を更新
    bUIRepeating = true;
    CurrentUIDir = Dir;

    if (!PC)
    {
        return;
    }

    UWorld* World = PC->GetWorld();
    if (!World)
    {
        return;
    }

    FTimerManager& TM = World->GetTimerManager();

    // 既存のUIリピートTimerを停止
    TM.ClearTimer(RepeatDelayHandle);
    TM.ClearTimer(RepeatIntervalHandle);

    // 初回遅延Timerを開始
    TM.SetTimer(
        RepeatDelayHandle,
        this,
        &UCmnInputRouter::OnRepeatDelayElapsed,
        UIRepeatDelay,
        false
    );
}

/** UIリピートを停止する */
void UCmnInputRouter::StopUIRepeat()
{
    // リピート状態と現在方向を初期化
    bUIRepeating = false;
    CurrentUIDir = FIntPoint::ZeroValue;

    if (!PC)
    {
        return;
    }

    UWorld* World = PC->GetWorld();
    if (!World)
    {
        return;
    }

    FTimerManager& TM = World->GetTimerManager();

    // UIリピートTimerを停止
    TM.ClearTimer(RepeatDelayHandle);
    TM.ClearTimer(RepeatIntervalHandle);
}

/** UIリピート遅延完了時の処理を行う */
void UCmnInputRouter::OnRepeatDelayElapsed()
{
    // UIリピート中かつUIモード時のみ処理
    if (!bUIRepeating || !IsUIMode())
    {
        return;
    }

    // 初回の方向入力通知を発火
    OnUIDirection.Broadcast(CurrentUIDir);

    if (!PC)
    {
        return;
    }

    UWorld* World = PC->GetWorld();
    if (!World)
    {
        return;
    }

    FTimerManager& TM = World->GetTimerManager();

    // 連続リピートTimerを開始
    TM.SetTimer(
        RepeatIntervalHandle,
        this,
        &UCmnInputRouter::OnRepeatIntervalElapsed,
        UIRepeatInterval,
        true
    );
}

/** UIリピート間隔ごとの処理を行う */
void UCmnInputRouter::OnRepeatIntervalElapsed()
{
    // UIリピート中かつUIモード時のみ処理
    if (!bUIRepeating || !IsUIMode())
    {
        return;
    }

    OnUIDirection.Broadcast(CurrentUIDir);
}

/** UI方向入力を処理する */
void UCmnInputRouter::HandleUIDirectionAxis(const FVector2D& Axis)
{
    // UIモード以外では処理しない
    if (!IsUIMode())
    {
        return;
    }

    FIntPoint Dir;
    if (!SnapToCardinal(Axis, Dir))
    {
        // デッドゾーン未満の入力時はUIリピート停止
        StopUIRepeat();
        return;
    }

    // 初回入力時または方向変更時は即時通知
    if (!bUIRepeating || Dir != CurrentUIDir)
    {
        OnUIDirection.Broadcast(Dir);
        StartUIRepeat(Dir);
    }
}

/** UIスクロール入力を処理する */
void UCmnInputRouter::HandleUIScrollAxis(float Amount)
{
    // UIモード以外では処理しない
    if (!IsUIMode())
    {
        return;
    }

    const float Threshold = 0.2f;

    // 微小入力は無視
    if (FMath::Abs(Amount) < Threshold)
    {
        return;
    }

    OnUIScroll.Broadcast(Amount);
}

/** 移動入力を4方向へ変換して返す */
bool UCmnInputRouter::GetMoveDirFromAxis(const FVector2D& Axis, FIntPoint& OutDir) const
{
    return SnapToCardinal(Axis, OutDir);
}
