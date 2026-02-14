#include "Common/Input/CmnInputRouter.h"

#include "GameFramework/PlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"

void UCmnInputRouter::Initialize(APlayerController* InPC, UEnhancedInputLocalPlayerSubsystem* InSubsys)
{
    PC = InPC;
    Subsys = InSubsys;
}

bool UCmnInputRouter::IsUIMode() const
{
    return (CurrentMode == ECmnInputMode::Menu) || (CurrentMode == ECmnInputMode::Dialog);
}

void UCmnInputRouter::StopUIRepeat()
{
    bUIRepeating = false;
    RepeatTimer = 0.0f;
    CurrentUIDir = FIntPoint::ZeroValue;
}

void UCmnInputRouter::Tick(float DeltaSeconds)
{
    // UIモードのみ、スティック保持による連続移動を処理
    if (!IsUIMode() || !bUIRepeating)
    {
        return;
    }

    RepeatTimer -= DeltaSeconds;
    if (RepeatTimer <= 0.0f)
    {
        OnUINavigation.Broadcast(CurrentUIDir);
        RepeatTimer = UIRepeatInterval;
    }
}

void UCmnInputRouter::SetInputMode(ECmnInputMode NewMode)
{
    if (CurrentMode == NewMode)
    {
        return;
    }

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
            // メニュー/会話はUI操作を想定（ゲームパッドのみならカーソル非表示でもOK）
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

void UCmnInputRouter::ApplyMappingContexts(bool bEnableUI)
{
    if (!Subsys)
    {
        return;
    }

    // いったん外して付け直す（状態ズレ対策）
    if (IMC_Game) { Subsys->RemoveMappingContext(IMC_Game); }
    if (IMC_UI) { Subsys->RemoveMappingContext(IMC_UI); }

    if (IMC_Game)
    {
        Subsys->AddMappingContext(IMC_Game, 0);
    }

    if (bEnableUI && IMC_UI)
    {
        // UIを優先させたいので優先度は高め
        Subsys->AddMappingContext(IMC_UI, 10);
    }
}

bool UCmnInputRouter::SnapToCardinal(const FVector2D& Axis, FIntPoint& OutDir) const
{
    OutDir = FIntPoint::ZeroValue;

    if (Axis.SizeSquared() < DeadZone * DeadZone)
    {
        return false;
    }

    // 大きい方の軸へスナップ
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

void UCmnInputRouter::HandleUINavigateAxis(const FVector2D& Axis)
{
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

    // 初回 or 方向変更時は即時発火（十字はこの挙動が自然）
    if (!bUIRepeating || Dir != CurrentUIDir)
    {
        OnUINavigation.Broadcast(Dir);

        CurrentUIDir = Dir;
        bUIRepeating = true;
        RepeatTimer = UIRepeatDelay;
    }
}

bool UCmnInputRouter::GetMoveDirFromAxis(const FVector2D& Axis, FIntPoint& OutDir) const
{
    return SnapToCardinal(Axis, OutDir);
}
