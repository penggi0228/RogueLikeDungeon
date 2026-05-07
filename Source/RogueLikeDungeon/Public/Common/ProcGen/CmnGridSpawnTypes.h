// CmnGridSpawnTypes.h

#pragma once

#include "CoreMinimal.h"
#include "CmnGridSpawnTypes.generated.h"

/**
 * グリッドスポーン除外ルール
 * 特定座標から一定距離以内を候補から除外する
 */
USTRUCT(BlueprintType)
struct FCmnGridSpawnExclusionRule
{
    GENERATED_BODY()

public:

    // 基準グリッド座標
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cmn|ProcGen")
    FIntPoint centerGridCoord = FIntPoint::ZeroValue;

    // 除外する最小マンハッタン距離
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cmn|ProcGen", meta = (ClampMin = "0"))
    int32 minManhattanDistance = 0;

public:

    /** ルールを初期化する */
    FCmnGridSpawnExclusionRule() = default;

    /**
     * ルールを初期化する
     *
     * @param inCenterGridCoord 基準グリッド座標
     * @param inMinManhattanDistance 除外する最小マンハッタン距離
     */
    FCmnGridSpawnExclusionRule(
        const FIntPoint& inCenterGridCoord,
        int32 inMinManhattanDistance
    )
        : centerGridCoord(inCenterGridCoord)
        , minManhattanDistance(inMinManhattanDistance)
    {
    }
};
