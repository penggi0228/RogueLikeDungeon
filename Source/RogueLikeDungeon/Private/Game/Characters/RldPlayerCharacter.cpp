// RldPlayerCharacter.cpp

#include "Game/Characters/RldPlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogRldPlayerCharacter, Log, All);

/** コンストラクタ */
ARldPlayerCharacter::ARldPlayerCharacter()
{
    // ----- Camera Components -----

    // スプリングアーム生成
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(RootComponent);
    SpringArm->bUsePawnControlRotation = false;
    
    // カメラ生成
    PlayerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PlayerCamera"));
    PlayerCamera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
    PlayerCamera->bUsePawnControlRotation = false;
}

/**
 * 開始時処理
 */
void ARldPlayerCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Blueprint派生側で調整した初期カメラ設定を反映
    ApplyInitialCameraSettings();
}

/**
 * 移動方向入力を受け取る
 */
void ARldPlayerCharacter::RequestMoveDirection(const FIntPoint& Direction)
{
    // 実際の移動処理は専用関数へ委譲
    HandleMoveRequest(Direction);
}

/**
 * カメラ視点入力を受け取る
 */
void ARldPlayerCharacter::RequestLookInput(const FVector2D& Axis)
{
    // SpringArm未生成なら何もしない
    if (!SpringArm)
    {
        return;
    }

    // 現在回転を取得
    FRotator NewRotation = SpringArm->GetRelativeRotation();

    // 水平回転
    NewRotation.Yaw += Axis.X * CameraYawSpeed;

    // 垂直回転
    NewRotation.Pitch = FMath::Clamp(
        NewRotation.Pitch + (Axis.Y * CameraPitchSpeed),
        MinCameraPitch,
        MaxCameraPitch
    );

    // 回転反映
    SpringArm->SetRelativeRotation(NewRotation);

    UE_LOG(
        LogRldPlayerCharacter,
        Verbose,
        TEXT("RequestLookInput: Axis(X=%f Y=%f) NewRot(P=%f Y=%f R=%f)"),
        Axis.X,
        Axis.Y,
        NewRotation.Pitch,
        NewRotation.Yaw,
        NewRotation.Roll
    );
}

/**
 * カメラズーム入力を受け取る
 */
void ARldPlayerCharacter::RequestZoomInput(float Value)
{
    // SpringArm未生成なら何もしない
    if (!SpringArm)
    {
        return;
    }

    // 新しいアーム長を計算
    const float NewTargetArmLength = FMath::Clamp(
        SpringArm->TargetArmLength + (-Value * ZoomSpeed),
        MinTargetArmLength,
        MaxTargetArmLength
    );

    // ズーム反映
    SpringArm->TargetArmLength = NewTargetArmLength;

    UE_LOG(
        LogRldPlayerCharacter,
        Verbose,
        TEXT("RequestZoomInput: Value=%f TargetArmLength=%f"),
        Value,
        SpringArm->TargetArmLength
    );
}

/**
 * カメラの平面前方向を取得する
 */
FVector ARldPlayerCharacter::GetCameraPlanarForward() const
{
    // Camera優先で取得
    const USceneComponent* DirectionSource = PlayerCamera ? static_cast<USceneComponent*>(PlayerCamera) : static_cast<USceneComponent*>(SpringArm);
    if (!DirectionSource)
    {
        return FVector::ForwardVector;
    }

    FVector Forward = DirectionSource->GetForwardVector();
    Forward.Z = 0.0f;

    if (Forward.IsNearlyZero(KINDA_SMALL_NUMBER))
    {
        return FVector::ForwardVector;
    }

    Forward.Normalize();
    return Forward;
}

/**
 * カメラの平面右方向を取得する
 */
FVector ARldPlayerCharacter::GetCameraPlanarRight() const
{
    // Camera優先で取得
    const USceneComponent* DirectionSource = PlayerCamera ? static_cast<USceneComponent*>(PlayerCamera) : static_cast<USceneComponent*>(SpringArm);
    if (!DirectionSource)
    {
        return FVector::RightVector;
    }

    FVector Right = DirectionSource->GetRightVector();
    Right.Z = 0.0f;

    if (Right.IsNearlyZero(KINDA_SMALL_NUMBER))
    {
        return FVector::RightVector;
    }

    Right.Normalize();
    return Right;
}

/**
 * 移動入力を処理する
 */
void ARldPlayerCharacter::HandleMoveRequest(const FIntPoint& Direction)
{
    // 次のグリッド座標を計算
    const FIntPoint NextGridCoord = CurrentGridCoord + Direction;

    UE_LOG(
        LogRldPlayerCharacter,
        Log,
        TEXT("HandleMoveRequest: Current(%d, %d) Direction(%d, %d) Next(%d, %d)"),
        CurrentGridCoord.X,
        CurrentGridCoord.Y,
        Direction.X,
        Direction.Y,
        NextGridCoord.X,
        NextGridCoord.Y
    );

    // 現段階では通行可否判定なしで更新
    SetCurrentGridCoord(NextGridCoord);
}

/**
 * 初期カメラ設定を適用する
 */
void ARldPlayerCharacter::ApplyInitialCameraSettings()
{
    // SpringArm未生成なら何もしない
    if (!SpringArm)
    {
        return;
    }

    // Blueprint派生側で調整した値を反映
    SpringArm->TargetArmLength = InitialTargetArmLength;
    SpringArm->bDoCollisionTest = bEnableCameraCollisionTest;
    SpringArm->SetRelativeRotation(FRotator(InitialCameraPitch, InitialCameraYaw, 0.0f));

    UE_LOG(
        LogRldPlayerCharacter,
        Log,
        TEXT("ApplyInitialCameraSettings: ArmLength=%f Pitch=%f Yaw=%f Collision=%d"),
        SpringArm->TargetArmLength,
        InitialCameraPitch,
        InitialCameraYaw,
        bEnableCameraCollisionTest ? 1 : 0
    );
}
