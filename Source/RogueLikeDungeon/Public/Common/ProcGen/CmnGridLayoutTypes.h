// CmnGridLayoutTypes.h

#pragma once

#include "CoreMinimal.h"
#include "Common/ProcGen/CmnGridSection.h"
#include "CmnGridLayoutTypes.generated.h"

/**
 * グリッドレイアウト生成結果
 */
USTRUCT(BlueprintType)
struct FCmnGridLayoutBuildResult
{
    GENERATED_BODY()

public:

    // グリッド横幅
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cmn|ProcGen")
    int32 gridWidth = 0;

    // グリッド縦幅
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cmn|ProcGen")
    int32 gridHeight = 0;

    // 床マス一覧
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cmn|ProcGen")
    TArray<FIntPoint> floorCells;

    // 壁マス一覧
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cmn|ProcGen")
    TArray<FIntPoint> wallCells;

    // セクション一覧
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cmn|ProcGen")
    TArray<FCmnGridSection> sections;

    // プレイヤー開始座標
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cmn|ProcGen")
    FIntPoint playerStartGridCoord = FIntPoint::ZeroValue;

    // 階段座標
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cmn|ProcGen")
    FIntPoint stairsGridCoord = FIntPoint::ZeroValue;

public:

    /** 内容を初期化する */
    void Reset()
    {
        gridWidth = 0;
        gridHeight = 0;
        floorCells.Reset();
        wallCells.Reset();
        sections.Reset();
        playerStartGridCoord = FIntPoint::ZeroValue;
        stairsGridCoord = FIntPoint::ZeroValue;
    }

    /** 最低限の生成結果が成立しているか判定する */
    bool IsValid() const
    {
        return
            gridWidth > 0 &&
            gridHeight > 0 &&
            floorCells.Num() > 0;
    }
};
