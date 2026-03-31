// RldTurnManager.cpp

#include "Game/Turn/RldTurnManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogRldTurnManager, Log, All);

/** ターン管理Actorを初期化する */
ARldTurnManager::ARldTurnManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

/** ターンを1進める */
int32 ARldTurnManager::AdvanceTurn()
{
    ++currentTurnIndex;

    UE_LOG(
        LogRldTurnManager,
        Log,
        TEXT("AdvanceTurn: ターン数=%d"),
        currentTurnIndex
    );

    return currentTurnIndex;
}

/** ターン数を初期状態へ戻す */
int32 ARldTurnManager::ResetTurn()
{
    currentTurnIndex = 1;

    UE_LOG(
        LogRldTurnManager,
        Log,
        TEXT("ResetTurn: ターン数=%d"),
        currentTurnIndex
    );

    return currentTurnIndex;
}
