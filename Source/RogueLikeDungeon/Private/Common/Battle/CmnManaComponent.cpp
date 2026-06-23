// CmnManaComponent.cpp

#include "Common/Battle/CmnManaComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogCmnManaComponent, Log, All);

/** MP管理コンポーネントを初期化する */
UCmnManaComponent::UCmnManaComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

/** 開始時処理 */
void UCmnManaComponent::BeginPlay()
{
    Super::BeginPlay();

    // 開始時に現在のMPが範囲外にならないよう補正
    currentMP = FMath::Clamp(currentMP, 0, maxMP);

    UE_LOG(
        LogCmnManaComponent,
        Verbose,
        TEXT("BeginPlay: Owner=%s 最大MP=%d 現在のMP=%d"),
        *GetNameSafe(GetOwner()),
        maxMP,
        currentMP
    );
}

/** 最大MPを設定する */
void UCmnManaComponent::SetMaxMP(int32 newMaxMP)
{
    maxMP = FMath::Max(0, newMaxMP);
    currentMP = FMath::Clamp(currentMP, 0, maxMP);

    UE_LOG(
        LogCmnManaComponent,
        Verbose,
        TEXT("SetMaxMP: Owner=%s 最大MP=%d 現在のMP=%d"),
        *GetNameSafe(GetOwner()),
        maxMP,
        currentMP
    );
}

/** 現在のMPを設定する */
void UCmnManaComponent::SetCurrentMP(int32 newCurrentMP)
{
    currentMP = FMath::Clamp(newCurrentMP, 0, maxMP);

    UE_LOG(
        LogCmnManaComponent,
        Verbose,
        TEXT("SetCurrentMP: Owner=%s 現在のMP=%d"),
        *GetNameSafe(GetOwner()),
        currentMP
    );
}

/** MPを消費する */
int32 UCmnManaComponent::ConsumeMana(int32 consumeAmount)
{
    // 0以下の消費量は適用しない
    if (consumeAmount <= 0)
    {
        UE_LOG(
            LogCmnManaComponent,
            Warning,
            TEXT("ConsumeMana: Owner=%s 0以下の消費量は適用しません 消費量=%d"),
            *GetNameSafe(GetOwner()),
            consumeAmount
        );

        return currentMP;
    }

    // MP不足時は消費しない
    if (!HasEnoughMana(consumeAmount))
    {
        UE_LOG(
            LogCmnManaComponent,
            Warning,
            TEXT("ConsumeMana: Owner=%s MP不足のため消費しません 消費量=%d 現在のMP=%d"),
            *GetNameSafe(GetOwner()),
            consumeAmount,
            currentMP
        );

        return currentMP;
    }

    currentMP = FMath::Clamp(currentMP - consumeAmount, 0, maxMP);

    UE_LOG(
        LogCmnManaComponent,
        Log,
        TEXT("ConsumeMana: Owner=%s 消費量=%d 現在のMP=%d"),
        *GetNameSafe(GetOwner()),
        consumeAmount,
        currentMP
    );

    return currentMP;
}

/** MPを回復する */
int32 UCmnManaComponent::RecoverMana(int32 recoverAmount)
{
    // 0以下の回復量は適用しない
    if (recoverAmount <= 0)
    {
        UE_LOG(
            LogCmnManaComponent,
            Warning,
            TEXT("RecoverMana: Owner=%s 0以下の回復量は適用しません 回復量=%d"),
            *GetNameSafe(GetOwner()),
            recoverAmount
        );

        return currentMP;
    }

    currentMP = FMath::Clamp(currentMP + recoverAmount, 0, maxMP);

    UE_LOG(
        LogCmnManaComponent,
        Log,
        TEXT("RecoverMana: Owner=%s 回復量=%d 現在のMP=%d"),
        *GetNameSafe(GetOwner()),
        recoverAmount,
        currentMP
    );

    return currentMP;
}

/** 現在のMPを最大MPまで戻す */
int32 UCmnManaComponent::ResetMP()
{
    currentMP = maxMP;

    UE_LOG(
        LogCmnManaComponent,
        Verbose,
        TEXT("ResetMP: Owner=%s 最大MP=%d 現在のMP=%d"),
        *GetNameSafe(GetOwner()),
        maxMP,
        currentMP
    );

    return currentMP;
}

/** 指定量以上のMPを持っているか判定する */
bool UCmnManaComponent::HasEnoughMana(int32 requiredAmount) const
{
    if (requiredAmount <= 0)
    {
        return true;
    }

    return currentMP >= requiredAmount;
}
