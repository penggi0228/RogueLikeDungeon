// RldGridManager.cpp

#include "Game/Grid/RldGridManager.h"

#include "Common/Grid/CmnGridCoordFunctionLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(LogRldGridManager, Log, All);

/** コンストラクタ */
ARldGridManager::ARldGridManager()
{
    // グリッド管理Actor自体は毎フレーム更新不要なためTickを無効化する
    PrimaryActorTick.bCanEverTick = false;
}

/**
 * 指定グリッド座標が有効範囲内か判定する
 */
bool ARldGridManager::IsInsideGrid(const FIntPoint& gridCoord) const
{
    const bool bIsInsideX = (gridCoord.X >= 0) && (gridCoord.X < gridWidth);
    const bool bIsInsideY = (gridCoord.Y >= 0) && (gridCoord.Y < gridHeight);
    const bool bIsInsideGrid = bIsInsideX && bIsInsideY;

    UE_LOG(
        LogRldGridManager,
        Verbose,
        TEXT("IsInsideGrid: グリッド座標=(%d,%d) グリッドサイズ=(%d,%d) 判定結果=%d"),
        gridCoord.X,
        gridCoord.Y,
        gridWidth,
        gridHeight,
        bIsInsideGrid ? 1 : 0
    );

    return bIsInsideGrid;
}

/**
 * 指定グリッド座標が壁マスか判定する
 */
bool ARldGridManager::IsWallCell(const FIntPoint& gridCoord) const
{
    // 壁マス判定の保持方法を将来変更しやすくするため、判定口を関数に閉じ込める
    const bool bIsWallCell = wallCells.Contains(gridCoord);

    UE_LOG(
        LogRldGridManager,
        Verbose,
        TEXT("IsWallCell: グリッド座標=(%d,%d) 判定結果=%d"),
        gridCoord.X,
        gridCoord.Y,
        bIsWallCell ? 1 : 0
    );

    return bIsWallCell;
}

/**
 * 指定グリッド座標が階段マスか判定する
 */
bool ARldGridManager::IsStairsCell(const FIntPoint& gridCoord) const
{
    const bool bIsStairsCell = (gridCoord == stairsGridCoord);

    UE_LOG(
        LogRldGridManager,
        Verbose,
        TEXT("IsStairsCell: グリッド座標=(%d,%d) 階段座標=(%d,%d) 判定結果=%d"),
        gridCoord.X,
        gridCoord.Y,
        stairsGridCoord.X,
        stairsGridCoord.Y,
        bIsStairsCell ? 1 : 0
    );

    return bIsStairsCell;
}

/**
 * 指定グリッド座標へ通行可能か判定する
 */
bool ARldGridManager::IsWalkable(const FIntPoint& gridCoord) const
{
    // 通行可否判定をここへ集約することで、後から占有情報や地形判定を追加しやすくする
    if (!IsInsideGrid(gridCoord))
    {
        UE_LOG(
            LogRldGridManager,
            Verbose,
            TEXT("IsWalkable: グリッド座標=(%d,%d) 判定結果=0 理由=範囲外"),
            gridCoord.X,
            gridCoord.Y
        );

        return false;
    }

    if (IsWallCell(gridCoord))
    {
        UE_LOG(
            LogRldGridManager,
            Verbose,
            TEXT("IsWalkable: グリッド座標=(%d,%d) 判定結果=0 理由=壁マス"),
            gridCoord.X,
            gridCoord.Y
        );

        return false;
    }

    UE_LOG(
        LogRldGridManager,
        Verbose,
        TEXT("IsWalkable: グリッド座標=(%d,%d) 判定結果=1"),
        gridCoord.X,
        gridCoord.Y
    );

    return true;
}

/**
 * グリッド座標をワールド座標へ変換する
 */
FVector ARldGridManager::GridToWorld(const FIntPoint& gridCoord) const
{
    // 変換式は共通化しつつ、実際の定義値だけをゲーム側で差し替えられるようにする
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

/**
 * ワールド座標をグリッド座標へ変換する
 */
FIntPoint ARldGridManager::WorldToGrid(const FVector& worldLocation) const
{
    // 共通関数を利用することで、別ゲームでも同じ変換ロジックを流用しやすくする
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

/**
 * 階段マス座標を設定する
 */
void ARldGridManager::SetStairsGridCoord(const FIntPoint& newGridCoord)
{
    stairsGridCoord = newGridCoord;

    UE_LOG(
        LogRldGridManager,
        Log,
        TEXT("SetStairsGridCoord: 階段座標=(%d,%d)"),
        stairsGridCoord.X,
        stairsGridCoord.Y
    );
}
