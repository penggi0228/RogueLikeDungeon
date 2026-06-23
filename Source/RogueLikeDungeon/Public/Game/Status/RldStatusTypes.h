// RldStatusTypes.h

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "RldStatusTypes.generated.h"

/**
 * 戦闘用基本ステータス
 * プレイヤーとエネミーで共通して使用する数値を保持する
 */
USTRUCT(BlueprintType)
struct ROGUELIKEDUNGEON_API FRldBattleStatus
{
    GENERATED_BODY()

    // 最大HP
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Status|Resource", meta = (ClampMin = "1"))
    int32 maxHP = 10;

    // 最大MP
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Status|Resource", meta = (ClampMin = "0"))
    int32 maxMP = 5;

    // 物理攻撃力
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Status|Attack", meta = (ClampMin = "0"))
    int32 attackPower = 3;

    // 魔法攻撃力
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Status|Attack", meta = (ClampMin = "0"))
    int32 magicAttackPower = 0;

    // 物理防御力
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Status|Defense", meta = (ClampMin = "0"))
    int32 defensePower = 1;

    // 魔法防御力
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Status|Defense", meta = (ClampMin = "0"))
    int32 magicDefensePower = 0;

    // 素早さ
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Status|Action", meta = (ClampMin = "0"))
    int32 speed = 10;
};

/**
 * プレイヤーステータス定義DataTable用構造体
 * プレイヤー種別ごとの基本情報と初期ステータスを保持する
 */
USTRUCT(BlueprintType)
struct ROGUELIKEDUNGEON_API FRldPlayerStatusDefinition : public FTableRowBase
{
    GENERATED_BODY()

    // プレイヤーID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Player|Basic")
    FName playerId = NAME_None;

    // 内部名
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Player|Basic")
    FName internalName = NAME_None;

    // 表示名
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Player|Basic")
    FText displayName;

    // 戦闘用基本ステータス
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Player|Status")
    FRldBattleStatus battleStatus;
};

/**
 * エネミーステータス定義DataTable用構造体
 * エネミー種別ごとの基本情報とステータスを保持する
 */
USTRUCT(BlueprintType)
struct ROGUELIKEDUNGEON_API FRldEnemyStatusDefinition : public FTableRowBase
{
    GENERATED_BODY()

    // エネミーID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Enemy|Basic")
    FName enemyId = NAME_None;

    // 内部名
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Enemy|Basic")
    FName internalName = NAME_None;

    // 表示名
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Enemy|Basic")
    FText displayName;

    // 戦闘用基本ステータス
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Enemy|Status")
    FRldBattleStatus battleStatus;

    // 獲得経験値
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Enemy|Reward", meta = (ClampMin = "0"))
    int32 expReward = 1;

    // 獲得ゴールド
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Enemy|Reward", meta = (ClampMin = "0"))
    int32 goldReward = 0;
};
