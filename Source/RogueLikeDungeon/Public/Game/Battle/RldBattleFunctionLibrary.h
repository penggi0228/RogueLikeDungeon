// RldBattleFunctionLibrary.h

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Game/Status/RldStatusTypes.h"
#include "RldBattleFunctionLibrary.generated.h"

/**
 * RogueLikeDungeon用戦闘計算FunctionLibrary
 * ダメージ計算などのゲーム固有戦闘ルールを提供する
 */
UCLASS()
class ROGUELIKEDUNGEON_API URldBattleFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    /**
     * 物理ダメージを計算する
     *
     * @param attackPower 物理攻撃力
     * @param defensePower 物理防御力
     * @return ダメージ量
     */
    UFUNCTION(BlueprintPure, Category = "Rld|Battle")
    static int32 CalculatePhysicalDamage(int32 attackPower, int32 defensePower);

    /**
     * 魔法ダメージを計算する
     *
     * @param magicAttackPower 魔法攻撃力
     * @param magicDefensePower 魔法防御力
     * @return ダメージ量
     */
    UFUNCTION(BlueprintPure, Category = "Rld|Battle")
    static int32 CalculateMagicalDamage(int32 magicAttackPower, int32 magicDefensePower);

    /**
     * 攻撃側と防御側のステータスから物理ダメージを計算する
     *
     * @param attackerStatus 攻撃側ステータス
     * @param defenderStatus 防御側ステータス
     * @return ダメージ量
     */
    UFUNCTION(BlueprintPure, Category = "Rld|Battle")
    static int32 CalculatePhysicalDamageFromStatus(
        const FRldBattleStatus& attackerStatus,
        const FRldBattleStatus& defenderStatus
    );

    /**
     * 攻撃側と防御側のステータスから魔法ダメージを計算する
     *
     * @param attackerStatus 攻撃側ステータス
     * @param defenderStatus 防御側ステータス
     * @return ダメージ量
     */
    UFUNCTION(BlueprintPure, Category = "Rld|Battle")
    static int32 CalculateMagicalDamageFromStatus(
        const FRldBattleStatus& attackerStatus,
        const FRldBattleStatus& defenderStatus
    );

    /**
     * 毒のターンダメージを取得する
     *
     * @return 毒ダメージ量
     */
    UFUNCTION(BlueprintPure, Category = "Rld|Battle")
    static int32 GetPoisonTurnDamage();
};
