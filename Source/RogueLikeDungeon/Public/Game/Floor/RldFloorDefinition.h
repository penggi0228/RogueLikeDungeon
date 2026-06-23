// RldFloorDefinition.h

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Common/Grid/CmnGridDefinition.h"
#include "RldFloorDefinition.generated.h"

class ARldEnemyBase;

/**
 * フロア定義DataTable用構造体
 */
USTRUCT(BlueprintType)
struct FRldFloorDefinition : public FTableRowBase
{
    GENERATED_BODY()

public:

    // ----- 基本設定 -----

    // グリッド横幅
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Floor", meta = (ClampMin = "3"))
    int32 gridWidth = 20;

    // グリッド縦幅
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Floor", meta = (ClampMin = "3"))
    int32 gridHeight = 20;

    // グリッド座標変換設定
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Floor")
    FCmnGridDefinition gridDefinition;

    // 自動生成を使うか
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Floor")
    bool bUseProceduralLayout = true;

    // 自動生成時にランダムSeedを使うか(falseなら自動生成結果が固定される)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Floor")
    bool bUseRandomSeed = false;

    // 固定Seed値(自動生成結果を固定)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Floor")
    int32 randomSeed = 12345;

public:

    // ----- 自動生成設定 -----

    // セクション数最小
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Floor|ProcGen", meta = (ClampMin = "1"))
    int32 minSectionCount = 4;

    // セクション数最大
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Floor|ProcGen", meta = (ClampMin = "1"))
    int32 maxSectionCount = 7;

    // セクション横幅最小
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Floor|ProcGen", meta = (ClampMin = "3"))
    int32 minSectionWidth = 4;

    // セクション横幅最大
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Floor|ProcGen", meta = (ClampMin = "3"))
    int32 maxSectionWidth = 8;

    // セクション縦幅最小
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Floor|ProcGen", meta = (ClampMin = "3"))
    int32 minSectionHeight = 4;

    // セクション縦幅最大
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Floor|ProcGen", meta = (ClampMin = "3"))
    int32 maxSectionHeight = 8;

    // セクション配置試行回数
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Floor|ProcGen", meta = (ClampMin = "1"))
    int32 sectionPlacementAttempts = 80;

    // セクション同士の余白
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Floor|ProcGen", meta = (ClampMin = "0"))
    int32 sectionSeparationPadding = 1;

public:

    // ----- エネミー自動生成設定 -----

    // 自動生成フロアで使用するエネミークラス
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Floor|ProcGen|Enemy")
    TSubclassOf<ARldEnemyBase> proceduralEnemyClass = nullptr;

    // 自動生成フロアのエネミー数最小
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Floor|ProcGen|Enemy", meta = (ClampMin = "0"))
    int32 minProceduralEnemyCount = 0;

    // 自動生成フロアのエネミー数最大
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Floor|ProcGen|Enemy", meta = (ClampMin = "0"))
    int32 maxProceduralEnemyCount = 3;

    // プレイヤー開始位置からの最小距離
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Floor|ProcGen|Enemy", meta = (ClampMin = "0"))
    int32 minDistanceFromPlayerStartForEnemySpawn = 4;

    // 階段位置からの最小距離
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Floor|ProcGen|Enemy", meta = (ClampMin = "0"))
    int32 minDistanceFromStairsForEnemySpawn = 4;

public:

    // ----- 固定レイアウト設定 -----

    // 開始座標
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Floor|Fixed")
    FIntPoint playerStartGridCoord = FIntPoint(1, 1);

    // 階段座標
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Floor|Fixed")
    FIntPoint stairsGridCoord = FIntPoint(18, 18);

    // 固定レイアウト用の壁マス一覧
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Floor|Fixed")
    TArray<FIntPoint> wallCells;
};
