// RldPlayerController.cpp

#include "Game/Input/RldPlayerController.h"

#include "Common/Input/CmnInputRouter.h"
#include "Common/Input/CmnInputConfig.h"
#include "Common/Input/CmnInputOptionsSave.h"
#include "Common/Save/CmnSaveGameLibrary.h"
#include "Common/Settings/CmnSettingsSubsystem.h"
#include "Engine/GameInstance.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

DEFINE_LOG_CATEGORY_STATIC(LogRldInput, Log, All);


ARldPlayerController::ARldPlayerController()
{
    // Routerは早期確保（GC対策兼用）
    InputRouter = CreateDefaultSubobject<UCmnInputRouter>(TEXT("InputRouter"));

    // 入力設定DataAssetのデフォルト参照
    // ※必ずエディタのCopy Referenceと一致させる
    InputConfigAsset = TSoftObjectPtr<UCmnInputConfig>(
        FSoftObjectPath(TEXT("/Game/Game/Input/DA_CmnInputConfig_Default.DA_CmnInputConfig_Default"))
    );
}

// ----- AActor -----

/**
 * コンポーネント初期化後に呼ばれる
 * Bind時にIA参照がnullにならないよう、
 * SetupInputComponentより前に入力設定(DataAsset)をロードしてRouterへ適用する
 */
void ARldPlayerController::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    LoadAndApplyInputConfig();
}

/**
 * プレイ開始時に呼ばれる
 * 入力設定(DataAsset)が未適用なら適用
 * 入力オプション(反転など)をSaveGameからロード
 * EnhancedInput Subsystem取得後にRouterをInitialize
 * 初期入力モード(Game)を適用してIMCを反映
 */
void ARldPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // 念のため:まだApplyConfigされていないケースに備える
    if (!LoadedInputConfig)
    {
        LoadAndApplyInputConfig();
    }

    // 反転などのオプションはController側の状態として先に確定させる
    LoadInputOptions();

    ULocalPlayer* LP = GetLocalPlayer();
    UEnhancedInputLocalPlayerSubsystem* Subsys = LP ? LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>() : nullptr;

    if (!InputRouter || !Subsys)
    {
        UE_LOG(LogRldInput, Warning, TEXT("BeginPlay: InputRouter or Subsys is NULL"));
        return;
    }

    // Subsystemが取れたタイミングでInitialize
    InputRouter->Initialize(this, Subsys);

    // 初期モード(ここでIMC適用される)
    InputRouter->SetInputMode(ECmnInputMode::Game);
}

// ----- AController -----

/**
 * InputComponent生成後に呼ばれる
 * EnhancedInputComponentへ差し替えた上でIAをBind
 * IMCの適用はBeginPlay → RouterのSetInputMode側へ委譲
 */
void ARldPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    // InputComponentClassは使わない前提なので、ここで確実にEnhancedへ差し替える
    EnsureEnhancedInputComponent();

   // ここは「Bindするだけ」。IMC適用はBeginPlay側でやる
    UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent);
    if (!EIC || !InputRouter)
    {
        UE_LOG(LogRldInput, Warning, TEXT("SetupInputComponent: EIC or InputRouter is NULL (InputComponent is not Enhanced?)"));
        return;
    }

    // ----- Bind: Game -----

    // ゲーム移動(押した瞬間=1ターン)
    if (InputRouter->IA_Move)
    {
        EIC->BindAction(InputRouter->IA_Move, ETriggerEvent::Started, this, &ARldPlayerController::OnMoveStarted);
    }

    // カメラ視点
    if (InputRouter->IA_CameraLook)
    {
        EIC->BindAction(InputRouter->IA_CameraLook, ETriggerEvent::Triggered, this, &ARldPlayerController::OnCameraLookTriggered);
    }

    // カメラズーム
    if (InputRouter->IA_CameraZoom)
    {
        EIC->BindAction(InputRouter->IA_CameraZoom, ETriggerEvent::Triggered, this, &ARldPlayerController::OnCameraZoomTriggered);
    }

    // ----- Bind: UI -----

    // UI方向入力(保持入力を受け続ける:リピートはRouter内Timerが担当)
    if (InputRouter->IA_UI_Direction)
    {
        EIC->BindAction(InputRouter->IA_UI_Direction, ETriggerEvent::Triggered, this, &ARldPlayerController::OnUIDirectionTriggered);
    }

    // UIスクロール(保持入力を受け続ける:リピートはRouter内Timerが担当)
    if (InputRouter->IA_UI_Scroll)
    {
        EIC->BindAction(InputRouter->IA_UI_Scroll, ETriggerEvent::Triggered, this, &ARldPlayerController::OnUIScrollTriggered);
    }
}

// ----- Public API -----

/**
 * 共通入力モードを切り替える
 * ・IMCの適用/解除
 * ・InputMode(GameOnly / GameAndUI / UIOnly)
 * ・カーソル表示
 * はRouter側で一括制御する
 */
void ARldPlayerController::SetCommonInputMode(ECmnInputMode Mode)
{
    if (InputRouter)
    {
        InputRouter->SetInputMode(Mode);
    }
}

/**
 * カメラ左右反転を設定する(即時反映)
 * ・基本は設定のみで、保存は「適用」ボタンでまとめて行う想定
 */
void ARldPlayerController::SetInvertCameraX(bool bInvert, bool bSaveImmediately)
{
    bInvertCameraX = bInvert;

    UE_LOG(LogRldInput, Log, TEXT("SetInvertCameraX: %d (SaveImmediately=%d)"),
        bInvertCameraX ? 1 : 0,
        bSaveImmediately ? 1 : 0);

    if (bSaveImmediately)
    {
        SaveInputOptions();
    }
}

/**
 * カメラ上下反転を設定する(即時反映)
 * ・基本は設定のみで、保存は「適用」ボタンでまとめて行う想定
 */
void ARldPlayerController::SetInvertCameraY(bool bInvert, bool bSaveImmediately)
{
    bInvertCameraY = bInvert;

    UE_LOG(LogRldInput, Log, TEXT("SetInvertCameraY: %d (SaveImmediately=%d)"),
        bInvertCameraY ? 1 : 0,
        bSaveImmediately ? 1 : 0);

    if (bSaveImmediately)
    {
        SaveInputOptions();
    }
}

/**
 * 現在の反転設定をSaveGameへ保存する(オプション画面の「適用」用)。
 */
void ARldPlayerController::ApplyAndSaveInputOptions()
{
    SaveInputOptions();
}

/**
 * SaveGameから反転設定を読み直して反映する(オプション画面の「破棄/戻す」用)。
 */
void ARldPlayerController::ReloadInputOptions()
{
    LoadInputOptions();

    UE_LOG(LogRldInput, Log, TEXT("ReloadInputOptions: InvertX=%d InvertY=%d"),
        bInvertCameraX ? 1 : 0,
        bInvertCameraY ? 1 : 0);
}

// ----- Private -----

/**
 * SetupInputComponentでのBind前にIA参照を揃えるため、
 * 入力設定のDataAssetをロードしてRouterへ適用
 * ソフト参照なので同期ロードで間に合わせる
 */
void ARldPlayerController::LoadAndApplyInputConfig()
{
    LoadedInputConfig = nullptr;

    if (InputConfigAsset.IsNull())
    {
        if (InputRouter)
        {
            InputRouter->ApplyConfig(nullptr);
        }
        return;
    }

    // 同期ロード(SetupInputComponent前に間に合わせるため)
    LoadedInputConfig = InputConfigAsset.LoadSynchronous();

    if (InputRouter)
    {
        InputRouter->ApplyConfig(LoadedInputConfig);
    }
}

/**
 * InputComponentがEnhancedInputComponentでない場合に差し替える
 * UE5.3ではInputComponentClassを前提に固定できないケースがあるため
 * InputStackから既存を外して、Enhancedを生成して積み直す
 */
void ARldPlayerController::EnsureEnhancedInputComponent()
{
    // すでにEnhancedなら何もしない
    if (Cast<UEnhancedInputComponent>(InputComponent))
    {
        return;
    }

    // 既存InputComponentをInputStackから外す
    if (InputComponent)
    {
        PopInputComponent(InputComponent);
        InputComponent = nullptr;
    }

    // EnhancedInputComponentを生成してInputStackへ積む
    UEnhancedInputComponent* NewInputComponent = NewObject<UEnhancedInputComponent>(
        this, UEnhancedInputComponent::StaticClass(), NAME_None, RF_Transient
    );

    // PlayerControllerのInputStackへ追加(重複スタック回避)
    PushInputComponent(NewInputComponent);

    // メンバへセット
    InputComponent = NewInputComponent;
}

// ----- Input Handlers -----

/**
 * ゲーム移動入力(Axis2D)の開始(Started)で呼ばれる
 * ・1入力=1ターン想定なのでStartedで拾う
 * ・Axis2Dを4方向へ変換してゲーム側へ渡す
 */
void ARldPlayerController::OnMoveStarted(const FInputActionValue& Value)
{
    // ゲームモード以外では無効
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

    // TODO: TurnManagerへ方向を渡す
    UE_LOG(LogRldInput, Log, TEXT("Move: %d, %d"), Dir.X, Dir.Y);
}

/**
 * カメラ視点入力(Axis2D)のTriggeredで呼ばれる
 * 反転設定(オプション)をController側で適用する
 * 最終的なカメラ制御への入力はここで確定させる(IMCのModifierに依存しない)
 */
void ARldPlayerController::OnCameraLookTriggered(const FInputActionValue& Value)
{
    if (!InputRouter || !InputRouter->IsGameMode())
    {
        return;
    }

    const FVector2D Raw = Value.Get<FVector2D>();
    FVector2D Axis = Raw;

    UCmnSettingsSubsystem* Settings = GetGameInstance() ? GetGameInstance()->GetSubsystem<UCmnSettingsSubsystem>() : nullptr;
    if (Settings)
    {
        if (Settings->GetInvertCameraX())
        {
            Axis.X *= -1.0f;
        }
        if (Settings->GetInvertCameraY())
        {
            Axis.Y *= -1.0f;
        }
    }

    UE_LOG(LogRldInput, Log, TEXT("CameraLook Raw: %f, %f | AfterInvert: %f, %f"), Raw.X, Raw.Y, Axis.X, Axis.Y);

    // TODO: カメラ制御に渡す
}

/**
 * カメラズーム入力(Axis1D)のTriggeredで呼ばれる
 * ・L/R2やマウスホイールなど、ズーム入力を統一して受け取る
 */
void ARldPlayerController::OnCameraZoomTriggered(const FInputActionValue& Value)
{
    if (!InputRouter || !InputRouter->IsGameMode())
    {
        return;
    }

    const float Zoom = Value.Get<float>();
    UE_LOG(LogRldInput, Log, TEXT("CameraZoom Axis:%f"), Zoom);

    // TODO: カメラ制御に渡す
}

/**
 * UI方向入力(Axis2D)のTriggeredで呼ばれる
 * ・Menu/Dialogモードのみ有効
 * ・4方向化とリピート制御はRouterに委譲する
 */
void ARldPlayerController::OnUIDirectionTriggered(const FInputActionValue& Value)
{
    // Menu / Dialogモードのみ有効
    if (!InputRouter || (!InputRouter->IsMenuMode() && !InputRouter->IsDialogMode()))
    {
        return;
    }

    const FVector2D Axis = Value.Get<FVector2D>();
    InputRouter->HandleUIDirectionAxis(Axis);
}

/**
 * UIスクロール入力(Axis1D)のTriggeredで呼ばれる
 * ・Menu/Dialogモードのみ有効
 * ・スクロールのしきい値判定などはRouterに委譲する
 */
void ARldPlayerController::OnUIScrollTriggered(const FInputActionValue& Value)
{
    // Menu / Dialogモードのみ有効
    if (!InputRouter || (!InputRouter->IsMenuMode() && !InputRouter->IsDialogMode()))
    {
        return;
    }

    const float Amount = Value.Get<float>();
    InputRouter->HandleUIScrollAxis(Amount);
}

// ----- Save: Input Options -----

/**
 * 入力オプション(反転など)をSaveGameからロードしてControllerへ反映する
 * スロットが無い場合は新規作成されたデータが返る(LoadOrCreate)
 * 初期値はSaveGame側の値が優先される
 */
void ARldPlayerController::LoadInputOptions()
{
    CachedInputOptions = Cast<UCmnInputOptionsSave>(
        UCmnSaveGameLibrary::LoadOrCreate(UCmnInputOptionsSave::StaticClass(), InputOptionsSlotName, InputOptionsUserIndex)
    );

    if (!CachedInputOptions)
    {
        UE_LOG(LogRldInput, Warning, TEXT("LoadInputOptions: Failed to load/create save. Slot=%s UserIndex=%d"),
            *InputOptionsSlotName, InputOptionsUserIndex);
        return;
    }

    bInvertCameraX = CachedInputOptions->bInvertCameraX;
    bInvertCameraY = CachedInputOptions->bInvertCameraY;

    UE_LOG(LogRldInput, Log, TEXT("LoadInputOptions: InvertX=%d InvertY=%d"),
        bInvertCameraX ? 1 : 0,
        bInvertCameraY ? 1 : 0);
}

/**
 * 入力オプション(反転など)をSaveGameへ保存する
 * ・Cachedが無い場合も保存できるようにLoadOrCreateで確保する
 * ・Controllerの現在値をSaveGameへ書き戻して保存する
 */
void ARldPlayerController::SaveInputOptions()
{
    if (!CachedInputOptions)
    {
        CachedInputOptions = Cast<UCmnInputOptionsSave>(
            UCmnSaveGameLibrary::LoadOrCreate(UCmnInputOptionsSave::StaticClass(), InputOptionsSlotName, InputOptionsUserIndex)
        );
    }

    if (!CachedInputOptions)
    {
        return;
    }

    CachedInputOptions->bInvertCameraX = bInvertCameraX;
    CachedInputOptions->bInvertCameraY = bInvertCameraY;

    const bool bOK = UCmnSaveGameLibrary::Save(CachedInputOptions, InputOptionsSlotName, InputOptionsUserIndex);

    UE_LOG(LogRldInput, Log, TEXT("SaveInputOptions: OK=%d InvertX=%d InvertY=%d"),
        bOK ? 1 : 0,
        bInvertCameraX ? 1 : 0,
        bInvertCameraY ? 1 : 0);
}