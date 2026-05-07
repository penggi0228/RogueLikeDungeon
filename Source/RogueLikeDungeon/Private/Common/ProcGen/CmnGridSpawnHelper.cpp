// CmnGridSpawnHelper.cpp

#include "Common/ProcGen/CmnGridSpawnHelper.h"

DEFINE_LOG_CATEGORY_STATIC(LogCmnGridSpawnHelper, Log, All);

/**
 * 除外ルールを適用してスポーン候補マス一覧を構築する
 */
void FCmnGridSpawnHelper::BuildCandidateCells(
    const TArray<FIntPoint>& sourceCells,
    const TArray<FCmnGridSpawnExclusionRule>& exclusionRules,
    TArray<FIntPoint>& outCandidateCells
)
{
    outCandidateCells.Reset();

    for (const FIntPoint& cell : sourceCells)
    {
        if (!IsCellAllowedByRules(cell, exclusionRules))
        {
            continue;
        }

        outCandidateCells.Add(cell);
    }

    // 結果順序を安定化
    outCandidateCells.Sort(
        [](const FIntPoint& a, const FIntPoint& b)
        {
            if (a.Y != b.Y)
            {
                return a.Y < b.Y;
            }

            return a.X < b.X;
        }
    );

    UE_LOG(
        LogCmnGridSpawnHelper,
        Log,
        TEXT("BuildCandidateCells: 元マス数=%d ルール数=%d 候補数=%d"),
        sourceCells.Num(),
        exclusionRules.Num(),
        outCandidateCells.Num()
    );
}

/**
 * 指定マスが除外ルールを満たすか判定する
 */
bool FCmnGridSpawnHelper::IsCellAllowedByRules(
    const FIntPoint& cell,
    const TArray<FCmnGridSpawnExclusionRule>& exclusionRules
)
{
    for (const FCmnGridSpawnExclusionRule& rule : exclusionRules)
    {
        const int32 distance = GetManhattanDistance(cell, rule.centerGridCoord);

        if (distance < rule.minManhattanDistance)
        {
            return false;
        }
    }

    return true;
}

/**
 * 2座標間のマンハッタン距離を取得する
 */
int32 FCmnGridSpawnHelper::GetManhattanDistance(const FIntPoint& a, const FIntPoint& b)
{
    return FMath::Abs(a.X - b.X) + FMath::Abs(a.Y - b.Y);
}
