// RldStatusEffectComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Game/Battle/RldBattleTypes.h"
#include "RldStatusEffectComponent.generated.h"

class UCmnHealthComponent;

/**
 * RogueLikeDungeon用状態異常管理Component
 * 毒などの状態異常の保持とターン処理を行う
 */
UCLASS(ClassGroup = (Rld), meta = (BlueprintSpawnableComponent))
class ROGUELIKEDUNGEON_API URldStatusEffectComponent : public UActorComponent
{
    GENERATED_BODY()

public:

    /** 状態異常管理Componentを初期化する */
    URldStatusEffectComponent();

protected:

    // ----- UActorComponent -----

    virtual void BeginPlay() override;

public:

    // ----- 状態異常操作 -----

    /**
     * 状態異常を追加する
     *
     * @param effectType 状態異常種別
     * @param remainingTurns 残りターン数
     * @return 追加または更新できた場合はtrue
     */
    UFUNCTION(BlueprintCallable, Category = "Rld|Battle|StatusEffect")
    bool AddStatusEffect(ERldStatusEffectType effectType, int32 remainingTurns = 0);

    /**
     * 状態異常を解除する
     *
     * @param effectType 状態異常種別
     * @return 解除できた場合はtrue
     */
    UFUNCTION(BlueprintCallable, Category = "Rld|Battle|StatusEffect")
    bool RemoveStatusEffect(ERldStatusEffectType effectType);

    /** すべての状態異常を解除する */
    UFUNCTION(BlueprintCallable, Category = "Rld|Battle|StatusEffect")
    void ClearAllStatusEffects();

public:

    // ----- 状態判定 -----

    /**
     * 指定状態異常を保持しているか判定する
     *
     * @param effectType 状態異常種別
     * @return 保持しているならtrue
     */
    UFUNCTION(BlueprintPure, Category = "Rld|Battle|StatusEffect")
    bool HasStatusEffect(ERldStatusEffectType effectType) const;

    /**
     * 状態異常数を取得する
     *
     * @return 状態異常数
     */
    UFUNCTION(BlueprintPure, Category = "Rld|Battle|StatusEffect")
    int32 GetStatusEffectCount() const
    {
        return statusEffects.Num();
    }

    /**
     * 状態異常一覧を取得する
     *
     * @return 状態異常一覧
     */
    UFUNCTION(BlueprintPure, Category = "Rld|Battle|StatusEffect")
    const TArray<FRldStatusEffectState>& GetStatusEffects() const
    {
        return statusEffects;
    }

public:

    // ----- ターン処理 -----

    /**
     * ターン終了時の状態異常効果を適用する
     *
     * @param healthComponent HP管理Component
     */
    UFUNCTION(BlueprintCallable, Category = "Rld|Battle|StatusEffect")
    void ApplyTurnEndEffects(UCmnHealthComponent* healthComponent);

private:

    // ----- 内部処理 -----

    /**
     * 指定状態異常のIndexを取得する
     *
     * @param effectType 状態異常種別
     * @return 見つかったIndex。見つからない場合はINDEX_NONE
     */
    int32 FindStatusEffectIndex(ERldStatusEffectType effectType) const;

    /**
     * 毒のターン終了時効果を適用する
     *
     * @param healthComponent HP管理Component
     */
    void ApplyPoisonTurnEndEffect(UCmnHealthComponent* healthComponent);

    /** 残りターン数を更新し、期限切れ状態異常を解除する */
    void UpdateRemainingTurns();

private:

    // ----- 状態異常 -----

    // 付与中の状態異常一覧
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rld|Battle|StatusEffect", meta = (AllowPrivateAccess = "true"))
    TArray<FRldStatusEffectState> statusEffects;
};
