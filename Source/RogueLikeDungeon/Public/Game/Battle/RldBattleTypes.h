// RldBattleTypes.h

#pragma once

#include "CoreMinimal.h"
#include "RldBattleTypes.generated.h"

/**
 * ダメージ種別
 * 戦闘中に発生するダメージの種類を表す
 */
UENUM(BlueprintType)
enum class ERldDamageType : uint8
{
    /** 種別なし */
    None UMETA(DisplayName = "None"),

    /** 物理ダメージ */
    Physical UMETA(DisplayName = "Physical"),

    /** 魔法ダメージ */
    Magical UMETA(DisplayName = "Magical"),

    /** 状態異常ダメージ */
    StatusEffect UMETA(DisplayName = "StatusEffect"),

    /** 固定ダメージ */
    Fixed UMETA(DisplayName = "Fixed")
};

/**
 * 状態異常種別
 * Actorに付与される一時的な状態変化を表す
 */
UENUM(BlueprintType)
enum class ERldStatusEffectType : uint8
{
    /** 状態異常なし */
    None UMETA(DisplayName = "None"),

    /** 毒 */
    Poison UMETA(DisplayName = "Poison")
};

/**
 * 状態異常の保持情報
 * 付与中の状態異常種別と残りターン数を保持する
 */
USTRUCT(BlueprintType)
struct ROGUELIKEDUNGEON_API FRldStatusEffectState
{
    GENERATED_BODY()

    // 状態異常種別
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Battle|StatusEffect")
    ERldStatusEffectType effectType = ERldStatusEffectType::None;

    // 残りターン数
    // 0以下の場合は永続扱いとして使用する想定
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rld|Battle|StatusEffect", meta = (ClampMin = "0"))
    int32 remainingTurns = 0;

    /** 状態異常情報が有効か判定する */
    bool IsValid() const
    {
        return effectType != ERldStatusEffectType::None;
    }
};
