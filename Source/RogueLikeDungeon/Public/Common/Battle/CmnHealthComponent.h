// CmnHealthComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CmnHealthComponent.generated.h"

/**
 * 共通HP管理用Component
 * 最大HP・現在HP・ダメージ・回復・生死判定を管理する
 */
UCLASS(ClassGroup = (Common), meta = (BlueprintSpawnableComponent))
class ROGUELIKEDUNGEON_API UCmnHealthComponent : public UActorComponent
{
    GENERATED_BODY()

public:

    /** HP管理Componentを初期化する */
    UCmnHealthComponent();

protected:

    // ----- UActorComponent -----

    virtual void BeginPlay() override;

public:

    // ----- Getter -----

    /**
     * 最大HPを取得する
     *
     * @return 最大HP
     */
    UFUNCTION(BlueprintPure, Category = "Cmn|Health")
    int32 GetMaxHP() const
    {
        return maxHP;
    }

    /**
     * 現在HPを取得する
     *
     * @return 現在HP
     */
    UFUNCTION(BlueprintPure, Category = "Cmn|Health")
    int32 GetCurrentHP() const
    {
        return currentHP;
    }

public:

    // ----- Setter -----

    /**
     * 最大HPを設定する
     *
     * @param newMaxHP 更新後の最大HP
     */
    UFUNCTION(BlueprintCallable, Category = "Cmn|Health")
    void SetMaxHP(int32 newMaxHP);

    /**
     * 現在HPを設定する
     *
     * @param newCurrentHP 更新後の現在HP
     */
    UFUNCTION(BlueprintCallable, Category = "Cmn|Health")
    void SetCurrentHP(int32 newCurrentHP);

public:

    // ----- HP操作 -----

    /**
     * ダメージを適用する
     *
     * @param damageAmount ダメージ量
     * @return 適用後の現在HP
     */
    UFUNCTION(BlueprintCallable, Category = "Cmn|Health")
    int32 ApplyDamage(int32 damageAmount);

    /**
     * 回復を適用する
     *
     * @param healAmount 回復量
     * @return 適用後の現在HP
     */
    UFUNCTION(BlueprintCallable, Category = "Cmn|Health")
    int32 Heal(int32 healAmount);

    /**
     * 現在HPを最大HPまで戻す
     *
     * @return 更新後の現在HP
     */
    UFUNCTION(BlueprintCallable, Category = "Cmn|Health")
    int32 ResetHP();

public:

    // ----- 状態判定 -----

    /**
     * 生存中か判定する
     *
     * @return 生存中ならtrue
     */
    UFUNCTION(BlueprintPure, Category = "Cmn|Health")
    bool IsAlive() const;

    /**
     * 戦闘不能か判定する
     *
     * @return 戦闘不能ならtrue
     */
    UFUNCTION(BlueprintPure, Category = "Cmn|Health")
    bool IsDead() const;

private:

    // ----- HP状態 -----

    // 最大HP
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cmn|Health", meta = (ClampMin = "1", AllowPrivateAccess = "true"))
    int32 maxHP = 10;

    // 現在HP
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cmn|Health", meta = (AllowPrivateAccess = "true"))
    int32 currentHP = 10;
};
