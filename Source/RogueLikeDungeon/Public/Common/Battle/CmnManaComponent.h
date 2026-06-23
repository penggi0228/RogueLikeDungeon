// CmnManaComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CmnManaComponent.generated.h"

/**
 * 共通MP管理用Component
 * 最大MP・現在のMP・消費・回復を管理する
 */
UCLASS(ClassGroup = (Common), meta = (BlueprintSpawnableComponent))
class ROGUELIKEDUNGEON_API UCmnManaComponent : public UActorComponent
{
    GENERATED_BODY()

public:

    /** MP管理コンポーネントを初期化する */
    UCmnManaComponent();

protected:

    // ----- UActorComponent -----

    virtual void BeginPlay() override;

public:

    // ----- Getter -----

    /**
     * 最大MPを取得する
     *
     * @return 最大MP
     */
    UFUNCTION(BlueprintPure, Category = "Cmn|Mana")
    int32 GetMaxMP() const
    {
        return maxMP;
    }

    /**
     * 現在のMPを取得する
     *
     * @return 現在のMP
     */
    UFUNCTION(BlueprintPure, Category = "Cmn|Mana")
    int32 GetCurrentMP() const
    {
        return currentMP;
    }

public:

    // ----- Setter -----

    /**
     * 最大MPを設定する
     *
     * @param newMaxMP 更新後の最大MP
     */
    UFUNCTION(BlueprintCallable, Category = "Cmn|Mana")
    void SetMaxMP(int32 newMaxMP);

    /**
     * 現在のMPを設定する
     *
     * @param newCurrentMP 更新後の現在のMP
     */
    UFUNCTION(BlueprintCallable, Category = "Cmn|Mana")
    void SetCurrentMP(int32 newCurrentMP);

public:

    // ----- MP操作 -----

    /**
     * MPを消費する
     *
     * @param consumeAmount 消費量
     * @return 消費後の現在のMP
     */
    UFUNCTION(BlueprintCallable, Category = "Cmn|Mana")
    int32 ConsumeMana(int32 consumeAmount);

    /**
     * MPを回復する
     *
     * @param recoverAmount 回復量
     * @return 回復後の現在のMP
     */
    UFUNCTION(BlueprintCallable, Category = "Cmn|Mana")
    int32 RecoverMana(int32 recoverAmount);

    /**
     * 現在のMPを最大MPまで戻す
     *
     * @return 更新後の現在のMP
     */
    UFUNCTION(BlueprintCallable, Category = "Cmn|Mana")
    int32 ResetMP();

public:

    // ----- 状態判定 -----

    /**
     * 指定量以上のMPを持っているか判定する
     *
     * @param requiredAmount 必要MP
     * @return 足りているならtrue
     */
    UFUNCTION(BlueprintPure, Category = "Cmn|Mana")
    bool HasEnoughMana(int32 requiredAmount) const;

private:

    // ----- MP状態 -----

    // 最大MP
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cmn|Mana", meta = (ClampMin = "0", AllowPrivateAccess = "true"))
    int32 maxMP = 5;

    // 現在のMP
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cmn|Mana", meta = (AllowPrivateAccess = "true"))
    int32 currentMP = 5;
};
