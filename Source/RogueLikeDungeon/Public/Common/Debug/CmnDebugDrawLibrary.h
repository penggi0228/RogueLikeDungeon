// CmnDebugDrawLibrary.h

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Common/Debug/CmnDebugDrawTypes.h"
#include "Common/Grid/CmnGridDefinition.h"
#include "Common/ProcGen/CmnGridRoom.h"
#include "CmnDebugDrawLibrary.generated.h"

/**
 * 共通デバッグ描画FunctionLibrary
 * グリッド系のデバッグ描画を共通化する
 */
UCLASS()
class ROGUELIKEDUNGEON_API UCmnDebugDrawLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    /**
     * 単一グリッドマスをデバッグ描画する
     *
     * @param world 描画先World
     * @param gridDefinition グリッド定義
     * @param gridCoord 描画対象グリッド座標
     * @param drawStyle 描画設定
     */
    UFUNCTION(BlueprintCallable, Category = "Cmn|Debug")
    static void DrawGridCell(
        UWorld* world,
        const FCmnGridDefinition& gridDefinition,
        const FIntPoint& gridCoord,
        const FCmnDebugDrawStyle& drawStyle
    );

    /**
     * 複数グリッドマスをデバッグ描画する
     *
     * @param world 描画先World
     * @param gridDefinition グリッド定義
     * @param gridCoords 描画対象グリッド座標一覧
     * @param drawStyle 描画設定
     */
    UFUNCTION(BlueprintCallable, Category = "Cmn|Debug")
    static void DrawGridCells(
        UWorld* world,
        const FCmnGridDefinition& gridDefinition,
        const TArray<FIntPoint>& gridCoords,
        const FCmnDebugDrawStyle& drawStyle
    );

    /**
     * 部屋矩形の外枠をデバッグ描画する
     *
     * @param world 描画先World
     * @param gridDefinition グリッド定義
     * @param room 描画対象部屋
     * @param drawStyle 描画設定
     */
    UFUNCTION(BlueprintCallable, Category = "Cmn|Debug")
    static void DrawGridRoomBounds(
        UWorld* world,
        const FCmnGridDefinition& gridDefinition,
        const FCmnGridRoom& room,
        const FCmnDebugDrawStyle& drawStyle
    );

private:

    /** グリッド座標をデバッグ描画用ワールド座標へ変換する */
    static FVector BuildDebugWorldLocation(
        const FCmnGridDefinition& gridDefinition,
        const FIntPoint& gridCoord,
        float zOffset
    );

    /** セル描画に使用する半径を解決する */
    static FVector BuildDebugHalfExtent(
        float cellSize,
        float sizeScale
    );
};
