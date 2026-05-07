// CmnGridLayoutBuilder.cpp

#include "Common/ProcGen/CmnGridLayoutBuilder.h"

DEFINE_LOG_CATEGORY_STATIC(LogCmnGridLayoutBuilder, Log, All);

/** ビルダーを初期化する */
FCmnGridLayoutBuilder::FCmnGridLayoutBuilder()
{
}

/** グリッドサイズを設定して内部状態を初期化する */
void FCmnGridLayoutBuilder::Initialize(int32 inGridWidth, int32 inGridHeight)
{
    gridWidth = FMath::Max(1, inGridWidth);
    gridHeight = FMath::Max(1, inGridHeight);

    Reset();

    UE_LOG(
        LogCmnGridLayoutBuilder,
        Log,
        TEXT("Initialize: グリッドサイズ=(%d,%d)"),
        gridWidth,
        gridHeight
    );
}

/** 内部状態を初期化する */
void FCmnGridLayoutBuilder::Reset()
{
    floorCellSet.Reset();
}

/** 指定座標がグリッド範囲内か判定する */
bool FCmnGridLayoutBuilder::IsInsideGrid(const FIntPoint& gridCoord) const
{
    return
        (gridCoord.X >= 0) &&
        (gridCoord.X < gridWidth) &&
        (gridCoord.Y >= 0) &&
        (gridCoord.Y < gridHeight);
}

/** 指定座標が床マスか判定する */
bool FCmnGridLayoutBuilder::IsFloorCell(const FIntPoint& gridCoord) const
{
    return floorCellSet.Contains(gridCoord);
}

/** 単一マスを床として掘る */
void FCmnGridLayoutBuilder::CarveCell(const FIntPoint& gridCoord)
{
    if (!IsInsideGrid(gridCoord))
    {
        return;
    }

    floorCellSet.Add(gridCoord);
}

/** 部屋矩形を床として掘る */
void FCmnGridLayoutBuilder::CarveRoom(const FCmnGridRoom& room)
{
    if (!room.IsValid())
    {
        return;
    }

    for (int32 y = room.top; y <= room.GetBottom(); ++y)
    {
        for (int32 x = room.left; x <= room.GetRight(); ++x)
        {
            CarveCell(FIntPoint(x, y));
        }
    }
}

/** 横通路を掘る */
void FCmnGridLayoutBuilder::CarveHorizontalTunnel(int32 startX, int32 endX, int32 y)
{
    const int32 minX = FMath::Min(startX, endX);
    const int32 maxX = FMath::Max(startX, endX);

    for (int32 x = minX; x <= maxX; ++x)
    {
        CarveCell(FIntPoint(x, y));
    }
}

/** 縦通路を掘る */
void FCmnGridLayoutBuilder::CarveVerticalTunnel(int32 startY, int32 endY, int32 x)
{
    const int32 minY = FMath::Min(startY, endY);
    const int32 maxY = FMath::Max(startY, endY);

    for (int32 y = minY; y <= maxY; ++y)
    {
        CarveCell(FIntPoint(x, y));
    }
}

/** 現在の床マス一覧を取得する */
TArray<FIntPoint> FCmnGridLayoutBuilder::GetFloorCells() const
{
    TArray<FIntPoint> result;
    result.Reserve(floorCellSet.Num());

    for (const FIntPoint& floorCell : floorCellSet)
    {
        result.Add(floorCell);
    }

    SortGridCoords(result);
    return result;
}

/** 床マスから壁マス一覧を構築する */
TArray<FIntPoint> FCmnGridLayoutBuilder::BuildWallCells() const
{
    TSet<FIntPoint> wallCellSet;

    // 外周壁を追加
    for (int32 x = 0; x < gridWidth; ++x)
    {
        AddWallCandidateUnique(wallCellSet, FIntPoint(x, 0));
        AddWallCandidateUnique(wallCellSet, FIntPoint(x, gridHeight - 1));
    }

    for (int32 y = 0; y < gridHeight; ++y)
    {
        AddWallCandidateUnique(wallCellSet, FIntPoint(0, y));
        AddWallCandidateUnique(wallCellSet, FIntPoint(gridWidth - 1, y));
    }

    // 床マス周辺を壁候補化
    static const FIntPoint NeighborOffsets[8] =
    {
        FIntPoint(-1, -1),
        FIntPoint(0, -1),
        FIntPoint(1, -1),
        FIntPoint(-1,  0),
        FIntPoint(1,  0),
        FIntPoint(-1,  1),
        FIntPoint(0,  1),
        FIntPoint(1,  1)
    };

    for (const FIntPoint& floorCell : floorCellSet)
    {
        for (const FIntPoint& offset : NeighborOffsets)
        {
            AddWallCandidateUnique(wallCellSet, floorCell + offset);
        }
    }

    TArray<FIntPoint> result;
    result.Reserve(wallCellSet.Num());

    for (const FIntPoint& wallCell : wallCellSet)
    {
        result.Add(wallCell);
    }

    SortGridCoords(result);
    return result;
}

/**
 * 指定開始座標から最遠床マスを探索する
 */
bool FCmnGridLayoutBuilder::FindFarthestReachableCell(
    const FIntPoint& startGridCoord,
    FIntPoint& outFarthestGridCoord
) const
{
    outFarthestGridCoord = FIntPoint::ZeroValue;

    if (!IsFloorCell(startGridCoord))
    {
        return false;
    }

    TQueue<FIntPoint> openQueue;
    TMap<FIntPoint, int32> distanceMap;

    openQueue.Enqueue(startGridCoord);
    distanceMap.Add(startGridCoord, 0);

    FIntPoint farthestCell = startGridCoord;
    int32 farthestDistance = 0;

    static const FIntPoint NeighborOffsets[4] =
    {
        FIntPoint(1,  0),
        FIntPoint(-1,  0),
        FIntPoint(0,  1),
        FIntPoint(0, -1)
    };

    while (!openQueue.IsEmpty())
    {
        FIntPoint currentCell;
        openQueue.Dequeue(currentCell);

        const int32 currentDistance = distanceMap[currentCell];

        if (currentDistance > farthestDistance)
        {
            farthestDistance = currentDistance;
            farthestCell = currentCell;
        }

        for (const FIntPoint& offset : NeighborOffsets)
        {
            const FIntPoint nextCell = currentCell + offset;

            if (!IsFloorCell(nextCell))
            {
                continue;
            }

            if (distanceMap.Contains(nextCell))
            {
                continue;
            }

            distanceMap.Add(nextCell, currentDistance + 1);
            openQueue.Enqueue(nextCell);
        }
    }

    outFarthestGridCoord = farthestCell;
    return true;
}

/** 壁マス候補を重複なしで追加する */
void FCmnGridLayoutBuilder::AddWallCandidateUnique(TSet<FIntPoint>& wallCellSet, const FIntPoint& gridCoord) const
{
    if (!IsInsideGrid(gridCoord))
    {
        return;
    }

    if (IsFloorCell(gridCoord))
    {
        return;
    }

    wallCellSet.Add(gridCoord);
}

/** 座標一覧を安定順序へ並べ替える */
void FCmnGridLayoutBuilder::SortGridCoords(TArray<FIntPoint>& gridCoords) const
{
    gridCoords.Sort(
        [](const FIntPoint& a, const FIntPoint& b)
        {
            if (a.Y != b.Y)
            {
                return a.Y < b.Y;
            }

            return a.X < b.X;
        }
    );
}
