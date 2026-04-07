// CmnGridCoordFunctionLibrary.cpp

#include "Common/Grid/CmnGridCoordFunctionLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(LogCmnGridCoordFunctionLibrary, Log, All);

/**
 * グリッド座標をワールド座標へ変換する
 */
FVector UCmnGridCoordFunctionLibrary::GridToWorld(
    const FCmnGridDefinition& gridDefinition,
    const FIntPoint& gridCoord
)
{
    const FVector worldLocation(
        gridDefinition.originWorld.X + (static_cast<float>(gridCoord.X) * gridDefinition.cellSize),
        gridDefinition.originWorld.Y + (static_cast<float>(gridCoord.Y) * gridDefinition.cellSize),
        gridDefinition.originWorld.Z
    );

    UE_LOG(
        LogCmnGridCoordFunctionLibrary,
        Verbose,
        TEXT("GridToWorld: グリッド=(%d,%d)、ワールド=(%f,%f,%f)"),
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
FIntPoint UCmnGridCoordFunctionLibrary::WorldToGrid(
    const FCmnGridDefinition& gridDefinition,
    const FVector& worldLocation
)
{
    const float localX = worldLocation.X - gridDefinition.originWorld.X;
    const float localY = worldLocation.Y - gridDefinition.originWorld.Y;

    const int32 gridX = FMath::RoundToInt(localX / gridDefinition.cellSize);
    const int32 gridY = FMath::RoundToInt(localY / gridDefinition.cellSize);

    const FIntPoint gridCoord(gridX, gridY);

    UE_LOG(
        LogCmnGridCoordFunctionLibrary,
        Verbose,
        TEXT("WorldToGrid: ワールド=(%f,%f,%f)、 グリッド=(%d,%d)"),
        worldLocation.X,
        worldLocation.Y,
        worldLocation.Z,
        gridCoord.X,
        gridCoord.Y
    );

    return gridCoord;
}
