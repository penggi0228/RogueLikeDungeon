// RldTurnManager.cpp

#include "Game/Turn/RldTurnManager.h"

#include "Kismet/GameplayStatics.h"

#include "Game/Enemies/RldEnemyManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogRldTurnManager, Log, All);

/** ターン管理Actorを初期化する */
ARldTurnManager::ARldTurnManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

/** 開始時処理 */
void ARldTurnManager::BeginPlay()
{
    Super::BeginPlay();

    // 各管理Actorを取得
    ResolveEnemyManager();
}

/** ターンを1進める */
int32 ARldTurnManager::AdvanceTurn()
{
    // エネミー管理Actor取得時は先に敵ターンを実行
    if (enemyManager)
    {
        enemyManager->ExecuteEnemyTurn();
    }
    else
    {
        UE_LOG(
            LogRldTurnManager,
            Warning,
            TEXT("AdvanceTurn: EnemyManager未取得のためエネミーターンを実行しません")
        );
    }

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

/** エネミー管理Actorを取得する */
void ARldTurnManager::ResolveEnemyManager()
{
    AActor* foundActor = UGameplayStatics::GetActorOfClass(GetWorld(), ARldEnemyManager::StaticClass());
    enemyManager = Cast<ARldEnemyManager>(foundActor);

    if (!enemyManager)
    {
        UE_LOG(
            LogRldTurnManager,
            Warning,
            TEXT("ResolveEnemyManager: ARldEnemyManagerがレベル上に見つからない")
        );
        return;
    }

    UE_LOG(
        LogRldTurnManager,
        Log,
        TEXT("ResolveEnemyManager: 名前=%s クラス=%s エネミー数=%d"),
        *enemyManager->GetName(),
        *enemyManager->GetClass()->GetName(),
        enemyManager->GetEnemyCount()
    );
}
