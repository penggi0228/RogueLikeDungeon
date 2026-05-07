// RldFloorGenerator.cpp

#include "Game/Floor/RldFloorGenerator.h"

DEFINE_LOG_CATEGORY_STATIC(LogRldFloorGenerator, Log, All);

/**
 * フロア定義からレイアウトを生成する
 */
bool FRldFloorGenerator::GenerateFloorLayout(
    const FRldFloorDefinition& floorDefinition,
    FCmnGridLayoutBuildResult& outBuildResult
)
{
    outBuildResult.Reset();

    const int32 resolvedSeed = ResolveSeed(floorDefinition);

    InitializeGeneration(
        floorDefinition.gridWidth,
        floorDefinition.gridHeight,
        resolvedSeed
    );

    // 有効な内側領域を作れないサイズでは生成しない
    if (gridWidth < 3 || gridHeight < 3)
    {
        UE_LOG(
            LogRldFloorGenerator,
            Warning,
            TEXT("GenerateFloorLayout: グリッドサイズが小さすぎるため生成しません サイズ=(%d,%d)"),
            gridWidth,
            gridHeight
        );
        return false;
    }

    if (floorDefinition.bUseProceduralLayout)
    {
        return GenerateProceduralLayout(floorDefinition, outBuildResult);
    }

    return GenerateFixedLayout(floorDefinition, outBuildResult);
}

/** 固定レイアウトを生成する */
bool FRldFloorGenerator::GenerateFixedLayout(
    const FRldFloorDefinition& floorDefinition,
    FCmnGridLayoutBuildResult& outBuildResult
)
{
    TSet<FIntPoint> blockedCellSet;

    // 外周を壁扱いにする
    for (int32 x = 0; x < gridWidth; ++x)
    {
        blockedCellSet.Add(FIntPoint(x, 0));
        blockedCellSet.Add(FIntPoint(x, gridHeight - 1));
    }

    for (int32 y = 0; y < gridHeight; ++y)
    {
        blockedCellSet.Add(FIntPoint(0, y));
        blockedCellSet.Add(FIntPoint(gridWidth - 1, y));
    }

    // 定義済み内壁を反映
    for (const FIntPoint& wallCell : floorDefinition.wallCells)
    {
        if (wallCell.X < 0 || wallCell.X >= gridWidth || wallCell.Y < 0 || wallCell.Y >= gridHeight)
        {
            continue;
        }

        blockedCellSet.Add(wallCell);
    }

    // 壁以外の内側を床にする
    for (int32 y = 1; y < gridHeight - 1; ++y)
    {
        for (int32 x = 1; x < gridWidth - 1; ++x)
        {
            const FIntPoint cell(x, y);

            if (blockedCellSet.Contains(cell))
            {
                continue;
            }

            layoutBuilder.CarveCell(cell);
        }
    }

    outBuildResult.gridWidth = gridWidth;
    outBuildResult.gridHeight = gridHeight;
    outBuildResult.floorCells = layoutBuilder.GetFloorCells();
    outBuildResult.wallCells = layoutBuilder.BuildWallCells();
    outBuildResult.playerStartGridCoord = floorDefinition.playerStartGridCoord;
    outBuildResult.stairsGridCoord = floorDefinition.stairsGridCoord;

    if (!ValidateFixedSpecialCells(floorDefinition, outBuildResult))
    {
        UE_LOG(
            LogRldFloorGenerator,
            Warning,
            TEXT("GenerateFixedLayout: 開始座標または階段座標が無効なため生成に失敗しました")
        );
        return false;
    }

    UE_LOG(
        LogRldFloorGenerator,
        Log,
        TEXT("GenerateFixedLayout: 床マス数=%d 壁マス数=%d 開始座標=(%d,%d) 階段座標=(%d,%d)"),
        outBuildResult.floorCells.Num(),
        outBuildResult.wallCells.Num(),
        outBuildResult.playerStartGridCoord.X,
        outBuildResult.playerStartGridCoord.Y,
        outBuildResult.stairsGridCoord.X,
        outBuildResult.stairsGridCoord.Y
    );

    return true;
}

/** 部屋生成レイアウトを生成する */
bool FRldFloorGenerator::GenerateProceduralLayout(
    const FRldFloorDefinition& floorDefinition,
    FCmnGridLayoutBuildResult& outBuildResult
)
{
    TArray<FCmnGridRoom> rooms;
    GenerateRooms(floorDefinition, rooms);

    // 有効な部屋が1つもない場合は失敗
    if (rooms.Num() == 0)
    {
        UE_LOG(
            LogRldFloorGenerator,
            Warning,
            TEXT("GenerateProceduralLayout: 有効な部屋を生成できませんでした")
        );
        return false;
    }

    // 部屋を掘る
    for (const FCmnGridRoom& room : rooms)
    {
        layoutBuilder.CarveRoom(room);
    }

    // 部屋同士を接続
    ConnectRooms(rooms);

    // 結果化
    if (!FinalizeBuildResult(rooms, outBuildResult))
    {
        UE_LOG(
            LogRldFloorGenerator,
            Warning,
            TEXT("GenerateProceduralLayout: 生成結果の確定に失敗しました")
        );
        return false;
    }

    UE_LOG(
        LogRldFloorGenerator,
        Log,
        TEXT("GenerateProceduralLayout: 部屋数=%d 床マス数=%d 壁マス数=%d 開始座標=(%d,%d) 階段座標=(%d,%d)"),
        rooms.Num(),
        outBuildResult.floorCells.Num(),
        outBuildResult.wallCells.Num(),
        outBuildResult.playerStartGridCoord.X,
        outBuildResult.playerStartGridCoord.Y,
        outBuildResult.stairsGridCoord.X,
        outBuildResult.stairsGridCoord.Y
    );

    return true;
}

/** 部屋一覧を生成する */
void FRldFloorGenerator::GenerateRooms(
    const FRldFloorDefinition& floorDefinition,
    TArray<FCmnGridRoom>& outRooms
)
{
    outRooms.Reset();

    const int32 minRoomCount = FMath::Max(1, floorDefinition.minRoomCount);
    const int32 maxRoomCount = FMath::Max(minRoomCount, floorDefinition.maxRoomCount);

    const int32 minRoomWidth = FMath::Max(3, floorDefinition.minRoomWidth);
    const int32 maxRoomWidth = FMath::Max(minRoomWidth, floorDefinition.maxRoomWidth);

    const int32 minRoomHeight = FMath::Max(3, floorDefinition.minRoomHeight);
    const int32 maxRoomHeight = FMath::Max(minRoomHeight, floorDefinition.maxRoomHeight);

    const int32 targetRoomCount = RandRange(minRoomCount, maxRoomCount);

    for (int32 attemptIndex = 0; attemptIndex < floorDefinition.roomPlacementAttempts; ++attemptIndex)
    {
        if (outRooms.Num() >= targetRoomCount)
        {
            break;
        }

        const int32 roomWidth = RandRange(minRoomWidth, maxRoomWidth);
        const int32 roomHeight = RandRange(minRoomHeight, maxRoomHeight);

        // 外周壁を残すため1マス内側から配置する
        const int32 minLeft = 1;
        const int32 minTop = 1;
        const int32 maxLeft = gridWidth - roomWidth - 1;
        const int32 maxTop = gridHeight - roomHeight - 1;

        if (maxLeft < minLeft || maxTop < minTop)
        {
            continue;
        }

        const FCmnGridRoom candidateRoom(
            RandRange(minLeft, maxLeft),
            RandRange(minTop, maxTop),
            roomWidth,
            roomHeight
        );

        if (!CanPlaceRoom(candidateRoom, outRooms, floorDefinition.roomSeparationPadding))
        {
            continue;
        }

        outRooms.Add(candidateRoom);
    }

    UE_LOG(
        LogRldFloorGenerator,
        Log,
        TEXT("GenerateRooms: 目標部屋数=%d 生成部屋数=%d"),
        targetRoomCount,
        outRooms.Num()
    );
}

/** 指定部屋候補が配置可能か判定する */
bool FRldFloorGenerator::CanPlaceRoom(
    const FCmnGridRoom& candidateRoom,
    const TArray<FCmnGridRoom>& existingRooms,
    int32 padding
) const
{
    if (!candidateRoom.IsValid())
    {
        return false;
    }

    // 外周へ接触する部屋は許可しない
    if (candidateRoom.left <= 0 || candidateRoom.top <= 0)
    {
        return false;
    }

    if (candidateRoom.GetRight() >= (gridWidth - 1) || candidateRoom.GetBottom() >= (gridHeight - 1))
    {
        return false;
    }

    for (const FCmnGridRoom& existingRoom : existingRooms)
    {
        if (candidateRoom.IntersectsWithPadding(existingRoom, padding))
        {
            return false;
        }
    }

    return true;
}

/** 部屋同士を通路で接続する */
void FRldFloorGenerator::ConnectRooms(const TArray<FCmnGridRoom>& rooms)
{
    if (rooms.Num() <= 1)
    {
        return;
    }

    for (int32 roomIndex = 1; roomIndex < rooms.Num(); ++roomIndex)
    {
        const FIntPoint previousCenter = rooms[roomIndex - 1].GetCenter();
        const FIntPoint currentCenter = rooms[roomIndex].GetCenter();

        // L字接続
        const bool bHorizontalFirst = (RandRange(0, 1) == 0);

        if (bHorizontalFirst)
        {
            layoutBuilder.CarveHorizontalTunnel(previousCenter.X, currentCenter.X, previousCenter.Y);
            layoutBuilder.CarveVerticalTunnel(previousCenter.Y, currentCenter.Y, currentCenter.X);
        }
        else
        {
            layoutBuilder.CarveVerticalTunnel(previousCenter.Y, currentCenter.Y, previousCenter.X);
            layoutBuilder.CarveHorizontalTunnel(previousCenter.X, currentCenter.X, currentCenter.Y);
        }
    }
}

/** 固定フロアの特殊マスが有効か判定する */
bool FRldFloorGenerator::ValidateFixedSpecialCells(
    const FRldFloorDefinition& floorDefinition,
    const FCmnGridLayoutBuildResult& buildResult
) const
{
    const bool bStartInside =
        floorDefinition.playerStartGridCoord.X >= 0 &&
        floorDefinition.playerStartGridCoord.X < buildResult.gridWidth &&
        floorDefinition.playerStartGridCoord.Y >= 0 &&
        floorDefinition.playerStartGridCoord.Y < buildResult.gridHeight;

    const bool bStairsInside =
        floorDefinition.stairsGridCoord.X >= 0 &&
        floorDefinition.stairsGridCoord.X < buildResult.gridWidth &&
        floorDefinition.stairsGridCoord.Y >= 0 &&
        floorDefinition.stairsGridCoord.Y < buildResult.gridHeight;

    if (!bStartInside || !bStairsInside)
    {
        return false;
    }

    const TSet<FIntPoint> floorCellSet(buildResult.floorCells);

    if (!floorCellSet.Contains(floorDefinition.playerStartGridCoord))
    {
        return false;
    }

    if (!floorCellSet.Contains(floorDefinition.stairsGridCoord))
    {
        return false;
    }

    return true;
}

/** 生成結果のメタ情報を設定する */
bool FRldFloorGenerator::FinalizeBuildResult(
    const TArray<FCmnGridRoom>& rooms,
    FCmnGridLayoutBuildResult& outBuildResult
) const
{
    if (rooms.Num() == 0)
    {
        return false;
    }

    outBuildResult.gridWidth = gridWidth;
    outBuildResult.gridHeight = gridHeight;
    outBuildResult.rooms = rooms;
    outBuildResult.floorCells = layoutBuilder.GetFloorCells();
    outBuildResult.wallCells = layoutBuilder.BuildWallCells();
    outBuildResult.playerStartGridCoord = rooms[0].GetCenter();

    FIntPoint farthestCell = outBuildResult.playerStartGridCoord;

    if (!layoutBuilder.FindFarthestReachableCell(outBuildResult.playerStartGridCoord, farthestCell))
    {
        return false;
    }

    outBuildResult.stairsGridCoord = farthestCell;
    return outBuildResult.IsValid();
}

/** 使用Seedを解決する */
int32 FRldFloorGenerator::ResolveSeed(const FRldFloorDefinition& floorDefinition) const
{
    if (floorDefinition.bUseRandomSeed)
    {
        return FMath::Rand();
    }

    return floorDefinition.randomSeed;
}
