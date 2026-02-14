#include "Game/Input/RldPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Common/Input/CmnInputRouter.h"

void ARldPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // Enhanced Input Subsystem取得
    ULocalPlayer* LP = GetLocalPlayer();
    UEnhancedInputLocalPlayerSubsystem* Subsys = LP ? LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>() : nullptr;

    // ルータ生成（Outer=Controller）
    InputRouter = NewObject<UCmnInputRouter>(this);
    InputRouter->Initialize(this, Subsys);

    // 初期はゲーム
    InputRouter->SetInputMode(ECmnInputMode::Game);
}

void ARldPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent);
    if (!EIC || !InputRouter)
    {
        return;
    }

    if (InputRouter->IA_Move)
    {
        // Started：押した瞬間だけ（1入力=1手）
        EIC->BindAction(InputRouter->IA_Move, ETriggerEvent::Started, this, &ARldPlayerController::OnMoveStarted);
    }

    if (InputRouter->IA_UI_Navigate)
    {
        // Triggered：軸を受け続ける（保持リピートのため）
        EIC->BindAction(InputRouter->IA_UI_Navigate, ETriggerEvent::Triggered, this, &ARldPlayerController::OnUINavigateTriggered);
    }
}

void ARldPlayerController::PlayerTick(float DeltaTime)
{
    Super::PlayerTick(DeltaTime);

    if (InputRouter)
    {
        InputRouter->Tick(DeltaTime);
    }
}

void ARldPlayerController::SetCommonInputMode(ECmnInputMode Mode)
{
    if (InputRouter)
    {
        InputRouter->SetInputMode(Mode);
    }
}

void ARldPlayerController::OnMoveStarted(const FInputActionValue& Value)
{
    if (!InputRouter || !InputRouter->IsGameMode())
    {
        return;
    }

    const FVector2D Axis = Value.Get<FVector2D>();

    FIntPoint Dir;
    if (!InputRouter->GetMoveDirFromAxis(Axis, Dir))
    {
        return;
    }

    // TODO: TurnManagerへ方向を渡す（次ステップ）
}

void ARldPlayerController::OnUINavigateTriggered(const FInputActionValue& Value)
{
    if (!InputRouter || (!InputRouter->IsMenuMode() && !InputRouter->IsDialogMode()))
    {
        return;
    }

    const FVector2D Axis = Value.Get<FVector2D>();
    InputRouter->HandleUINavigateAxis(Axis);
}
