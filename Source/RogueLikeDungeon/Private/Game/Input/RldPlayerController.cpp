// RldPlayerController.cpp

#include "Game/Input/RldPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/GameInstance.h"
#include "InputCoreTypes.h"

#include "Common/Input/CmnInputRouter.h"
#include "Common/Input/CmnInputConfig.h"
#include "Common/Settings/CmnSettingsSubsystem.h"
#include "Game/Characters/RldPlayerCharacter.h"

DEFINE_LOG_CATEGORY_STATIC(LogRldInput, Log, All);

ARldPlayerController::ARldPlayerController()
{
    // GCの安全性のため、Routerは早めに確保
    InputRouter = CreateDefaultSubobject<UCmnInputRouter>(TEXT("InputRouter"));

    // 入力設定DataAssetのデフォルト参照
    InputConfigAsset = TSoftObjectPtr<UCmnInputConfig>(
        FSoftObjectPath(TEXT("/Game/Game/Input/DA_CmnInputConfig_Default.DA_CmnInputConfig_Default"))
    );
}

// ----- AActor -----

/**
 * IA参照をBind前に確定させるため
 * DataAssetを同期ロードしRouterへ適用する
 */
void ARldPlayerController::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    LoadAndApplyInputConfig();
}

/**
 * プレイ開始時処理
 * 設定Subsystemを取得し、設定変更通知に登録する
 * その後EnhancedInputを初期化し、初期入力モードを適用する
 */
void ARldPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // InputConfig取得
    if (!LoadedInputConfig)
    {
        LoadAndApplyInputConfig();
    }

    // GameInstance取得
    if (GetGameInstance())
    {
        SettingsSubsystem = GetGameInstance()->GetSubsystem<UCmnSettingsSubsystem>();
    }

    // SettingsSubsystem取得
    if (SettingsSubsystem)
    {
        SettingsSubsystem->GetOnInputSettingsChanged().AddUObject(
            this,
            &ARldPlayerController::HandleInputSettingsChanged
        );

        // 初期設定同期
        HandleInputSettingsChanged(SettingsSubsystem->GetCurrentSettings());
    }

    ULocalPlayer* LocalPlayer = GetLocalPlayer();
    UEnhancedInputLocalPlayerSubsystem* Subsystem =
        LocalPlayer ? LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>() : nullptr;

    if (!ensure(InputRouter && Subsystem))
    {
        return;
    }

    InputRouter->Initialize(this, Subsystem);
    InputRouter->SetInputMode(ECmnInputMode::Game);
}

/**
 * 終了時にデリゲートを解除する
 * 多重登録やDangling参照を防ぐための安全措置
 */
void ARldPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopMoveRepeat();

    if (SettingsSubsystem)
    {
        SettingsSubsystem->GetOnInputSettingsChanged().RemoveAll(this);
    }

    Super::EndPlay(EndPlayReason);
}

// ----- AController -----

/**
 * IAのBindのみ行う
 * IMC適用はRouterへ委譲する
 */
void ARldPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    EnsureEnhancedInputComponent();

    UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent);
    if (!ensure(EIC && InputRouter))
    {
        UE_LOG(LogRldInput, Error, TEXT("SetupInputComponent failed: EIC or InputRouter is null"));
        return;
    }

    UE_LOG(LogRldInput, Log, TEXT("SetupInputComponent: Begin Bind"));

    // ----- ゲーム操作 -----

    // 移動
    if (InputRouter->IA_Move)
    {
        UE_LOG(LogRldInput, Log, TEXT("Bind IA_Move"));

        EIC->BindAction(InputRouter->IA_Move, ETriggerEvent::Started, this, &ARldPlayerController::OnMoveStarted);
        EIC->BindAction(InputRouter->IA_Move, ETriggerEvent::Triggered, this, &ARldPlayerController::OnMoveTriggered);
    }
    else
    {
        UE_LOG(LogRldInput, Warning, TEXT("IA_Move is null"));
    }

    // カメラ視点
    if (InputRouter->IA_CameraLook)
    {
        UE_LOG(LogRldInput, Log, TEXT("Bind IA_CameraLook"));
        EIC->BindAction(InputRouter->IA_CameraLook, ETriggerEvent::Triggered, this, &ARldPlayerController::OnCameraLookTriggered);
    }
    else
    {
        UE_LOG(LogRldInput, Warning, TEXT("IA_CameraLook is null"));
    }

    // カメラズーム
    if (InputRouter->IA_CameraZoom)
    {
        UE_LOG(LogRldInput, Log, TEXT("Bind IA_CameraZoom"));
        EIC->BindAction(InputRouter->IA_CameraZoom, ETriggerEvent::Triggered, this, &ARldPlayerController::OnCameraZoomTriggered);
    }
    else
    {
        UE_LOG(LogRldInput, Warning, TEXT("IA_CameraZoom is null"));
    }

    // ----- UI操作 -----

    // UI方向入力
    if (InputRouter->IA_UI_Direction)
    {
        UE_LOG(LogRldInput, Log, TEXT("Bind IA_UI_Direction"));
        EIC->BindAction(InputRouter->IA_UI_Direction, ETriggerEvent::Triggered, this, &ARldPlayerController::OnUIDirectionTriggered);
    }
    else
    {
        UE_LOG(LogRldInput, Warning, TEXT("IA_UI_Direction is null"));
    }

    // UIスクロール
    if (InputRouter->IA_UI_Scroll)
    {
        UE_LOG(LogRldInput, Log, TEXT("Bind IA_UI_Scroll"));
        EIC->BindAction(InputRouter->IA_UI_Scroll, ETriggerEvent::Triggered, this, &ARldPlayerController::OnUIScrollTriggered);
    }
    else
    {
        UE_LOG(LogRldInput, Warning, TEXT("IA_UI_Scroll is null"));
    }
}

// ----- 公開API -----

/**
 * 共通入力モードを切り替える
 * IMC適用およびInputMode制御はRouterへ委譲する
 */
void ARldPlayerController::SetCommonInputMode(ECmnInputMode Mode)
{
    if (InputRouter)
    {
        InputRouter->SetInputMode(Mode);
    }
}

/**
 * Routerを取得する
 */
UCmnInputRouter* ARldPlayerController::GetInputRouter() const
{
    return InputRouter;
}

// ----- 設定通知 -----

/**
 * 設定変更時にローカルキャッシュを更新する
 * 毎フレームGetせず、イベント駆動で同期する
 */
void ARldPlayerController::HandleInputSettingsChanged(const FInputRuntimeSettings& NewSettings)
{
    bInvertCameraX = NewSettings.bInvertCameraX;
    bInvertCameraY = NewSettings.bInvertCameraY;

    UE_LOG(
        LogRldInput,
        Log,
        TEXT("SettingsChanged: InvertX=%d InvertY=%d"),
        bInvertCameraX ? 1 : 0,
        bInvertCameraY ? 1 : 0
    );
}

// ----- ゲーム操作の入力 -----

/**
 * ゲーム移動入力を4方向へ変換する
 * 1入力=1ターン想定のためStartedで受ける
 */
void ARldPlayerController::OnMoveStarted(const FInputActionValue& Value)
{
    if (!InputRouter || !InputRouter->IsGameMode())
    {
        UE_LOG(LogRldInput, Verbose, TEXT("OnMoveStarted skipped: not game mode"));
        return;
    }

    const FVector2D Axis = Value.Get<FVector2D>();

    // 左スティックはTriggered側で専用処理する
    if (IsLikelyLeftStickInput(Axis))
    {
        return;
    }

    const FString InputSourceText = BuildMoveInputSourceDebugText();

    UE_LOG(
        LogRldInput,
        Log,
        TEXT("Move Started: Source=%s RawAxis(X=%f Y=%f)"),
        *InputSourceText,
        Axis.X,
        Axis.Y
    );

    FIntPoint Direction;
    if (!TryConvertMoveAxisToGridDir(Axis, Direction))
    {
        UE_LOG(
            LogRldInput,
            Warning,
            TEXT("Move direction conversion failed: Source=%s RawAxis(X=%f Y=%f)"),
            *InputSourceText,
            Axis.X,
            Axis.Y
        );
        return;
    }

    ProcessResolvedMoveDirection(Direction, InputSourceText, Axis);
}

/**
 * 移動入力の生値をログ出力する
 * デバッグ時に入力値の入り方を確認するために使用する
 */
void ARldPlayerController::OnMoveTriggered(const FInputActionValue& Value)
{
    const FVector2D Axis = Value.Get<FVector2D>();
    const FString InputSourceText = BuildMoveInputSourceDebugText();

    UE_LOG(
        LogRldInput,
        Verbose,
        TEXT("Move Triggered: Source=%s RawAxis(X=%f Y=%f)"),
        *InputSourceText,
        Axis.X,
        Axis.Y
    );

    // 左スティック入力のみTriggered側で専用処理する
    if (IsLikelyLeftStickInput(Axis))
    {
        HandleLeftStickMoveTriggered(Axis);
    }
}

/**
 * カメラ視点入力処理
 * 反転設定はController側で最終適用する
 */
void ARldPlayerController::OnCameraLookTriggered(const FInputActionValue& Value)
{
    const FVector2D RawAxis = Value.Get<FVector2D>();

    UE_LOG(
        LogRldInput,
        Log,
        TEXT("CameraLook RawAxis: X=%f Y=%f"),
        RawAxis.X,
        RawAxis.Y
    );

    if (!InputRouter || !InputRouter->IsGameMode())
    {
        UE_LOG(LogRldInput, Verbose, TEXT("CameraLook skipped: not game mode"));
        return;
    }

    FVector2D Axis = RawAxis;

    if (bInvertCameraX)
    {
        Axis.X *= -1.0f;
    }

    if (bInvertCameraY)
    {
        Axis.Y *= -1.0f;
    }

    UE_LOG(
        LogRldInput,
        Log,
        TEXT("CameraLook AfterInvert: X=%f Y=%f"),
        Axis.X,
        Axis.Y
    );

    ARldPlayerCharacter* PlayerCharacter = GetRldPlayerCharacter();
    if (!PlayerCharacter)
    {
        UE_LOG(LogRldInput, Warning, TEXT("OnCameraLookTriggered: PlayerCharacter is null"));
        return;
    }

    if (!PlayerCharacter->CanAcceptLookInput())
    {
        UE_LOG(LogRldInput, Verbose, TEXT("OnCameraLookTriggered: Look input rejected"));
        return;
    }

    PlayerCharacter->RequestLookInput(Axis);
}

/**
 * カメラズーム入力処理
 * L2 / R2やマウスホイール等のズーム入力を統一的に受け取る
 */
void ARldPlayerController::OnCameraZoomTriggered(const FInputActionValue& Value)
{
    const float ZoomValue = Value.Get<float>();

    UE_LOG(
        LogRldInput,
        Log,
        TEXT("CameraZoom RawValue: %f"),
        ZoomValue
    );

    if (!InputRouter || !InputRouter->IsGameMode())
    {
        UE_LOG(LogRldInput, Verbose, TEXT("CameraZoom skipped: not game mode"));
        return;
    }

    UE_LOG(
        LogRldInput,
        Log,
        TEXT("CameraZoom: %f"),
        ZoomValue
    );

    ARldPlayerCharacter* PlayerCharacter = GetRldPlayerCharacter();
    if (!PlayerCharacter)
    {
        UE_LOG(LogRldInput, Warning, TEXT("OnCameraZoomTriggered: PlayerCharacter is null"));
        return;
    }

    if (!PlayerCharacter->CanAcceptZoomInput())
    {
        UE_LOG(LogRldInput, Verbose, TEXT("OnCameraZoomTriggered: Zoom input rejected"));
        return;
    }

    PlayerCharacter->RequestZoomInput(ZoomValue);
}

// ----- UI操作の入力 -----

/**
 * UI方向入力をRouterへ委譲する
 * 4方向化およびリピート制御はRouter側で行う
 */
void ARldPlayerController::OnUIDirectionTriggered(const FInputActionValue& Value)
{
    if (!InputRouter || (!InputRouter->IsMenuMode() && !InputRouter->IsDialogMode()))
    {
        return;
    }

    const FVector2D Axis = Value.Get<FVector2D>();
    InputRouter->HandleUIDirectionAxis(Axis);
}

/**
 * UIスクロール入力をRouterへ委譲する
 * スクロールのしきい値判定はRouter側で行う
 */
void ARldPlayerController::OnUIScrollTriggered(const FInputActionValue& Value)
{
    if (!InputRouter || (!InputRouter->IsMenuMode() && !InputRouter->IsDialogMode()))
    {
        return;
    }

    const float Amount = Value.Get<float>();
    InputRouter->HandleUIScrollAxis(Amount);
}

// ----- ゲーム固有入力変換 -----

/**
 * 入力軸をローグライク用4方向へ変換する
 *
 * この変換はゲーム固有仕様であり、Common層には置かない
 * 入力はカメラ基準で解釈し、最終的にグリッド4方向へ丸める
 */
bool ARldPlayerController::TryConvertMoveAxisToGridDir(
    const FVector2D& Axis,
    FIntPoint& OutDirection
) const
{
    FVector WorldDirection;
    if (!TryConvertInputAxisToCameraRelativeWorldDir(Axis, WorldDirection))
    {
        return false;
    }

    return TryConvertWorldDirToGridDir(WorldDirection, OutDirection);
}

/**
 * 入力軸をカメラ基準のワールド平面方向へ変換する
 *
 * W / ↑ / 左スティック↑ は「画面上方向」
 * D / → は「画面右方向」
 * を意味するように、ControlRotationのYawを基準に平面ベクトルへ変換する
 */
bool ARldPlayerController::TryConvertInputAxisToCameraRelativeWorldDir(
    const FVector2D& Axis,
    FVector& OutWorldDirection
) const
{
    OutWorldDirection = FVector::ZeroVector;

    // 入力しきい値
    const float InputDeadZone = 0.5f;

    const float AbsX = FMath::Abs(Axis.X);
    const float AbsY = FMath::Abs(Axis.Y);

    if (AbsX < InputDeadZone && AbsY < InputDeadZone)
    {
        return false;
    }

    // 実際に見えているカメラ向きを基準にする
    const ARldPlayerCharacter* PlayerCharacter = GetRldPlayerCharacter();
    if (!PlayerCharacter)
    {
        return false;
    }

    const FVector CameraForward = PlayerCharacter->GetCameraPlanarForward();
    const FVector CameraRight = PlayerCharacter->GetCameraPlanarRight();

    // 入力をカメラ基準の平面方向へ変換
    FVector MoveWorldDirection =
        (CameraForward * Axis.Y) +
        (CameraRight * Axis.X);

    MoveWorldDirection.Z = 0.0f;

    if (MoveWorldDirection.IsNearlyZero(KINDA_SMALL_NUMBER))
    {
        return false;
    }

    MoveWorldDirection.Normalize();
    OutWorldDirection = MoveWorldDirection;

    UE_LOG(
        LogRldInput,
        Verbose,
        TEXT("CameraRelativeWorldDir: Axis(X=%f Y=%f) WorldDir(X=%f Y=%f Z=%f)"),
        Axis.X,
        Axis.Y,
        OutWorldDirection.X,
        OutWorldDirection.Y,
        OutWorldDirection.Z
    );

    return true;
}

/**
 * ワールド平面方向をグリッド4方向へ変換する
 *
 * 現在のグリッド定義は
 * Grid X = World X
 * Grid Y = World Y
 * として扱う
 */
bool ARldPlayerController::TryConvertWorldDirToGridDir(
    const FVector& WorldDirection,
    FIntPoint& OutGridDirection
) const
{
    OutGridDirection = FIntPoint::ZeroValue;

    const float AbsX = FMath::Abs(WorldDirection.X);
    const float AbsY = FMath::Abs(WorldDirection.Y);

    if (AbsX < KINDA_SMALL_NUMBER && AbsY < KINDA_SMALL_NUMBER)
    {
        return false;
    }

    // 主軸優先で4方向へ変換
    if (AbsX >= AbsY)
    {
        OutGridDirection = (WorldDirection.X >= 0.0f)
            ? FIntPoint(1, 0)
            : FIntPoint(-1, 0);
    }
    else
    {
        OutGridDirection = (WorldDirection.Y >= 0.0f)
            ? FIntPoint(0, 1)
            : FIntPoint(0, -1);
    }

    UE_LOG(
        LogRldInput,
        Verbose,
        TEXT("WorldDirToGridDir: WorldDir(X=%f Y=%f) -> GridDir(%d, %d)"),
        WorldDirection.X,
        WorldDirection.Y,
        OutGridDirection.X,
        OutGridDirection.Y
    );

    return true;
}

/**
 * 確定した移動方向を処理する
 */
void ARldPlayerController::ProcessResolvedMoveDirection(
    const FIntPoint& Direction,
    const FString& InputSourceText,
    const FVector2D& Axis
)
{
    UE_LOG(
        LogRldInput,
        Log,
        TEXT("Move: Source=%s Dir(%d, %d) RawAxis(X=%f Y=%f)"),
        *InputSourceText,
        Direction.X,
        Direction.Y,
        Axis.X,
        Axis.Y
    );

    StartMoveRepeat(Direction, InputSourceText, Axis);

    ARldPlayerCharacter* PlayerCharacter = GetRldPlayerCharacter();
    if (!PlayerCharacter)
    {
        UE_LOG(LogRldInput, Warning, TEXT("ProcessResolvedMoveDirection: PlayerCharacter is null"));
        return;
    }

    if (!PlayerCharacter->CanAcceptMoveInput())
    {
        UE_LOG(LogRldInput, Verbose, TEXT("ProcessResolvedMoveDirection: Move input rejected"));
        return;
    }

    PlayerCharacter->RequestMoveDirection(Direction);
}

// ----- 左スティック専用処理 -----

/**
 * 左スティック入力かどうかを判定する
 *
 * 現在のデバッグ補助では、十字キーとキーボードはSource文字列で判定しやすいが、
 * 左スティックはStarted直後にUnknownになることがある
 * そのため、アナログキー状態から左スティック入力かどうかを判定する
 */
bool ARldPlayerController::IsLikelyLeftStickInput(const FVector2D& Axis) const
{
    const float LeftStickX = GetInputAnalogKeyState(EKeys::Gamepad_LeftX);
    const float LeftStickY = GetInputAnalogKeyState(EKeys::Gamepad_LeftY);

    const bool bHasLeftStickAnalog =
        !FMath::IsNearlyZero(LeftStickX, KINDA_SMALL_NUMBER) ||
        !FMath::IsNearlyZero(LeftStickY, KINDA_SMALL_NUMBER);

    if (bHasLeftStickAnalog)
    {
        return true;
    }

    // 念のため、微小アナログ値の軸入力も左スティック候補として扱う
    const bool bHasAnalogLikeAxis =
        !FMath::IsNearlyZero(Axis.X, KINDA_SMALL_NUMBER) ||
        !FMath::IsNearlyZero(Axis.Y, KINDA_SMALL_NUMBER);

    return bHasAnalogLikeAxis &&
        FMath::Abs(Axis.X) < 1.0f &&
        FMath::Abs(Axis.Y) < 1.0f;
}

/**
 * 左スティックの1入力=1ターン確定処理
 *
 * Triggeredで継続的に値を監視し、
 * しきい値を超えた瞬間のみ方向入力を確定する
 * その後はニュートラルへ戻るまで再入力を受け付けない
 */
void ARldPlayerController::HandleLeftStickMoveTriggered(const FVector2D& Axis)
{
    // ニュートラル復帰判定用しきい値
    const float ReleaseThreshold = 0.25f;

    // 入力確定用しきい値
    const float ConfirmThreshold = 0.5f;

    const float AbsX = FMath::Abs(Axis.X);
    const float AbsY = FMath::Abs(Axis.Y);

    // ニュートラルへ戻ったら再入力可能にする
    if (AbsX < ReleaseThreshold && AbsY < ReleaseThreshold)
    {
        if (bLeftStickMoveConsumed)
        {
            UE_LOG(LogRldInput, Verbose, TEXT("LeftStick reset to neutral"));
        }

        bLeftStickMoveConsumed = false;
        StopMoveRepeat();
        return;
    }

    // すでに1回確定済みなら、ニュートラル復帰まで無視
    if (bLeftStickMoveConsumed)
    {
        return;
    }

    // しきい値未満ならまだ確定しない
    if (AbsX < ConfirmThreshold && AbsY < ConfirmThreshold)
    {
        return;
    }

    FIntPoint Direction;
    if (!TryConvertMoveAxisToGridDir(Axis, Direction))
    {
        UE_LOG(
            LogRldInput,
            Warning,
            TEXT("LeftStick conversion failed: RawAxis(X=%f Y=%f)"),
            Axis.X,
            Axis.Y
        );
        return;
    }

    bLeftStickMoveConsumed = true;

    // 確定した移動方向の値を処理メソッドへ渡す
    ProcessResolvedMoveDirection(
        Direction,
        TEXT("ゲームパッドLスティック"),
        Axis
    );
}

// ----- Character取得 -----

/**
 * 現在操作中のRogueLikeDungeon用プレイヤーCharacterを取得する
 */
ARldPlayerCharacter* ARldPlayerController::GetRldPlayerCharacter() const
{
    return Cast<ARldPlayerCharacter>(GetPawn());
}

// ----- デバッグ補助 -----

/**
 * 現在の移動入力元をデバッグ文字列として取得する
 *
 * EnhancedInputのコールバック引数だけでは物理入力元を直接特定しづらいため、
 * その瞬間のキー状態とアナログ値から推定してログ出力に使用する
 */
FString ARldPlayerController::BuildMoveInputSourceDebugText() const
{
    TArray<FString> Sources;

    // ----- キーボード -----

    if (IsInputKeyDown(EKeys::W))
    {
        Sources.Add(TEXT("キーボード(W)"));
    }

    if (IsInputKeyDown(EKeys::S))
    {
        Sources.Add(TEXT("キーボード(S)"));
    }

    if (IsInputKeyDown(EKeys::A))
    {
        Sources.Add(TEXT("キーボード(A)"));
    }

    if (IsInputKeyDown(EKeys::D))
    {
        Sources.Add(TEXT("キーボード(D)"));
    }

    // ----- ゲームパッド十字キー -----

    if (IsInputKeyDown(EKeys::Gamepad_DPad_Up))
    {
        Sources.Add(TEXT("ゲームパッド十字キー(↑)"));
    }

    if (IsInputKeyDown(EKeys::Gamepad_DPad_Down))
    {
        Sources.Add(TEXT("ゲームパッド十字キー(↓)"));
    }

    if (IsInputKeyDown(EKeys::Gamepad_DPad_Left))
    {
        Sources.Add(TEXT("ゲームパッド十字キー(←)"));
    }

    if (IsInputKeyDown(EKeys::Gamepad_DPad_Right))
    {
        Sources.Add(TEXT("ゲームパッド十字キー(→)"));
    }

    // ----- ゲームパッドLスティック -----

    const float LeftStickX = GetInputAnalogKeyState(EKeys::Gamepad_LeftX);
    const float LeftStickY = GetInputAnalogKeyState(EKeys::Gamepad_LeftY);

    const bool bHasLeftStickInput =
        !FMath::IsNearlyZero(LeftStickX, KINDA_SMALL_NUMBER) ||
        !FMath::IsNearlyZero(LeftStickY, KINDA_SMALL_NUMBER);

    if (bHasLeftStickInput)
    {
        Sources.Add(FString::Printf(
            TEXT("ゲームパッドLスティック(X=%0.3f Y=%0.3f)"),
            LeftStickX,
            LeftStickY
        ));
    }

    // ----- 判定結果なし -----

    if (Sources.Num() == 0)
    {
        return TEXT("Unknown");
    }

    return FString::Join(Sources, TEXT(" / "));
}

// ----- 移動リピート制御 -----

/**
 * 押しっぱなし移動のリピートを開始する
 */
void ARldPlayerController::StartMoveRepeat(
    const FIntPoint& Direction,
    const FString& InputSourceText,
    const FVector2D& Axis
)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    RepeatingMoveDirection = Direction;
    RepeatingMoveInputSourceText = InputSourceText;
    RepeatingMoveAxis = Axis;
    bMoveRepeatActive = true;

    World->GetTimerManager().ClearTimer(MoveRepeatStartTimerHandle);
    World->GetTimerManager().ClearTimer(MoveRepeatTickTimerHandle);

    World->GetTimerManager().SetTimer(
        MoveRepeatStartTimerHandle,
        this,
        &ARldPlayerController::BeginMoveRepeat,
        MoveRepeatInitialDelay,
        false
    );
}

/**
 * 押しっぱなし移動のリピートを停止する
 */
void ARldPlayerController::StopMoveRepeat()
{
    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().ClearTimer(MoveRepeatStartTimerHandle);
        World->GetTimerManager().ClearTimer(MoveRepeatTickTimerHandle);
    }

    bMoveRepeatActive = false;
    RepeatingMoveDirection = FIntPoint::ZeroValue;
    RepeatingMoveInputSourceText.Empty();
    RepeatingMoveAxis = FVector2D::ZeroVector;
}

/**
 * 押しっぱなし移動の初回遅延後に呼ばれる
 */
void ARldPlayerController::BeginMoveRepeat()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    if (!ShouldContinueMoveRepeat())
    {
        StopMoveRepeat();
        return;
    }

    TickMoveRepeat();

    World->GetTimerManager().SetTimer(
        MoveRepeatTickTimerHandle,
        this,
        &ARldPlayerController::TickMoveRepeat,
        MoveRepeatInterval,
        true
    );
}

/**
 * 押しっぱなし移動の定期実行
 */
void ARldPlayerController::TickMoveRepeat()
{
    if (!ShouldContinueMoveRepeat())
    {
        StopMoveRepeat();
        return;
    }

    UE_LOG(
        LogRldInput,
        Log,
        TEXT("Move Repeat: Source=%s Dir(%d, %d)"),
        *RepeatingMoveInputSourceText,
        RepeatingMoveDirection.X,
        RepeatingMoveDirection.Y
    );

    ARldPlayerCharacter* PlayerCharacter = GetRldPlayerCharacter();
    if (!PlayerCharacter)
    {
        UE_LOG(LogRldInput, Warning, TEXT("TickMoveRepeat: PlayerCharacter is null"));
        StopMoveRepeat();
        return;
    }

    if (!PlayerCharacter->CanAcceptMoveInput())
    {
        UE_LOG(LogRldInput, Verbose, TEXT("TickMoveRepeat: Move input rejected"));
        return;
    }

    PlayerCharacter->RequestMoveDirection(RepeatingMoveDirection);
}

/**
 * 現在の入力状態から、押しっぱなし移動を継続すべきか判定する
 */
bool ARldPlayerController::ShouldContinueMoveRepeat() const
{
    if (!bMoveRepeatActive)
    {
        return false;
    }

    FVector2D CurrentAxis = FVector2D::ZeroVector;

    // ----- キーボード -----

    if (IsInputKeyDown(EKeys::W))
    {
        CurrentAxis.Y += 1.0f;
    }

    if (IsInputKeyDown(EKeys::S))
    {
        CurrentAxis.Y -= 1.0f;
    }

    if (IsInputKeyDown(EKeys::D))
    {
        CurrentAxis.X += 1.0f;
    }

    if (IsInputKeyDown(EKeys::A))
    {
        CurrentAxis.X -= 1.0f;
    }

    // ----- ゲームパッド十字キー -----

    if (IsInputKeyDown(EKeys::Gamepad_DPad_Up))
    {
        CurrentAxis.Y += 1.0f;
    }

    if (IsInputKeyDown(EKeys::Gamepad_DPad_Down))
    {
        CurrentAxis.Y -= 1.0f;
    }

    if (IsInputKeyDown(EKeys::Gamepad_DPad_Right))
    {
        CurrentAxis.X += 1.0f;
    }

    if (IsInputKeyDown(EKeys::Gamepad_DPad_Left))
    {
        CurrentAxis.X -= 1.0f;
    }

    // ----- ゲームパッドLスティック -----

    const float LeftStickX = GetInputAnalogKeyState(EKeys::Gamepad_LeftX);
    const float LeftStickY = GetInputAnalogKeyState(EKeys::Gamepad_LeftY);

    // アナログ入力があれば、そちらを優先して使う
    if (!FMath::IsNearlyZero(LeftStickX, KINDA_SMALL_NUMBER) ||
        !FMath::IsNearlyZero(LeftStickY, KINDA_SMALL_NUMBER))
    {
        CurrentAxis = FVector2D(LeftStickX, LeftStickY);
    }

    FIntPoint CurrentDirection;
    if (!TryConvertMoveAxisToGridDir(CurrentAxis, CurrentDirection))
    {
        return false;
    }

    return CurrentDirection == RepeatingMoveDirection;
}

// ----- 内部処理 -----

/**
 * 入力設定DataAssetを同期ロードしRouterへ適用する
 * Bind前にIA参照を確定させるために使用する
 */
void ARldPlayerController::LoadAndApplyInputConfig()
{
    LoadedInputConfig = nullptr;

    if (InputConfigAsset.IsNull())
    {
        UE_LOG(LogRldInput, Error, TEXT("InputConfigAsset is null"));

        if (InputRouter)
        {
            InputRouter->ApplyConfig(nullptr);
        }

        return;
    }

    LoadedInputConfig = InputConfigAsset.LoadSynchronous();

    UE_LOG(
        LogRldInput,
        Log,
        TEXT("InputConfig loaded: %s"),
        LoadedInputConfig ? *LoadedInputConfig->GetName() : TEXT("None")
    );

    if (InputRouter)
    {
        InputRouter->ApplyConfig(LoadedInputConfig);

        UE_LOG(
            LogRldInput,
            Log,
            TEXT("After ApplyConfig: Move=%s Look=%s Zoom=%s UIDir=%s UIScroll=%s"),
            InputRouter->IA_Move ? TEXT("OK") : TEXT("None"),
            InputRouter->IA_CameraLook ? TEXT("OK") : TEXT("None"),
            InputRouter->IA_CameraZoom ? TEXT("OK") : TEXT("None"),
            InputRouter->IA_UI_Direction ? TEXT("OK") : TEXT("None"),
            InputRouter->IA_UI_Scroll ? TEXT("OK") : TEXT("None")
        );
    }
}

/**
 * EnhancedInputComponentでない場合に差し替える
 * InputComponentClass依存を排除するための安全措置
 */
void ARldPlayerController::EnsureEnhancedInputComponent()
{
    if (Cast<UEnhancedInputComponent>(InputComponent))
    {
        return;
    }

    if (InputComponent)
    {
        PopInputComponent(InputComponent);
        InputComponent = nullptr;
    }

    UEnhancedInputComponent* NewInputComponent = NewObject<UEnhancedInputComponent>(
        this,
        UEnhancedInputComponent::StaticClass(),
        NAME_None,
        RF_Transient
    );

    PushInputComponent(NewInputComponent);
    InputComponent = NewInputComponent;
}
