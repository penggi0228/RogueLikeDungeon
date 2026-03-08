// CmnInputRouter.cpp

#include "Common/Input/CmnInputRouter.h"

#include "Common/Input/CmnInputConfig.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "TimerManager.h"

/** 所有者(PC)とEnhancedInputのSubsystemを保持 */
void UCmnInputRouter::Initialize(APlayerController* InPC, UEnhancedInputLocalPlayerSubsystem* InSubsys)
{
    PC = InPC;
    Subsys = InSubsys;
}

/** 入力アセット参照とパラメータをDataAssetから適用 */
void UCmnInputRouter::ApplyConfig(const UCmnInputConfig* Config)
{
    if (!Config)
    {
        IMC_Game = nullptr;
        IMC_UI = nullptr;

        IA_Move = nullptr;
        IA_CameraLook = nullptr;
        IA_CameraZoom = nullptr;
        IA_UI_Direction = nullptr;
        IA_UI_Scroll = nullptr;
        return;
    }

    IMC_Game = Config->IMC_Game;                         // ゲーム操作用IMC
    IMC_UI = Config->IMC_UI;                                   // UI操作用IMC

    IA_Move = Config->IA_Move;                               // ゲーム移動(Axis2D)
    IA_CameraLook = Config->IA_CameraLook;           // カメラ視点(Axis2D)
    IA_CameraZoom = Config->IA_CameraZoom;        // カメラズーム(Axis1D)
    IA_UI_Direction = Config->IA_UI_Direction;           // UI方向入力(Axis2D)
    IA_UI_Scroll = Config->IA_UI_Scroll;                     // UIスクロール(Axis1D)

    UIRepeatDelay = Config->UIRepeatDelay;              // UIリピート：初回遅延(秒)
    UIRepeatInterval = Config->UIRepeatInterval;        // UIリピート：連続間隔(秒)
    DeadZone = Config->DeadZone;                           // 方向入力のデッドゾーン(0..1)
}

/** UI入力を受けるモードかどうか */
bool UCmnInputRouter::IsUIMode() const
{
    return (CurrentMode == ECmnInputMode::Menu) || (CurrentMode == ECmnInputMode::Dialog);
}

/** 入力モード切替(IMC + InputMode + カーソル) */
void UCmnInputRouter::SetInputMode(ECmnInputMode NewMode)
{
    // 入力モード切替(IMC+InputMode+カーソル)
    // ※初回は同じモードでも適用する
    if (bAppliedAtLeastOnce && CurrentMode == NewMode)
    {
        return;
    }

    bAppliedAtLeastOnce = true;
    CurrentMode = NewMode;
    StopUIRepeat();

    switch (CurrentMode)
    {
    case ECmnInputMode::Game:
        ApplyMappingContexts(false);
        if (PC)
        {
            PC->SetInputMode(FInputModeGameOnly());
            PC->bShowMouseCursor = false;
        }
        break;

    case ECmnInputMode::Menu:
    case ECmnInputMode::Dialog:
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

/** IMC入れ替え */
void UCmnInputRouter::ApplyMappingContexts(bool bEnableUI)
{
    if (!Subsys)
    {
        UE_LOG(LogTemp, Warning, TEXT("ApplyMappingContexts: Subsys NULL"));
        return;
    }

    // 最初にIMCをクリア(入力奪取防止)
    Subsys->ClearAllMappings();

    // いったん外して付け直す
    if (IMC_Game)
    {
        Subsys->RemoveMappingContext(IMC_Game);
    }

    if (IMC_UI) 
    {
        Subsys->RemoveMappingContext(IMC_UI);
    }

    // Gameは常時
    if (IMC_Game)
    {
        Subsys->AddMappingContext(IMC_Game, 0);
    }

    // UIは必要時のみ
    if (bEnableUI && IMC_UI)
    {
        Subsys->AddMappingContext(IMC_UI, 10);
    }

    UE_LOG(LogTemp, Warning, TEXT("ApplyMappingContexts: GameIMC=%s UIIMC=%s EnableUI=%d"),
        IMC_Game ? *IMC_Game->GetName() : TEXT("NULL"),
        IMC_UI ? *IMC_UI->GetName() : TEXT("NULL"),
        bEnableUI ? 1 : 0);
}

/** Axis2D を 4方向にスナップ(デッドゾーン考慮) */
bool UCmnInputRouter::SnapToCardinal(const FVector2D& Axis, FIntPoint& OutDir) const
{
    OutDir = FIntPoint::ZeroValue;

    if (Axis.SizeSquared() < DeadZone * DeadZone)
    {
        return false;
    }

    // 大きい方の軸へスナップ(斜め入力は水平/垂直へ寄せる)
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

/** UIリピート：遅延 → 連続 のTimerを開始 */
void UCmnInputRouter::StartUIRepeat(const FIntPoint& Dir)
{
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

    // 既存Timerを停止して再スタート(方向変更時もここを通す)
    TM.ClearTimer(RepeatDelayHandle);
    TM.ClearTimer(RepeatIntervalHandle);

    // 遅延後に初回リピート開始
    TM.SetTimer(
        RepeatDelayHandle,
        this,
        &UCmnInputRouter::OnRepeatDelayElapsed,
        UIRepeatDelay,
        false
    );
}

/** UIリピート停止(Timer停止) */
void UCmnInputRouter::StopUIRepeat()
{
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
    TM.ClearTimer(RepeatDelayHandle);
    TM.ClearTimer(RepeatIntervalHandle);
}

void UCmnInputRouter::OnRepeatDelayElapsed()
{
    // 遅延後：まず1回発火して、その後は連続リピートへ
    if (!bUIRepeating || !IsUIMode())
    {
        return;
    }

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

    // 連続リピート開始
    TM.SetTimer(
        RepeatIntervalHandle,
        this,
        &UCmnInputRouter::OnRepeatIntervalElapsed,
        UIRepeatInterval,
        true
    );
}

void UCmnInputRouter::OnRepeatIntervalElapsed()
{
    // 連続リピート：一定間隔で方向イベントを投げる
    if (!bUIRepeating || !IsUIMode())
    {
        return;
    }

    OnUIDirection.Broadcast(CurrentUIDir);
}

void UCmnInputRouter::HandleUIDirectionAxis(const FVector2D& Axis)
{
    // UI方向入力：Axis2D入力を方向イベントへ変換(Menu/Dialog のみ)
    if (!IsUIMode())
    {
        return;
    }

    FIntPoint Dir;
    if (!SnapToCardinal(Axis, Dir))
    {
        // スティックを戻したらリピート停止
        StopUIRepeat();
        return;
    }

    // 初回 or 方向変更時は即時発火(十字の感覚に合わせる)
    if (!bUIRepeating || Dir != CurrentUIDir)
    {
        OnUIDirection.Broadcast(Dir);
        StartUIRepeat(Dir);
    }
}

void UCmnInputRouter::HandleUIScrollAxis(float Amount)
{
    if (!IsUIMode())
    {
        return;
    }

    // デッドゾーン的なしきい値(右スティック微小入力を無視)
    const float Threshold = 0.2f;
    if (FMath::Abs(Amount) < Threshold)
    {
        return;
    }

    OnUIScroll.Broadcast(Amount);
}

bool UCmnInputRouter::GetMoveDirFromAxis(const FVector2D& Axis, FIntPoint& OutDir) const
{
    // ゲーム移動：Axis2D を 4方向にスナップして返す
    return SnapToCardinal(Axis, OutDir);
}
