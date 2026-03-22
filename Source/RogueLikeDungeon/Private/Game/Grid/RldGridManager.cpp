// RldGridManager.cpp

#include "Game/Grid/RldGridManager.h"

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
        TEXT("IsInsideGrid: GridCoord=(%d,%d) GridSize=(%d,%d) Result=%d"),
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
    // 壁マス判定を関数化しておくことで、将来の保持構造変更をここに閉じ込められる
    const bool bIsWallCell = wallCells.Contains(gridCoord);

    UE_LOG(
        LogRldGridManager,
        Verbose,
        TEXT("IsWallCell: GridCoord=(%d,%d) Result=%d"),
        gridCoord.X,
        gridCoord.Y,
        bIsWallCell ? 1 : 0
    );

    return bIsWallCell;
}

/**
 * 指定グリッド座標へ通行可能か判定する
 */
bool ARldGridManager::IsWalkable(const FIntPoint& gridCoord) const
{
    // 通行可否判定をここへ集約しておくことで、後から壁や占有や地形コストを追加しやすくする
    if (!IsInsideGrid(gridCoord))
    {
        UE_LOG(
            LogRldGridManager,
            Verbose,
            TEXT("IsWalkable: GridCoord=(%d,%d) Result=0 Reason=OutOfRange"),
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
            TEXT("IsWalkable: GridCoord=(%d,%d) Result=0 Reason=WallCell"),
            gridCoord.X,
            gridCoord.Y
        );

        return false;
    }

    UE_LOG(
        LogRldGridManager,
        Verbose,
        TEXT("IsWalkable: GridCoord=(%d,%d) Result=1"),
        gridCoord.X,
        gridCoord.Y
    );

    return true;
}
