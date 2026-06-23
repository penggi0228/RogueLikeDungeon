// CmnDebugDrawLibrary.cpp

#include "Common/Debug/CmnDebugDrawLibrary.h"

#include "Common/Grid/CmnGridCoordFunctionLibrary.h"
#include "DrawDebugHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogCmnDebugDrawLibrary, Log, All);

/** 単一グリッドマスをデバッグ描画する */
void UCmnDebugDrawLibrary::DrawGridCell(
    UWorld* world,
    const FCmnGridDefinition& gridDefinition,
    const FIntPoint& gridCoord,
    const FCmnDebugDrawStyle& drawStyle
)
{
    if (!world)
    {
        UE_LOG(
            LogCmnDebugDrawLibrary,
            Warning,
            TEXT("DrawGridCell: Worldがnullのため描画しません")
        );

        return;
    }

    const FVector drawLocation = BuildDebugWorldLocation(
        gridDefinition,
        gridCoord,
        drawStyle.zOffset
    );

    const FVector drawHalfExtent = BuildDebugHalfExtent(
        gridDefinition.cellSize,
        drawStyle.sizeScale
    );

    DrawDebugSolidBox(
        world,
        drawLocation,
        drawHalfExtent,
        drawStyle.drawColor,
        drawStyle.bPersistentLines,
        drawStyle.duration
    );

    DrawDebugBox(
        world,
        drawLocation,
        drawHalfExtent,
        drawStyle.drawColor,
        drawStyle.bPersistentLines,
        drawStyle.duration,
        0,
        drawStyle.lineThickness
    );
}

/** 複数グリッドマスをデバッグ描画する */
void UCmnDebugDrawLibrary::DrawGridCells(
    UWorld* world,
    const FCmnGridDefinition& gridDefinition,
    const TArray<FIntPoint>& gridCoords,
    const FCmnDebugDrawStyle& drawStyle
)
{
    if (!world)
    {
        UE_LOG(
            LogCmnDebugDrawLibrary,
            Warning,
            TEXT("DrawGridCells: Worldがnullのため描画しません")
        );

        return;
    }

    for (const FIntPoint& gridCoord : gridCoords)
    {
        DrawGridCell(
            world,
            gridDefinition,
            gridCoord,
            drawStyle
        );
    }
}

/** セクション矩形の外枠をデバッグ描画する */
void UCmnDebugDrawLibrary::DrawGridSectionBounds(
    UWorld* world,
    const FCmnGridDefinition& gridDefinition,
    const FCmnGridSection& section,
    const FCmnDebugDrawStyle& drawStyle
)
{
    if (!world)
    {
        UE_LOG(
            LogCmnDebugDrawLibrary,
            Warning,
            TEXT("DrawGridSectionBounds: Worldがnullのため描画しません")
        );

        return;
    }

    if (!section.IsValid())
    {
        UE_LOG(
            LogCmnDebugDrawLibrary,
            Warning,
            TEXT("DrawGridSectionBounds: セクション情報が無効のため描画しません セクション=(Left=%d Top=%d Width=%d Height=%d)"),
            section.left,
            section.top,
            section.width,
            section.height
        );

        return;
    }

    const FVector topLeft = BuildDebugWorldLocation(
        gridDefinition,
        FIntPoint(section.left, section.top),
        drawStyle.zOffset
    );

    const FVector topRight = BuildDebugWorldLocation(
        gridDefinition,
        FIntPoint(section.GetRight(), section.top),
        drawStyle.zOffset
    );

    const FVector bottomLeft = BuildDebugWorldLocation(
        gridDefinition,
        FIntPoint(section.left, section.GetBottom()),
        drawStyle.zOffset
    );

    const FVector bottomRight = BuildDebugWorldLocation(
        gridDefinition,
        FIntPoint(section.GetRight(), section.GetBottom()),
        drawStyle.zOffset
    );

    DrawDebugLine(
        world,
        topLeft,
        topRight,
        drawStyle.drawColor,
        drawStyle.bPersistentLines,
        drawStyle.duration,
        0,
        drawStyle.lineThickness
    );

    DrawDebugLine(
        world,
        topRight,
        bottomRight,
        drawStyle.drawColor,
        drawStyle.bPersistentLines,
        drawStyle.duration,
        0,
        drawStyle.lineThickness
    );

    DrawDebugLine(
        world,
        bottomRight,
        bottomLeft,
        drawStyle.drawColor,
        drawStyle.bPersistentLines,
        drawStyle.duration,
        0,
        drawStyle.lineThickness
    );

    DrawDebugLine(
        world,
        bottomLeft,
        topLeft,
        drawStyle.drawColor,
        drawStyle.bPersistentLines,
        drawStyle.duration,
        0,
        drawStyle.lineThickness
    );
}

/** グリッド座標をデバッグ描画用ワールド座標へ変換する */
FVector UCmnDebugDrawLibrary::BuildDebugWorldLocation(
    const FCmnGridDefinition& gridDefinition,
    const FIntPoint& gridCoord,
    float zOffset
)
{
    const FVector worldLocation = UCmnGridCoordFunctionLibrary::GridToWorld(
        gridDefinition,
        gridCoord
    );

    return worldLocation + FVector(0.0f, 0.0f, zOffset);
}

/** セル描画に使用する半径を解決する */
FVector UCmnDebugDrawLibrary::BuildDebugHalfExtent(
    float cellSize,
    float sizeScale
)
{
    const float clampedCellSize = FMath::Max(1.0f, cellSize);
    const float clampedSizeScale = FMath::Clamp(sizeScale, 0.01f, 1.0f);
    const float halfExtentValue = FMath::Max(1.0f, (clampedCellSize * 0.5f) * clampedSizeScale);

    return FVector(
        halfExtentValue,
        halfExtentValue,
        halfExtentValue
    );
}
