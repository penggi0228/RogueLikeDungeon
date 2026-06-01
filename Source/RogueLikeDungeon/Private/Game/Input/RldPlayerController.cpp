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
#include "Game/UI/RldDebugOptionsWidget.h"

DEFINE_LOG_CATEGORY_STATIC(LogRldInput, Log, All);

/** PlayerControllerを初期化する */
ARldPlayerController::ARldPlayerController()
{
    // 入力ルーター生成
    InputRouter = CreateDefaultSubobject<UCmnInputRouter>(TEXT("InputRouter"));

    // 入力設定DataAsset参照を設定
    InputConfigAsset = TSoftObjectPtr<UCmnInputConfig>(
        FSoftObjectPath(TEXT("/Game/Game/Input/DataAssets/DA_CmnInputConfig_Default.DA_CmnInputConfig_Default"))
    );

    // デバッグオプションWidgetクラス参照を設定
    DebugOptionsWidgetClass = nullptr;
}

// ----- AActor -----

/** 入力設定をロードして入力ルーターへ適用する */
void ARldPlayerController::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    LoadAndApplyInputConfig();
}

/** プレイ開始時処理 */
void ARldPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // 入力設定未ロード時は再ロード
    if (!LoadedInputConfig)
    {
        LoadAndApplyInputConfig();
    }

    // GameInstance取得時はSettingsSubsystem参照を取得
    if (GetGameInstance())
    {
        SettingsSubsystem = GetGameInstance()->GetSubsystem<UCmnSettingsSubsystem>();
    }

    // SettingsSubsystem取得時は変更通知へ登録
    if (SettingsSubsystem)
    {
        SettingsSubsystem->GetOnInputSettingsChanged().AddUObject(
            this,
            &ARldPlayerController::HandleInputSettingsChanged
        );

        // 現在の設定値をローカルキャッシュへ反映
        HandleInputSettingsChanged(SettingsSubsystem->GetCurrentSettings());
    }

    ULocalPlayer* LocalPlayer = GetLocalPlayer();
    UEnhancedInputLocalPlayerSubsystem* Subsystem =
        LocalPlayer ? LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>() : nullptr;

    // 入力ルーターまたはEnhancedInputSubsystem未取得時は初期化しない
    if (!ensure(InputRouter && Subsystem))
    {
        return;
    }

    // 入力ルーターを初期化してゲームモードを適用
    InputRouter->Initialize(this, Subsystem);
    InputRouter->SetInputMode(ECmnInputMode::Game);
}

/** 終了時処理 */
void ARldPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopMoveRepeat();

    // SettingsSubsystem取得時は変更通知を解除
    if (SettingsSubsystem)
    {
        SettingsSubsystem->GetOnInputSettingsChanged().RemoveAll(this);
    }

    Super::EndPlay(EndPlayReason);
}

// ----- AController -----

/** 入力ActionをBindする */
void ARldPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    EnsureEnhancedInputComponent();

    UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent);

    // EnhancedInputComponentまたは入力ルーター未取得時はBindしない
    if (!ensure(EIC && InputRouter))
    {
        UE_LOG(LogRldInput, Error, TEXT("SetupInputComponent: EnhancedInputComponentまたはInputRouterがnull"));
        return;
    }

    UE_LOG(LogRldInput, Log, TEXT("SetupInputComponent: 入力Bind開始"));

    // ----- ゲーム操作 -----

    // 移動InputAction取得時はBind
    if (InputRouter->IA_Move)
    {
        UE_LOG(LogRldInput, Log, TEXT("SetupInputComponent: IA_MoveをBindします"));

        EIC->BindAction(InputRouter->IA_Move, ETriggerEvent::Started, this, &ARldPlayerController::OnMoveStarted);
        EIC->BindAction(InputRouter->IA_Move, ETriggerEvent::Triggered, this, &ARldPlayerController::OnMoveTriggered);
    }
    else
    {
        UE_LOG(LogRldInput, Warning, TEXT("SetupInputComponent: IA_Moveがnull"));
    }

    // 待機InputAction取得時はBind
    if (InputRouter->IA_Wait)
    {
        UE_LOG(LogRldInput, Log, TEXT("SetupInputComponent: IA_WaitをBindします"));
        EIC->BindAction(InputRouter->IA_Wait, ETriggerEvent::Started, this, &ARldPlayerController::OnWaitStarted);
    }
    else
    {
        UE_LOG(LogRldInput, Warning, TEXT("SetupInputComponent: IA_Waitがnull"));
    }

    // 通常攻撃InputAction取得時はBind
    if (InputRouter->IA_Attack)
    {
        UE_LOG(LogRldInput, Log, TEXT("SetupInputComponent: IA_AttackをBindします"));
        EIC->BindAction(InputRouter->IA_Attack, ETriggerEvent::Started, this, &ARldPlayerController::OnAttackStarted);
    }
    else
    {
        UE_LOG(LogRldInput, Warning, TEXT("SetupInputComponent: IA_Attackがnull"));
    }

    // インタラクトInputAction取得時はBind
    if (InputRouter->IA_Interact)
    {
        UE_LOG(LogRldInput, Log, TEXT("SetupInputComponent: IA_InteractをBindします"));
        EIC->BindAction(InputRouter->IA_Interact, ETriggerEvent::Started, this, &ARldPlayerController::OnInteractStarted);
    }
    else
    {
        UE_LOG(LogRldInput, Warning, TEXT("SetupInputComponent: IA_Interactがnull"));
    }

    // メニューInputAction取得時はBind
    if (InputRouter->IA_Menu)
    {
        UE_LOG(LogRldInput, Log, TEXT("SetupInputComponent: IA_MenuをBindします"));
        EIC->BindAction(InputRouter->IA_Menu, ETriggerEvent::Started, this, &ARldPlayerController::OnMenuStarted);
    }
    else
    {
        UE_LOG(LogRldInput, Warning, TEXT("SetupInputComponent: IA_Menuがnull"));
    }

    // カメラ視点InputAction取得時はBind
    if (InputRouter->IA_CameraLook)
    {
        UE_LOG(LogRldInput, Log, TEXT("SetupInputComponent: IA_CameraLookをBindします"));
        EIC->BindAction(InputRouter->IA_CameraLook, ETriggerEvent::Triggered, this, &ARldPlayerController::OnCameraLookTriggered);
    }
    else
    {
        UE_LOG(LogRldInput, Warning, TEXT("SetupInputComponent: IA_CameraLookがnull"));
    }

    // カメラズームInputAction取得時はBind
    if (InputRouter->IA_CameraZoom)
    {
        UE_LOG(LogRldInput, Log, TEXT("SetupInputComponent: IA_CameraZoomをBindします"));
        EIC->BindAction(InputRouter->IA_CameraZoom, ETriggerEvent::Triggered, this, &ARldPlayerController::OnCameraZoomTriggered);
    }
    else
    {
        UE_LOG(LogRldInput, Warning, TEXT("SetupInputComponent: IA_CameraZoomがnull"));
    }

    // ----- UI操作 -----

    // UI方向入力InputAction取得時はBind
    if (InputRouter->IA_UI_Direction)
    {
        UE_LOG(LogRldInput, Log, TEXT("SetupInputComponent: IA_UI_DirectionをBindします"));
        EIC->BindAction(InputRouter->IA_UI_Direction, ETriggerEvent::Triggered, this, &ARldPlayerController::OnUIDirectionTriggered);
    }
    else
    {
        UE_LOG(LogRldInput, Warning, TEXT("SetupInputComponent: IA_UI_Directionがnull"));
    }

    // UI決定InputAction取得時はBind
    if (InputRouter->IA_UI_Confirm)
    {
        UE_LOG(LogRldInput, Log, TEXT("SetupInputComponent: IA_UI_ConfirmをBindします"));
        EIC->BindAction(InputRouter->IA_UI_Confirm, ETriggerEvent::Started, this, &ARldPlayerController::OnUIConfirmStarted);
    }
    else
    {
        UE_LOG(LogRldInput, Warning, TEXT("SetupInputComponent: IA_UI_Confirmがnull"));
    }

    // UI閉じるInputAction取得時はBind
    if (InputRouter->IA_UI_Close)
    {
        UE_LOG(LogRldInput, Log, TEXT("SetupInputComponent: IA_UI_CloseをBindします"));
        EIC->BindAction(InputRouter->IA_UI_Close, ETriggerEvent::Started, this, &ARldPlayerController::OnUICloseStarted);
    }
    else
    {
        UE_LOG(LogRldInput, Warning, TEXT("SetupInputComponent: IA_UI_Closeがnull"));
    }

    // UIスクロールInputAction取得時はBind
    if (InputRouter->IA_UI_Scroll)
    {
        UE_LOG(LogRldInput, Log, TEXT("SetupInputComponent: IA_UI_ScrollをBindします"));
        EIC->BindAction(InputRouter->IA_UI_Scroll, ETriggerEvent::Triggered, this, &ARldPlayerController::OnUIScrollTriggered);
    }
    else
    {
        UE_LOG(LogRldInput, Warning, TEXT("SetupInputComponent: IA_UI_Scrollがnull"));
    }

    // ----- デバッグコマンド -----

    // デバッグコマンド開始InputAction取得時はBind
    if (InputRouter->IA_DebugCommandPrefix)
    {
        UE_LOG(LogRldInput, Log, TEXT("SetupInputComponent: IA_DebugCommandPrefixをBindします"));
        EIC->BindAction(InputRouter->IA_DebugCommandPrefix, ETriggerEvent::Started, this, &ARldPlayerController::OnDebugCommandPrefixStarted);
    }
    else
    {
        UE_LOG(LogRldInput, Warning, TEXT("SetupInputComponent: IA_DebugCommandPrefixがnull"));
    }

    // キーボード用デバッグコマンド1InputAction取得時はBind
    if (InputRouter->IA_DebugCommandKeyboard1)
    {
        UE_LOG(LogRldInput, Log, TEXT("SetupInputComponent: IA_DebugCommandKeyboard1をBindします"));
        EIC->BindAction(InputRouter->IA_DebugCommandKeyboard1, ETriggerEvent::Started, this, &ARldPlayerController::OnDebugCommandKeyboard1Started);
    }
    else
    {
        UE_LOG(LogRldInput, Warning, TEXT("SetupInputComponent: IA_DebugCommandKeyboard1がnull"));
    }

    // キーボード用デバッグコマンド2InputAction取得時はBind
    if (InputRouter->IA_DebugCommandKeyboard2)
    {
        UE_LOG(LogRldInput, Log, TEXT("SetupInputComponent: IA_DebugCommandKeyboard2をBindします"));
        EIC->BindAction(InputRouter->IA_DebugCommandKeyboard2, ETriggerEvent::Started, this, &ARldPlayerController::OnDebugCommandKeyboard2Started);
    }
    else
    {
        UE_LOG(LogRldInput, Warning, TEXT("SetupInputComponent: IA_DebugCommandKeyboard2がnull"));
    }

    // キーボード用デバッグコマンド3InputAction取得時はBind
    if (InputRouter->IA_DebugCommandKeyboard3)
    {
        UE_LOG(LogRldInput, Log, TEXT("SetupInputComponent: IA_DebugCommandKeyboard3をBindします"));
        EIC->BindAction(InputRouter->IA_DebugCommandKeyboard3, ETriggerEvent::Started, this, &ARldPlayerController::OnDebugCommandKeyboard3Started);
    }
    else
    {
        UE_LOG(LogRldInput, Warning, TEXT("SetupInputComponent: IA_DebugCommandKeyboard3がnull"));
    }

    // キーボード用デバッグコマンド4InputAction取得時はBind
    if (InputRouter->IA_DebugCommandKeyboard4)
    {
        UE_LOG(LogRldInput, Log, TEXT("SetupInputComponent: IA_DebugCommandKeyboard4をBindします"));
        EIC->BindAction(InputRouter->IA_DebugCommandKeyboard4, ETriggerEvent::Started, this, &ARldPlayerController::OnDebugCommandKeyboard4Started);
    }
    else
    {
        UE_LOG(LogRldInput, Warning, TEXT("SetupInputComponent: IA_DebugCommandKeyboard4がnull"));
    }

    // キーボード用デバッグコマンド5InputAction取得時はBind
    if (InputRouter->IA_DebugCommandKeyboard5)
    {
        UE_LOG(LogRldInput, Log, TEXT("SetupInputComponent: IA_DebugCommandKeyboard5をBindします"));
        EIC->BindAction(InputRouter->IA_DebugCommandKeyboard5, ETriggerEvent::Started, this, &ARldPlayerController::OnDebugCommandKeyboard5Started);
    }
    else
    {
        UE_LOG(LogRldInput, Warning, TEXT("SetupInputComponent: IA_DebugCommandKeyboard5がnull"));
    }

    // ゲームパッド用デバッグコマンド1InputAction取得時はBind
    if (InputRouter->IA_DebugCommandGamepad1)
    {
        UE_LOG(LogRldInput, Log, TEXT("SetupInputComponent: IA_DebugCommandGamepad1をBindします"));
        EIC->BindAction(InputRouter->IA_DebugCommandGamepad1, ETriggerEvent::Started, this, &ARldPlayerController::OnDebugCommandGamepad1Started);
    }
    else
    {
        UE_LOG(LogRldInput, Warning, TEXT("SetupInputComponent: IA_DebugCommandGamepad1がnull"));
    }

    // ゲームパッド用デバッグコマンド2InputAction取得時はBind
    if (InputRouter->IA_DebugCommandGamepad2)
    {
        UE_LOG(LogRldInput, Log, TEXT("SetupInputComponent: IA_DebugCommandGamepad2をBindします"));
        EIC->BindAction(InputRouter->IA_DebugCommandGamepad2, ETriggerEvent::Started, this, &ARldPlayerController::OnDebugCommandGamepad2Started);
    }
    else
    {
        UE_LOG(LogRldInput, Warning, TEXT("SetupInputComponent: IA_DebugCommandGamepad2がnull"));
    }
}

// ----- 公開API -----

/** 共通入力モードを切り替える */
void ARldPlayerController::SetCommonInputMode(ECmnInputMode Mode)
{
    if (InputRouter)
    {
        InputRouter->SetInputMode(Mode);
    }
}

/** 入力ルーターを取得する */
UCmnInputRouter* ARldPlayerController::GetInputRouter() const
{
    return InputRouter;
}

// ----- 設定通知 -----

/** 入力設定変更通知を受け取る */
void ARldPlayerController::HandleInputSettingsChanged(const FInputRuntimeSettings& NewSettings)
{
    bInvertCameraX = NewSettings.bInvertCameraX;
    bInvertCameraY = NewSettings.bInvertCameraY;

    UE_LOG(
        LogRldInput,
        Log,
        TEXT("HandleInputSettingsChanged: 左右反転=%d 上下反転=%d"),
        bInvertCameraX ? 1 : 0,
        bInvertCameraY ? 1 : 0
    );
}

// ----- ゲーム操作の入力 -----

/** 移動入力開始時の処理を行う */
void ARldPlayerController::OnMoveStarted(const FInputActionValue& Value)
{
    // ゲームモード以外では移動処理しない
    if (!InputRouter || !InputRouter->IsGameMode())
    {
        UE_LOG(LogRldInput, Verbose, TEXT("OnMoveStarted: ゲームモード以外のため処理しない"));
        return;
    }

    // デバッグコマンド入力受付中は通常ゲーム入力を受け付けない
    if (IsHiddenDebugCommandInputActive())
    {
        UE_LOG(LogRldInput, Verbose, TEXT("OnMoveStarted: デバッグコマンド入力受付中のため処理しない"));
        return;
    }

    const FVector2D Axis = Value.Get<FVector2D>();

    // 左スティック入力はTriggered側で処理
    if (IsLikelyLeftStickInput(Axis))
    {
        return;
    }

    const FString InputSourceText = BuildMoveInputSourceDebugText();

    UE_LOG(
        LogRldInput,
        Log,
        TEXT("OnMoveStarted: 入力元=%s 生入力=(%f,%f)"),
        *InputSourceText,
        Axis.X,
        Axis.Y
    );

    FIntPoint Direction;

    // 方向確定失敗時は移動処理しない
    if (!TryConvertMoveAxisToGridDir(Axis, Direction))
    {
        UE_LOG(
            LogRldInput,
            Warning,
            TEXT("OnMoveStarted: 方向変換に失敗しました 入力元=%s 生入力=(%f,%f)"),
            *InputSourceText,
            Axis.X,
            Axis.Y
        );
        return;
    }

    ProcessResolvedMoveDirection(Direction, InputSourceText, Axis);
}

/** 移動入力の生値ログを出力する */
void ARldPlayerController::OnMoveTriggered(const FInputActionValue& Value)
{
    const FVector2D Axis = Value.Get<FVector2D>();
    const FString InputSourceText = BuildMoveInputSourceDebugText();

    UE_LOG(
        LogRldInput,
        Verbose,
        TEXT("OnMoveTriggered: 入力元=%s 生入力=(%f,%f)"),
        *InputSourceText,
        Axis.X,
        Axis.Y
    );

    // デバッグコマンド入力受付中は通常ゲーム入力を受け付けない
    if (IsHiddenDebugCommandInputActive())
    {
        UE_LOG(LogRldInput, Verbose, TEXT("OnMoveTriggered: デバッグコマンド入力受付中のため処理しない"));
        return;
    }

    // 左スティック入力のみTriggered側で処理
    if (IsLikelyLeftStickInput(Axis))
    {
        HandleLeftStickMoveTriggered(Axis);
    }
}

/** 待機入力を処理する */
void ARldPlayerController::OnWaitStarted(const FInputActionValue& Value)
{
    // 未使用引数
    (void)Value;

    // ゲームモード以外では待機処理しない
    if (!InputRouter || !InputRouter->IsGameMode())
    {
        UE_LOG(LogRldInput, Verbose, TEXT("OnWaitStarted: ゲームモード以外のため処理しない"));
        return;
    }

    // デバッグコマンド入力受付中は通常ゲーム入力を受け付けない
    if (IsHiddenDebugCommandInputActive())
    {
        UE_LOG(LogRldInput, Verbose, TEXT("OnWaitStarted: デバッグコマンド入力受付中のため処理しない"));
        return;
    }

    ARldPlayerCharacter* PlayerCharacter = GetRldPlayerCharacter();

    // プレイヤーキャラクター未取得時は待機しない
    if (!PlayerCharacter)
    {
        UE_LOG(LogRldInput, Warning, TEXT("OnWaitStarted: PlayerCharacterがnull"));
        return;
    }

    // 待機入力時は押しっぱなし移動を停止
    StopMoveRepeat();

    UE_LOG(
        LogRldInput,
        Log,
        TEXT("OnWaitStarted: 待機入力を受け付けました")
    );

    PlayerCharacter->RequestWaitAction();
}

/** 通常攻撃入力を処理する */
void ARldPlayerController::OnAttackStarted(const FInputActionValue& Value)
{
    // 未使用引数
    (void)Value;

    // ゲームモード以外では通常攻撃処理しない
    if (!InputRouter || !InputRouter->IsGameMode())
    {
        UE_LOG(LogRldInput, Verbose, TEXT("OnAttackStarted: ゲームモード以外のため処理しない"));
        return;
    }

    // デバッグコマンド入力受付中は通常ゲーム入力を受け付けない
    if (IsHiddenDebugCommandInputActive())
    {
        UE_LOG(LogRldInput, Verbose, TEXT("OnAttackStarted: デバッグコマンド入力受付中のため処理しない"));
        return;
    }

    ARldPlayerCharacter* PlayerCharacter = GetRldPlayerCharacter();

    // プレイヤーキャラクター未取得時は処理しない
    if (!PlayerCharacter)
    {
        UE_LOG(LogRldInput, Warning, TEXT("OnAttackStarted: PlayerCharacterがnull"));
        return;
    }

    StopMoveRepeat();

    UE_LOG(
        LogRldInput,
        Log,
        TEXT("OnAttackStarted: 通常攻撃入力を受け付けました")
    );

    PlayerCharacter->RequestAttackAction();
}

/** インタラクト入力を処理する */
void ARldPlayerController::OnInteractStarted(const FInputActionValue& Value)
{
    // 未使用引数
    (void)Value;

    // ゲームモード以外ではインタラクト処理しない
    if (!InputRouter || !InputRouter->IsGameMode())
    {
        UE_LOG(LogRldInput, Verbose, TEXT("OnInteractStarted: ゲームモード以外のため処理しない"));
        return;
    }

    // デバッグコマンド入力受付中は通常ゲーム入力を受け付けない
    if (IsHiddenDebugCommandInputActive())
    {
        UE_LOG(LogRldInput, Verbose, TEXT("OnInteractStarted: デバッグコマンド入力受付中のため処理しない"));
        return;
    }

    ARldPlayerCharacter* PlayerCharacter = GetRldPlayerCharacter();

    // プレイヤーキャラクター未取得時は処理しない
    if (!PlayerCharacter)
    {
        UE_LOG(LogRldInput, Warning, TEXT("OnInteractStarted: PlayerCharacterがnull"));
        return;
    }

    StopMoveRepeat();

    UE_LOG(
        LogRldInput,
        Log,
        TEXT("OnInteractStarted: インタラクト入力を受け付けました")
    );

    PlayerCharacter->RequestInteractAction();
}

/** メニュー入力を処理する */
void ARldPlayerController::OnMenuStarted(const FInputActionValue& Value)
{
    // 未使用引数
    (void)Value;

    // ゲームモード以外ではメニュー処理しない
    if (!InputRouter || !InputRouter->IsGameMode())
    {
        UE_LOG(LogRldInput, Verbose, TEXT("OnMenuStarted: ゲームモード以外のため処理しない"));
        return;
    }

    // デバッグコマンド入力受付中は通常ゲーム入力を受け付けない
    if (IsHiddenDebugCommandInputActive())
    {
        UE_LOG(LogRldInput, Verbose, TEXT("OnMenuStarted: デバッグコマンド入力受付中のため処理しない"));
        return;
    }

    StopMoveRepeat();

    UE_LOG(
        LogRldInput,
        Log,
        TEXT("OnMenuStarted: メニュー入力を受け付けました")
    );

    // TODO: メインメニューWidgetを表示する
}

/** カメラ視点入力を処理する */
void ARldPlayerController::OnCameraLookTriggered(const FInputActionValue& Value)
{
    const FVector2D RawAxis = Value.Get<FVector2D>();

    UE_LOG(
        LogRldInput,
        Log,
        TEXT("OnCameraLookTriggered: 生入力=(%f,%f)"),
        RawAxis.X,
        RawAxis.Y
    );

    // ゲームモード以外ではカメラ視点処理しない
    if (!InputRouter || !InputRouter->IsGameMode())
    {
        UE_LOG(LogRldInput, Verbose, TEXT("OnCameraLookTriggered: ゲームモード以外のため処理しない"));
        return;
    }

    // デバッグコマンド入力受付中は通常ゲーム入力を受け付けない
    if (IsHiddenDebugCommandInputActive())
    {
        UE_LOG(LogRldInput, Verbose, TEXT("OnCameraLookTriggered: デバッグコマンド入力受付中のため処理しない"));
        return;
    }

    FVector2D Axis = RawAxis;

    // 反転設定を適用
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
        TEXT("OnCameraLookTriggered: 反転適用後入力=(%f,%f)"),
        Axis.X,
        Axis.Y
    );

    ARldPlayerCharacter* PlayerCharacter = GetRldPlayerCharacter();

    // プレイヤーキャラクター未取得時は視点入力しない
    if (!PlayerCharacter)
    {
        UE_LOG(LogRldInput, Warning, TEXT("OnCameraLookTriggered: PlayerCharacterがnull"));
        return;
    }

    // 視点入力受付不可時は処理しない
    if (!PlayerCharacter->CanAcceptLookInput())
    {
        UE_LOG(LogRldInput, Verbose, TEXT("OnCameraLookTriggered: 視点入力を受け付けないため処理しない"));
        return;
    }

    PlayerCharacter->RequestLookInput(Axis);
}

/** カメラズーム入力を処理する */
void ARldPlayerController::OnCameraZoomTriggered(const FInputActionValue& Value)
{
    const float ZoomValue = Value.Get<float>();

    UE_LOG(
        LogRldInput,
        Log,
        TEXT("OnCameraZoomTriggered: 生入力=%f"),
        ZoomValue
    );

    // ゲームモード以外ではカメラズーム処理しない
    if (!InputRouter || !InputRouter->IsGameMode())
    {
        UE_LOG(LogRldInput, Verbose, TEXT("OnCameraZoomTriggered: ゲームモード以外のため処理しない"));
        return;
    }

    // デバッグコマンド入力受付中は通常ゲーム入力を受け付けない
    if (IsHiddenDebugCommandInputActive())
    {
        UE_LOG(LogRldInput, Verbose, TEXT("OnCameraZoomTriggered: デバッグコマンド入力受付中のため処理しない"));
        return;
    }

    ARldPlayerCharacter* PlayerCharacter = GetRldPlayerCharacter();

    // プレイヤーキャラクター未取得時はズーム入力しない
    if (!PlayerCharacter)
    {
        UE_LOG(LogRldInput, Warning, TEXT("OnCameraZoomTriggered: PlayerCharacterがnull"));
        return;
    }

    // ズーム入力受付不可時は処理しない
    if (!PlayerCharacter->CanAcceptZoomInput())
    {
        UE_LOG(LogRldInput, Verbose, TEXT("OnCameraZoomTriggered: ズーム入力を受け付けないため処理しない"));
        return;
    }

    PlayerCharacter->RequestZoomInput(ZoomValue);
}

// ----- UI操作の入力 -----

/** UI方向入力を処理する */
void ARldPlayerController::OnUIDirectionTriggered(const FInputActionValue& Value)
{
    // メニューまたはダイアログ以外ではUI方向入力を処理しない
    if (!InputRouter || (!InputRouter->IsMenuMode() && !InputRouter->IsDialogMode()))
    {
        return;
    }

    const FVector2D Axis = Value.Get<FVector2D>();

    // 表示中のデバッグオプションがある場合は専用フォーカス移動を行う
    if (URldDebugOptionsWidget* DebugOptionsWidget = GetActiveDebugOptionsWidget())
    {
        FIntPoint Direction;

        if (InputRouter->GetMoveDirFromAxis(Axis, Direction))
        {
            if (Direction.Y > 0)
            {
                DebugOptionsWidget->MoveDebugFocus(-1);
            }
            else if (Direction.Y < 0)
            {
                DebugOptionsWidget->MoveDebugFocus(1);
            }
        }

        return;
    }

    InputRouter->HandleUIDirectionAxis(Axis);
}

/** UI決定入力を処理する */
void ARldPlayerController::OnUIConfirmStarted(const FInputActionValue& Value)
{
    // 未使用引数
    (void)Value;

    // メニューまたはダイアログ以外ではUI決定入力を処理しない
    if (!InputRouter || (!InputRouter->IsMenuMode() && !InputRouter->IsDialogMode()))
    {
        return;
    }

    if (URldDebugOptionsWidget* DebugOptionsWidget = GetActiveDebugOptionsWidget())
    {
        DebugOptionsWidget->ConfirmFocusedDebugItem();
    }
}

/** UIクローズ入力を処理する */
void ARldPlayerController::OnUICloseStarted(const FInputActionValue& Value)
{
    // 未使用引数
    (void)Value;

    // メニューまたはダイアログ以外ではUIクローズ入力を処理しない
    if (!InputRouter || (!InputRouter->IsMenuMode() && !InputRouter->IsDialogMode()))
    {
        return;
    }

    if (URldDebugOptionsWidget* DebugOptionsWidget = GetActiveDebugOptionsWidget())
    {
        DebugOptionsWidget->CloseDebugOptions();
        SetCommonInputMode(ECmnInputMode::Game);
    }
}

/** UIスクロール入力を処理する */
void ARldPlayerController::OnUIScrollTriggered(const FInputActionValue& Value)
{
    // メニューまたはダイアログ以外ではUIスクロール入力を処理しない
    if (!InputRouter || (!InputRouter->IsMenuMode() && !InputRouter->IsDialogMode()))
    {
        return;
    }

    const float Amount = Value.Get<float>();
    InputRouter->HandleUIScrollAxis(Amount);
}

// ----- デバッグUI入力 -----

/** デバッグコマンド開始入力を処理する */
void ARldPlayerController::OnDebugCommandPrefixStarted(const FInputActionValue& Value)
{
    // 未使用引数
    (void)Value;

    // ゲームモード以外ではデバッグUI表示コマンドを受け付けない
    if (!InputRouter || !InputRouter->IsGameMode())
    {
        return;
    }

    UWorld* World = GetWorld();

    if (!World)
    {
        return;
    }

    const double CurrentTime = World->GetTimeSeconds();

    // 入力間隔が空いた場合はコマンド状態を初期化
    if ((CurrentTime - lastHiddenDebugInputTime) > hiddenDebugCommandTimeout)
    {
        ResetDebugCommandState();
    }

    lastHiddenDebugInputTime = CurrentTime;
    ++debugCommandPrefixCount;
    debugCommandIndex = 0;
    debugCommandInputType = ERldDebugCommandInputType::None;

    UE_LOG(
        LogRldInput,
        Verbose,
        TEXT("OnDebugCommandPrefixStarted: デバッグコマンド開始入力回数=%d"),
        debugCommandPrefixCount
    );
}

/** キーボード用デバッグコマンド第1入力を処理する */
void ARldPlayerController::OnDebugCommandKeyboard1Started(const FInputActionValue& Value)
{
    (void)Value;
    HandleKeyboardDebugCommandCharacter(0);
}

/** キーボード用デバッグコマンド第2入力を処理する */
void ARldPlayerController::OnDebugCommandKeyboard2Started(const FInputActionValue& Value)
{
    (void)Value;
    HandleKeyboardDebugCommandCharacter(1);
}

/** キーボード用デバッグコマンド第3入力を処理する */
void ARldPlayerController::OnDebugCommandKeyboard3Started(const FInputActionValue& Value)
{
    (void)Value;
    HandleKeyboardDebugCommandCharacter(2);
}

/** キーボード用デバッグコマンド第4入力を処理する */
void ARldPlayerController::OnDebugCommandKeyboard4Started(const FInputActionValue& Value)
{
    (void)Value;
    HandleKeyboardDebugCommandCharacter(3);
}

/** キーボード用デバッグコマンド第5入力を処理する */
void ARldPlayerController::OnDebugCommandKeyboard5Started(const FInputActionValue& Value)
{
    (void)Value;
    HandleKeyboardDebugCommandCharacter(4);
}

/** ゲームパッド用デバッグコマンド第1入力を処理する */
void ARldPlayerController::OnDebugCommandGamepad1Started(const FInputActionValue& Value)
{
    (void)Value;
    HandleGamepadDebugCommandInput(true);
}

/** ゲームパッド用デバッグコマンド第2入力を処理する */
void ARldPlayerController::OnDebugCommandGamepad2Started(const FInputActionValue& Value)
{
    (void)Value;
    HandleGamepadDebugCommandInput(false);
}

// ----- ゲーム固有入力変換 -----

/** 入力軸をカメラ基準のグリッド4方向へ変換する */
bool ARldPlayerController::TryConvertMoveAxisToGridDir(
    const FVector2D& Axis,
    FIntPoint& OutDirection
) const
{
    FVector WorldDirection;

    // ワールド平面方向に変換できない場合は失敗
    if (!TryConvertInputAxisToCameraRelativeWorldDir(Axis, WorldDirection))
    {
        return false;
    }

    return TryConvertWorldDirToGridDir(WorldDirection, OutDirection);
}

/** 入力軸をカメラ基準のワールド平面方向へ変換する */
bool ARldPlayerController::TryConvertInputAxisToCameraRelativeWorldDir(
    const FVector2D& Axis,
    FVector& OutWorldDirection
) const
{
    OutWorldDirection = FVector::ZeroVector;

    const float InputDeadZone = 0.5f;

    const float AbsX = FMath::Abs(Axis.X);
    const float AbsY = FMath::Abs(Axis.Y);

    // 入力しきい値未満なら無効
    if (AbsX < InputDeadZone && AbsY < InputDeadZone)
    {
        return false;
    }

    // プレイヤーキャラクター未取得時は変換失敗
    const ARldPlayerCharacter* PlayerCharacter = GetRldPlayerCharacter();
    if (!PlayerCharacter)
    {
        return false;
    }

    const FVector CameraForward = PlayerCharacter->GetCameraPlanarForward();
    const FVector CameraRight = PlayerCharacter->GetCameraPlanarRight();

    // 入力軸をワールド平面方向へ変換
    FVector MoveWorldDirection =
        (CameraForward * Axis.Y) +
        (CameraRight * Axis.X);

    MoveWorldDirection.Z = 0.0f;

    // 平面方向が無効な場合は変換失敗
    if (MoveWorldDirection.IsNearlyZero(KINDA_SMALL_NUMBER))
    {
        return false;
    }

    MoveWorldDirection.Normalize();
    OutWorldDirection = MoveWorldDirection;

    UE_LOG(
        LogRldInput,
        Verbose,
        TEXT("TryConvertInputAxisToCameraRelativeWorldDir: 入力=(%f,%f) ワールド方向=(%f,%f,%f)"),
        Axis.X,
        Axis.Y,
        OutWorldDirection.X,
        OutWorldDirection.Y,
        OutWorldDirection.Z
    );

    return true;
}

/** ワールド平面方向をグリッド4方向へ変換する */
bool ARldPlayerController::TryConvertWorldDirToGridDir(
    const FVector& WorldDirection,
    FIntPoint& OutGridDirection
) const
{
    OutGridDirection = FIntPoint::ZeroValue;

    const float AbsX = FMath::Abs(WorldDirection.X);
    const float AbsY = FMath::Abs(WorldDirection.Y);

    // ワールド平面方向が無効な場合は変換失敗
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
        TEXT("TryConvertWorldDirToGridDir: ワールド方向=(%f,%f) グリッド方向=(%d,%d)"),
        WorldDirection.X,
        WorldDirection.Y,
        OutGridDirection.X,
        OutGridDirection.Y
    );

    return true;
}

/** 確定した移動方向を処理する */
void ARldPlayerController::ProcessResolvedMoveDirection(
    const FIntPoint& Direction,
    const FString& InputSourceText,
    const FVector2D& Axis
)
{
    UE_LOG(
        LogRldInput,
        Log,
        TEXT("ProcessResolvedMoveDirection: 入力元=%s 確定方向=(%d,%d) 生入力=(%f,%f)"),
        *InputSourceText,
        Direction.X,
        Direction.Y,
        Axis.X,
        Axis.Y
    );

    StartMoveRepeat(Direction, InputSourceText, Axis);

    ARldPlayerCharacter* PlayerCharacter = GetRldPlayerCharacter();

    // プレイヤーキャラクター未取得時は移動処理しない
    if (!PlayerCharacter)
    {
        UE_LOG(LogRldInput, Warning, TEXT("ProcessResolvedMoveDirection: PlayerCharacterがnull"));
        return;
    }

    // 移動入力受付不可時は処理しない
    if (!PlayerCharacter->CanAcceptMoveInput())
    {
        UE_LOG(LogRldInput, Verbose, TEXT("ProcessResolvedMoveDirection: 移動入力を受け付けないため処理しない"));
        return;
    }

    PlayerCharacter->RequestMoveDirection(Direction);
}

// ----- 左スティック専用処理 -----

/** 左スティック入力かどうかを判定する */
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

    // 微小なアナログ入力も左スティック候補として扱う
    const bool bHasAnalogLikeAxis =
        !FMath::IsNearlyZero(Axis.X, KINDA_SMALL_NUMBER) ||
        !FMath::IsNearlyZero(Axis.Y, KINDA_SMALL_NUMBER);

    return bHasAnalogLikeAxis &&
        FMath::Abs(Axis.X) < 1.0f &&
        FMath::Abs(Axis.Y) < 1.0f;
}

/** 左スティックの移動入力を処理する */
void ARldPlayerController::HandleLeftStickMoveTriggered(const FVector2D& Axis)
{
    const float ReleaseThreshold = 0.25f;
    const float ConfirmThreshold = 0.5f;

    const float AbsX = FMath::Abs(Axis.X);
    const float AbsY = FMath::Abs(Axis.Y);

    // ニュートラル復帰時は再入力可能にする
    if (AbsX < ReleaseThreshold && AbsY < ReleaseThreshold)
    {
        if (bLeftStickMoveConsumed)
        {
            UE_LOG(LogRldInput, Verbose, TEXT("HandleLeftStickMoveTriggered: 左スティックがニュートラルへ戻りました"));
        }

        bLeftStickMoveConsumed = false;
        StopMoveRepeat();
        return;
    }

    // 入力確定済みならニュートラル復帰まで無視
    if (bLeftStickMoveConsumed)
    {
        return;
    }

    // 確定しきい値未満ならまだ処理しない
    if (AbsX < ConfirmThreshold && AbsY < ConfirmThreshold)
    {
        return;
    }

    FIntPoint Direction;

    // 方向確定失敗時は処理しない
    if (!TryConvertMoveAxisToGridDir(Axis, Direction))
    {
        UE_LOG(
            LogRldInput,
            Warning,
            TEXT("HandleLeftStickMoveTriggered: 方向変換に失敗しました 生入力=(%f,%f)"),
            Axis.X,
            Axis.Y
        );
        return;
    }

    bLeftStickMoveConsumed = true;

    ProcessResolvedMoveDirection(
        Direction,
        TEXT("ゲームパッドLスティック"),
        Axis
    );
}

// ----- キャラクター取得 -----

/** 現在操作中のプレイヤーキャラクターを取得する */
ARldPlayerCharacter* ARldPlayerController::GetRldPlayerCharacter() const
{
    return Cast<ARldPlayerCharacter>(GetPawn());
}

// ----- デバッグ補助 -----

/** 現在の移動入力元をデバッグ文字列で取得する */
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

    // 入力元未検出時はUnknown
    if (Sources.Num() == 0)
    {
        return TEXT("Unknown");
    }

    return FString::Join(Sources, TEXT(" / "));
}

// ----- 移動リピート制御 -----

/** 押しっぱなし移動のリピートを開始する */
void ARldPlayerController::StartMoveRepeat(
    const FIntPoint& Direction,
    const FString& InputSourceText,
    const FVector2D& Axis
)
{
    UWorld* World = GetWorld();

    // World未取得時は開始しない
    if (!World)
    {
        return;
    }

    RepeatingMoveDirection = Direction;
    RepeatingMoveInputSourceText = InputSourceText;
    RepeatingMoveAxis = Axis;
    bMoveRepeatActive = true;

    // 既存の移動リピートTimerを停止
    World->GetTimerManager().ClearTimer(MoveRepeatStartTimerHandle);
    World->GetTimerManager().ClearTimer(MoveRepeatTickTimerHandle);

    // 初回遅延Timerを開始
    World->GetTimerManager().SetTimer(
        MoveRepeatStartTimerHandle,
        this,
        &ARldPlayerController::BeginMoveRepeat,
        MoveRepeatInitialDelay,
        false
    );
}

/** 押しっぱなし移動のリピートを停止する */
void ARldPlayerController::StopMoveRepeat()
{
    UWorld* World = GetWorld();

    // World取得時は移動リピートTimerを停止
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

/** 押しっぱなし移動の初回遅延処理を行う */
void ARldPlayerController::BeginMoveRepeat()
{
    UWorld* World = GetWorld();

    // World未取得時は処理しない
    if (!World)
    {
        return;
    }

    // 継続不要時はリピート停止
    if (!ShouldContinueMoveRepeat())
    {
        StopMoveRepeat();
        return;
    }

    TickMoveRepeat();

    // 継続リピートTimerを開始
    World->GetTimerManager().SetTimer(
        MoveRepeatTickTimerHandle,
        this,
        &ARldPlayerController::TickMoveRepeat,
        MoveRepeatInterval,
        true
    );
}

/** 押しっぱなし移動の定期実行を行う */
void ARldPlayerController::TickMoveRepeat()
{
    // 継続不要時はリピート停止
    if (!ShouldContinueMoveRepeat())
    {
        StopMoveRepeat();
        return;
    }

    UE_LOG(
        LogRldInput,
        Log,
        TEXT("TickMoveRepeat: 入力元=%s リピート方向=(%d,%d)"),
        *RepeatingMoveInputSourceText,
        RepeatingMoveDirection.X,
        RepeatingMoveDirection.Y
    );

    ARldPlayerCharacter* PlayerCharacter = GetRldPlayerCharacter();

    // プレイヤーキャラクター未取得時は処理しない
    if (!PlayerCharacter)
    {
        UE_LOG(LogRldInput, Warning, TEXT("TickMoveRepeat: PlayerCharacterがnull"));
        StopMoveRepeat();
        return;
    }

    // 移動入力受付不可時は今回の処理のみしない
    if (!PlayerCharacter->CanAcceptMoveInput())
    {
        UE_LOG(LogRldInput, Verbose, TEXT("TickMoveRepeat: 移動入力を受け付けないため今回の処理をしない"));
        return;
    }

    PlayerCharacter->RequestMoveDirection(RepeatingMoveDirection);
}

/** 押しっぱなし移動を継続すべきか判定する */
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

    // アナログ入力がある場合は左スティック入力を優先
    if (!FMath::IsNearlyZero(LeftStickX, KINDA_SMALL_NUMBER) ||
        !FMath::IsNearlyZero(LeftStickY, KINDA_SMALL_NUMBER))
    {
        CurrentAxis = FVector2D(LeftStickX, LeftStickY);
    }

    FIntPoint CurrentDirection;

    // 入力から方向確定できない場合は継続しない
    if (!TryConvertMoveAxisToGridDir(CurrentAxis, CurrentDirection))
    {
        return false;
    }

    return CurrentDirection == RepeatingMoveDirection;
}

// ----- デバッグUI表示 -----

/** キーボード用デバッグコマンドの1入力分を処理する */
void ARldPlayerController::HandleKeyboardDebugCommandCharacter(int32 commandIndex)
{
    // ゲームモード以外ではデバッグUI表示コマンドを受け付けない
    if (!InputRouter || !InputRouter->IsGameMode())
    {
        return;
    }

    // 入力間隔が空いた場合はコマンド状態を初期化
    if (IsDebugCommandTimedOut())
    {
        ResetDebugCommandState();
        return;
    }

    UWorld* World = GetWorld();

    if (!World)
    {
        return;
    }

    lastHiddenDebugInputTime = World->GetTimeSeconds();

    // 開始入力が必要回数に満たない場合は文字入力を受け付けない
    if (debugCommandPrefixCount < debugCommandPrefixRequiredCount)
    {
        return;
    }

    // 入力方式未確定の場合はキーボード方式として確定
    if (debugCommandInputType == ERldDebugCommandInputType::None)
    {
        debugCommandInputType = ERldDebugCommandInputType::Keyboard;
    }

    // ゲームパッド方式の入力中にキーボード入力が来た場合は初期化
    if (debugCommandInputType != ERldDebugCommandInputType::Keyboard)
    {
        ResetDebugCommandState();
        return;
    }

    // 期待している文字順と異なる場合は初期化
    if (commandIndex != debugCommandIndex)
    {
        ResetDebugCommandState();
        return;
    }

    ++debugCommandIndex;

    // DEBUGの5文字分が揃ったらデバッグUIを表示
    if (debugCommandIndex >= 5)
    {
        ResetDebugCommandState();
        OpenDebugOptionsWidget();
    }
}

/** ゲームパッド用デバッグコマンドの1入力分を処理する */
void ARldPlayerController::HandleGamepadDebugCommandInput(bool bIsGamepad1Input)
{
    // ゲームモード以外ではデバッグUI表示コマンドを受け付けない
    if (!InputRouter || !InputRouter->IsGameMode())
    {
        return;
    }

    // 入力間隔が空いた場合はコマンド状態を初期化
    if (IsDebugCommandTimedOut())
    {
        ResetDebugCommandState();
        return;
    }

    UWorld* World = GetWorld();

    if (!World)
    {
        return;
    }

    lastHiddenDebugInputTime = World->GetTimeSeconds();

    // 開始入力が必要回数に満たない場合はゲームパッド用デバッグ入力を受け付けない
    if (debugCommandPrefixCount < debugCommandPrefixRequiredCount)
    {
        return;
    }

    // 入力方式未確定の場合はゲームパッド方式として確定
    if (debugCommandInputType == ERldDebugCommandInputType::None)
    {
        debugCommandInputType = ERldDebugCommandInputType::Gamepad;
    }

    // キーボード方式の入力中にゲームパッド入力が来た場合は初期化
    if (debugCommandInputType != ERldDebugCommandInputType::Gamepad)
    {
        ResetDebugCommandState();
        return;
    }

    // IA_DebugCommandGamepad1に割り当てたボタンはコマンド入力カウントが偶数の時に入力する
    const bool bExpectedGamepad1Input = (debugCommandIndex % 2) == 0;

    // 期待している交互順と異なる場合は初期化
    if (bIsGamepad1Input != bExpectedGamepad1Input)
    {
        ResetDebugCommandState();
        return;
    }

    ++debugCommandIndex;

    // Gamepad1→Gamepad2→Gamepad1→Gamepad2の4入力が揃ったらデバッグUIを表示
    if (debugCommandIndex >= 4)
    {
        ResetDebugCommandState();
        OpenDebugOptionsWidget();
    }
}

/** デバッグコマンド入力状態を初期化する */
void ARldPlayerController::ResetDebugCommandState()
{
    debugCommandPrefixCount = 0;
    debugCommandIndex = 0;
    debugCommandInputType = ERldDebugCommandInputType::None;
    lastHiddenDebugInputTime = 0.0;
}

/** デバッグコマンド入力受付中か判定する */
bool ARldPlayerController::IsHiddenDebugCommandInputActive() const
{
    // タイムアウト後は入力受付中でない扱いにする
    if (IsDebugCommandTimedOut())
    {
        return false;
    }

    return debugCommandPrefixCount >= debugCommandPrefixRequiredCount;
}

/** デバッグコマンド入力がタイムアウトしているか判定する */
bool ARldPlayerController::IsDebugCommandTimedOut() const
{
    if (debugCommandPrefixCount <= 0)
    {
        return false;
    }

    UWorld* World = GetWorld();

    if (!World)
    {
        return false;
    }

    return (World->GetTimeSeconds() - lastHiddenDebugInputTime) > hiddenDebugCommandTimeout;
}

/** デバッグオプションWidgetを表示する */
void ARldPlayerController::OpenDebugOptionsWidget()
{
    // すでに表示中の場合は再生成しない
    if (ActiveDebugOptionsWidget && ActiveDebugOptionsWidget->IsInViewport())
    {
        SetCommonInputMode(ECmnInputMode::Menu);
        return;
    }

    // デバッグオプションWidgetクラス未設定時は表示しない
    if (!DebugOptionsWidgetClass)
    {
        UE_LOG(
            LogRldInput,
            Warning,
            TEXT("OpenDebugOptionsWidget: DebugOptionsWidgetClass未設定のため表示できません")
        );
        return;
    }

    ActiveDebugOptionsWidget = CreateWidget<URldDebugOptionsWidget>(
        this,
        DebugOptionsWidgetClass
    );

    // デバッグオプションWidgetが有効でない場合は表示しない
    if (!ActiveDebugOptionsWidget)
    {
        UE_LOG(
            LogRldInput,
            Warning,
            TEXT("OpenDebugOptionsWidget: DebugOptionsWidget生成に失敗しました")
        );
        return;
    }

    ActiveDebugOptionsWidget->AddToViewport(100);
    SetCommonInputMode(ECmnInputMode::Menu);

    UE_LOG(LogRldInput, Log, TEXT("OpenDebugOptionsWidget: デバッグオプションを表示しました"));
}

/** 表示中のデバッグオプションWidgetを取得する */
URldDebugOptionsWidget* ARldPlayerController::GetActiveDebugOptionsWidget() const
{
    if (!ActiveDebugOptionsWidget || !ActiveDebugOptionsWidget->IsInViewport())
    {
        return nullptr;
    }

    return ActiveDebugOptionsWidget;
}

// ----- 内部処理 -----

/** 入力設定をロードして入力ルーターへ適用する */
void ARldPlayerController::LoadAndApplyInputConfig()
{
    LoadedInputConfig = nullptr;

    // 入力設定DataAsset未指定時はルーターへnull適用
    if (InputConfigAsset.IsNull())
    {
        UE_LOG(LogRldInput, Error, TEXT("LoadAndApplyInputConfig: InputConfigAssetがnull"));

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
        TEXT("LoadAndApplyInputConfig: 入力設定をロードしました 名前=%s"),
        LoadedInputConfig ? *LoadedInputConfig->GetName() : TEXT("None")
    );

    // 入力ルーター取得時は入力設定を適用
    if (InputRouter)
    {
        InputRouter->ApplyConfig(LoadedInputConfig);

        UE_LOG(
            LogRldInput,
            Log,
            TEXT("LoadAndApplyInputConfig: 入力設定を適用しました 移動=%s 待機=%s 攻撃=%s インタラクト=%s メニュー=%s 視点=%s ズーム=%s UI方向=%s UI決定=%s UI閉じる=%s UIスクロール=%s DebugPrefix=%s DebugKeyboard=%s DebugGamepad=%s"),
            InputRouter->IA_Move ? TEXT("あり") : TEXT("なし"),
            InputRouter->IA_Wait ? TEXT("あり") : TEXT("なし"),
            InputRouter->IA_Attack ? TEXT("あり") : TEXT("なし"),
            InputRouter->IA_Interact ? TEXT("あり") : TEXT("なし"),
            InputRouter->IA_Menu ? TEXT("あり") : TEXT("なし"),
            InputRouter->IA_CameraLook ? TEXT("あり") : TEXT("なし"),
            InputRouter->IA_CameraZoom ? TEXT("あり") : TEXT("なし"),
            InputRouter->IA_UI_Direction ? TEXT("あり") : TEXT("なし"),
            InputRouter->IA_UI_Confirm ? TEXT("あり") : TEXT("なし"),
            InputRouter->IA_UI_Close ? TEXT("あり") : TEXT("なし"),
            InputRouter->IA_UI_Scroll ? TEXT("あり") : TEXT("なし"),
            InputRouter->IA_DebugCommandPrefix ? TEXT("あり") : TEXT("なし"),
            (InputRouter->IA_DebugCommandKeyboard1 &&
                InputRouter->IA_DebugCommandKeyboard2 &&
                InputRouter->IA_DebugCommandKeyboard3 &&
                InputRouter->IA_DebugCommandKeyboard4 &&
                InputRouter->IA_DebugCommandKeyboard5) ? TEXT("あり") : TEXT("なし"),
            (InputRouter->IA_DebugCommandGamepad1 &&
                InputRouter->IA_DebugCommandGamepad2) ? TEXT("あり") : TEXT("なし")
        );
    }
}

/** EnhancedInputComponentへ差し替える */
void ARldPlayerController::EnsureEnhancedInputComponent()
{
    // すでにEnhancedInputComponentなら差し替え不要
    if (Cast<UEnhancedInputComponent>(InputComponent))
    {
        return;
    }

    // 既存InputComponentがある場合は取り外す
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
