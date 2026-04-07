// CmnHealthComponent.cpp

#include "Common/Battle/CmnHealthComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogCmnHealthComponent, Log, All);

/** HP管理Componentを初期化する */
UCmnHealthComponent::UCmnHealthComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

/** 開始時処理 */
void UCmnHealthComponent::BeginPlay()
{
    Super::BeginPlay();

    // 開始時に現在HPが範囲外にならないよう補正
    currentHP = FMath::Clamp(currentHP, 0, maxHP);

    UE_LOG(
        LogCmnHealthComponent,
        Log,
        TEXT("BeginPlay: 最大HP=%d 現在HP=%d"),
        maxHP,
        currentHP
    );
}

/** 最大HPを設定する */
void UCmnHealthComponent::SetMaxHP(int32 newMaxHP)
{
    maxHP = FMath::Max(1, newMaxHP);
    currentHP = FMath::Clamp(currentHP, 0, maxHP);

    UE_LOG(
        LogCmnHealthComponent,
        Log,
        TEXT("SetMaxHP: 最大HP=%d 現在HP=%d"),
        maxHP,
        currentHP
    );
}

/** 現在HPを設定する */
void UCmnHealthComponent::SetCurrentHP(int32 newCurrentHP)
{
    currentHP = FMath::Clamp(newCurrentHP, 0, maxHP);

    UE_LOG(
        LogCmnHealthComponent,
        Log,
        TEXT("SetCurrentHP: 現在HP=%d"),
        currentHP
    );
}

/** ダメージを適用する */
int32 UCmnHealthComponent::ApplyDamage(int32 damageAmount)
{
    // 0以下のダメージは適用しない
    if (damageAmount <= 0)
    {
        UE_LOG(
            LogCmnHealthComponent,
            Warning,
            TEXT("ApplyDamage: 0以下のダメージは適用しません ダメージ量=%d"),
            damageAmount
        );

        return currentHP;
    }

    currentHP = FMath::Clamp(currentHP - damageAmount, 0, maxHP);

    UE_LOG(
        LogCmnHealthComponent,
        Log,
        TEXT("ApplyDamage: ダメージ量=%d 現在HP=%d"),
        damageAmount,
        currentHP
    );

    return currentHP;
}

/** 回復を適用する */
int32 UCmnHealthComponent::Heal(int32 healAmount)
{
    // 0以下の回復は適用しない
    if (healAmount <= 0)
    {
        UE_LOG(
            LogCmnHealthComponent,
            Warning,
            TEXT("Heal: 0以下の回復は適用しません 回復量=%d"),
            healAmount
        );

        return currentHP;
    }

    currentHP = FMath::Clamp(currentHP + healAmount, 0, maxHP);

    UE_LOG(
        LogCmnHealthComponent,
        Log,
        TEXT("Heal: 回復量=%d 現在HP=%d"),
        healAmount,
        currentHP
    );

    return currentHP;
}

/** 現在HPを最大HPまで戻す */
int32 UCmnHealthComponent::ResetHP()
{
    currentHP = maxHP;

    UE_LOG(
        LogCmnHealthComponent,
        Log,
        TEXT("ResetHP: 最大HP=%d 現在HP=%d"),
        maxHP,
        currentHP
    );

    return currentHP;
}

/** 生存中か判定する */
bool UCmnHealthComponent::IsAlive() const
{
    return currentHP > 0;
}

/** 戦闘不能か判定する */
bool UCmnHealthComponent::IsDead() const
{
    return currentHP <= 0;
}
