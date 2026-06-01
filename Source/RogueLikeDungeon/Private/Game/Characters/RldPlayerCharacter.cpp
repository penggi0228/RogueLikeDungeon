// RldPlayerCharacter.cpp

#include "Game/Characters/RldPlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"

#include "Game/Floor/RldFloorManager.h"
#include "Game/Grid/RldGridManager.h"
#include "Game/Turn/RldTurnManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogRldPlayerCharacter, Log, All);

/** プレイヤーキャラクターを初期化する */
ARldPlayerCharacter::ARldPlayerCharacter()
{
    // ----- カメラコンポーネント -----

    springArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    springArm->SetupAttachment(RootComponent);
    springArm->bUsePawnControlRotation = false;

    playerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PlayerCamera"));
    playerCamera->SetupAttachment(springArm, USpringArmComponent::SocketName);
    playerCamera->bUsePawnControlRotation = false;
}

/** 開始時処理 */
void ARldPlayerCharacter::BeginPlay()
{
    Super::BeginPlay();

    // 初期カメラ設定を適用
    ApplyInitialCameraSettings();

    // 各管理Actorを取得
    ResolveGridManager();
    ResolveTurnManager();
    ResolveFloorManager();

    // 現在位置から初期グリッド座標を取得
    if (gridManager)
    {
        const FVector currentWorldLocation = GetActorLocation();
        const FIntPoint initialGridCoord = gridManager->WorldToGrid(currentWorldLocation);
        SetCurrentGridCoord(initialGridCoord);

        // グリッド中心位置へ補正
        const FVector snappedWorldLocation = gridManager->GridToWorld(initialGridCoord);
        SetActorLocation(snappedWorldLocation);

        UE_LOG(
            LogRldPlayerCharacter,
            Log,
            TEXT("BeginPlay: 初期ワールド座標=(%f,%f,%f) 初期グリッド座標=(%d,%d) 補正後ワールド座標=(%f,%f,%f)"),
            currentWorldLocation.X,
            currentWorldLocation.Y,
            currentWorldLocation.Z,
            initialGridCoord.X,
            initialGridCoord.Y,
            snappedWorldLocation.X,
            snappedWorldLocation.Y,
            snappedWorldLocation.Z
        );

        // 初期配置時の占有登録はFloorManager側で行う
        // ここで再登録すると、開始時に二重登録で警告が出る
        if (floorManager)
        {
            UE_LOG(
                LogRldPlayerCharacter,
                Verbose,
                TEXT("BeginPlay: 初期位置の占有登録はFloorManagerに委譲します グリッド座標=(%d,%d)"),
                initialGridCoord.X,
                initialGridCoord.Y
            );
        }
        else
        {
            UE_LOG(
                LogRldPlayerCharacter,
                Verbose,
                TEXT("BeginPlay: FloorManager未取得のため占有登録は行いません グリッド座標=(%d,%d)"),
                initialGridCoord.X,
                initialGridCoord.Y
            );
        }
    }
}

/** 移動方向入力を受け取る */
void ARldPlayerCharacter::RequestMoveDirection(const FIntPoint& Direction)
{
    HandleMoveRequest(Direction);
}

/** カメラ視点入力を受け取る */
void ARldPlayerCharacter::RequestLookInput(const FVector2D& Axis)
{
    // スプリングアーム未生成時は処理しない
    if (!springArm)
    {
        return;
    }

    FRotator newRotation = springArm->GetRelativeRotation();

    // 水平回転を反映
    newRotation.Yaw += Axis.X * cameraYawSpeed;

    // 垂直回転を反映
    newRotation.Pitch = FMath::Clamp(
        newRotation.Pitch + (Axis.Y * cameraPitchSpeed),
        minCameraPitch,
        maxCameraPitch
    );

    springArm->SetRelativeRotation(newRotation);

    UE_LOG(
        LogRldPlayerCharacter,
        Verbose,
        TEXT("RequestLookInput: 入力値=(%f,%f) 回転値=(ピッチ=%f ヨー=%f ロール=%f)"),
        Axis.X,
        Axis.Y,
        newRotation.Pitch,
        newRotation.Yaw,
        newRotation.Roll
    );
}

/** カメラズーム入力を受け取る */
void ARldPlayerCharacter::RequestZoomInput(float Value)
{
    // スプリングアーム未生成時は処理しない
    if (!springArm)
    {
        return;
    }

    // アーム長をズーム範囲内に補正
    const float newTargetArmLength = FMath::Clamp(
        springArm->TargetArmLength + (-Value * zoomSpeed),
        minTargetArmLength,
        maxTargetArmLength
    );

    springArm->TargetArmLength = newTargetArmLength;

    UE_LOG(
        LogRldPlayerCharacter,
        Verbose,
        TEXT("RequestZoomInput: 入力値=%f アーム長=%f"),
        Value,
        springArm->TargetArmLength
    );
}

/** 待機行動を実行する */
void ARldPlayerCharacter::RequestWaitAction()
{
    HandleWaitRequest();
}

/** 通常攻撃行動を実行する */
void ARldPlayerCharacter::RequestAttackAction()
{
    HandleAttackRequest();
}

/** インタラクト行動を実行する */
void ARldPlayerCharacter::RequestInteractAction()
{
    HandleInteractRequest();
}

/** カメラの平面前方向を取得する */
FVector ARldPlayerCharacter::GetCameraPlanarForward() const
{
    // カメラ優先で方向取得元を決定
    const USceneComponent* directionSource =
        playerCamera ? static_cast<USceneComponent*>(playerCamera) : static_cast<USceneComponent*>(springArm);

    if (!directionSource)
    {
        return FVector::ForwardVector;
    }

    FVector forward = directionSource->GetForwardVector();
    forward.Z = 0.0f;

    // 平面方向がほぼゼロの場合は前方向を返す
    if (forward.IsNearlyZero(KINDA_SMALL_NUMBER))
    {
        return FVector::ForwardVector;
    }

    forward.Normalize();
    return forward;
}

/** カメラの平面右方向を取得する */
FVector ARldPlayerCharacter::GetCameraPlanarRight() const
{
    // カメラ優先で方向取得元を決定
    const USceneComponent* directionSource =
        playerCamera ? static_cast<USceneComponent*>(playerCamera) : static_cast<USceneComponent*>(springArm);

    if (!directionSource)
    {
        return FVector::RightVector;
    }

    FVector right = directionSource->GetRightVector();
    right.Z = 0.0f;

    // 平面方向がほぼゼロの場合は右方向を返す
    if (right.IsNearlyZero(KINDA_SMALL_NUMBER))
    {
        return FVector::RightVector;
    }

    right.Normalize();
    return right;
}

/** グリッド管理Actorを取得する */
void ARldPlayerCharacter::ResolveGridManager()
{
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
        TEXT("ResolveGridManager: 名前=%s クラス=%s"),
        *gridManager->GetName(),
        *gridManager->GetClass()->GetName()
    );
}

/** ターン管理Actorを取得する */
void ARldPlayerCharacter::ResolveTurnManager()
{
    AActor* foundActor = UGameplayStatics::GetActorOfClass(GetWorld(), ARldTurnManager::StaticClass());
    turnManager = Cast<ARldTurnManager>(foundActor);

    if (!turnManager)
    {
        UE_LOG(
            LogRldPlayerCharacter,
            Warning,
            TEXT("ResolveTurnManager: ARldTurnManagerがレベル上に見つからないため、ターン進行なしで処理します")
        );
        return;
    }

    UE_LOG(
        LogRldPlayerCharacter,
        Log,
        TEXT("ResolveTurnManager: 名前=%s クラス=%s 現在ターン数=%d"),
        *turnManager->GetName(),
        *turnManager->GetClass()->GetName(),
        turnManager->GetCurrentTurnIndex()
    );
}

/** フロア管理Actorを取得する */
void ARldPlayerCharacter::ResolveFloorManager()
{
    AActor* foundActor = UGameplayStatics::GetActorOfClass(GetWorld(), ARldFloorManager::StaticClass());
    floorManager = Cast<ARldFloorManager>(foundActor);

    if (!floorManager)
    {
        UE_LOG(
            LogRldPlayerCharacter,
            Warning,
            TEXT("ResolveFloorManager: ARldFloorManagerがレベル上に見つからないため、階段遷移なしで処理します")
        );
        return;
    }

    UE_LOG(
        LogRldPlayerCharacter,
        Log,
        TEXT("ResolveFloorManager: 名前=%s クラス=%s 現在フロア番号=%d"),
        *floorManager->GetName(),
        *floorManager->GetClass()->GetName(),
        floorManager->GetCurrentFloorIndex()
    );
}

/** 移動入力を処理する */
void ARldPlayerCharacter::HandleMoveRequest(const FIntPoint& Direction)
{
    const FIntPoint currentCoord = GetCurrentGridCoord();
    const FIntPoint nextGridCoord = currentCoord + Direction;

    UE_LOG(
        LogRldPlayerCharacter,
        Log,
        TEXT("HandleMoveRequest: 現在座標=(%d,%d) 入力方向=(%d,%d) 次座標=(%d,%d)"),
        currentCoord.X,
        currentCoord.Y,
        Direction.X,
        Direction.Y,
        nextGridCoord.X,
        nextGridCoord.Y
    );

    // GridManager未取得時は通行判定なしで移動
    if (!gridManager)
    {
        UE_LOG(
            LogRldPlayerCharacter,
            Warning,
            TEXT("HandleMoveRequest: GridManager未取得のため通行判定を行わず移動します 次座標=(%d,%d)"),
            nextGridCoord.X,
            nextGridCoord.Y
        );

        SetCurrentGridCoord(nextGridCoord);

        if (turnManager)
        {
            const int32 newTurnIndex = turnManager->AdvanceTurn();

            UE_LOG(
                LogRldPlayerCharacter,
                Log,
                TEXT("HandleMoveRequest: 移動成功によりターンを進めました 現在ターン数=%d"),
                newTurnIndex
            );
        }

        return;
    }

    // 範囲外マスへの移動は中止
    if (!gridManager->IsInsideGrid(nextGridCoord))
    {
        UE_LOG(
            LogRldPlayerCharacter,
            Log,
            TEXT("HandleMoveRequest: 範囲外のため移動を中止します 次座標=(%d,%d)"),
            nextGridCoord.X,
            nextGridCoord.Y
        );
        return;
    }

    // 壁マスへの移動は中止
    if (gridManager->IsWallCell(nextGridCoord))
    {
        UE_LOG(
            LogRldPlayerCharacter,
            Log,
            TEXT("HandleMoveRequest: 壁マスのため移動を中止します 次座標=(%d,%d)"),
            nextGridCoord.X,
            nextGridCoord.Y
        );
        return;
    }

    // 通行不可マスへの移動は中止
    if (!gridManager->IsWalkable(nextGridCoord))
    {
        UE_LOG(
            LogRldPlayerCharacter,
            Warning,
            TEXT("HandleMoveRequest: 通行不可のため移動を中止します 次座標=(%d,%d)"),
            nextGridCoord.X,
            nextGridCoord.Y
        );
        return;
    }

    // 占有情報を移動
    if (!gridManager->MoveOccupant(currentCoord, nextGridCoord, this))
    {
        UE_LOG(
            LogRldPlayerCharacter,
            Warning,
            TEXT("HandleMoveRequest: 占有情報の移動に失敗したため移動を中止します 現在座標=(%d,%d) 次座標=(%d,%d)"),
            currentCoord.X,
            currentCoord.Y,
            nextGridCoord.X,
            nextGridCoord.Y
        );
        return;
    }

    SetCurrentGridCoord(nextGridCoord);

    const FVector newWorldLocation = gridManager->GridToWorld(nextGridCoord);
    SetActorLocation(newWorldLocation);

    UE_LOG(
        LogRldPlayerCharacter,
        Log,
        TEXT("HandleMoveRequest: 移動成功 次座標=(%d,%d) ワールド座標=(%f,%f,%f)"),
        nextGridCoord.X,
        nextGridCoord.Y,
        newWorldLocation.X,
        newWorldLocation.Y,
        newWorldLocation.Z
    );

    // 階段マス到達時は次フロアへ進む
    if (floorManager && gridManager->IsStairsCell(nextGridCoord))
    {
        UE_LOG(
            LogRldPlayerCharacter,
            Log,
            TEXT("HandleMoveRequest: 階段到達により次フロアへ進みます グリッド座標=(%d,%d)"),
            nextGridCoord.X,
            nextGridCoord.Y
        );

        floorManager->GoToNextFloor();
        return;
    }

    // TurnManager未取得時はターン進行しない
    if (!turnManager)
    {
        UE_LOG(
            LogRldPlayerCharacter,
            Warning,
            TEXT("HandleMoveRequest: TurnManager未取得のためターン進行を行いません")
        );
        return;
    }

    const int32 newTurnIndex = turnManager->AdvanceTurn();

    UE_LOG(
        LogRldPlayerCharacter,
        Log,
        TEXT("HandleMoveRequest: ターン進行完了 現在ターン数=%d"),
        newTurnIndex
    );
}

/** 待機行動を処理する */
void ARldPlayerCharacter::HandleWaitRequest()
{
    UE_LOG(
        LogRldPlayerCharacter,
        Log,
        TEXT("HandleWaitRequest: 待機行動を実行します")
    );

    // TurnManager未取得時はターン進行しない
    if (!turnManager)
    {
        UE_LOG(
            LogRldPlayerCharacter,
            Warning,
            TEXT("HandleWaitRequest: TurnManager未取得のためターン進行を行いません")
        );
        return;
    }

    const int32 newTurnIndex = turnManager->AdvanceTurn();

    UE_LOG(
        LogRldPlayerCharacter,
        Log,
        TEXT("HandleWaitRequest: 待機によりターンを進めました 現在ターン数=%d"),
        newTurnIndex
    );
}

/** 通常攻撃行動を処理する */
void ARldPlayerCharacter::HandleAttackRequest()
{
    UE_LOG(
        LogRldPlayerCharacter,
        Log,
        TEXT("HandleAttackRequest: 通常攻撃行動を受け付けました")
    );

    // TODO: 正面マスの状態に応じて通常攻撃または空振りを実行する
}

/** インタラクト行動を処理する */
void ARldPlayerCharacter::HandleInteractRequest()
{
    UE_LOG(
        LogRldPlayerCharacter,
        Log,
        TEXT("HandleInteractRequest: インタラクト行動を受け付けました")
    );

    // TODO: 正面マスの罠調査、宝箱、イベントなどを処理する
}

/** 初期カメラ設定を適用する */
void ARldPlayerCharacter::ApplyInitialCameraSettings()
{
    // スプリングアーム未生成時は処理しない
    if (!springArm)
    {
        return;
    }

    springArm->TargetArmLength = initialTargetArmLength;
    springArm->bDoCollisionTest = bEnableCameraCollisionTest;
    springArm->SetRelativeRotation(FRotator(initialCameraPitch, initialCameraYaw, 0.0f));

    UE_LOG(
        LogRldPlayerCharacter,
        Log,
        TEXT("ApplyInitialCameraSettings: アーム長=%f ピッチ=%f ヨー=%f 衝突判定=%d"),
        springArm->TargetArmLength,
        initialCameraPitch,
        initialCameraYaw,
        bEnableCameraCollisionTest ? 1 : 0
    );
}
