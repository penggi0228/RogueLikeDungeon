// RldBattleFunctionLibrary.cpp

#include "Game/Battle/RldBattleFunctionLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(LogRldBattleFunctionLibrary, Log, All);

/** 物理ダメージを計算する */
int32 URldBattleFunctionLibrary::CalculatePhysicalDamage(int32 attackPower, int32 defensePower)
{
    const int32 clampedAttackPower = FMath::Max(0, attackPower);
    const int32 clampedDefensePower = FMath::Max(0, defensePower);

    const int32 damageAmount = FMath::Max(1, clampedAttackPower - clampedDefensePower);

    UE_LOG(
        LogRldBattleFunctionLibrary,
        Verbose,
        TEXT("CalculatePhysicalDamage: 攻撃力=%d 防御力=%d 計算ダメージ=%d"),
        clampedAttackPower,
        clampedDefensePower,
        damageAmount
    );

    return damageAmount;
}

/** 魔法ダメージを計算する */
int32 URldBattleFunctionLibrary::CalculateMagicalDamage(int32 magicAttackPower, int32 magicDefensePower)
{
    const int32 clampedMagicAttackPower = FMath::Max(0, magicAttackPower);
    const int32 clampedMagicDefensePower = FMath::Max(0, magicDefensePower);

    const int32 damageAmount = FMath::Max(1, clampedMagicAttackPower - clampedMagicDefensePower);

    UE_LOG(
        LogRldBattleFunctionLibrary,
        Verbose,
        TEXT("CalculateMagicalDamage: 魔法攻撃力=%d 魔法防御力=%d 計算ダメージ=%d"),
        clampedMagicAttackPower,
        clampedMagicDefensePower,
        damageAmount
    );

    return damageAmount;
}

/** 攻撃側と防御側のステータスから物理ダメージを計算する */
int32 URldBattleFunctionLibrary::CalculatePhysicalDamageFromStatus(
    const FRldBattleStatus& attackerStatus,
    const FRldBattleStatus& defenderStatus
)
{
    return CalculatePhysicalDamage(
        attackerStatus.attackPower,
        defenderStatus.defensePower
    );
}

/** 攻撃側と防御側のステータスから魔法ダメージを計算する */
int32 URldBattleFunctionLibrary::CalculateMagicalDamageFromStatus(
    const FRldBattleStatus& attackerStatus,
    const FRldBattleStatus& defenderStatus
)
{
    return CalculateMagicalDamage(
        attackerStatus.magicAttackPower,
        defenderStatus.magicDefensePower
    );
}

/** 毒のターンダメージを取得する */
int32 URldBattleFunctionLibrary::GetPoisonTurnDamage()
{
    return 1;
}
