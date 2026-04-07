// RldEnemyManager.cpp

#include "Game/Enemies/RldEnemyManager.h"

#include "Kismet/GameplayStatics.h"

#include "Game/Enemies/RldEnemyBase.h"

DEFINE_LOG_CATEGORY_STATIC(LogRldEnemyManager, Log, All);

/** エネミー管理Actorを初期化する */
ARldEnemyManager::ARldEnemyManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

/** 開始時処理 */
void ARldEnemyManager::BeginPlay()
{
    Super::BeginPlay();

    RefreshEnemyList();
}

/** レベル上のエネミー一覧を再取得する */
void ARldEnemyManager::RefreshEnemyList()
{
    TArray<AActor*> foundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARldEnemyBase::StaticClass(), foundActors);

    enemyList.Empty();

    for (AActor* foundActor : foundActors)
    {
        ARldEnemyBase* enemy = Cast<ARldEnemyBase>(foundActor);

        if (!enemy)
        {
            continue;
        }

        enemyList.Add(enemy);
    }

    UE_LOG(
        LogRldEnemyManager,
        Log,
        TEXT("RefreshEnemyList: エネミー数=%d"),
        enemyList.Num()
    );
}

/** 全エネミーのターン行動を実行する */
void ARldEnemyManager::ExecuteEnemyTurn()
{
    // 念のため都度一覧を更新
    RefreshEnemyList();

    if (enemyList.Num() == 0)
    {
        UE_LOG(
            LogRldEnemyManager,
            Log,
            TEXT("ExecuteEnemyTurn: エネミーが存在しないため処理しません")
        );
        return;
    }

    UE_LOG(
        LogRldEnemyManager,
        Log,
        TEXT("ExecuteEnemyTurn: エネミーターンを開始します エネミー数=%d"),
        enemyList.Num()
    );

    for (ARldEnemyBase* enemy : enemyList)
    {
        if (!enemy)
        {
            continue;
        }

        enemy->ExecuteTurn();
    }

    UE_LOG(
        LogRldEnemyManager,
        Log,
        TEXT("ExecuteEnemyTurn: エネミーターンを終了しました")
    );
}

/** フロア開始時に全エネミーを初期状態へ戻す */
void ARldEnemyManager::ResetAllEnemiesToInitialState()
{
    // 念のため都度一覧を更新
    RefreshEnemyList();

    if (enemyList.Num() == 0)
    {
        UE_LOG(
            LogRldEnemyManager,
            Log,
            TEXT("ResetAllEnemiesToInitialState: エネミーが存在しないため処理しません")
        );
        return;
    }

    UE_LOG(
        LogRldEnemyManager,
        Log,
        TEXT("ResetAllEnemiesToInitialState: 全エネミーの初期化を開始します エネミー数=%d"),
        enemyList.Num()
    );

    for (ARldEnemyBase* enemy : enemyList)
    {
        if (!enemy)
        {
            continue;
        }

        enemy->ResetToInitialState();
    }

    UE_LOG(
        LogRldEnemyManager,
        Log,
        TEXT("ResetAllEnemiesToInitialState: 全エネミーの初期化を終了しました")
    );
}
