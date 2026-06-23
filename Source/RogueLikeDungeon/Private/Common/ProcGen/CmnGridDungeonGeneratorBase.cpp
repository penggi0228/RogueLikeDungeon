// CmnGridDungeonGeneratorBase.cpp

#include "Common/ProcGen/CmnGridDungeonGeneratorBase.h"

DEFINE_LOG_CATEGORY_STATIC(LogCmnGridDungeonGeneratorBase, Log, All);

/** 生成前の共通初期化を行う */
void FCmnGridDungeonGeneratorBase::InitializeGeneration(int32 inGridWidth, int32 inGridHeight, int32 randomSeed)
{
    gridWidth = FMath::Max(1, inGridWidth);
    gridHeight = FMath::Max(1, inGridHeight);

    randomStream.Initialize(randomSeed);
    layoutBuilder.Initialize(gridWidth, gridHeight);

    UE_LOG(
        LogCmnGridDungeonGeneratorBase,
        Verbose,
        TEXT("InitializeGeneration: グリッドサイズ=(%d,%d) Seed=%d"),
        gridWidth,
        gridHeight,
        randomSeed
    );
}

/** ランダム値を取得する */
int32 FCmnGridDungeonGeneratorBase::RandRange(int32 minValue, int32 maxValue)
{
    return randomStream.RandRange(minValue, maxValue);
}
