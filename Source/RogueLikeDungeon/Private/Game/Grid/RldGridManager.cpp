// RldGridManager.cpp

#include "Game/Grid/RldGridManager.h"

#include "Common/Debug/CmnDebugCategories.h"
#include "Common/Debug/CmnDebugWorldSubsystem.h"
#include "Common/Grid/CmnGridCoordFunctionLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(LogRldGridManager, Log, All);

/** コンストラクタ */
ARldGridManager::ARldGridManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // 床マス描画設定
    floorDebugStyle.drawColor = FColor::Green;
    floorDebugStyle.bPersistentLines = false;
    floorDebugStyle.duration = 0.15f;
    floorDebugStyle.lineThickness = 1.5f;
    floorDebugStyle.zOffset = 5.0f;
    floorDebugStyle.sizeScale = 0.18f;

    // 壁マス描画設定
    wallDebugStyle.drawColor = FColor::Red;
    wallDebugStyle.bPersistentLines = false;
    wallDebugStyle.duration = 0.15f;
    wallDebugStyle.lineThickness = 1.5f;
    wallDebugStyle.zOffset = 25.0f;
    wallDebugStyle.sizeScale = 0.45f;

    // 階段マス描画設定
    stairsDebugStyle.drawColor = FColor::Blue;
    stairsDebugStyle.bPersistentLines = false;
    stairsDebugStyle.duration = 0.15f;
    stairsDebugStyle.lineThickness = 1.5f;
    stairsDebugStyle.zOffset = 45.0f;
    stairsDebugStyle.sizeScale = 0.30f;
}

/** 開始時処理 */
void ARldGridManager::BeginPlay()
{
    Super::BeginPlay();
}

/** 毎フレーム処理 */
void ARldGridManager::Tick(float deltaSeconds)
{
    Super::Tick(deltaSeconds);

    UpdateContinuousDebugDraw(deltaSeconds);
}

/** フロア定義と生成結果を現在のグリッド状態へ反映する */
void ARldGridManager::ApplyFloorLayout(
    const FRldFloorDefinition& floorDefinition,
    const FCmnGridLayoutBuildResult& floorLayout
)
{
    gridWidth = FMath::Max(1, floorLayout.gridWidth);
    gridHeight = FMath::Max(1, floorLayout.gridHeight);
    gridDefinition = floorDefinition.gridDefinition;

    floorCells = floorLayout.floorCells;
    wallCells = floorLayout.wallCells;
    stairsGridCoord = floorLayout.stairsGridCoord;

    RebuildCellSets();

    // フロア反映時は占有情報を初期化
    occupantMap.Empty();

    // 常時デバッグ描画の状態を初期化
    continuousDebugDrawElapsed = 0.0f;
    bDebugInfoLogged = false;

    UE_LOG(
        LogRldGridManager,
        Log,
        TEXT("ApplyFloorLayout: グリッドレイアウト反映完了 グリッドサイズ=(%d,%d) 床数=%d 壁数=%d 階段座標=(%d,%d)"),
        gridWidth,
        gridHeight,
        floorCells.Num(),
        wallCells.Num(),
        stairsGridCoord.X,
        stairsGridCoord.Y
    );

    // フロア反映直後の手動描画としてログありで描画する
    if (bDrawDebugOnApplyFloorLayout)
    {
        DrawDebugGridState();
        LogDebugDrawInfo();
    }
}

/** 指定グリッド座標が有効範囲内か判定する */
bool ARldGridManager::IsInsideGrid(const FIntPoint& gridCoord) const
{
    const bool bIsInsideX = (gridCoord.X >= 0) && (gridCoord.X < gridWidth);
    const bool bIsInsideY = (gridCoord.Y >= 0) && (gridCoord.Y < gridHeight);
    const bool bIsInsideGrid = bIsInsideX && bIsInsideY;

    UE_LOG(
        LogRldGridManager,
        Verbose,
        TEXT("IsInsideGrid: グリッド座標=(%d,%d) グリッドサイズ=(%d,%d) 範囲内=%s"),
        gridCoord.X,
        gridCoord.Y,
        gridWidth,
        gridHeight,
        bIsInsideGrid ? TEXT("true") : TEXT("false")
    );

    return bIsInsideGrid;
}

/** 指定グリッド座標が床マスか判定する */
bool ARldGridManager::IsFloorCell(const FIntPoint& gridCoord) const
{
    const bool bIsFloor = floorCellSet.Contains(gridCoord);

    UE_LOG(
        LogRldGridManager,
        Verbose,
        TEXT("IsFloorCell: グリッド座標=(%d,%d) 床マス=%s"),
        gridCoord.X,
        gridCoord.Y,
        bIsFloor ? TEXT("true") : TEXT("false")
    );

    return bIsFloor;
}

/** 指定グリッド座標が壁マスか判定する */
bool ARldGridManager::IsWallCell(const FIntPoint& gridCoord) const
{
    const bool bIsWallCell = wallCellSet.Contains(gridCoord);

    UE_LOG(
        LogRldGridManager,
        Verbose,
        TEXT("IsWallCell: グリッド座標=(%d,%d) 壁マス=%s"),
        gridCoord.X,
        gridCoord.Y,
        bIsWallCell ? TEXT("true") : TEXT("false")
    );

    return bIsWallCell;
}

/** 指定グリッド座標が階段マスか判定する */
bool ARldGridManager::IsStairsCell(const FIntPoint& gridCoord) const
{
    const bool bIsStairsCell = (gridCoord == stairsGridCoord);

    UE_LOG(
        LogRldGridManager,
        Verbose,
        TEXT("IsStairsCell: グリッド座標=(%d,%d) 階段座標=(%d,%d) 階段マス=%s"),
        gridCoord.X,
        gridCoord.Y,
        stairsGridCoord.X,
        stairsGridCoord.Y,
        bIsStairsCell ? TEXT("true") : TEXT("false")
    );

    return bIsStairsCell;
}

/** 指定グリッド座標が占有されているか判定する */
bool ARldGridManager::IsOccupied(const FIntPoint& gridCoord) const
{
    const TWeakObjectPtr<AActor>* foundActor = occupantMap.Find(gridCoord);
    const bool bIsOccupied = (foundActor != nullptr) && foundActor->IsValid();

    UE_LOG(
        LogRldGridManager,
        Verbose,
        TEXT("IsOccupied: グリッド座標=(%d,%d) 占有あり=%s 占有Actor=%s"),
        gridCoord.X,
        gridCoord.Y,
        bIsOccupied ? TEXT("あり") : TEXT("なし"),
        bIsOccupied ? *GetNameSafe(foundActor->Get()) : TEXT("None")
    );

    return bIsOccupied;
}

/** 指定グリッド座標へ通行可能か判定する */
bool ARldGridManager::IsWalkable(const FIntPoint& gridCoord) const
{
    // 範囲外マスは通行不可
    if (!IsInsideGrid(gridCoord))
    {
        UE_LOG(
            LogRldGridManager,
            Verbose,
            TEXT("IsWalkable: グリッド座標=(%d,%d) 通行可否=不可 理由=範囲外"),
            gridCoord.X,
            gridCoord.Y
        );

        return false;
    }

    // 床マス以外は通行不可
    if (!IsFloorCell(gridCoord))
    {
        UE_LOG(
            LogRldGridManager,
            Verbose,
            TEXT("IsWalkable: グリッド座標=(%d,%d) 通行可否=不可 理由=床マスではない"),
            gridCoord.X,
            gridCoord.Y
        );

        return false;
    }

    // 占有中のマスは通行不可
    if (IsOccupied(gridCoord))
    {
        UE_LOG(
            LogRldGridManager,
            Verbose,
            TEXT("IsWalkable: グリッド座標=(%d,%d) 通行可否=不可 理由=占有中"),
            gridCoord.X,
            gridCoord.Y

        );

        return false;
    }

    UE_LOG(
        LogRldGridManager,
        Verbose,
        TEXT("IsWalkable: グリッド座標=(%d,%d) 通行可否=可"),
        gridCoord.X,
        gridCoord.Y
    );

    return true;
}

/** 移動ルールに応じて指定グリッド座標へ進入可能か判定する */
bool ARldGridManager::CanEnterCell(const FIntPoint& gridCoord, bool bCanPassThroughWalls) const
{
    // 範囲外マスへは進入不可
    if (!IsInsideGrid(gridCoord))
    {
        UE_LOG(
            LogRldGridManager,
            Verbose,
            TEXT("CanEnterCell: グリッド座標=(%d,%d) 壁通過可否=%s 進入可否=不可 理由=範囲外"),
            gridCoord.X,
            gridCoord.Y,
            bCanPassThroughWalls ? TEXT("可") : TEXT("不可")
        );

        return false;
    }

    // 占有中のマスへは進入不可
    if (IsOccupied(gridCoord))
    {
        UE_LOG(
            LogRldGridManager,
            Verbose,
            TEXT("CanEnterCell: グリッド座標=(%d,%d) 壁通過可否=%s 進入可否=不可 理由=占有中"),
            gridCoord.X,
            gridCoord.Y,
            bCanPassThroughWalls ? TEXT("可") : TEXT("不可")
        );

        return false;
    }

    // 床マスへは進入可能
    if (IsFloorCell(gridCoord))
    {
        UE_LOG(
            LogRldGridManager,
            Verbose,
            TEXT("CanEnterCell: グリッド座標=(%d,%d) 壁通過可否=%s 進入可否=可 理由=床マス"),
            gridCoord.X,
            gridCoord.Y,
            bCanPassThroughWalls ? TEXT("可") : TEXT("不可")
        );

        return true;
    }

    // 壁通過可能なActorは壁マスへ進入可能
    if (bCanPassThroughWalls && IsWallCell(gridCoord))
    {
        UE_LOG(
            LogRldGridManager,
            Verbose,
            TEXT("CanEnterCell: グリッド座標=(%d,%d) 壁通過可否=%s 進入可否=可 理由=壁通過可能"),
            gridCoord.X,
            gridCoord.Y,
            bCanPassThroughWalls ? TEXT("可") : TEXT("不可")
        );

        return true;
    }

    UE_LOG(
        LogRldGridManager,
        Verbose,
        TEXT("CanEnterCell: グリッド座標=(%d,%d) 壁通過可否=%s 進入可否=不可 理由=進入不可マス"),
        gridCoord.X,
        gridCoord.Y,
        bCanPassThroughWalls ? TEXT("可") : TEXT("不可")
    );

    return false;
}

/** 斜め移動や1マス近接攻撃で角の通過可否を判定する */
bool ARldGridManager::CanPassDiagonalCorner(const FIntPoint& fromCoord, const FIntPoint& direction) const
{
    return CanPassDiagonalCornerWithWallPass(fromCoord, direction, false);
}

/** 移動ルールに応じて斜め移動や1マス近接攻撃で角の通過可否を判定する */
bool ARldGridManager::CanPassDiagonalCornerWithWallPass(
    const FIntPoint& fromCoord,
    const FIntPoint& direction,
    bool bCanPassThroughWalls
) const
{
    const int32 absX = FMath::Abs(direction.X);
    const int32 absY = FMath::Abs(direction.Y);

    // 1マス方向以外はこの判定の対象外
    if (absX > 1 || absY > 1 || (absX + absY) == 0)
    {
        UE_LOG(
            LogRldGridManager,
            Warning,
            TEXT("CanPassDiagonalCornerWithWallPass: 1マス方向ではないため角通過不可として扱います 起点=(%d,%d) 方向=(%d,%d) 壁通過可否=%s 角通過可否=不可"),
            fromCoord.X,
            fromCoord.Y,
            direction.X,
            direction.Y,
            bCanPassThroughWalls ? TEXT("可") : TEXT("不可")
        );

        return false;
    }

    // 斜め方向でない場合は角抜け判定不要
    if (absX == 0 || absY == 0)
    {
        return true;
    }

    const FIntPoint horizontalSideCoord = fromCoord + FIntPoint(direction.X, 0);
    const FIntPoint verticalSideCoord = fromCoord + FIntPoint(0, direction.Y);

    const bool bHorizontalPassable = CanEnterCell(horizontalSideCoord, bCanPassThroughWalls);
    const bool bVerticalPassable = CanEnterCell(verticalSideCoord, bCanPassThroughWalls);

    const bool bCanPass = bHorizontalPassable && bVerticalPassable;

    UE_LOG(
        LogRldGridManager,
        Verbose,
        TEXT("CanPassDiagonalCornerWithWallPass: 起点=(%d,%d) 方向=(%d,%d) 壁通過可否=%s X方向隣接座標=(%d,%d) Y方向隣接座標=(%d,%d) X方向通過可否=%s Y方向通過可否=%s 角通過可否=%s"),
        fromCoord.X,
        fromCoord.Y,
        direction.X,
        direction.Y,
        bCanPassThroughWalls ? TEXT("可") : TEXT("不可"),
        horizontalSideCoord.X,
        horizontalSideCoord.Y,
        verticalSideCoord.X,
        verticalSideCoord.Y,
        bHorizontalPassable ? TEXT("可") : TEXT("不可"),
        bVerticalPassable ? TEXT("可") : TEXT("不可"),
        bCanPass ? TEXT("可") : TEXT("不可")
    );

    return bCanPass;
}

/** 指定グリッド座標の占有Actorを取得する */
AActor* ARldGridManager::GetOccupyingActor(const FIntPoint& gridCoord) const
{
    const TWeakObjectPtr<AActor>* foundActor = occupantMap.Find(gridCoord);

    // 指定座標に登録がない場合はnullptrを返す
    if (!foundActor)
    {
        return nullptr;
    }

    return foundActor->Get();
}

/** グリッド座標へ占有Actorを登録する */
bool ARldGridManager::RegisterOccupant(const FIntPoint& gridCoord, AActor* occupantActor)
{
    // 登録対象Actorが無効な場合は登録しない
    if (!occupantActor)
    {
        UE_LOG(
            LogRldGridManager,
            Warning,
            TEXT("RegisterOccupant: 占有Actorがnullのため登録しません グリッド座標=(%d,%d)"),
            gridCoord.X,
            gridCoord.Y
        );

        return false;
    }

    // 範囲外マスへは登録しない
    if (!IsInsideGrid(gridCoord))
    {
        UE_LOG(
            LogRldGridManager,
            Warning,
            TEXT("RegisterOccupant: Actor=%s 範囲外のため登録しません グリッド座標=(%d,%d)"),
            *GetNameSafe(occupantActor),
            gridCoord.X,
            gridCoord.Y
        );

        return false;
    }

    // 床マス以外へは登録しない
    if (!IsFloorCell(gridCoord))
    {
        UE_LOG(
            LogRldGridManager,
            Warning,
            TEXT("RegisterOccupant: Actor=%s 床マスではないため登録しません グリッド座標=(%d,%d)"),
            *GetNameSafe(occupantActor),
            gridCoord.X,
            gridCoord.Y
        );

        return false;
    }

    // すでに占有中のマスへは登録しない
    if (IsOccupied(gridCoord))
    {
        AActor* currentOccupant = GetOccupyingActor(gridCoord);

        UE_LOG(
            LogRldGridManager,
            Warning,
            TEXT("RegisterOccupant: Actor=%s すでに占有中のため登録しません グリッド座標=(%d,%d) 現在の占有Actor=%s"),
            *GetNameSafe(occupantActor),
            gridCoord.X,
            gridCoord.Y,
            *GetNameSafe(currentOccupant)
        );

        return false;
    }

    occupantMap.Add(gridCoord, occupantActor);

    UE_LOG(
        LogRldGridManager,
        Verbose,
        TEXT("RegisterOccupant: Actor=%s 登録しました グリッド座標=(%d,%d)"),
        *GetNameSafe(occupantActor),
        gridCoord.X,
        gridCoord.Y
    );

    return true;
}

/** 移動ルールに応じてグリッド座標へ占有Actorを登録する */
bool ARldGridManager::RegisterOccupantWithWallPass(
    const FIntPoint& gridCoord,
    AActor* occupantActor,
    bool bCanPassThroughWalls
)
{
    // 登録対象Actorが無効な場合は登録しない
    if (!occupantActor)
    {
        UE_LOG(
            LogRldGridManager,
            Warning,
            TEXT("RegisterOccupantWithWallPass: 占有Actorがnullのため登録しません グリッド座標=(%d,%d) 壁通過可否=%s"),
            gridCoord.X,
            gridCoord.Y,
            bCanPassThroughWalls ? TEXT("可") : TEXT("不可")
        );

        return false;
    }

    // 登録ルール上、進入不可のマスへは登録しない
    if (!CanEnterCell(gridCoord, bCanPassThroughWalls))
    {
        UE_LOG(
            LogRldGridManager,
            Warning,
            TEXT("RegisterOccupantWithWallPass: Actor=%s 登録先へ進入できないため登録しません グリッド座標=(%d,%d) 壁通過可否=%s"),
            *GetNameSafe(occupantActor),
            gridCoord.X,
            gridCoord.Y,
            bCanPassThroughWalls ? TEXT("可") : TEXT("不可")
        );

        return false;
    }

    occupantMap.Add(gridCoord, occupantActor);

    UE_LOG(
        LogRldGridManager,
        Verbose,
        TEXT("RegisterOccupantWithWallPass: Actor=%s 登録しました グリッド座標=(%d,%d) 壁通過可否=%s"),
        *GetNameSafe(occupantActor),
        gridCoord.X,
        gridCoord.Y,
        bCanPassThroughWalls ? TEXT("可") : TEXT("不可")
    );

    return true;
}

/** グリッド座標から占有Actorを解除する */
bool ARldGridManager::UnregisterOccupant(const FIntPoint& gridCoord, AActor* occupantActor)
{
    // 解除対象Actorが無効な場合は解除しない
    if (!occupantActor)
    {
        UE_LOG(
            LogRldGridManager,
            Warning,
            TEXT("UnregisterOccupant: 占有Actorがnullのため解除しません グリッド座標=(%d,%d)"),
            gridCoord.X,
            gridCoord.Y
        );

        return false;
    }

    const TWeakObjectPtr<AActor>* foundActor = occupantMap.Find(gridCoord);

    // 指定座標に有効な登録がない場合は解除しない
    if (!foundActor || !foundActor->IsValid())
    {
        UE_LOG(
            LogRldGridManager,
            Warning,
            TEXT("UnregisterOccupant: Actor=%s 登録が存在しないため解除しません グリッド座標=(%d,%d)"),
            *GetNameSafe(occupantActor),
            gridCoord.X,
            gridCoord.Y
        );

        return false;
    }

    // 登録済みActorと解除対象Actorが異なる場合は解除しない
    if (foundActor->Get() != occupantActor)
    {
        UE_LOG(
            LogRldGridManager,
            Warning,
            TEXT("UnregisterOccupant: Actor=%s 登録Actorが一致しないため解除しません グリッド座標=(%d,%d) 登録済みActor=%s"),
            *GetNameSafe(occupantActor),
            gridCoord.X,
            gridCoord.Y,
            *GetNameSafe(foundActor->Get())
        );

        return false;
    }

    occupantMap.Remove(gridCoord);

    UE_LOG(
        LogRldGridManager,
        Verbose,
        TEXT("UnregisterOccupant: Actor=%s 解除しました グリッド座標=(%d,%d)"),
        *GetNameSafe(occupantActor),
        gridCoord.X,
        gridCoord.Y
    );

    return true;
}

/** 占有Actorを別グリッド座標へ移動する */
bool ARldGridManager::MoveOccupant(
    const FIntPoint& fromGridCoord,
    const FIntPoint& toGridCoord,
    AActor* occupantActor
)
{
    // 移動対象Actorが無効な場合は移動しない
    if (!occupantActor)
    {
        UE_LOG(
            LogRldGridManager,
            Warning,
            TEXT("MoveOccupant: 占有Actorがnullのため移動しません 移動前=(%d,%d) 移動先=(%d,%d)"),
            fromGridCoord.X,
            fromGridCoord.Y,
            toGridCoord.X,
            toGridCoord.Y
        );

        return false;
    }

    const TWeakObjectPtr<AActor>* foundActor = occupantMap.Find(fromGridCoord);

    // 移動前座標に移動対象Actorが登録されていない場合は移動しない
    if (!foundActor || !foundActor->IsValid() || foundActor->Get() != occupantActor)
    {
        UE_LOG(
            LogRldGridManager,
            Warning,
            TEXT("MoveOccupant: Actor=%s 移動前登録が不正のため移動しません 移動前=(%d,%d) 移動先=(%d,%d) 登録済みActor=%s"),
            *GetNameSafe(occupantActor),
            fromGridCoord.X,
            fromGridCoord.Y,
            toGridCoord.X,
            toGridCoord.Y,
            (foundActor && foundActor->IsValid()) ? *GetNameSafe(foundActor->Get()) : TEXT("None")
        );

        return false;
    }

    // 移動先が範囲外の場合は移動しない
    if (!IsInsideGrid(toGridCoord))
    {
        UE_LOG(
            LogRldGridManager,
            Warning,
            TEXT("MoveOccupant: Actor=%s 移動先が範囲外のため移動しません 移動先=(%d,%d)"),
            *GetNameSafe(occupantActor),
            toGridCoord.X,
            toGridCoord.Y
        );

        return false;
    }

    // 移動先が床マスではない場合は移動しない
    if (!IsFloorCell(toGridCoord))
    {
        UE_LOG(
            LogRldGridManager,
            Warning,
            TEXT("MoveOccupant: Actor=%s 移動先が床マスではないため移動しません 移動先=(%d,%d)"),
            *GetNameSafe(occupantActor),
            toGridCoord.X,
            toGridCoord.Y
        );

        return false;
    }

    // 移動先が占有中の場合は移動しない
    if (IsOccupied(toGridCoord))
    {
        AActor* currentOccupant = GetOccupyingActor(toGridCoord);

        UE_LOG(
            LogRldGridManager,
            Warning,
            TEXT("MoveOccupant: Actor=%s 移動先が占有中のため移動しません 移動先=(%d,%d) 現在の占有Actor=%s"),
            *GetNameSafe(occupantActor),
            toGridCoord.X,
            toGridCoord.Y,
            *GetNameSafe(currentOccupant)
        );

        return false;
    }

    occupantMap.Remove(fromGridCoord);
    occupantMap.Add(toGridCoord, occupantActor);

    UE_LOG(
        LogRldGridManager,
        Verbose,
        TEXT("MoveOccupant: Actor=%s 移動しました 移動前=(%d,%d) 移動先=(%d,%d)"),
        *GetNameSafe(occupantActor),
        fromGridCoord.X,
        fromGridCoord.Y,
        toGridCoord.X,
        toGridCoord.Y
    );

    return true;
}

/** 移動ルールに応じて占有Actorを別グリッド座標へ移動する */
bool ARldGridManager::MoveOccupantWithWallPass(
    const FIntPoint& fromGridCoord,
    const FIntPoint& toGridCoord,
    AActor* occupantActor,
    bool bCanPassThroughWalls
)
{
    // 移動対象Actorが無効な場合は移動しない
    if (!occupantActor)
    {
        UE_LOG(
            LogRldGridManager,
            Warning,
            TEXT("MoveOccupantWithWallPass: 占有Actorがnullのため移動しません 移動前=(%d,%d) 移動先=(%d,%d) 壁通過可否=%s"),
            fromGridCoord.X,
            fromGridCoord.Y,
            toGridCoord.X,
            toGridCoord.Y,
            bCanPassThroughWalls ? TEXT("可") : TEXT("不可")
        );

        return false;
    }

    const TWeakObjectPtr<AActor>* foundActor = occupantMap.Find(fromGridCoord);

    // 移動前座標に移動対象Actorが登録されていない場合は移動しない
    if (!foundActor || !foundActor->IsValid() || foundActor->Get() != occupantActor)
    {
        UE_LOG(
            LogRldGridManager,
            Warning,
            TEXT("MoveOccupantWithWallPass: Actor=%s 移動前登録が不正のため移動しません 移動前=(%d,%d) 移動先=(%d,%d) 壁通過可否=%s 登録済みActor=%s"),
            *GetNameSafe(occupantActor),
            fromGridCoord.X,
            fromGridCoord.Y,
            toGridCoord.X,
            toGridCoord.Y,
            bCanPassThroughWalls ? TEXT("可") : TEXT("不可"),
            (foundActor && foundActor->IsValid()) ? *GetNameSafe(foundActor->Get()) : TEXT("None")
        );

        return false;
    }

    // 移動ルール上、進入不可のマスへは移動しない
    if (!CanEnterCell(toGridCoord, bCanPassThroughWalls))
    {
        UE_LOG(
            LogRldGridManager,
            Warning,
            TEXT("MoveOccupantWithWallPass: Actor=%s 移動先へ進入できないため移動しません 移動前=(%d,%d) 移動先=(%d,%d) 壁通過可否=%s"),
            *GetNameSafe(occupantActor),
            fromGridCoord.X,
            fromGridCoord.Y,
            toGridCoord.X,
            toGridCoord.Y,
            bCanPassThroughWalls ? TEXT("可") : TEXT("不可")
        );

        return false;
    }

    occupantMap.Remove(fromGridCoord);
    occupantMap.Add(toGridCoord, occupantActor);

    UE_LOG(
        LogRldGridManager,
        Verbose,
        TEXT("MoveOccupantWithWallPass: Actor=%s 移動しました 移動前=(%d,%d) 移動先=(%d,%d) 壁通過可否=%s"),
        *GetNameSafe(occupantActor),
        fromGridCoord.X,
        fromGridCoord.Y,
        toGridCoord.X,
        toGridCoord.Y,
        bCanPassThroughWalls ? TEXT("可") : TEXT("不可")
    );

    return true;
}

/** すべての占有情報をクリアする */
void ARldGridManager::ClearAllOccupants()
{
    const int32 previousOccupantCount = occupantMap.Num();

    occupantMap.Empty();

    UE_LOG(
        LogRldGridManager,
        Verbose,
        TEXT("ClearAllOccupants: 占有情報をクリアしました 件数=%d"),
        previousOccupantCount
    );
}

/** グリッド座標をワールド座標へ変換する */
FVector ARldGridManager::GridToWorld(const FIntPoint& gridCoord) const
{
    const FVector worldLocation = UCmnGridCoordFunctionLibrary::GridToWorld(gridDefinition, gridCoord);

    UE_LOG(
        LogRldGridManager,
        Verbose,
        TEXT("GridToWorld: グリッド座標=(%d,%d) ワールド座標=(%f,%f,%f)"),
        gridCoord.X,
        gridCoord.Y,
        worldLocation.X,
        worldLocation.Y,
        worldLocation.Z
    );

    return worldLocation;
}

/** ワールド座標をグリッド座標へ変換する */
FIntPoint ARldGridManager::WorldToGrid(const FVector& worldLocation) const
{
    const FIntPoint gridCoord = UCmnGridCoordFunctionLibrary::WorldToGrid(gridDefinition, worldLocation);

    UE_LOG(
        LogRldGridManager,
        Verbose,
        TEXT("WorldToGrid: ワールド座標=(%f,%f,%f) グリッド座標=(%d,%d)"),
        worldLocation.X,
        worldLocation.Y,
        worldLocation.Z,
        gridCoord.X,
        gridCoord.Y
    );

    return gridCoord;
}

/** 現在のフロア状態をデバッグ描画する */
void ARldGridManager::DrawDebugGridState() const
{
    DrawDebugGridStateInternal(true);
}

/** デバッグ描画情報をログ出力する */
void ARldGridManager::LogDebugDrawInfo()
{
    // すでに出力済みの場合は再出力しない
    if (bDebugInfoLogged)
    {
        return;
    }

    bDebugInfoLogged = true;

    UE_LOG(
        LogRldGridManager,
        Verbose,
        TEXT("LogDebugDrawInfo: グリッドデバッグ描画凡例 緑=床マス 赤=壁マス 青=階段マス")
    );

    UE_LOG(
        LogRldGridManager,
        Verbose,
        TEXT("LogDebugDrawInfo: グリッドデバッグ描画情報 床数=%d 壁数=%d 階段座標=(%d,%d)"),
        floorCells.Num(),
        wallCells.Num(),
        stairsGridCoord.X,
        stairsGridCoord.Y
    );
}

/** 階段マス座標を設定する */
void ARldGridManager::SetStairsGridCoord(const FIntPoint& newGridCoord)
{
    stairsGridCoord = newGridCoord;

    UE_LOG(
        LogRldGridManager,
        Log,
        TEXT("SetStairsGridCoord: 階段座標設定完了 階段座標=(%d,%d)"),
        stairsGridCoord.X,
        stairsGridCoord.Y
    );
}

/** 配列から床マスと壁マスの検索用Setを再構築する */
void ARldGridManager::RebuildCellSets()
{
    floorCellSet.Reset();
    wallCellSet.Reset();

    for (const FIntPoint& floorCell : floorCells)
    {
        floorCellSet.Add(floorCell);
    }

    for (const FIntPoint& wallCell : wallCells)
    {
        wallCellSet.Add(wallCell);
    }
}

/** グリッドデバッグ描画を更新する */
void ARldGridManager::UpdateContinuousDebugDraw(float deltaSeconds)
{
    // 常時デバッグ描画が無効な場合は経過時間を初期化する
    if (!ShouldDrawContinuousDebug())
    {
        continuousDebugDrawElapsed = 0.0f;
        return;
    }

    continuousDebugDrawElapsed += deltaSeconds;

    // 再描画間隔に達していない場合は描画しない
    if (continuousDebugDrawElapsed < continuousDebugDrawInterval)
    {
        return;
    }

    continuousDebugDrawElapsed = 0.0f;

    // 常時描画ではログを出さない
    DrawDebugGridStateInternal(false);
}

/** グリッド常時デバッグ描画が有効か判定する */
bool ARldGridManager::ShouldDrawContinuousDebug() const
{
    // 常時デバッグ描画設定が無効な場合は描画しない
    if (!bEnableContinuousDebugDraw)
    {
        return false;
    }

    UWorld* world = GetWorld();

    // World未取得時は描画しない
    if (!world)
    {
        return false;
    }

    const UCmnDebugWorldSubsystem* debugSubsystem = world->GetSubsystem<UCmnDebugWorldSubsystem>();

    // DebugSubsystem未取得時は描画しない
    if (!debugSubsystem)
    {
        return false;
    }

    return debugSubsystem->IsDebugEnabled()
        && debugSubsystem->IsCategoryEnabled(CmnDebugCategories::Grid);
}

/** 現在のフロア状態をデバッグ描画する */
void ARldGridManager::DrawDebugGridStateInternal(bool bOutputLog) const
{
    UWorld* world = GetWorld();

    // World未取得時は描画しない
    if (!world)
    {
        if (bOutputLog)
        {
            UE_LOG(
                LogRldGridManager,
                Warning,
                TEXT("DrawDebugGridState: World未取得のため描画しません")
            );
        }

        return;
    }

    UCmnDebugWorldSubsystem* debugSubsystem = world->GetSubsystem<UCmnDebugWorldSubsystem>();

    // DebugSubsystem未取得時は描画しない
    if (!debugSubsystem)
    {
        if (bOutputLog)
        {
            UE_LOG(
                LogRldGridManager,
                Warning,
                TEXT("DrawDebugGridState: DebugSubsystem未取得のため描画しません")
            );
        }

        return;
    }

    // デバッグ描画全体またはGridカテゴリが無効な場合は描画しない
    if (!debugSubsystem->IsDebugEnabled() || !debugSubsystem->IsCategoryEnabled(CmnDebugCategories::Grid))
    {
        return;
    }

    // 床マスを描画
    debugSubsystem->DrawGridCells(
        gridDefinition,
        floorCells,
        floorDebugStyle
    );

    // 壁マスを描画
    debugSubsystem->DrawGridCells(
        gridDefinition,
        wallCells,
        wallDebugStyle
    );

    // 階段マスを描画
    debugSubsystem->DrawGridCell(
        gridDefinition,
        stairsGridCoord,
        stairsDebugStyle
    );

    // 手動描画時のみ描画完了ログを出力する
    if (bOutputLog)
    {
        UE_LOG(
            LogRldGridManager,
            Log,
            TEXT("DrawDebugGridState: グリッドデバッグ描画完了 床数=%d 壁数=%d 階段座標=(%d,%d)"),
            floorCells.Num(),
            wallCells.Num(),
            stairsGridCoord.X,
            stairsGridCoord.Y
        );
    }
}
