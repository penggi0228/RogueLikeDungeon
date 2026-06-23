// CmnInputRouter.cpp

#include "Common/Input/CmnInputRouter.h"

#include "Common/Input/CmnInputConfig.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "InputMappingContext.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogCmnInputRouter, Log, All);

/** PlayerControllerとEnhancedInputSubsystemを設定する */
void UCmnInputRouter::Initialize(APlayerController* InPC, UEnhancedInputLocalPlayerSubsystem* InSubsys)
{
    PC = InPC;
    Subsys = InSubsys;

    UE_LOG(
        LogCmnInputRouter,
        Verbose,
        TEXT("Initialize: PlayerController=%s EnhancedInputSubsystem=%s"),
        *GetNameSafe(PC),
        *GetNameSafe(Subsys)
    );
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
        IA_StepInPlaceModifier = nullptr;                // 足踏み用入力
        IA_Attack = nullptr;                                   // 通常攻撃
        IA_Interact = nullptr;                                 // インタラクト
        IA_Menu = nullptr;                                    // メニュー表示
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

        UE_LOG(
            LogCmnInputRouter,
            Warning,
            TEXT("ApplyConfig: Configがnullのため入力アセット参照をクリアしました")
        );

        return;
    }

    // InputMappingContext参照を反映
    IMC_Game = Config->IMC_Game;
    IMC_UI = Config->IMC_UI;

    // ゲーム入力アセット参照を反映
    IA_Move = Config->IA_Move;
    IA_StepInPlaceModifier = Config->IA_StepInPlaceModifier;
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

    UE_LOG(
        LogCmnInputRouter,
        Verbose,
        TEXT("ApplyConfig: Config=%s 入力設定を適用しました GameIMC=%s UIIMC=%s DeadZone=%f UIRepeatDelay=%f UIRepeatInterval=%f"),
        *GetNameSafe(Config),
        *GetNameSafe(IMC_Game),
        *GetNameSafe(IMC_UI),
        DeadZone,
        UIRepeatDelay,
        UIRepeatInterval
    );
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

    const ECmnInputMode previousMode = CurrentMode;

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
            FInputModeGameAndUI mode;
            mode.SetHideCursorDuringCapture(false);
            PC->SetInputMode(mode);
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

    UE_LOG(
        LogCmnInputRouter,
        Log,
        TEXT("SetInputMode: PlayerController=%s 入力モード変更 前=%d 後=%d UI入力=%s"),
        *GetNameSafe(PC),
        static_cast<int32>(previousMode),
        static_cast<int32>(CurrentMode),
        IsUIMode() ? TEXT("有効") : TEXT("無効")
    );
}

/** IMCを適用する */
void UCmnInputRouter::ApplyMappingContexts(bool bEnableUI)
{
    // EnhancedInputSubsystem未取得時はIMCを適用しない
    if (!Subsys)
    {
        UE_LOG(
            LogCmnInputRouter,
            Warning,
            TEXT("ApplyMappingContexts: PlayerController=%s EnhancedInputSubsystemがnullのためIMCを適用しません"),
            *GetNameSafe(PC)
        );

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
        Verbose,
        TEXT("ApplyMappingContexts: PlayerController=%s IMC参照状態 GameIMC=%s UIIMC=%s"),
        *GetNameSafe(PC),
        *GetNameSafe(IMC_Game),
        *GetNameSafe(IMC_UI)
    );
}

/** Axis2D入力を8方向へ変換する */
bool UCmnInputRouter::SnapToGridDirection(const FVector2D& Axis, FIntPoint& OutDir) const
{
    OutDir = FIntPoint::ZeroValue;

    // デッドゾーン未満の入力は無効
    if (Axis.SizeSquared() < DeadZone * DeadZone)
    {
        return false;
    }

    const float absX = FMath::Abs(Axis.X);
    const float absY = FMath::Abs(Axis.Y);

    // 軸入力がほぼゼロの場合は変換しない
    if (absX < KINDA_SMALL_NUMBER && absY < KINDA_SMALL_NUMBER)
    {
        return false;
    }

    const float maxAxis = FMath::Max(absX, absY);
    const float minAxis = FMath::Min(absX, absY);

    // 大きい軸に対して小さい軸がこの割合以上なら斜め方向として扱う
    const float diagonalRatioThreshold = 0.50f;

    const int32 signX = (Axis.X >= 0.0f) ? 1 : -1;
    const int32 signY = (Axis.Y >= 0.0f) ? 1 : -1;

    // X/Y両方の入力が十分にある場合は斜め方向へ変換
    if ((minAxis / maxAxis) >= diagonalRatioThreshold)
    {
        OutDir = FIntPoint(signX, signY);
    }
    // X軸が強い場合は左右方向へ変換
    else if (absX > absY)
    {
        OutDir = FIntPoint(signX, 0);
    }
    // Y軸が強い場合は上下方向へ変換
    else
    {
        OutDir = FIntPoint(0, signY);
    }

    // 入力変換ログが必要な場合のみ有効化する
    //UE_LOG(
    //    LogCmnInputRouter,
    //    Verbose,
    //    TEXT("SnapToGridDirection: Axis2D入力を8方向へ変換しました 入力=(%f,%f) 方向=(%d,%d)"),
    //    Axis.X,
    //    Axis.Y,
    //    OutDir.X,
    //    OutDir.Y
    //);

    return true;
}

/** UIリピートを開始する */
void UCmnInputRouter::StartUIRepeat(const FIntPoint& Dir)
{
    // PlayerController未取得時はTimerを開始しない
    if (!PC)
    {
        UE_LOG(
            LogCmnInputRouter,
            Warning,
            TEXT("StartUIRepeat: PlayerController未取得のためUIリピートTimerを開始しません")
        );

        return;
    }

    UWorld* world = PC->GetWorld();

    // World未取得時はTimerを開始しない
    if (!world)
    {
        UE_LOG(
            LogCmnInputRouter,
            Warning,
            TEXT("StartUIRepeat: PlayerController=%s World未取得のためUIリピートTimerを開始しません"),
            *GetNameSafe(PC)
        );

        return;
    }

    // リピート状態と現在の方向を更新
    bUIRepeating = true;
    CurrentUIDir = Dir;

    FTimerManager& timerManager = world->GetTimerManager();

    // 既存のUIリピートTimerを停止
    timerManager.ClearTimer(RepeatDelayHandle);
    timerManager.ClearTimer(RepeatIntervalHandle);

    // 初回遅延Timerを開始
    timerManager.SetTimer(
        RepeatDelayHandle,
        this,
        &UCmnInputRouter::OnRepeatDelayElapsed,
        UIRepeatDelay,
        false
    );

    UE_LOG(
        LogCmnInputRouter,
        Verbose,
        TEXT("StartUIRepeat: PlayerController=%s UIリピート開始 方向=(%d,%d) 初回遅延=%f"),
        *GetNameSafe(PC),
        Dir.X,
        Dir.Y,
        UIRepeatDelay
    );
}

/** UIリピートを停止する */
void UCmnInputRouter::StopUIRepeat()
{
    // リピート状態と現在の方向を初期化
    bUIRepeating = false;
    CurrentUIDir = FIntPoint::ZeroValue;

    // PlayerController未取得時はTimer停止しない
    if (!PC)
    {
        return;
    }

    UWorld* world = PC->GetWorld();

    // World未取得時はTimer停止しない
    if (!world)
    {
        return;
    }

    FTimerManager& timerManager = world->GetTimerManager();

    // UIリピートTimerを停止
    timerManager.ClearTimer(RepeatDelayHandle);
    timerManager.ClearTimer(RepeatIntervalHandle);
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

    // PlayerController未取得時は継続リピートTimerを開始しない
    if (!PC)
    {
        return;
    }

    UWorld* world = PC->GetWorld();

    // World未取得時は継続リピートTimerを開始しない
    if (!world)
    {
        return;
    }

    FTimerManager& timerManager = world->GetTimerManager();

    // 連続リピートTimerを開始
    timerManager.SetTimer(
        RepeatIntervalHandle,
        this,
        &UCmnInputRouter::OnRepeatIntervalElapsed,
        UIRepeatInterval,
        true
    );

    UE_LOG(
        LogCmnInputRouter,
        Verbose,
        TEXT("OnRepeatDelayElapsed: PlayerController=%s UIリピート継続開始 方向=(%d,%d) 間隔=%f"),
        *GetNameSafe(PC),
        CurrentUIDir.X,
        CurrentUIDir.Y,
        UIRepeatInterval
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

    FIntPoint dir;

    // デッドゾーン未満の入力時はUIリピート停止
    if (!SnapToGridDirection(Axis, dir))
    {
        StopUIRepeat();
        return;
    }

    // 初回入力時または方向変更時は即時通知
    if (!bUIRepeating || dir != CurrentUIDir)
    {
        OnUIDirection.Broadcast(dir);
        StartUIRepeat(dir);
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

    const float threshold = 0.2f;

    // 微小入力は無視
    if (FMath::Abs(Amount) < threshold)
    {
        return;
    }

    OnUIScroll.Broadcast(Amount);

    // UIスクロール入力ログが必要な場合のみ有効化する
    //UE_LOG(
    //    LogCmnInputRouter,
    //    Verbose,
    //    TEXT("HandleUIScrollAxis: UIスクロール入力を通知しました Amount=%f"),
    //    Amount
    //);
}

/** 移動入力を8方向へ変換して返す */
bool UCmnInputRouter::GetMoveDirFromAxis(const FVector2D& Axis, FIntPoint& OutDir) const
{
    return SnapToGridDirection(Axis, OutDir);
}
