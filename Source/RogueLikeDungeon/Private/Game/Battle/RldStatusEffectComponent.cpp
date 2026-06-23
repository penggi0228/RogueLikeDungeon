// RldStatusEffectComponent.cpp

#include "Game/Battle/RldStatusEffectComponent.h"

#include "Common/Battle/CmnHealthComponent.h"
#include "Game/Battle/RldBattleFunctionLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(LogRldStatusEffectComponent, Log, All);

namespace
{
    // 状態異常種別をログ表示用文字列へ変換する
    FString GetStatusEffectTypeLogText(ERldStatusEffectType effectType)
    {
        const UEnum* enumPtr = StaticEnum<ERldStatusEffectType>();

        // Enum情報を取得できない場合は数値を出力して調査できるようにする
        if (!enumPtr)
        {
            return FString::Printf(TEXT("Unknown(%d)"), static_cast<int32>(effectType));
        }

        return enumPtr->GetNameStringByValue(static_cast<int64>(effectType));
    }

    // 残りターン数から継続区分を返す
    const TCHAR* GetStatusEffectDurationType(int32 remainingTurns)
    {
        // 残りターン数0以下は永続状態異常として扱う
        return (remainingTurns > 0) ? TEXT("期限付き") : TEXT("永続");
    }

    // 状態異常の再付与時に、永続を優先しつつ残りターン数が短くならない値を返す
    int32 SelectLongerRemainingTurns(int32 currentRemainingTurns, int32 incomingRemainingTurns)
    {
        // どちらかが永続の場合は永続を優先する
        if (currentRemainingTurns <= 0 || incomingRemainingTurns <= 0)
        {
            return 0;
        }

        // 期限付き同士の場合は長い方を優先する
        return FMath::Max(currentRemainingTurns, incomingRemainingTurns);
    }
}

/** 状態異常管理Componentを初期化する */
URldStatusEffectComponent::URldStatusEffectComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

/** 開始時処理 */
void URldStatusEffectComponent::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(
        LogRldStatusEffectComponent,
        Verbose,
        TEXT("BeginPlay: Owner=%s 状態異常数=%d"),
        *GetNameSafe(GetOwner()),
        statusEffects.Num()
    );
}

/** 状態異常を追加する */
bool URldStatusEffectComponent::AddStatusEffect(ERldStatusEffectType effectType, int32 remainingTurns)
{
    // Noneは状態異常として追加しない
    if (effectType == ERldStatusEffectType::None)
    {
        UE_LOG(
            LogRldStatusEffectComponent,
            Warning,
            TEXT("AddStatusEffect: Owner=%s 状態異常=Noneは追加しません"),
            *GetNameSafe(GetOwner())
        );

        return false;
    }

    // 内部的な永続表現を0に統一するため、負数は0へ丸める
    const int32 normalizedRemainingTurns = FMath::Max(0, remainingTurns);
    const FString effectTypeText = GetStatusEffectTypeLogText(effectType);
    const int32 foundIndex = FindStatusEffectIndex(effectType);

    // 既存状態異常が付与されている場合は、重複追加せず残りターン数が短くならない値を優先して更新する
    if (foundIndex != INDEX_NONE)
    {
        FRldStatusEffectState& existingState = statusEffects[foundIndex];
        const int32 previousRemainingTurns = existingState.remainingTurns;

        existingState.remainingTurns = SelectLongerRemainingTurns(
            existingState.remainingTurns,
            normalizedRemainingTurns
        );

        UE_LOG(
            LogRldStatusEffectComponent,
            Log,
            TEXT("AddStatusEffect: Owner=%s 状態異常を再付与しました 状態異常=%s 変更前残りターン数=%d 付与残りターン数=%d 更新後残りターン数=%d 継続区分=%s"),
            *GetNameSafe(GetOwner()),
            *effectTypeText,
            previousRemainingTurns,
            normalizedRemainingTurns,
            existingState.remainingTurns,
            GetStatusEffectDurationType(existingState.remainingTurns)
        );

        return true;
    }

    // 未保持の状態異常は新規状態として追加する
    FRldStatusEffectState newState;
    newState.effectType = effectType;
    newState.remainingTurns = normalizedRemainingTurns;

    statusEffects.Add(newState);

    UE_LOG(
        LogRldStatusEffectComponent,
        Log,
        TEXT("AddStatusEffect: Owner=%s 状態異常を追加しました 状態異常=%s 残りターン数=%d 継続区分=%s"),
        *GetNameSafe(GetOwner()),
        *effectTypeText,
        normalizedRemainingTurns,
        GetStatusEffectDurationType(normalizedRemainingTurns)
    );

    return true;
}

/** 状態異常を解除する */
bool URldStatusEffectComponent::RemoveStatusEffect(ERldStatusEffectType effectType)
{
    const int32 foundIndex = FindStatusEffectIndex(effectType);
    const FString effectTypeText = GetStatusEffectTypeLogText(effectType);

    // 対象状態異常がない場合は解除しない
    if (foundIndex == INDEX_NONE)
    {
        UE_LOG(
            LogRldStatusEffectComponent,
            Verbose,
            TEXT("RemoveStatusEffect: Owner=%s 対象状態異常が存在しません 状態異常=%s"),
            *GetNameSafe(GetOwner()),
            *effectTypeText
        );

        return false;
    }

    // 対象状態異常のみ解除する
    statusEffects.RemoveAt(foundIndex);

    UE_LOG(
        LogRldStatusEffectComponent,
        Log,
        TEXT("RemoveStatusEffect: Owner=%s 状態異常を解除しました 状態異常=%s"),
        *GetNameSafe(GetOwner()),
        *effectTypeText
    );

    return true;
}

/** すべての状態異常を解除する */
void URldStatusEffectComponent::ClearAllStatusEffects()
{
    // ログ出力用に解除前の件数を保持する
    const int32 previousCount = statusEffects.Num();

    statusEffects.Reset();

    UE_LOG(
        LogRldStatusEffectComponent,
        Log,
        TEXT("ClearAllStatusEffects: Owner=%s 状態異常をすべて解除しました 解除数=%d"),
        *GetNameSafe(GetOwner()),
        previousCount
    );
}

/** 指定状態異常を保持しているか判定する */
bool URldStatusEffectComponent::HasStatusEffect(ERldStatusEffectType effectType) const
{
    return FindStatusEffectIndex(effectType) != INDEX_NONE;
}

/** ターン終了時の状態異常効果を適用する */
void URldStatusEffectComponent::ApplyTurnEndEffects(UCmnHealthComponent* healthComponent)
{
    // 状態異常がない場合は処理しない
    if (statusEffects.Num() == 0)
    {
        return;
    }

    // HP管理コンポーネント未取得時は効果を適用しない
    if (!healthComponent)
    {
        UE_LOG(
            LogRldStatusEffectComponent,
            Warning,
            TEXT("ApplyTurnEndEffects: Owner=%s HealthComponentがnullのため状態異常効果を適用しません"),
            *GetNameSafe(GetOwner())
        );

        return;
    }

    // 毒状態ならターン終了時に毒ダメージを適用する
    if (HasStatusEffect(ERldStatusEffectType::Poison))
    {
        ApplyPoisonTurnEndEffect(healthComponent);
    }

    // 状態異常効果を適用した後、期限付き状態異常の残りターン数を更新する
    UpdateRemainingTurns();
}

/** 指定状態異常のIndexを取得する */
int32 URldStatusEffectComponent::FindStatusEffectIndex(ERldStatusEffectType effectType) const
{
    // 保持中の状態異常から指定種別に一致するものを探す
    for (int32 effectIndex = 0; effectIndex < statusEffects.Num(); ++effectIndex)
    {
        if (statusEffects[effectIndex].effectType == effectType)
        {
            return effectIndex;
        }
    }

    return INDEX_NONE;
}

/** 毒のターン終了時効果を適用する */
void URldStatusEffectComponent::ApplyPoisonTurnEndEffect(UCmnHealthComponent* healthComponent)
{
    // HP管理コンポーネント未取得時は毒効果を適用しない
    if (!healthComponent)
    {
        UE_LOG(
            LogRldStatusEffectComponent,
            Warning,
            TEXT("ApplyPoisonTurnEndEffect: Owner=%s HealthComponentがnullのため毒効果を適用しません"),
            *GetNameSafe(GetOwner())
        );

        return;
    }

    // 毒ダメージを取得し、HP管理コンポーネントへ反映する
    const int32 poisonDamage = URldBattleFunctionLibrary::GetPoisonTurnDamage();
    const int32 currentHPAfterDamage = healthComponent->ApplyDamage(poisonDamage);

    UE_LOG(
        LogRldStatusEffectComponent,
        Log,
        TEXT("ApplyPoisonTurnEndEffect: Owner=%s 毒ダメージ適用完了 毒ダメージ=%d 現在のHP=%d"),
        *GetNameSafe(GetOwner()),
        poisonDamage,
        currentHPAfterDamage
    );
}

/** 残りターン数を更新し、期限切れ状態異常を解除する */
void URldStatusEffectComponent::UpdateRemainingTurns()
{
    // ループ中にRemoveAtするため、末尾から逆順に走査する
    for (int32 effectIndex = statusEffects.Num() - 1; effectIndex >= 0; --effectIndex)
    {
        FRldStatusEffectState& statusEffect = statusEffects[effectIndex];

        // 0以下は永続扱いのため更新しない
        if (statusEffect.remainingTurns <= 0)
        {
            continue;
        }

        // 期限付き状態異常のみ残りターン数を減らす
        --statusEffect.remainingTurns;

        // 期限付き状態異常の残りターン数が0になった場合は期限切れとして解除する
        if (statusEffect.remainingTurns == 0)
        {
            const FString effectTypeText = GetStatusEffectTypeLogText(statusEffect.effectType);

            UE_LOG(
                LogRldStatusEffectComponent,
                Log,
                TEXT("UpdateRemainingTurns: Owner=%s 残りターン数が0になったため状態異常を解除しました 状態異常=%s"),
                *GetNameSafe(GetOwner()),
                *effectTypeText
            );

            statusEffects.RemoveAt(effectIndex);
        }
    }
}
