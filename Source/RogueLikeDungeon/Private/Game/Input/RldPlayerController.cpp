// RldPlayerController.cpp

#include "Game/Input/RldPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/GameInstance.h"
#include "InputCoreTypes.h"

#include "Common/Input/CmnInputConfig.h"
#include "Common/Input/CmnInputRouter.h"
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

    ULocalPlayer* localPlayer = GetLocalPlayer();
    UEnhancedInputLocalPlayerSubsystem* subsystem =
        localPlayer ? localPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>() : nullptr;

    // 入力ルーターまたはEnhancedInputSubsystem未取得時は初期化しない
    if (!ensure(InputRouter && subsystem))
    {
        UE_LOG(
            LogRldInput,
            Error,
            TEXT("BeginPlay: Actor=%s 入力初期化に失敗しました InputRouter=%s Subsystem=%s"),
            *GetNameSafe(this),
            InputRouter ? TEXT("有効") : TEXT("null"),
            subsystem ? TEXT("有効") : TEXT("null")
        );

        return;
    }

    // 入力ルーターを初期化してゲームモードを適用
    InputRouter->Initialize(this, subsystem);
    InputRouter->SetInputMode(ECmnInputMode::Game);

    UE_LOG(
        LogRldInput,
        Log,
        TEXT("BeginPlay: Actor=%s 入力初期化完了 初期入力モード=Game"),
        *GetNameSafe(this)
    );
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

    UEnhancedInputComponent* enhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);

    // EnhancedInputComponentまたは入力ルーター未取得時はBindしない
    if (!ensure(enhancedInputComponent && InputRouter))
    {
        UE_LOG(
            LogRldInput,
            Error,
            TEXT("SetupInputComponent: Actor=%s 入力Bindに失敗しました EnhancedInputComponent=%s InputRouter=%s"),
            *GetNameSafe(this),
            enhancedInputComponent ? TEXT("有効") : TEXT("null"),
            InputRouter ? TEXT("有効") : TEXT("null")
        );

        return;
    }

    UE_LOG(
        LogRldInput,
        Verbose,
        TEXT("SetupInputComponent: Actor=%s 入力Bind開始"),
        *GetNameSafe(this)
    );

    // ----- ゲーム操作 -----

    // 移動InputAction取得時はBind
    if (InputRouter->IA_Move)
    {
        UE_LOG(
            LogRldInput,
            Verbose,
            TEXT("SetupInputComponent: Actor=%s IA_MoveをBindします"),
            *GetNameSafe(this)
        );

        enhancedInputComponent->BindAction(
            InputRouter->IA_Move,
            ETriggerEvent::Started,
            this,
            &ARldPlayerController::OnMoveStarted
        );

        enhancedInputComponent->BindAction(
            InputRouter->IA_Move,
            ETriggerEvent::Triggered,
            this,
            &ARldPlayerController::OnMoveTriggered
        );
    }
    else
    {
        UE_LOG(
            LogRldInput,
            Warning,
            TEXT("SetupInputComponent: Actor=%s IA_MoveがnullのためBindしません"),
            *GetNameSafe(this)
        );
    }

    // 足踏み用InputAction取得時はBind
    if (InputRouter->IA_StepInPlaceModifier)
    {
        UE_LOG(
            LogRldInput,
            Verbose,
            TEXT("SetupInputComponent: Actor=%s IA_StepInPlaceModifierをBindします"),
            *GetNameSafe(this)
        );

        enhancedInputComponent->BindAction(
            InputRouter->IA_StepInPlaceModifier,
            ETriggerEvent::Started,
            this,
            &ARldPlayerController::OnStepInPlaceModifierStarted
        );

        enhancedInputComponent->BindAction(
            InputRouter->IA_StepInPlaceModifier,
            ETriggerEvent::Completed,
            this,
            &ARldPlayerController::OnStepInPlaceModifierCompleted
        );

        enhancedInputComponent->BindAction(
            InputRouter->IA_StepInPlaceModifier,
            ETriggerEvent::Canceled,
            this,
            &ARldPlayerController::OnStepInPlaceModifierCompleted
        );
    }
    else
    {
        UE_LOG(
            LogRldInput,
            Warning,
            TEXT("SetupInputComponent: Actor=%s IA_StepInPlaceModifierがnullのためBindしません"),
            *GetNameSafe(this)
        );
    }

    // 通常攻撃InputAction取得時はBind
    if (InputRouter->IA_Attack)
    {
        UE_LOG(
            LogRldInput,
            Verbose,
            TEXT("SetupInputComponent: Actor=%s IA_AttackをBindします"),
            *GetNameSafe(this)
        );

        enhancedInputComponent->BindAction(
            InputRouter->IA_Attack,
            ETriggerEvent::Started,
            this,
            &ARldPlayerController::OnAttackStarted
        );
    }
    else
    {
        UE_LOG(
            LogRldInput,
            Warning,
            TEXT("SetupInputComponent: Actor=%s IA_AttackがnullのためBindしません"),
            *GetNameSafe(this)
        );
    }

    // インタラクトInputAction取得時はBind
    if (InputRouter->IA_Interact)
    {
        UE_LOG(
            LogRldInput,
            Verbose,
            TEXT("SetupInputComponent: Actor=%s IA_InteractをBindします"),
            *GetNameSafe(this)
        );

        enhancedInputComponent->BindAction(
            InputRouter->IA_Interact,
            ETriggerEvent::Started,
            this,
            &ARldPlayerController::OnInteractStarted
        );
    }
    else
    {
        UE_LOG(
            LogRldInput,
            Warning,
            TEXT("SetupInputComponent: Actor=%s IA_InteractがnullのためBindしません"),
            *GetNameSafe(this)
        );
    }

    // メニューInputAction取得時はBind
    if (InputRouter->IA_Menu)
    {
        UE_LOG(
            LogRldInput,
            Verbose,
            TEXT("SetupInputComponent: Actor=%s IA_MenuをBindします"),
            *GetNameSafe(this)
        );

        enhancedInputComponent->BindAction(
            InputRouter->IA_Menu,
            ETriggerEvent::Started,
            this,
            &ARldPlayerController::OnMenuStarted
        );
    }
    else
    {
        UE_LOG(
            LogRldInput,
            Warning,
            TEXT("SetupInputComponent: Actor=%s IA_MenuがnullのためBindしません"),
            *GetNameSafe(this)
        );
    }

    // カメラ視点InputAction取得時はBind
    if (InputRouter->IA_CameraLook)
    {
        UE_LOG(
            LogRldInput,
            Verbose,
            TEXT("SetupInputComponent: Actor=%s IA_CameraLookをBindします"),
            *GetNameSafe(this)
        );

        enhancedInputComponent->BindAction(
            InputRouter->IA_CameraLook,
            ETriggerEvent::Triggered,
            this,
            &ARldPlayerController::OnCameraLookTriggered
        );
    }
    else
    {
        UE_LOG(
            LogRldInput,
            Warning,
            TEXT("SetupInputComponent: Actor=%s IA_CameraLookがnullのためBindしません"),
            *GetNameSafe(this)
        );
    }

    // カメラズームInputAction取得時はBind
    if (InputRouter->IA_CameraZoom)
    {
        UE_LOG(
            LogRldInput,
            Verbose,
            TEXT("SetupInputComponent: Actor=%s IA_CameraZoomをBindします"),
            *GetNameSafe(this)
        );

        enhancedInputComponent->BindAction(
            InputRouter->IA_CameraZoom,
            ETriggerEvent::Triggered,
            this,
            &ARldPlayerController::OnCameraZoomTriggered
        );
    }
    else
    {
        UE_LOG(
            LogRldInput,
            Warning,
            TEXT("SetupInputComponent: Actor=%s IA_CameraZoomがnullのためBindしません"),
            *GetNameSafe(this)
        );
    }

    // ----- UI操作 -----

    // UI方向入力InputAction取得時はBind
    if (InputRouter->IA_UI_Direction)
    {
        UE_LOG(
            LogRldInput,
            Verbose,
            TEXT("SetupInputComponent: Actor=%s IA_UI_DirectionをBindします"),
            *GetNameSafe(this)
        );

        enhancedInputComponent->BindAction(
            InputRouter->IA_UI_Direction,
            ETriggerEvent::Triggered,
            this,
            &ARldPlayerController::OnUIDirectionTriggered
        );
    }
    else
    {
        UE_LOG(
            LogRldInput,
            Warning,
            TEXT("SetupInputComponent: Actor=%s IA_UI_DirectionがnullのためBindしません"),
            *GetNameSafe(this)
        );
    }

    // UI決定InputAction取得時はBind
    if (InputRouter->IA_UI_Confirm)
    {
        UE_LOG(
            LogRldInput,
            Verbose,
            TEXT("SetupInputComponent: Actor=%s IA_UI_ConfirmをBindします"),
            *GetNameSafe(this)
        );

        enhancedInputComponent->BindAction(
            InputRouter->IA_UI_Confirm,
            ETriggerEvent::Started,
            this,
            &ARldPlayerController::OnUIConfirmStarted
        );
    }
    else
    {
        UE_LOG(
            LogRldInput,
            Warning,
            TEXT("SetupInputComponent: Actor=%s IA_UI_ConfirmがnullのためBindしません"),
            *GetNameSafe(this)
        );
    }

    // UI閉じるInputAction取得時はBind
    if (InputRouter->IA_UI_Close)
    {
        UE_LOG(
            LogRldInput,
            Verbose,
            TEXT("SetupInputComponent: Actor=%s IA_UI_CloseをBindします"),
            *GetNameSafe(this)
        );

        enhancedInputComponent->BindAction(
            InputRouter->IA_UI_Close,
            ETriggerEvent::Started,
            this,
            &ARldPlayerController::OnUICloseStarted
        );
    }
    else
    {
        UE_LOG(
            LogRldInput,
            Warning,
            TEXT("SetupInputComponent: Actor=%s IA_UI_CloseがnullのためBindしません"),
            *GetNameSafe(this)
        );
    }

    // UIスクロールInputAction取得時はBind
    if (InputRouter->IA_UI_Scroll)
    {
        UE_LOG(
            LogRldInput,
            Verbose,
            TEXT("SetupInputComponent: Actor=%s IA_UI_ScrollをBindします"),
            *GetNameSafe(this)
        );

        enhancedInputComponent->BindAction(
            InputRouter->IA_UI_Scroll,
            ETriggerEvent::Triggered,
            this,
            &ARldPlayerController::OnUIScrollTriggered
        );
    }
    else
    {
        UE_LOG(
            LogRldInput,
            Warning,
            TEXT("SetupInputComponent: Actor=%s IA_UI_ScrollがnullのためBindしません"),
            *GetNameSafe(this)
        );
    }

    // ----- デバッグコマンド -----

    // デバッグコマンド開始InputAction取得時はBind
    if (InputRouter->IA_DebugCommandPrefix)
    {
        UE_LOG(
            LogRldInput,
            Verbose,
            TEXT("SetupInputComponent: Actor=%s IA_DebugCommandPrefixをBindします"),
            *GetNameSafe(this)
        );

        enhancedInputComponent->BindAction(
            InputRouter->IA_DebugCommandPrefix,
            ETriggerEvent::Started,
            this,
            &ARldPlayerController::OnDebugCommandPrefixStarted
        );
    }
    else
    {
        UE_LOG(
            LogRldInput,
            Warning,
            TEXT("SetupInputComponent: Actor=%s IA_DebugCommandPrefixがnullのためBindしません"),
            *GetNameSafe(this)
        );
    }

    // キーボード用デバッグコマンド1InputAction取得時はBind
    if (InputRouter->IA_DebugCommandKeyboard1)
    {
        UE_LOG(
            LogRldInput,
            Verbose,
            TEXT("SetupInputComponent: Actor=%s IA_DebugCommandKeyboard1をBindします"),
            *GetNameSafe(this)
        );

        enhancedInputComponent->BindAction(
            InputRouter->IA_DebugCommandKeyboard1,
            ETriggerEvent::Started,
            this,
            &ARldPlayerController::OnDebugCommandKeyboard1Started
        );
    }
    else
    {
        UE_LOG(
            LogRldInput,
            Warning,
            TEXT("SetupInputComponent: Actor=%s IA_DebugCommandKeyboard1がnullのためBindしません"),
            *GetNameSafe(this)
        );
    }

    // キーボード用デバッグコマンド2InputAction取得時はBind
    if (InputRouter->IA_DebugCommandKeyboard2)
    {
        UE_LOG(
            LogRldInput,
            Verbose,
            TEXT("SetupInputComponent: Actor=%s IA_DebugCommandKeyboard2をBindします"),
            *GetNameSafe(this)
        );

        enhancedInputComponent->BindAction(
            InputRouter->IA_DebugCommandKeyboard2,
            ETriggerEvent::Started,
            this,
            &ARldPlayerController::OnDebugCommandKeyboard2Started
        );
    }
    else
    {
        UE_LOG(
            LogRldInput,
            Warning,
            TEXT("SetupInputComponent: Actor=%s IA_DebugCommandKeyboard2がnullのためBindしません"),
            *GetNameSafe(this)
        );
    }

    // キーボード用デバッグコマンド3InputAction取得時はBind
    if (InputRouter->IA_DebugCommandKeyboard3)
    {
        UE_LOG(
            LogRldInput,
            Verbose,
            TEXT("SetupInputComponent: Actor=%s IA_DebugCommandKeyboard3をBindします"),
            *GetNameSafe(this)
        );

        enhancedInputComponent->BindAction(
            InputRouter->IA_DebugCommandKeyboard3,
            ETriggerEvent::Started,
            this,
            &ARldPlayerController::OnDebugCommandKeyboard3Started
        );
    }
    else
    {
        UE_LOG(
            LogRldInput,
            Warning,
            TEXT("SetupInputComponent: Actor=%s IA_DebugCommandKeyboard3がnullのためBindしません"),
            *GetNameSafe(this)
        );
    }

    // キーボード用デバッグコマンド4InputAction取得時はBind
    if (InputRouter->IA_DebugCommandKeyboard4)
    {
        UE_LOG(
            LogRldInput,
            Verbose,
            TEXT("SetupInputComponent: Actor=%s IA_DebugCommandKeyboard4をBindします"),
            *GetNameSafe(this)
        );

        enhancedInputComponent->BindAction(
            InputRouter->IA_DebugCommandKeyboard4,
            ETriggerEvent::Started,
            this,
            &ARldPlayerController::OnDebugCommandKeyboard4Started
        );
    }
    else
    {
        UE_LOG(
            LogRldInput,
            Warning,
            TEXT("SetupInputComponent: Actor=%s IA_DebugCommandKeyboard4がnullのためBindしません"),
            *GetNameSafe(this)
        );
    }

    // キーボード用デバッグコマンド5InputAction取得時はBind
    if (InputRouter->IA_DebugCommandKeyboard5)
    {
        UE_LOG(
            LogRldInput,
            Verbose,
            TEXT("SetupInputComponent: Actor=%s IA_DebugCommandKeyboard5をBindします"),
            *GetNameSafe(this)
        );

        enhancedInputComponent->BindAction(
            InputRouter->IA_DebugCommandKeyboard5,
            ETriggerEvent::Started,
            this,
            &ARldPlayerController::OnDebugCommandKeyboard5Started
        );
    }
    else
    {
        UE_LOG(
            LogRldInput,
            Warning,
            TEXT("SetupInputComponent: Actor=%s IA_DebugCommandKeyboard5がnullのためBindしません"),
            *GetNameSafe(this)
        );
    }

    // ゲームパッド用デバッグコマンド1InputAction取得時はBind
    if (InputRouter->IA_DebugCommandGamepad1)
    {
        UE_LOG(
            LogRldInput,
            Verbose,
            TEXT("SetupInputComponent: Actor=%s IA_DebugCommandGamepad1をBindします"),
            *GetNameSafe(this)
        );

        enhancedInputComponent->BindAction(
            InputRouter->IA_DebugCommandGamepad1,
            ETriggerEvent::Started,
            this,
            &ARldPlayerController::OnDebugCommandGamepad1Started
        );
    }
    else
    {
        UE_LOG(
            LogRldInput,
            Warning,
            TEXT("SetupInputComponent: Actor=%s IA_DebugCommandGamepad1がnullのためBindしません"),
            *GetNameSafe(this)
        );
    }

    // ゲームパッド用デバッグコマンド2InputAction取得時はBind
    if (InputRouter->IA_DebugCommandGamepad2)
    {
        UE_LOG(
            LogRldInput,
            Verbose,
            TEXT("SetupInputComponent: Actor=%s IA_DebugCommandGamepad2をBindします"),
            *GetNameSafe(this)
        );

        enhancedInputComponent->BindAction(
            InputRouter->IA_DebugCommandGamepad2,
            ETriggerEvent::Started,
            this,
            &ARldPlayerController::OnDebugCommandGamepad2Started
        );
    }
    else
    {
        UE_LOG(
            LogRldInput,
            Warning,
            TEXT("SetupInputComponent: Actor=%s IA_DebugCommandGamepad2がnullのためBindしません"),
            *GetNameSafe(this)
        );
    }

    UE_LOG(
        LogRldInput,
        Verbose,
        TEXT("SetupInputComponent: Actor=%s 入力Bind完了"),
        *GetNameSafe(this)
    );
}

// ----- 公開API -----

/** 共通入力モードを切り替える */
void ARldPlayerController::SetCommonInputMode(ECmnInputMode Mode)
{
    // 入力ルーター未取得時は入力モードを切り替えない
    if (!InputRouter)
    {
        UE_LOG(
            LogRldInput,
            Warning,
            TEXT("SetCommonInputMode: Actor=%s InputRouterがnullのため入力モードを切り替えません"),
            *GetNameSafe(this)
        );

        return;
    }

    InputRouter->SetInputMode(Mode);
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
        Verbose,
        TEXT("HandleInputSettingsChanged: Actor=%s 入力設定を反映しました 左右反転=%s 上下反転=%s"),
        *GetNameSafe(this),
        bInvertCameraX ? TEXT("有効") : TEXT("無効"),
        bInvertCameraY ? TEXT("有効") : TEXT("無効")
    );
}

// ----- ゲーム操作の入力 -----

/** 移動入力開始時の処理を行う */
void ARldPlayerController::OnMoveStarted(const FInputActionValue& Value)
{
    // ゲームモード以外では移動処理しない
    if (!InputRouter || !InputRouter->IsGameMode())
    {
        UE_LOG(
            LogRldInput,
            Verbose,
            TEXT("OnMoveStarted: Actor=%s ゲームモード以外のため処理しません"),
            *GetNameSafe(this)
        );

        return;
    }

    // デバッグコマンド入力受付中は通常ゲーム入力を受け付けない
    if (IsHiddenDebugCommandInputActive())
    {
        UE_LOG(
            LogRldInput,
            Verbose,
            TEXT("OnMoveStarted: Actor=%s デバッグコマンド入力受付中のため処理しません"),
            *GetNameSafe(this)
        );

        return;
    }

    const FVector2D axis = Value.Get<FVector2D>();

    // 左スティック入力はTriggered側で処理
    if (IsLikelyLeftStickInput(axis))
    {
        return;
    }

    const FString inputSourceText = BuildMoveInputSourceDebugText();

    UE_LOG(
        LogRldInput,
        Verbose,
        TEXT("OnMoveStarted: Actor=%s 移動入力を受け付けました 入力元=%s 生入力=(%f,%f)"),
        *GetNameSafe(this),
        *inputSourceText,
        axis.X,
        axis.Y
    );

    FIntPoint direction;

    // 方向確定失敗時は移動処理しない
    if (!TryConvertMoveAxisToGridDir(axis, direction))
    {
        UE_LOG(
            LogRldInput,
            Warning,
            TEXT("OnMoveStarted: Actor=%s 方向変換に失敗しました 入力元=%s 生入力=(%f,%f)"),
            *GetNameSafe(this),
            *inputSourceText,
            axis.X,
            axis.Y
        );

        return;
    }

    ProcessResolvedMoveDirection(direction, inputSourceText, axis);
}

/** 移動入力の生値ログを出力する */
void ARldPlayerController::OnMoveTriggered(const FInputActionValue& Value)
{
    const FVector2D axis = Value.Get<FVector2D>();
    const FString inputSourceText = BuildMoveInputSourceDebugText();

    UE_LOG(
        LogRldInput,
        Verbose,
        TEXT("OnMoveTriggered: Actor=%s 入力元=%s 生入力=(%f,%f)"),
        *GetNameSafe(this),
        *inputSourceText,
        axis.X,
        axis.Y
    );

    // デバッグコマンド入力受付中は通常ゲーム入力を受け付けない
    if (IsHiddenDebugCommandInputActive())
    {
        UE_LOG(
            LogRldInput,
            Verbose,
            TEXT("OnMoveTriggered: Actor=%s デバッグコマンド入力受付中のため処理しません"),
            *GetNameSafe(this)
        );

        return;
    }

    // 左スティック入力のみTriggered側で処理
    if (IsLikelyLeftStickInput(axis))
    {
        HandleLeftStickMoveTriggered(axis);
    }
}

/** 足踏み入力開始時の処理を行う */
void ARldPlayerController::OnStepInPlaceModifierStarted(const FInputActionValue& Value)
{
    // 未使用引数
    (void)Value;

    // ゲームモード以外では足踏み修飾を受け付けない
    if (!InputRouter || !InputRouter->IsGameMode())
    {
        UE_LOG(
            LogRldInput,
            Verbose,
            TEXT("OnStepInPlaceModifierStarted: Actor=%s ゲームモード以外のため処理しません"),
            *GetNameSafe(this)
        );

        return;
    }

    // デバッグコマンド入力受付中は通常ゲーム入力を受け付けない
    if (IsHiddenDebugCommandInputActive())
    {
        UE_LOG(
            LogRldInput,
            Verbose,
            TEXT("OnStepInPlaceModifierStarted: Actor=%s デバッグコマンド入力受付中のため処理しません"),
            *GetNameSafe(this)
        );

        return;
    }

    bStepInPlaceModifierPressed = true;

    // 修飾状態が切り替わったため既存の押しっぱなし移動を停止する
    StopMoveRepeat();

    UE_LOG(
        LogRldInput,
        Verbose,
        TEXT("OnStepInPlaceModifierStarted: Actor=%s 足踏み修飾入力を開始しました"),
        *GetNameSafe(this)
    );
}

/** 足踏み修飾入力終了時の処理を行う */
void ARldPlayerController::OnStepInPlaceModifierCompleted(const FInputActionValue& Value)
{
    // 未使用引数
    (void)Value;

    bStepInPlaceModifierPressed = false;

    // 修飾解除後に足踏みリピートが移動へ変わらないよう停止する
    StopMoveRepeat();

    UE_LOG(
        LogRldInput,
        Verbose,
        TEXT("OnStepInPlaceModifierCompleted: Actor=%s 足踏み修飾入力を終了しました"),
        *GetNameSafe(this)
    );
}

/** 通常攻撃入力を処理する */
void ARldPlayerController::OnAttackStarted(const FInputActionValue& Value)
{
    // 未使用引数
    (void)Value;

    // ゲームモード以外では通常攻撃処理しない
    if (!InputRouter || !InputRouter->IsGameMode())
    {
        UE_LOG(
            LogRldInput,
            Verbose,
            TEXT("OnAttackStarted: Actor=%s ゲームモード以外のため処理しません"),
            *GetNameSafe(this)
        );

        return;
    }

    // デバッグコマンド入力受付中は通常ゲーム入力を受け付けない
    if (IsHiddenDebugCommandInputActive())
    {
        UE_LOG(
            LogRldInput,
            Verbose,
            TEXT("OnAttackStarted: Actor=%s デバッグコマンド入力受付中のため処理しません"),
            *GetNameSafe(this)
        );

        return;
    }

    ARldPlayerCharacter* playerCharacter = GetRldPlayerCharacter();

    // プレイヤーキャラクター未取得時は処理しない
    if (!playerCharacter)
    {
        UE_LOG(
            LogRldInput,
            Warning,
            TEXT("OnAttackStarted: Actor=%s PlayerCharacterがnullのため通常攻撃を実行しません"),
            *GetNameSafe(this)
        );

        return;
    }

    StopMoveRepeat();

    UE_LOG(
        LogRldInput,
        Verbose,
        TEXT("OnAttackStarted: Actor=%s 通常攻撃入力を受け付けました"),
        *GetNameSafe(playerCharacter)
    );

    playerCharacter->RequestAttackAction();
}

/** インタラクト入力を処理する */
void ARldPlayerController::OnInteractStarted(const FInputActionValue& Value)
{
    // 未使用引数
    (void)Value;

    // ゲームモード以外ではインタラクト処理しない
    if (!InputRouter || !InputRouter->IsGameMode())
    {
        UE_LOG(
            LogRldInput,
            Verbose,
            TEXT("OnInteractStarted: Actor=%s ゲームモード以外のため処理しません"),
            *GetNameSafe(this)
        );

        return;
    }

    // デバッグコマンド入力受付中は通常ゲーム入力を受け付けない
    if (IsHiddenDebugCommandInputActive())
    {
        UE_LOG(
            LogRldInput,
            Verbose,
            TEXT("OnInteractStarted: Actor=%s デバッグコマンド入力受付中のため処理しません"),
            *GetNameSafe(this)
        );

        return;
    }

    ARldPlayerCharacter* playerCharacter = GetRldPlayerCharacter();

    // プレイヤーキャラクター未取得時は処理しない
    if (!playerCharacter)
    {
        UE_LOG(
            LogRldInput,
            Warning,
            TEXT("OnInteractStarted: Actor=%s PlayerCharacterがnullのためインタラクトを実行しません"),
            *GetNameSafe(this)
        );

        return;
    }

    StopMoveRepeat();

    UE_LOG(
        LogRldInput,
        Verbose,
        TEXT("OnInteractStarted: Actor=%s インタラクト入力を受け付けました"),
        *GetNameSafe(playerCharacter)
    );

    playerCharacter->RequestInteractAction();
}

/** メニュー入力を処理する */
void ARldPlayerController::OnMenuStarted(const FInputActionValue& Value)
{
    // 未使用引数
    (void)Value;

    // ゲームモード以外ではメニュー処理しない
    if (!InputRouter || !InputRouter->IsGameMode())
    {
        UE_LOG(
            LogRldInput,
            Verbose,
            TEXT("OnMenuStarted: Actor=%s ゲームモード以外のため処理しません"),
            *GetNameSafe(this)
        );

        return;
    }

    // デバッグコマンド入力受付中は通常ゲーム入力を受け付けない
    if (IsHiddenDebugCommandInputActive())
    {
        UE_LOG(
            LogRldInput,
            Verbose,
            TEXT("OnMenuStarted: Actor=%s デバッグコマンド入力受付中のため処理しません"),
            *GetNameSafe(this)
        );

        return;
    }

    StopMoveRepeat();

    UE_LOG(
        LogRldInput,
        Verbose,
        TEXT("OnMenuStarted: Actor=%s メニュー入力を受け付けました"),
        *GetNameSafe(this)
    );

    // TODO: メインメニューWidgetを表示する
}

/** カメラ視点入力を処理する */
void ARldPlayerController::OnCameraLookTriggered(const FInputActionValue& Value)
{
    const FVector2D rawAxis = Value.Get<FVector2D>();

    UE_LOG(
        LogRldInput,
        Verbose,
        TEXT("OnCameraLookTriggered: Actor=%s 生入力=(%f,%f)"),
        *GetNameSafe(this),
        rawAxis.X,
        rawAxis.Y
    );

    // ゲームモード以外ではカメラ視点処理しない
    if (!InputRouter || !InputRouter->IsGameMode())
    {
        UE_LOG(
            LogRldInput,
            Verbose,
            TEXT("OnCameraLookTriggered: Actor=%s ゲームモード以外のため処理しません"),
            *GetNameSafe(this)
        );

        return;
    }

    // デバッグコマンド入力受付中は通常ゲーム入力を受け付けない
    if (IsHiddenDebugCommandInputActive())
    {
        UE_LOG(
            LogRldInput,
            Verbose,
            TEXT("OnCameraLookTriggered: Actor=%s デバッグコマンド入力受付中のため処理しません"),
            *GetNameSafe(this)
        );

        return;
    }

    FVector2D axis = rawAxis;

    // 左右反転設定を適用
    if (bInvertCameraX)
    {
        axis.X *= -1.0f;
    }

    // 上下反転設定を適用
    if (bInvertCameraY)
    {
        axis.Y *= -1.0f;
    }

    UE_LOG(
        LogRldInput,
        Verbose,
        TEXT("OnCameraLookTriggered: Actor=%s 反転適用後入力=(%f,%f)"),
        *GetNameSafe(this),
        axis.X,
        axis.Y
    );

    ARldPlayerCharacter* playerCharacter = GetRldPlayerCharacter();

    // プレイヤーキャラクター未取得時は視点入力しない
    if (!playerCharacter)
    {
        UE_LOG(
            LogRldInput,
            Warning,
            TEXT("OnCameraLookTriggered: Actor=%s PlayerCharacterがnullのため視点入力を処理しません"),
            *GetNameSafe(this)
        );

        return;
    }

    // 視点入力受付不可時は処理しない
    if (!playerCharacter->CanAcceptLookInput())
    {
        UE_LOG(
            LogRldInput,
            Verbose,
            TEXT("OnCameraLookTriggered: Actor=%s 視点入力を受け付けないため処理しません"),
            *GetNameSafe(playerCharacter)
        );

        return;
    }

    playerCharacter->RequestLookInput(axis);
}

/** カメラズーム入力を処理する */
void ARldPlayerController::OnCameraZoomTriggered(const FInputActionValue& Value)
{
    const float zoomValue = Value.Get<float>();

    UE_LOG(
        LogRldInput,
        Verbose,
        TEXT("OnCameraZoomTriggered: Actor=%s 生入力=%f"),
        *GetNameSafe(this),
        zoomValue
    );

    // ゲームモード以外ではカメラズーム処理しない
    if (!InputRouter || !InputRouter->IsGameMode())
    {
        UE_LOG(
            LogRldInput,
            Verbose,
            TEXT("OnCameraZoomTriggered: Actor=%s ゲームモード以外のため処理しません"),
            *GetNameSafe(this)
        );

        return;
    }

    // デバッグコマンド入力受付中は通常ゲーム入力を受け付けない
    if (IsHiddenDebugCommandInputActive())
    {
        UE_LOG(
            LogRldInput,
            Verbose,
            TEXT("OnCameraZoomTriggered: Actor=%s デバッグコマンド入力受付中のため処理しません"),
            *GetNameSafe(this)
        );

        return;
    }

    ARldPlayerCharacter* playerCharacter = GetRldPlayerCharacter();

    // プレイヤーキャラクター未取得時はズーム入力しない
    if (!playerCharacter)
    {
        UE_LOG(
            LogRldInput,
            Warning,
            TEXT("OnCameraZoomTriggered: Actor=%s PlayerCharacterがnullのためズーム入力を処理しません"),
            *GetNameSafe(this)
        );

        return;
    }

    // ズーム入力受付不可時は処理しない
    if (!playerCharacter->CanAcceptZoomInput())
    {
        UE_LOG(
            LogRldInput,
            Verbose,
            TEXT("OnCameraZoomTriggered: Actor=%s ズーム入力を受け付けないため処理しません"),
            *GetNameSafe(playerCharacter)
        );

        return;
    }

    playerCharacter->RequestZoomInput(zoomValue);
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

    const FVector2D axis = Value.Get<FVector2D>();

    // 表示中のデバッグオプションがある場合は専用フォーカス移動を行う
    if (URldDebugOptionsWidget* debugOptionsWidget = GetActiveDebugOptionsWidget())
    {
        FIntPoint direction;

        // UI方向入力を8方向へ変換できた場合のみフォーカスを移動
        if (InputRouter->GetMoveDirFromAxis(axis, direction))
        {
            if (direction.Y > 0)
            {
                debugOptionsWidget->MoveDebugFocus(-1);
            }
            else if (direction.Y < 0)
            {
                debugOptionsWidget->MoveDebugFocus(1);
            }
        }

        return;
    }

    InputRouter->HandleUIDirectionAxis(axis);
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

    // 表示中のデバッグオプションがある場合はフォーカス項目を決定
    if (URldDebugOptionsWidget* debugOptionsWidget = GetActiveDebugOptionsWidget())
    {
        debugOptionsWidget->ConfirmFocusedDebugItem();
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

    // 表示中のデバッグオプションがある場合は閉じる
    if (URldDebugOptionsWidget* debugOptionsWidget = GetActiveDebugOptionsWidget())
    {
        debugOptionsWidget->CloseDebugOptions();
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

    const float amount = Value.Get<float>();
    InputRouter->HandleUIScrollAxis(amount);
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

    UWorld* world = GetWorld();

    // World未取得時は処理しない
    if (!world)
    {
        return;
    }

    const double currentTime = world->GetTimeSeconds();

    // 入力間隔が空いた場合はコマンド状態を初期化
    if ((currentTime - lastHiddenDebugInputTime) > hiddenDebugCommandTimeout)
    {
        ResetDebugCommandState();
    }

    lastHiddenDebugInputTime = currentTime;
    ++debugCommandPrefixCount;
    debugCommandIndex = 0;
    debugCommandInputType = ERldDebugCommandInputType::None;

    UE_LOG(
        LogRldInput,
        Verbose,
        TEXT("OnDebugCommandPrefixStarted: Actor=%s デバッグコマンド開始入力回数=%d"),
        *GetNameSafe(this),
        debugCommandPrefixCount
    );
}

/** キーボード用デバッグコマンド第1入力を処理する */
void ARldPlayerController::OnDebugCommandKeyboard1Started(const FInputActionValue& Value)
{
    // 未使用引数
    (void)Value;

    HandleKeyboardDebugCommandCharacter(0);
}

/** キーボード用デバッグコマンド第2入力を処理する */
void ARldPlayerController::OnDebugCommandKeyboard2Started(const FInputActionValue& Value)
{
    // 未使用引数
    (void)Value;

    HandleKeyboardDebugCommandCharacter(1);
}

/** キーボード用デバッグコマンド第3入力を処理する */
void ARldPlayerController::OnDebugCommandKeyboard3Started(const FInputActionValue& Value)
{
    // 未使用引数
    (void)Value;

    HandleKeyboardDebugCommandCharacter(2);
}

/** キーボード用デバッグコマンド第4入力を処理する */
void ARldPlayerController::OnDebugCommandKeyboard4Started(const FInputActionValue& Value)
{
    // 未使用引数
    (void)Value;

    HandleKeyboardDebugCommandCharacter(3);
}

/** キーボード用デバッグコマンド第5入力を処理する */
void ARldPlayerController::OnDebugCommandKeyboard5Started(const FInputActionValue& Value)
{
    // 未使用引数
    (void)Value;

    HandleKeyboardDebugCommandCharacter(4);
}

/** ゲームパッド用デバッグコマンド第1入力を処理する */
void ARldPlayerController::OnDebugCommandGamepad1Started(const FInputActionValue& Value)
{
    // 未使用引数
    (void)Value;

    HandleGamepadDebugCommandInput(true);
}

/** ゲームパッド用デバッグコマンド第2入力を処理する */
void ARldPlayerController::OnDebugCommandGamepad2Started(const FInputActionValue& Value)
{
    // 未使用引数
    (void)Value;

    HandleGamepadDebugCommandInput(false);
}

// ----- ゲーム固有入力変換 -----

/** 入力軸をカメラ基準のグリッド8方向へ変換する */
bool ARldPlayerController::TryConvertMoveAxisToGridDir(
    const FVector2D& Axis,
    FIntPoint& OutDirection
) const
{
    FVector worldDirection;

    // ワールド平面方向に変換できない場合は失敗
    if (!TryConvertInputAxisToCameraRelativeWorldDir(Axis, worldDirection))
    {
        return false;
    }

    return TryConvertWorldDirToGridDir(worldDirection, OutDirection);
}

/** 入力軸をカメラ基準のワールド平面方向へ変換する */
bool ARldPlayerController::TryConvertInputAxisToCameraRelativeWorldDir(
    const FVector2D& Axis,
    FVector& OutWorldDirection
) const
{
    OutWorldDirection = FVector::ZeroVector;

    const float inputDeadZone = 0.5f;

    const float absX = FMath::Abs(Axis.X);
    const float absY = FMath::Abs(Axis.Y);

    // 入力しきい値未満なら無効
    if (absX < inputDeadZone && absY < inputDeadZone)
    {
        return false;
    }

    const ARldPlayerCharacter* playerCharacter = GetRldPlayerCharacter();

    // プレイヤーキャラクター未取得時は変換失敗
    if (!playerCharacter)
    {
        return false;
    }

    const FVector cameraForward = playerCharacter->GetCameraPlanarForward();
    const FVector cameraRight = playerCharacter->GetCameraPlanarRight();

    // 入力軸をワールド平面方向へ変換
    FVector moveWorldDirection =
        (cameraForward * Axis.Y) +
        (cameraRight * Axis.X);

    moveWorldDirection.Z = 0.0f;

    // 平面方向が無効な場合は変換失敗
    if (moveWorldDirection.IsNearlyZero(KINDA_SMALL_NUMBER))
    {
        return false;
    }

    moveWorldDirection.Normalize();
    OutWorldDirection = moveWorldDirection;

    UE_LOG(
        LogRldInput,
        Verbose,
        TEXT("TryConvertInputAxisToCameraRelativeWorldDir: Actor=%s 入力=(%f,%f) ワールド方向=(%f,%f,%f)"),
        *GetNameSafe(this),
        Axis.X,
        Axis.Y,
        OutWorldDirection.X,
        OutWorldDirection.Y,
        OutWorldDirection.Z
    );

    return true;
}

/** ワールド平面方向をグリッド8方向へ変換する */
bool ARldPlayerController::TryConvertWorldDirToGridDir(
    const FVector& WorldDirection,
    FIntPoint& OutGridDirection
) const
{
    OutGridDirection = FIntPoint::ZeroValue;

    const float absX = FMath::Abs(WorldDirection.X);
    const float absY = FMath::Abs(WorldDirection.Y);

    // ワールド平面方向が無効な場合は変換失敗
    if (absX < KINDA_SMALL_NUMBER && absY < KINDA_SMALL_NUMBER)
    {
        return false;
    }

    const float maxAxis = FMath::Max(absX, absY);
    const float minAxis = FMath::Min(absX, absY);

    // 大きい軸に対して小さい軸がこの割合以上なら斜め方向として扱う
    const float diagonalRatioThreshold = 0.50f;

    const int32 signX = (WorldDirection.X >= 0.0f) ? 1 : -1;
    const int32 signY = (WorldDirection.Y >= 0.0f) ? 1 : -1;

    // X/Y両方の入力が十分にある場合は斜め方向へ変換
    if ((minAxis / maxAxis) >= diagonalRatioThreshold)
    {
        OutGridDirection = FIntPoint(signX, signY);
    }
    // X軸が強い場合は左右方向へ変換
    else if (absX > absY)
    {
        OutGridDirection = FIntPoint(signX, 0);
    }
    // Y軸が強い場合は上下方向へ変換
    else
    {
        OutGridDirection = FIntPoint(0, signY);
    }

    UE_LOG(
        LogRldInput,
        Verbose,
        TEXT("TryConvertWorldDirToGridDir: Actor=%s ワールド方向=(%f,%f) グリッド方向=(%d,%d)"),
        *GetNameSafe(this),
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
        Verbose,
        TEXT("ProcessResolvedMoveDirection: Actor=%s 移動方向確定 入力元=%s 確定方向=(%d,%d) 生入力=(%f,%f) 足踏み修飾=%s"),
        *GetNameSafe(this),
        *InputSourceText,
        Direction.X,
        Direction.Y,
        Axis.X,
        Axis.Y,
        bStepInPlaceModifierPressed ? TEXT("有効") : TEXT("無効")
    );

    StartMoveRepeat(Direction, InputSourceText, Axis);

    ARldPlayerCharacter* playerCharacter = GetRldPlayerCharacter();

    // プレイヤーキャラクター未取得時は処理しない
    if (!playerCharacter)
    {
        UE_LOG(
            LogRldInput,
            Warning,
            TEXT("ProcessResolvedMoveDirection: Actor=%s PlayerCharacterがnullのため処理を行いません"),
            *GetNameSafe(this)
        );

        return;
    }

    // 移動入力受付不可時は処理しない
    if (!playerCharacter->CanAcceptMoveInput())
    {
        UE_LOG(
            LogRldInput,
            Verbose,
            TEXT("ProcessResolvedMoveDirection: Actor=%s 移動入力を受け付けないため処理しません"),
            *GetNameSafe(playerCharacter)
        );

        return;
    }

    // 足踏み入力中は移動せず、入力方向への足踏み行動として処理する
    if (bStepInPlaceModifierPressed)
    {
        playerCharacter->RequestStepInPlaceAction(Direction);
        return;
    }

    playerCharacter->RequestMoveDirection(Direction);
}
// ----- 左スティック専用処理 -----

/** 左スティック入力かどうかを判定する */
bool ARldPlayerController::IsLikelyLeftStickInput(const FVector2D& Axis) const
{
    const float leftStickX = GetInputAnalogKeyState(EKeys::Gamepad_LeftX);
    const float leftStickY = GetInputAnalogKeyState(EKeys::Gamepad_LeftY);

    const bool bHasLeftStickAnalog =
        !FMath::IsNearlyZero(leftStickX, KINDA_SMALL_NUMBER) ||
        !FMath::IsNearlyZero(leftStickY, KINDA_SMALL_NUMBER);

    // 左スティックのアナログ入力が取得できている場合は左スティック扱い
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
    const float releaseThreshold = 0.25f;
    const float confirmThreshold = 0.5f;

    const float absX = FMath::Abs(Axis.X);
    const float absY = FMath::Abs(Axis.Y);

    // ニュートラル復帰時は再入力可能にする
    if (absX < releaseThreshold && absY < releaseThreshold)
    {
        if (bLeftStickMoveConsumed)
        {
            UE_LOG(
                LogRldInput,
                Verbose,
                TEXT("HandleLeftStickMoveTriggered: Actor=%s 左スティックがニュートラルへ戻りました"),
                *GetNameSafe(this)
            );
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
    if (absX < confirmThreshold && absY < confirmThreshold)
    {
        return;
    }

    FIntPoint direction;

    // 方向確定失敗時は処理しない
    if (!TryConvertMoveAxisToGridDir(Axis, direction))
    {
        UE_LOG(
            LogRldInput,
            Warning,
            TEXT("HandleLeftStickMoveTriggered: Actor=%s 方向変換に失敗しました 生入力=(%f,%f)"),
            *GetNameSafe(this),
            Axis.X,
            Axis.Y
        );

        return;
    }

    bLeftStickMoveConsumed = true;

    ProcessResolvedMoveDirection(
        direction,
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
    TArray<FString> sources;

    // ----- キーボード -----

    if (IsInputKeyDown(EKeys::W))
    {
        sources.Add(TEXT("キーボード(W)"));
    }

    if (IsInputKeyDown(EKeys::S))
    {
        sources.Add(TEXT("キーボード(S)"));
    }

    if (IsInputKeyDown(EKeys::A))
    {
        sources.Add(TEXT("キーボード(A)"));
    }

    if (IsInputKeyDown(EKeys::D))
    {
        sources.Add(TEXT("キーボード(D)"));
    }

    // ----- ゲームパッド十字キー -----

    if (IsInputKeyDown(EKeys::Gamepad_DPad_Up))
    {
        sources.Add(TEXT("ゲームパッド十字キー(↑)"));
    }

    if (IsInputKeyDown(EKeys::Gamepad_DPad_Down))
    {
        sources.Add(TEXT("ゲームパッド十字キー(↓)"));
    }

    if (IsInputKeyDown(EKeys::Gamepad_DPad_Left))
    {
        sources.Add(TEXT("ゲームパッド十字キー(←)"));
    }

    if (IsInputKeyDown(EKeys::Gamepad_DPad_Right))
    {
        sources.Add(TEXT("ゲームパッド十字キー(→)"));
    }

    // ----- ゲームパッドLスティック -----

    const float leftStickX = GetInputAnalogKeyState(EKeys::Gamepad_LeftX);
    const float leftStickY = GetInputAnalogKeyState(EKeys::Gamepad_LeftY);

    const bool bHasLeftStickInput =
        !FMath::IsNearlyZero(leftStickX, KINDA_SMALL_NUMBER) ||
        !FMath::IsNearlyZero(leftStickY, KINDA_SMALL_NUMBER);

    if (bHasLeftStickInput)
    {
        sources.Add(FString::Printf(
            TEXT("ゲームパッドLスティック(X=%0.3f Y=%0.3f)"),
            leftStickX,
            leftStickY
        ));
    }

    // 入力元未検出時はUnknown
    if (sources.Num() == 0)
    {
        return TEXT("Unknown");
    }

    return FString::Join(sources, TEXT(" / "));
}

// ----- 移動リピート制御 -----

/** 押しっぱなし移動のリピートを開始する */
void ARldPlayerController::StartMoveRepeat(
    const FIntPoint& Direction,
    const FString& InputSourceText,
    const FVector2D& Axis
)
{
    UWorld* world = GetWorld();

    // World未取得時は開始しない
    if (!world)
    {
        return;
    }

    RepeatingMoveDirection = Direction;
    RepeatingMoveInputSourceText = InputSourceText;
    RepeatingMoveAxis = Axis;
    bMoveRepeatActive = true;

    // 既存の移動リピートTimerを停止
    world->GetTimerManager().ClearTimer(MoveRepeatStartTimerHandle);
    world->GetTimerManager().ClearTimer(MoveRepeatTickTimerHandle);

    // 初回遅延Timerを開始
    world->GetTimerManager().SetTimer(
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
    UWorld* world = GetWorld();

    // World取得時は移動リピートTimerを停止
    if (world)
    {
        world->GetTimerManager().ClearTimer(MoveRepeatStartTimerHandle);
        world->GetTimerManager().ClearTimer(MoveRepeatTickTimerHandle);
    }

    bMoveRepeatActive = false;
    RepeatingMoveDirection = FIntPoint::ZeroValue;
    RepeatingMoveInputSourceText.Empty();
    RepeatingMoveAxis = FVector2D::ZeroVector;
}

/** 押しっぱなし移動の初回遅延処理を行う */
void ARldPlayerController::BeginMoveRepeat()
{
    UWorld* world = GetWorld();

    // World未取得時は処理しない
    if (!world)
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
    world->GetTimerManager().SetTimer(
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
        Verbose,
        TEXT("TickMoveRepeat: Actor=%s 押しっぱなし入力 入力元=%s リピート方向=(%d,%d) 足踏み入力=%s"),
        *GetNameSafe(this),
        *RepeatingMoveInputSourceText,
        RepeatingMoveDirection.X,
        RepeatingMoveDirection.Y,
        bStepInPlaceModifierPressed ? TEXT("有効") : TEXT("無効")
    );

    ARldPlayerCharacter* playerCharacter = GetRldPlayerCharacter();

    // プレイヤーキャラクター未取得時は処理しない
    if (!playerCharacter)
    {
        UE_LOG(
            LogRldInput,
            Warning,
            TEXT("TickMoveRepeat: Actor=%s PlayerCharacterがnullのためリピートを停止します"),
            *GetNameSafe(this)
        );

        StopMoveRepeat();
        return;
    }

    // 移動入力受付不可時は今回の処理のみしない
    if (!playerCharacter->CanAcceptMoveInput())
    {
        UE_LOG(
            LogRldInput,
            Verbose,
            TEXT("TickMoveRepeat: Actor=%s 移動入力を受け付けないため今回の処理をしません"),
            *GetNameSafe(playerCharacter)
        );

        return;
    }

    // 足踏み入力中は押しっぱなし移動を足踏みの連続実行として処理する
    if (bStepInPlaceModifierPressed)
    {
        playerCharacter->RequestStepInPlaceAction(RepeatingMoveDirection);
        return;
    }

    playerCharacter->RequestMoveDirection(RepeatingMoveDirection);
}

/** 押しっぱなし移動を継続すべきか判定する */
bool ARldPlayerController::ShouldContinueMoveRepeat() const
{
    // 移動リピートが無効な場合は継続しない
    if (!bMoveRepeatActive)
    {
        return false;
    }

    FVector2D currentAxis = FVector2D::ZeroVector;

    // ----- キーボード -----

    if (IsInputKeyDown(EKeys::W))
    {
        currentAxis.Y += 1.0f;
    }

    if (IsInputKeyDown(EKeys::S))
    {
        currentAxis.Y -= 1.0f;
    }

    if (IsInputKeyDown(EKeys::D))
    {
        currentAxis.X += 1.0f;
    }

    if (IsInputKeyDown(EKeys::A))
    {
        currentAxis.X -= 1.0f;
    }

    // ----- ゲームパッド十字キー -----

    if (IsInputKeyDown(EKeys::Gamepad_DPad_Up))
    {
        currentAxis.Y += 1.0f;
    }

    if (IsInputKeyDown(EKeys::Gamepad_DPad_Down))
    {
        currentAxis.Y -= 1.0f;
    }

    if (IsInputKeyDown(EKeys::Gamepad_DPad_Right))
    {
        currentAxis.X += 1.0f;
    }

    if (IsInputKeyDown(EKeys::Gamepad_DPad_Left))
    {
        currentAxis.X -= 1.0f;
    }

    // ----- ゲームパッドLスティック -----

    const float leftStickX = GetInputAnalogKeyState(EKeys::Gamepad_LeftX);
    const float leftStickY = GetInputAnalogKeyState(EKeys::Gamepad_LeftY);

    // アナログ入力がある場合は左スティック入力を優先
    if (!FMath::IsNearlyZero(leftStickX, KINDA_SMALL_NUMBER) ||
        !FMath::IsNearlyZero(leftStickY, KINDA_SMALL_NUMBER))
    {
        currentAxis = FVector2D(leftStickX, leftStickY);
    }

    FIntPoint currentDirection;

    // 入力から方向確定できない場合は継続しない
    if (!TryConvertMoveAxisToGridDir(currentAxis, currentDirection))
    {
        return false;
    }

    return currentDirection == RepeatingMoveDirection;
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

    UWorld* world = GetWorld();

    // World未取得時は処理しない
    if (!world)
    {
        return;
    }

    lastHiddenDebugInputTime = world->GetTimeSeconds();

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

    UWorld* world = GetWorld();

    // World未取得時は処理しない
    if (!world)
    {
        return;
    }

    lastHiddenDebugInputTime = world->GetTimeSeconds();

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
    // 開始入力前はタイムアウト扱いにしない
    if (debugCommandPrefixCount <= 0)
    {
        return false;
    }

    UWorld* world = GetWorld();

    // World未取得時はタイムアウト扱いにしない
    if (!world)
    {
        return false;
    }

    return (world->GetTimeSeconds() - lastHiddenDebugInputTime) > hiddenDebugCommandTimeout;
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
            TEXT("OpenDebugOptionsWidget: Actor=%s DebugOptionsWidgetClass未設定のため表示できません"),
            *GetNameSafe(this)
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
            TEXT("OpenDebugOptionsWidget: Actor=%s DebugOptionsWidget生成に失敗しました"),
            *GetNameSafe(this)
        );

        return;
    }

    ActiveDebugOptionsWidget->AddToViewport(100);
    SetCommonInputMode(ECmnInputMode::Menu);

    UE_LOG(
        LogRldInput,
        Log,
        TEXT("OpenDebugOptionsWidget: Actor=%s デバッグオプションを表示しました"),
        *GetNameSafe(this)
    );
}

/** 表示中のデバッグオプションWidgetを取得する */
URldDebugOptionsWidget* ARldPlayerController::GetActiveDebugOptionsWidget() const
{
    // デバッグオプションWidget未生成または非表示の場合はnullptrを返す
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
        UE_LOG(
            LogRldInput,
            Error,
            TEXT("LoadAndApplyInputConfig: Actor=%s InputConfigAssetがnullのため入力設定をロードできません"),
            *GetNameSafe(this)
        );

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
        TEXT("LoadAndApplyInputConfig: Actor=%s 入力設定ロード完了 名前=%s"),
        *GetNameSafe(this),
        LoadedInputConfig ? *LoadedInputConfig->GetName() : TEXT("None")
    );

    // 入力ルーター取得時は入力設定を適用
    if (InputRouter)
    {
        InputRouter->ApplyConfig(LoadedInputConfig);

        UE_LOG(
            LogRldInput,
            Log,
            TEXT("LoadAndApplyInputConfig: Actor=%s 入力設定適用完了 移動=%s 足踏み=%s 攻撃=%s インタラクト=%s メニュー=%s 視点=%s ズーム=%s UI方向=%s UI決定=%s UI閉じる=%s UIスクロール=%s DebugPrefix=%s DebugKeyboard=%s DebugGamepad=%s"),
            *GetNameSafe(this),
            InputRouter->IA_Move ? TEXT("あり") : TEXT("なし"),
            InputRouter->IA_StepInPlaceModifier ? TEXT("あり") : TEXT("なし"),
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

    UEnhancedInputComponent* newInputComponent = NewObject<UEnhancedInputComponent>(
        this,
        UEnhancedInputComponent::StaticClass(),
        NAME_None,
        RF_Transient
    );

    PushInputComponent(newInputComponent);
    InputComponent = newInputComponent;
}
