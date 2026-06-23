// CmnPlayerCharacterGridBase.cpp

#include "Common/Characters/CmnPlayerCharacterGridBase.h"

DEFINE_LOG_CATEGORY_STATIC(LogCmnPlayerCharacterGridBase, Log, All);

/** グリッド移動用キャラクターを初期化する */
ACmnPlayerCharacterGridBase::ACmnPlayerCharacterGridBase()
{
    PrimaryActorTick.bCanEverTick = false;
}

/** 開始時処理 */
void ACmnPlayerCharacterGridBase::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(
        LogCmnPlayerCharacterGridBase,
        Verbose,
        TEXT("BeginPlay: Actor=%s 初期グリッド座標は未設定"),
        *GetNameSafe(this)
    );
}

/** グリッド座標を設定する */
void ACmnPlayerCharacterGridBase::SetCurrentGridCoord(const FIntPoint& newGridCoord)
{
    currentGridCoord = newGridCoord;

    UE_LOG(
        LogCmnPlayerCharacterGridBase,
        Verbose,
        TEXT("SetCurrentGridCoord: Actor=%s 現在の座標=(%d,%d)"),
        *GetNameSafe(this),
        currentGridCoord.X,
        currentGridCoord.Y
    );
}
