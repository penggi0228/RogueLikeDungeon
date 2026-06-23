// RldPlayerCharacter.cpp

#include "Game/Characters/RldPlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "Engine/DataTable.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"

#include "Common/Battle/CmnHealthComponent.h"
#include "Common/Battle/CmnManaComponent.h"
#include "Game/Battle/RldStatusEffectComponent.h"
#include "Game/Enemies/RldEnemyBase.h"
#include "Game/Enemies/RldEnemyManager.h"
#include "Game/Floor/RldFloorManager.h"
#include "Game/Grid/RldGridManager.h"
#include "Game/Turn/RldTurnManager.h"
#include "Game/Turn/RldActionTypes.h"

DEFINE_LOG_CATEGORY_STATIC(LogRldPlayerCharacter, Log, All);

/** プレイヤーキャラクターを初期化する */
ARldPlayerCharacter::ARldPlayerCharacter()
{
    // ----- コンポーネント -----

    healthComponent = CreateDefaultSubobject<UCmnHealthComponent>(TEXT("HealthComponent"));
    manaComponent = CreateDefaultSubobject<UCmnManaComponent>(TEXT("ManaComponent"));
    statusEffectComponent = CreateDefaultSubobject<URldStatusEffectComponent>(TEXT("StatusEffectComponent"));

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

    // ステータス定義を読み込んでコンポーネントへ反映
    LoadPlayerStatusDefinition();
    ApplyBattleStatusToComponents();

    // 初期カメラ設定を適用
    ApplyInitialCameraSettings();

    // 各管理Actorを取得
    ResolveGridManager();
    ResolveTurnManager();
    ResolveFloorManager();
    ResolveEnemyManager();

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
            Verbose,
            TEXT("BeginPlay: Actor=%s 初期位置をグリッドへ補正しました 初期ワールド座標=(%f,%f,%f) 初期グリッド座標=(%d,%d) 補正後ワールド座標=(%f,%f,%f)"),
            *GetNameSafe(this),
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
                TEXT("BeginPlay: Actor=%s 初期位置の占有登録はFloorManagerに委譲します グリッド座標=(%d,%d)"),
                *GetNameSafe(this),
                initialGridCoord.X,
                initialGridCoord.Y
            );
        }
        else
        {
            UE_LOG(
                LogRldPlayerCharacter,
                Verbose,
                TEXT("BeginPlay: Actor=%s FloorManager未取得のため初期位置の占有登録は行いません グリッド座標=(%d,%d)"),
                *GetNameSafe(this),
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

/** 向き変更入力を受け取る */
void ARldPlayerCharacter::RequestFaceDirection(const FIntPoint& Direction)
{
    if (!IsValidGridDir(Direction))
    {
        UE_LOG(
            LogRldPlayerCharacter,
            Warning,
            TEXT("RequestFaceDirection: Actor=%s 向き変更方向が無効のため処理しません 方向=(%d,%d)"),
            *GetNameSafe(this),
            Direction.X,
            Direction.Y
        );

        return;
    }

    SetCurrentFacingGridDir(Direction);
}

/** カメラ視点入力を受け取る */
void ARldPlayerCharacter::RequestLookInput(const FVector2D& Axis)
{
    // スプリングアーム未生成時は処理しない
    if (!springArm)
    {
        UE_LOG(
            LogRldPlayerCharacter,
            Warning,
            TEXT("RequestLookInput: Actor=%s SpringArmがnullのため視点入力を処理しません"),
            *GetNameSafe(this)
        );

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
        TEXT("RequestLookInput: Actor=%s 入力値=(%f,%f) 回転値=(Pitch=%f Yaw=%f Roll=%f)"),
        *GetNameSafe(this),
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
        UE_LOG(
            LogRldPlayerCharacter,
            Warning,
            TEXT("RequestZoomInput: Actor=%s SpringArmがnullのためズーム入力を処理しません"),
            *GetNameSafe(this)
        );

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
        TEXT("RequestZoomInput: Actor=%s 入力値=%f アーム長=%f"),
        *GetNameSafe(this),
        Value,
        springArm->TargetArmLength
    );
}

/** 足踏み行動を実行する */
void ARldPlayerCharacter::RequestStepInPlaceAction(const FIntPoint& Direction)
{
    HandleStepInPlaceRequest(Direction);
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
            TEXT("ResolveGridManager: Actor=%s ARldGridManagerがレベル上に見つからないためグリッド移動処理が制限されます"),
            *GetNameSafe(this)
        );

        return;
    }

    UE_LOG(
        LogRldPlayerCharacter,
        Verbose,
        TEXT("ResolveGridManager: Actor=%s GridManager=%s クラス=%s"),
        *GetNameSafe(this),
        *GetNameSafe(gridManager),
        *GetNameSafe(gridManager->GetClass())
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
            TEXT("ResolveTurnManager: Actor=%s ARldTurnManagerがレベル上に見つからないためターン進行なしで処理します"),
            *GetNameSafe(this)
        );

        return;
    }

    UE_LOG(
        LogRldPlayerCharacter,
        Verbose,
        TEXT("ResolveTurnManager: Actor=%s TurnManager=%s クラス=%s 現在のターン数=%d"),
        *GetNameSafe(this),
        *GetNameSafe(turnManager),
        *GetNameSafe(turnManager->GetClass()),
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
            TEXT("ResolveFloorManager: Actor=%s ARldFloorManagerがレベル上に見つからないため階段遷移なしで処理します"),
            *GetNameSafe(this)
        );

        return;
    }

    UE_LOG(
        LogRldPlayerCharacter,
        Verbose,
        TEXT("ResolveFloorManager: Actor=%s FloorManager=%s クラス=%s 現在のフロア番号=%d"),
        *GetNameSafe(this),
        *GetNameSafe(floorManager),
        *GetNameSafe(floorManager->GetClass()),
        floorManager->GetCurrentFloorIndex()
    );
}

/** エネミー管理Actorを取得する */
void ARldPlayerCharacter::ResolveEnemyManager()
{
    AActor* foundActor = UGameplayStatics::GetActorOfClass(GetWorld(), ARldEnemyManager::StaticClass());
    enemyManager = Cast<ARldEnemyManager>(foundActor);

    if (!enemyManager)
    {
        UE_LOG(
            LogRldPlayerCharacter,
            Warning,
            TEXT("ResolveEnemyManager: Actor=%s ARldEnemyManagerがレベル上に見つからないためエネミー管理リストの同期が制限されます"),
            *GetNameSafe(this)
        );

        return;
    }

    UE_LOG(
        LogRldPlayerCharacter,
        Verbose,
        TEXT("ResolveEnemyManager: Actor=%s EnemyManager=%s クラス=%s エネミー数=%d"),
        *GetNameSafe(this),
        *GetNameSafe(enemyManager),
        *GetNameSafe(enemyManager->GetClass()),
        enemyManager->GetEnemyCount()
    );
}

/** 移動入力を処理する */
void ARldPlayerCharacter::HandleMoveRequest(const FIntPoint& Direction)
{
    // 無効な方向入力では向きも移動も更新しない
    if (!IsValidGridDir(Direction))
    {
        UE_LOG(
            LogRldPlayerCharacter,
            Warning,
            TEXT("HandleMoveRequest: Actor=%s 移動方向が無効のため処理しません 入力方向=(%d,%d)"),
            *GetNameSafe(this),
            Direction.X,
            Direction.Y
        );

        return;
    }

    // 壁やエネミーへぶつかる場合でも、入力方向へ向きだけは更新する
    SetCurrentFacingGridDir(Direction);

    const FIntPoint currentCoord = GetCurrentGridCoord();
    const FIntPoint nextGridCoord = currentCoord + Direction;

    UE_LOG(
        LogRldPlayerCharacter,
        Verbose,
        TEXT("HandleMoveRequest: Actor=%s 移動入力 現在の座標=(%d,%d) 入力方向=(%d,%d) 次の座標=(%d,%d)"),
        *GetNameSafe(this),
        currentCoord.X,
        currentCoord.Y,
        Direction.X,
        Direction.Y,
        nextGridCoord.X,
        nextGridCoord.Y
    );

    // GridManager未取得時は移動しない
    if (!gridManager)
    {
        UE_LOG(
            LogRldPlayerCharacter,
            Warning,
            TEXT("HandleMoveRequest: Actor=%s GridManager未取得のため移動を中止します 次の座標=(%d,%d)"),
            *GetNameSafe(this),
            nextGridCoord.X,
            nextGridCoord.Y
        );

        return;
    }

    // 範囲外マスへの移動は中止
    if (!gridManager->IsInsideGrid(nextGridCoord))
    {
        UE_LOG(
            LogRldPlayerCharacter,
            Log,
            TEXT("HandleMoveRequest: Actor=%s 範囲外のため移動を中止します 次の座標=(%d,%d)"),
            *GetNameSafe(this),
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
            TEXT("HandleMoveRequest: Actor=%s 壁マスのため移動を中止します 次の座標=(%d,%d)"),
            *GetNameSafe(this),
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
            Log,
            TEXT("HandleMoveRequest: Actor=%s 通行不可のため移動を中止します 次の座標=(%d,%d)"),
            *GetNameSafe(this),
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
            TEXT("HandleMoveRequest: Actor=%s 占有情報の移動に失敗したため移動を中止します 現在の座標=(%d,%d) 次の座標=(%d,%d)"),
            *GetNameSafe(this),
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
        TEXT("HandleMoveRequest: Actor=%s 移動成功 グリッド座標=(%d,%d) ワールド座標=(%f,%f,%f)"),
        *GetNameSafe(this),
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
            TEXT("HandleMoveRequest: Actor=%s 階段到達により次フロアへ進みます グリッド座標=(%d,%d)"),
            *GetNameSafe(this),
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
            TEXT("HandleMoveRequest: Actor=%s TurnManager未取得のためターン進行を行いません"),
            *GetNameSafe(this)
        );

        return;
    }

    turnManager->AdvanceTurnByPlayerAction(this, ERldActionType::Move);
}

/** 現在の向きを設定する */
void ARldPlayerCharacter::SetCurrentFacingGridDir(const FIntPoint& newFacingGridDir)
{
    if (!IsValidGridDir(newFacingGridDir))
    {
        UE_LOG(
            LogRldPlayerCharacter,
            Warning,
            TEXT("SetCurrentFacingGridDir: Actor=%s 向きが無効のため更新しません 方向=(%d,%d)"),
            *GetNameSafe(this),
            newFacingGridDir.X,
            newFacingGridDir.Y
        );

        return;
    }

    currentFacingGridDir = newFacingGridDir;

    UE_LOG(
        LogRldPlayerCharacter,
        Verbose,
        TEXT("SetCurrentFacingGridDir: Actor=%s 現在の向き=(%d,%d)"),
        *GetNameSafe(this),
        currentFacingGridDir.X,
        currentFacingGridDir.Y
    );
}

/** グリッド方向が8方向として有効か判定する */
bool ARldPlayerCharacter::IsValidGridDir(const FIntPoint& gridDir) const
{
    const int32 absX = FMath::Abs(gridDir.X);
    const int32 absY = FMath::Abs(gridDir.Y);

    return
        absX <= 1 &&
        absY <= 1 &&
        (absX + absY) >= 1;
}

/** 足踏み行動を処理する */
void ARldPlayerCharacter::HandleStepInPlaceRequest(const FIntPoint& Direction)
{
    // 無効な方向入力では向きもターンも更新しない
    if (!IsValidGridDir(Direction))
    {
        UE_LOG(
            LogRldPlayerCharacter,
            Warning,
            TEXT("HandleStepInPlaceRequest: Actor=%s 足踏み方向が無効のため処理しません 方向=(%d,%d)"),
            *GetNameSafe(this),
            Direction.X,
            Direction.Y
        );

        return;
    }

    // 足踏みでは移動せず、入力方向へ向く
    SetCurrentFacingGridDir(Direction);

    UE_LOG(
        LogRldPlayerCharacter,
        Log,
        TEXT("HandleStepInPlaceRequest: Actor=%s 足踏みしました 現在の座標=(%d,%d) 向き=(%d,%d)"),
        *GetNameSafe(this),
        GetCurrentGridCoord().X,
        GetCurrentGridCoord().Y,
        currentFacingGridDir.X,
        currentFacingGridDir.Y
    );

    // TurnManager未取得時はターン進行しない
    if (!turnManager)
    {
        UE_LOG(
            LogRldPlayerCharacter,
            Warning,
            TEXT("HandleStepInPlaceRequest: Actor=%s TurnManager未取得のためターン進行を行いません"),
            *GetNameSafe(this)
        );

        return;
    }

    turnManager->AdvanceTurnByPlayerAction(this, ERldActionType::StepInPlace);
}

/** 通常攻撃行動を処理する */
void ARldPlayerCharacter::HandleAttackRequest()
{
    const FIntPoint currentCoord = GetCurrentGridCoord();

    UE_LOG(
        LogRldPlayerCharacter,
        Log,
        TEXT("HandleAttackRequest: Actor=%s 通常攻撃行動を受け付けました 現在の座標=(%d,%d) 向き=(%d,%d)"),
        *GetNameSafe(this),
        currentCoord.X,
        currentCoord.Y,
        currentFacingGridDir.X,
        currentFacingGridDir.Y
    );

    // 向きが無効な場合は攻撃しない
    if (!IsValidGridDir(currentFacingGridDir))
    {
        UE_LOG(
            LogRldPlayerCharacter,
            Warning,
            TEXT("HandleAttackRequest: Actor=%s 現在の向きが無効のため攻撃しません 向き=(%d,%d)"),
            *GetNameSafe(this),
            currentFacingGridDir.X,
            currentFacingGridDir.Y
        );

        return;
    }

    // GridManager未取得時は攻撃対象を取得できない
    if (!gridManager)
    {
        UE_LOG(
            LogRldPlayerCharacter,
            Warning,
            TEXT("HandleAttackRequest: Actor=%s GridManager未取得のため攻撃しません"),
            *GetNameSafe(this)
        );

        return;
    }

    const FIntPoint targetGridCoord = currentCoord + currentFacingGridDir;

    bool bTargetCoordInGrid = false;
    AActor* targetActor = ResolveAttackTargetActor(targetGridCoord, bTargetCoordInGrid);
    ARldEnemyBase* enemyActor = Cast<ARldEnemyBase>(targetActor);

    if (enemyActor)
    {
        if (!TryDestroyEnemyByTemporaryAttack(enemyActor, targetGridCoord))
        {
            return;
        }
    }
    else
    {
        LogAttackMiss(targetGridCoord, targetActor, bTargetCoordInGrid);
    }

    // TurnManager未取得時はターン進行しない
    if (!turnManager)
    {
        UE_LOG(
            LogRldPlayerCharacter,
            Warning,
            TEXT("HandleAttackRequest: Actor=%s TurnManager未取得のためターン進行を行いません"),
            *GetNameSafe(this)
        );

        return;
    }

    turnManager->AdvanceTurnByPlayerAction(this, ERldActionType::Attack);
}

/** 攻撃対象Actorを取得する */
AActor* ARldPlayerCharacter::ResolveAttackTargetActor(
    const FIntPoint& targetGridCoord,
    bool& bOutTargetCoordInGrid
) const
{
    bOutTargetCoordInGrid = false;

    // GridManager未取得時は攻撃対象を取得しない
    if (!gridManager)
    {
        return nullptr;
    }

    bOutTargetCoordInGrid = gridManager->IsInsideGrid(targetGridCoord);

    // 範囲外の場合は攻撃対象なし
    if (!bOutTargetCoordInGrid)
    {
        return nullptr;
    }

    return gridManager->GetOccupyingActor(targetGridCoord);
}

/** 仮攻撃でエネミーを破棄する */
bool ARldPlayerCharacter::TryDestroyEnemyByTemporaryAttack(
    ARldEnemyBase* enemyActor,
    const FIntPoint& targetGridCoord
)
{
    // 破棄対象エネミー未取得時は処理しない
    if (!enemyActor)
    {
        UE_LOG(
            LogRldPlayerCharacter,
            Warning,
            TEXT("TryDestroyEnemyByTemporaryAttack: Actor=%s EnemyActorがnullのため仮攻撃を中止します 対象座標=(%d,%d)"),
            *GetNameSafe(this),
            targetGridCoord.X,
            targetGridCoord.Y
        );

        return false;
    }

    // GridManager未取得時は占有情報を解除できない
    if (!gridManager)
    {
        UE_LOG(
            LogRldPlayerCharacter,
            Warning,
            TEXT("TryDestroyEnemyByTemporaryAttack: Actor=%s GridManager未取得のため仮攻撃を中止します 対象Actor=%s 対象座標=(%d,%d)"),
            *GetNameSafe(this),
            *GetNameSafe(enemyActor),
            targetGridCoord.X,
            targetGridCoord.Y
        );

        return false;
    }

    const FString enemyActorName = GetNameSafe(enemyActor);

    // Destroy前にGridManagerの占有情報を解除する
    if (!gridManager->UnregisterOccupant(targetGridCoord, enemyActor))
    {
        UE_LOG(
            LogRldPlayerCharacter,
            Warning,
            TEXT("TryDestroyEnemyByTemporaryAttack: Actor=%s 占有情報の解除に失敗したため仮攻撃を中止します 対象座標=(%d,%d) 対象Actor=%s"),
            *GetNameSafe(this),
            targetGridCoord.X,
            targetGridCoord.Y,
            *enemyActorName
        );

        return false;
    }

    // EnemyManager未取得時は再取得を試みる
    if (!enemyManager)
    {
        ResolveEnemyManager();
    }

    if (enemyManager)
    {
        enemyManager->RemoveEnemyFromLists(enemyActor);
    }
    else
    {
        UE_LOG(
            LogRldPlayerCharacter,
            Warning,
            TEXT("TryDestroyEnemyByTemporaryAttack: Actor=%s EnemyManager未取得のためエネミー管理リストから除外できません 対象Actor=%s"),
            *GetNameSafe(this),
            *enemyActorName
        );
    }

    const bool bDestroyRequested = enemyActor->Destroy();

    // Destroy要求に失敗した場合は占有情報を戻す
    if (!bDestroyRequested)
    {
        gridManager->RegisterOccupant(targetGridCoord, enemyActor);

        if (enemyManager)
        {
            enemyManager->RefreshEnemyList();
        }

        UE_LOG(
            LogRldPlayerCharacter,
            Warning,
            TEXT("TryDestroyEnemyByTemporaryAttack: Actor=%s エネミーDestroyに失敗したため仮攻撃を中止しました 対象座標=(%d,%d) 対象Actor=%s"),
            *GetNameSafe(this),
            targetGridCoord.X,
            targetGridCoord.Y,
            *enemyActorName
        );

        return false;
    }

    UE_LOG(
        LogRldPlayerCharacter,
        Log,
        TEXT("TryDestroyEnemyByTemporaryAttack: Actor=%s 仮攻撃成功 エネミーを破棄しました 対象座標=(%d,%d) 対象Actor=%s"),
        *GetNameSafe(this),
        targetGridCoord.X,
        targetGridCoord.Y,
        *enemyActorName
    );

    return true;
}

/** 攻撃空振りログを出力する */
void ARldPlayerCharacter::LogAttackMiss(
    const FIntPoint& targetGridCoord,
    AActor* targetActor,
    bool bTargetCoordInGrid
) const
{
    if (!bTargetCoordInGrid)
    {
        UE_LOG(
            LogRldPlayerCharacter,
            Log,
            TEXT("LogAttackMiss: Actor=%s 空振りしました 理由=攻撃先が範囲外 対象座標=(%d,%d)"),
            *GetNameSafe(this),
            targetGridCoord.X,
            targetGridCoord.Y
        );

        return;
    }

    if (!targetActor)
    {
        UE_LOG(
            LogRldPlayerCharacter,
            Log,
            TEXT("LogAttackMiss: Actor=%s 空振りしました 理由=攻撃対象なし 対象座標=(%d,%d)"),
            *GetNameSafe(this),
            targetGridCoord.X,
            targetGridCoord.Y
        );

        return;
    }

    UE_LOG(
        LogRldPlayerCharacter,
        Log,
        TEXT("LogAttackMiss: Actor=%s 空振りしました 理由=攻撃対象がエネミーではない 対象座標=(%d,%d) 対象Actor=%s"),
        *GetNameSafe(this),
        targetGridCoord.X,
        targetGridCoord.Y,
        *GetNameSafe(targetActor)
    );
}

/** インタラクト行動を処理する */
void ARldPlayerCharacter::HandleInteractRequest()
{
    UE_LOG(
        LogRldPlayerCharacter,
        Log,
        TEXT("HandleInteractRequest: Actor=%s インタラクト行動を受け付けました"),
        *GetNameSafe(this)
    );

    // TODO: 正面マスの罠調査、宝箱、イベントなどを処理する
}

/** プレイヤーステータス定義を読み込む */
void ARldPlayerCharacter::LoadPlayerStatusDefinition()
{
    // DataTable未設定時はデフォルトステータスを使用
    if (!playerStatusDataTable)
    {
        UE_LOG(
            LogRldPlayerCharacter,
            Warning,
            TEXT("LoadPlayerStatusDefinition: Actor=%s playerStatusDataTableがnullのためデフォルトステータスを使用します"),
            *GetNameSafe(this)
        );

        return;
    }

    // RowName未設定時はデフォルトステータスを使用
    if (playerStatusRowName.IsNone())
    {
        UE_LOG(
            LogRldPlayerCharacter,
            Warning,
            TEXT("LoadPlayerStatusDefinition: Actor=%s playerStatusRowNameがNoneのためデフォルトステータスを使用します"),
            *GetNameSafe(this)
        );

        return;
    }

    static const FString contextString = TEXT("RldPlayerStatusDefinitionLookup");

    const FRldPlayerStatusDefinition* foundDefinition =
        playerStatusDataTable->FindRow<FRldPlayerStatusDefinition>(
            playerStatusRowName,
            contextString,
            true
        );

    // 対応行未取得時はデフォルトステータスを使用
    if (!foundDefinition)
    {
        UE_LOG(
            LogRldPlayerCharacter,
            Warning,
            TEXT("LoadPlayerStatusDefinition: Actor=%s 対応行が存在しないためデフォルトステータスを使用します RowName=%s"),
            *GetNameSafe(this),
            *playerStatusRowName.ToString()
        );

        return;
    }

    currentBattleStatus = foundDefinition->battleStatus;

    UE_LOG(
        LogRldPlayerCharacter,
        Log,
        TEXT("LoadPlayerStatusDefinition: データロード完了 Actor=%s RowName=%s プレイヤーID=%s 表示名=%s 最大HP=%d 最大MP=%d 攻撃力=%d 防御力=%d"),
        *GetNameSafe(this),
        *playerStatusRowName.ToString(),
        *foundDefinition->playerId.ToString(),
        *foundDefinition->displayName.ToString(),
        currentBattleStatus.maxHP,
        currentBattleStatus.maxMP,
        currentBattleStatus.attackPower,
        currentBattleStatus.defensePower
    );
}

/** 戦闘ステータスをコンポーネントへ反映する */
void ARldPlayerCharacter::ApplyBattleStatusToComponents()
{
    // HP管理コンポーネント取得時は最大HPの値を更新して、現在のHPに反映する
    if (healthComponent)
    {
        healthComponent->SetMaxHP(currentBattleStatus.maxHP);
        healthComponent->ResetHP();
    }
    else
    {
        UE_LOG(
            LogRldPlayerCharacter,
            Warning,
            TEXT("ApplyBattleStatusToComponents: Actor=%s HealthComponentがnullのため最大HPを反映できません"),
            *GetNameSafe(this)
        );
    }

    // MP管理コンポーネント取得時は最大MPの値を更新して、現在のMPに反映する
    if (manaComponent)
    {
        manaComponent->SetMaxMP(currentBattleStatus.maxMP);
        manaComponent->ResetMP();
    }
    else
    {
        UE_LOG(
            LogRldPlayerCharacter,
            Warning,
            TEXT("ApplyBattleStatusToComponents: Actor=%s ManaComponentがnullのため最大MPを反映できません"),
            *GetNameSafe(this)
        );
    }

    UE_LOG(
        LogRldPlayerCharacter,
        Log,
        TEXT("ApplyBattleStatusToComponents: Actor=%s 戦闘ステータス反映完了 最大HP=%d 最大MP=%d 攻撃力=%d 防御力=%d"),
        *GetNameSafe(this),
        currentBattleStatus.maxHP,
        currentBattleStatus.maxMP,
        currentBattleStatus.attackPower,
        currentBattleStatus.defensePower
    );
}

/** 初期カメラ設定を適用する */
void ARldPlayerCharacter::ApplyInitialCameraSettings()
{
    // スプリングアーム未生成時は処理しない
    if (!springArm)
    {
        UE_LOG(
            LogRldPlayerCharacter,
            Warning,
            TEXT("ApplyInitialCameraSettings: Actor=%s SpringArmがnullのため初期カメラ設定を適用しません"),
            *GetNameSafe(this)
        );

        return;
    }

    springArm->TargetArmLength = initialTargetArmLength;
    springArm->bDoCollisionTest = bEnableCameraCollisionTest;
    springArm->SetRelativeRotation(FRotator(initialCameraPitch, initialCameraYaw, 0.0f));

    UE_LOG(
        LogRldPlayerCharacter,
        Verbose,
        TEXT("ApplyInitialCameraSettings: Actor=%s 初期カメラ設定適用完了 アーム長=%f Pitch=%f Yaw=%f カメラ衝突判定=%s"),
        *GetNameSafe(this),
        springArm->TargetArmLength,
        initialCameraPitch,
        initialCameraYaw,
        bEnableCameraCollisionTest ? TEXT("有効") : TEXT("無効")
    );
}
