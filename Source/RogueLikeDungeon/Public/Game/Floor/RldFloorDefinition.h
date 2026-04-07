// RldFloorDefinition.h

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Common/Grid/CmnGridDefinition.h"
#include "RldFloorDefinition.generated.h"

/**
 * フロア定義DataTableの1行分を表す構造体
 * グリッドサイズ、開始位置、階段位置、壁マス情報を保持する
 */
USTRUCT(BlueprintType)
struct FRldFloorDefinition : public FTableRowBase
{
    GENERATED_BODY()

public:

    // ----- グリッド設定 -----

    // グリッド横幅
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Floor")
    int32 gridWidth = 20;

    // グリッド縦幅
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Floor")
    int32 gridHeight = 20;

    // グリッド座標とワールド座標の変換定義
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Floor")
    FCmnGridDefinition gridDefinition;

    // ----- フロア座標設定 -----

    // プレイヤー開始グリッド座標
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Floor")
    FIntPoint playerStartGridCoord = FIntPoint(1, 1);

    // 階段グリッド座標
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Floor")
    FIntPoint stairsGridCoord = FIntPoint(18, 18);

    // ----- 壁マス設定 -----

    // 壁マス座標配列
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Floor")
    TArray<FIntPoint> wallCells;

    // BeginPlay時に外周壁を自動生成するか
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Floor")
    bool bGenerateOuterWalls = true;
};
