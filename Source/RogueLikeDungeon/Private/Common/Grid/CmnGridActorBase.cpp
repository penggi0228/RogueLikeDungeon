// CmnGridActorBase.cpp

#include "Common/Grid/CmnGridActorBase.h"

DEFINE_LOG_CATEGORY_STATIC(LogCmnGridActorBase, Log, All);

/** 共通グリッドActorを初期化する */
ACmnGridActorBase::ACmnGridActorBase()
{
    PrimaryActorTick.bCanEverTick = false;
}

/**
 * 現在のグリッド座標を設定する
 */
void ACmnGridActorBase::SetCurrentGridCoord(const FIntPoint& newGridCoord)
{
    currentGridCoord = newGridCoord;

    UE_LOG(
        LogCmnGridActorBase,
        Log,
        TEXT("SetCurrentGridCoord: 現在座標=(%d,%d)"),
        currentGridCoord.X,
        currentGridCoord.Y
    );
}
