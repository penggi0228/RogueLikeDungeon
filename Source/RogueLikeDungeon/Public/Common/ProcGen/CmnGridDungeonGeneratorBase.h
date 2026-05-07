// CmnGridDungeonGeneratorBase.h

#pragma once

#include "CoreMinimal.h"
#include "Common/ProcGen/CmnGridLayoutBuilder.h"

/**
 * グリッドダンジョン自動生成の共通基底クラス
 */
class FCmnGridDungeonGeneratorBase
{
public:

    /** デストラクタ */
    virtual ~FCmnGridDungeonGeneratorBase() = default;

protected:

    /** 生成前の共通初期化を行う */
    void InitializeGeneration(int32 inGridWidth, int32 inGridHeight, int32 randomSeed);

    /** ランダム値を取得する */
    int32 RandRange(int32 minValue, int32 maxValue);

protected:

    int32 gridWidth = 0;
    int32 gridHeight = 0;

    FRandomStream randomStream;
    FCmnGridLayoutBuilder layoutBuilder;
};
