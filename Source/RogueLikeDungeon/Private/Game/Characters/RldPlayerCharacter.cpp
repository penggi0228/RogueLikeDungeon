// RldPlayerCharacter.cpp

#include "Game/Characters/RldPlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"

#include "Game/Grid/RldGridManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogRldPlayerCharacter, Log, All);

/** コンストラクタ */
ARldPlayerCharacter::ARldPlayerCharacter()
{
    // ----- カメラコンポーネント -----

    // スプリングアームを持たせることで、回転とズームの責務をCharacter内に閉じ込める
    springArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    springArm->SetupAttachment(RootComponent);
    springArm->bUsePawnControlRotation = false;

    // 実際に見えている向きを入力変換へ使うため、専用CameraComponentを明示的に持つ
    playerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PlayerCamera"));
    playerCamera->SetupAttachment(springArm, USpringArmComponent::SocketName);
    playerCamera->bUsePawnControlRotation = false;
}

/**
 * 開始時処理
 */
void ARldPlayerCharacter::BeginPlay()
{
    Super::BeginPlay();

    // BP側で調整した初期値を実行時に反映するため、開始時にまとめて適用する
    ApplyInitialCameraSettings();

    // レベル依存のグリッド管理は実行時に解決する
    ResolveGridManager();
}

/**
 * 移動方向入力を受け取る
 */
void ARldPlayerCharacter::RequestMoveDirection(const FIntPoint& Direction)
{
    // 入力受付と実移動処理を分離しておくことで、後からターン制御を差し込みやすくする
    HandleMoveRequest(Direction);
}

/**
 * カメラ視点入力を受け取る
 */
void ARldPlayerCharacter::RequestLookInput(const FVector2D& Axis)
{
    // カメラ未生成時は処理継続しても意味がないため早期終了する
    if (!springArm)
    {
        return;
    }

    // 現在回転を基準に加算することで、BP調整値を崩さず相対回転できる
    FRotator newRotation = springArm->GetRelativeRotation();

    // 水平回転
    newRotation.Yaw += Axis.X * cameraYawSpeed;

    // 垂直回転
    newRotation.Pitch = FMath::Clamp(
        newRotation.Pitch + (Axis.Y * cameraPitchSpeed),
        minCameraPitch,
        maxCameraPitch
    );

    // ピッチ制限をここで吸収することで、カメラ破綻を防ぐ
    springArm->SetRelativeRotation(newRotation);

    UE_LOG(
        LogRldPlayerCharacter,
        Verbose,
        TEXT("RequestLookInput: Axis=(%f,%f) NewRot=(P=%f Y=%f R=%f)"),
        Axis.X,
        Axis.Y,
        newRotation.Pitch,
        newRotation.Yaw,
        newRotation.Roll
    );
}

/**
 * カメラズーム入力を受け取る
 */
void ARldPlayerCharacter::RequestZoomInput(float Value)
{
    // カメラ未生成時は処理継続しても意味がないため早期終了する
    if (!springArm)
    {
        return;
    }

    // ズーム範囲を固定しておくことで、見下ろし構図を壊さず調整できる
    const float newTargetArmLength = FMath::Clamp(
        springArm->TargetArmLength + (-Value * zoomSpeed),
        minTargetArmLength,
        maxTargetArmLength
    );

    springArm->TargetArmLength = newTargetArmLength;

    UE_LOG(
        LogRldPlayerCharacter,
        Verbose,
        TEXT("RequestZoomInput: Value=%f TargetArmLength=%f"),
        Value,
        springArm->TargetArmLength
    );
}

/**
 * カメラの平面前方向を取得する
 */
FVector ARldPlayerCharacter::GetCameraPlanarForward() const
{
    // 実際に見えている向きを基準に移動方向を決めるため、Camera優先で取得する
    const USceneComponent* directionSource =
        playerCamera ? static_cast<USceneComponent*>(playerCamera) : static_cast<USceneComponent*>(springArm);

    if (!directionSource)
    {
        return FVector::ForwardVector;
    }

    FVector forward = directionSource->GetForwardVector();
    forward.Z = 0.0f;

    // 真上真下に近い特殊角度でも入力変換が壊れないように退避方向を返す
    if (forward.IsNearlyZero(KINDA_SMALL_NUMBER))
    {
        return FVector::ForwardVector;
    }

    forward.Normalize();
    return forward;
}

/**
 * カメラの平面右方向を取得する
 */
FVector ARldPlayerCharacter::GetCameraPlanarRight() const
{
    // 実際に見えている向きを基準に移動方向を決めるため、Camera優先で取得する
    const USceneComponent* directionSource =
        playerCamera ? static_cast<USceneComponent*>(playerCamera) : static_cast<USceneComponent*>(springArm);

    if (!directionSource)
    {
        return FVector::RightVector;
    }

    FVector right = directionSource->GetRightVector();
    right.Z = 0.0f;

    // 真上真下に近い特殊角度でも入力変換が壊れないように退避方向を返す
    if (right.IsNearlyZero(KINDA_SMALL_NUMBER))
    {
        return FVector::RightVector;
    }

    right.Normalize();
    return right;
}

/**
 * グリッド管理Actorを取得する
 */
void ARldPlayerCharacter::ResolveGridManager()
{
    // 現段階ではレベル上に1個だけ配置する前提のため、最も単純な取得方法を使う
    AActor* foundActor = UGameplayStatics::GetActorOfClass(GetWorld(), ARldGridManager::StaticClass());
    gridManager = Cast<ARldGridManager>(foundActor);

    if (!gridManager)
    {
        UE_LOG(
            LogRldPlayerCharacter,
            Warning,
            TEXT("ResolveGridManager: ARldGridManagerがレベル上に見つからないため、通行判定なしで移動します")
        );
        return;
    }

    UE_LOG(
        LogRldPlayerCharacter,
        Log,
        TEXT("ResolveGridManager: GridManager=%s"),
        *gridManager->GetName()
    );
}

/**
 * 移動入力を処理する
 */
void ARldPlayerCharacter::HandleMoveRequest(const FIntPoint& Direction)
{
    // 次グリッド座標を先に求めることで、移動可否判定と座標更新を分離しやすくする
    const FIntPoint nextGridCoord = CurrentGridCoord + Direction;

    UE_LOG(
        LogRldPlayerCharacter,
        Log,
        TEXT("HandleMoveRequest: Current=(%d,%d) Direction=(%d,%d) Next=(%d,%d)"),
        CurrentGridCoord.X,
        CurrentGridCoord.Y,
        Direction.X,
        Direction.Y,
        nextGridCoord.X,
        nextGridCoord.Y
    );

    // GridManager未解決時は開発初期の動作確認を優先し、警告を出したうえで移動だけ許可する
    if (!gridManager)
    {
        UE_LOG(
            LogRldPlayerCharacter,
            Warning,
            TEXT("HandleMoveRequest: GridManager未取得のため通行判定を行わず移動します Next=(%d,%d)"),
            nextGridCoord.X,
            nextGridCoord.Y
        );

        SetCurrentGridCoord(nextGridCoord);
        return;
    }

    // 範囲外判定と壁判定を分けて出すことで、移動失敗理由をログから追いやすくする
    if (!gridManager->IsInsideGrid(nextGridCoord))
    {
        UE_LOG(
            LogRldPlayerCharacter,
            Log,
            TEXT("HandleMoveRequest: 範囲外のため移動を中止します Next=(%d,%d)"),
            nextGridCoord.X,
            nextGridCoord.Y
        );
        return;
    }

    if (gridManager->IsWallCell(nextGridCoord))
    {
        UE_LOG(
            LogRldPlayerCharacter,
            Log,
            TEXT("HandleMoveRequest: 壁マスのため移動を中止します Next=(%d,%d)"),
            nextGridCoord.X,
            nextGridCoord.Y
        );
        return;
    }

    // 最終的に通行可能と判断できた場合のみ座標を更新する
    if (!gridManager->IsWalkable(nextGridCoord))
    {
        UE_LOG(
            LogRldPlayerCharacter,
            Warning,
            TEXT("HandleMoveRequest: 通行不可のため移動を中止します Next=(%d,%d)"),
            nextGridCoord.X,
            nextGridCoord.Y
        );
        return;
    }

    UE_LOG(
        LogRldPlayerCharacter,
        Log,
        TEXT("HandleMoveRequest: 移動成功 Next=(%d,%d)"),
        nextGridCoord.X,
        nextGridCoord.Y
    );

    SetCurrentGridCoord(nextGridCoord);
}

/**
 * 初期カメラ設定を適用する
 */
void ARldPlayerCharacter::ApplyInitialCameraSettings()
{
    // カメラ未生成時は設定適用先が存在しないため何もしない
    if (!springArm)
    {
        return;
    }

    // BP派生側で調整した値を実行時に確実に反映する
    springArm->TargetArmLength = initialTargetArmLength;
    springArm->bDoCollisionTest = bEnableCameraCollisionTest;
    springArm->SetRelativeRotation(FRotator(initialCameraPitch, initialCameraYaw, 0.0f));

    UE_LOG(
        LogRldPlayerCharacter,
        Log,
        TEXT("ApplyInitialCameraSettings: ArmLength=%f Pitch=%f Yaw=%f Collision=%d"),
        springArm->TargetArmLength,
        initialCameraPitch,
        initialCameraYaw,
        bEnableCameraCollisionTest ? 1 : 0
    );
}
