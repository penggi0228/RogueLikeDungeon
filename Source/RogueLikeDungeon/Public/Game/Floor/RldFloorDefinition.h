// RldFloorDefinition.h

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Common/Grid/CmnGridDefinition.h"
#include "RldFloorDefinition.generated.h"

class ARldEnemyBase;

/**
 * 自動生成フロア用エネミー出現候補
 */
USTRUCT(BlueprintType)
struct ROGUELIKEDUNGEON_API FRldProceduralEnemySpawnEntry
{
    GENERATED_BODY()

    // 出現候補のエネミークラス
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Floor|Enemy")
    TSubclassOf<ARldEnemyBase> enemyClass = nullptr;

    // 出現重み
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Floor|Enemy", meta = (ClampMin = "1"))
    int32 spawnWeight = 1;
};

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

    // 最小セクション数
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Floor|ProcGen", meta = (ClampMin = "1"))
    int32 minSectionCount = 4;

    // 最大セクション数
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Floor|ProcGen", meta = (ClampMin = "1"))
    int32 maxSectionCount = 7;

    // 最小セクション横幅
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Floor|ProcGen", meta = (ClampMin = "3"))
    int32 minSectionWidth = 4;

    // 最大セクション横幅
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Floor|ProcGen", meta = (ClampMin = "3"))
    int32 maxSectionWidth = 8;

    // 最小セクション縦幅
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Floor|ProcGen", meta = (ClampMin = "3"))
    int32 minSectionHeight = 4;

    // 最大セクション縦幅
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

    // 自動生成で出現候補にするエネミークラス配列
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Floor|Enemy")
    TArray<FRldProceduralEnemySpawnEntry> proceduralEnemySpawnEntries;

    // 自動生成フロアの最小エネミー数
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Floor|ProcGen|Enemy", meta = (ClampMin = "0"))
    int32 minProceduralEnemyCount = 0;

    // 自動生成フロアの最大エネミー数
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
