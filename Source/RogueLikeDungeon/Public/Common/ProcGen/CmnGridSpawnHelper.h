// CmnGridSpawnHelper.h

#pragma once

#include "CoreMinimal.h"
#include "Common/ProcGen/CmnGridSpawnTypes.h"

/**
 * グリッドスポーン候補抽出補助クラス
 * 床マス一覧からスポーン候補マスを抽出する
 */
class FCmnGridSpawnHelper
{
public:

    /**
     * 除外ルールを適用してスポーン候補マス一覧を構築する
     *
     * @param sourceCells 元となる床マス一覧
     * @param exclusionRules 除外ルール一覧
     * @param outCandidateCells 抽出後の候補マス一覧
     */
    static void BuildCandidateCells(
        const TArray<FIntPoint>& sourceCells,
        const TArray<FCmnGridSpawnExclusionRule>& exclusionRules,
        TArray<FIntPoint>& outCandidateCells
    );

private:

    /**
     * 指定マスが除外ルールを満たすか判定する
     *
     * @param cell 判定対象マス
     * @param exclusionRules 除外ルール一覧
     * @return 候補として許可するならtrue
     */
    static bool IsCellAllowedByRules(
        const FIntPoint& cell,
        const TArray<FCmnGridSpawnExclusionRule>& exclusionRules
    );

    /**
     * 2座標間のマンハッタン距離を取得する
     *
     * @param a 比較元座標
     * @param b 比較先座標
     * @return マンハッタン距離
     */
    static int32 GetManhattanDistance(const FIntPoint& a, const FIntPoint& b);
};
