// RldTurnManager.cpp

#include "Game/Turn/RldTurnManager.h"

#include "Kismet/GameplayStatics.h"

#include "Game/Enemies/RldEnemyManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogRldTurnManager, Log, All);

namespace
{
    /** 行動種別のログ表示名を取得する */
    const TCHAR* GetActionTypeLogText(ERldActionType actionType)
    {
        switch (actionType)
        {
        case ERldActionType::Move:
            return TEXT("移動");

        case ERldActionType::StepInPlace:
            return TEXT("足踏み");

        case ERldActionType::Attack:
            return TEXT("攻撃");

        case ERldActionType::Face:
            return TEXT("向き変更");

        case ERldActionType::None:
        default:
            return TEXT("なし");
        }
    }
}

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
    return AdvanceTurnInternal(
        nullptr,
        ERldActionType::None,
        TEXT("AdvanceTurn")
    );
}

/** プレイヤー行動完了後にターンを1進める */
int32 ARldTurnManager::AdvanceTurnByPlayerAction(AActor* playerActor, ERldActionType actionType)
{
    UE_LOG(
        LogRldTurnManager,
        Log,
        TEXT("AdvanceTurnByPlayerAction: プレイヤー行動完了 Actor=%s 行動=%s 現在のターン数=%d"),
        *GetNameSafe(playerActor),
        GetActionTypeLogText(actionType),
        currentTurnIndex
    );

    return AdvanceTurnInternal(
        playerActor,
        actionType,
        TEXT("AdvanceTurnByPlayerAction")
    );
}

/** ターン数を初期状態へ戻す */
int32 ARldTurnManager::ResetTurn()
{
    currentTurnIndex = 1;

    UE_LOG(
        LogRldTurnManager,
        Log,
        TEXT("ResetTurn: ターン初期化完了 現在のターン数=%d"),
        currentTurnIndex
    );

    return currentTurnIndex;
}

/** エネミー管理Actorを取得する */
void ARldTurnManager::ResolveEnemyManager()
{
    AActor* foundActor = UGameplayStatics::GetActorOfClass(GetWorld(), ARldEnemyManager::StaticClass());
    enemyManager = Cast<ARldEnemyManager>(foundActor);

    // エネミー管理Actor未取得時は敵ターンを実行しない
    if (!enemyManager)
    {
        UE_LOG(
            LogRldTurnManager,
            Warning,
            TEXT("ResolveEnemyManager: ARldEnemyManagerがレベル上に見つからないため敵ターンの処理が制限されます")
        );

        return;
    }

    UE_LOG(
        LogRldTurnManager,
        Verbose,
        TEXT("ResolveEnemyManager: EnemyManager=%s クラス=%s エネミー数=%d"),
        *GetNameSafe(enemyManager),
        *GetNameSafe(enemyManager->GetClass()),
        enemyManager->GetEnemyCount()
    );
}

/** ターン進行本体を処理する */
int32 ARldTurnManager::AdvanceTurnInternal(
    AActor* actionActor,
    ERldActionType actionType,
    const TCHAR* sourceFunctionName
)
{
    const int32 previousTurnIndex = currentTurnIndex;

    UE_LOG(
        LogRldTurnManager,
        Verbose,
        TEXT("%s: ターン進行開始 Actor=%s 行動=%s 現在のターン数=%d"),
        sourceFunctionName,
        *GetNameSafe(actionActor),
        GetActionTypeLogText(actionType),
        currentTurnIndex
    );

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
            TEXT("%s: EnemyManager未取得のためエネミーターンを実行しません 現在のターン数=%d"),
            sourceFunctionName,
            currentTurnIndex
        );
    }

    ++currentTurnIndex;

    UE_LOG(
        LogRldTurnManager,
        Log,
        TEXT("%s: ターン進行完了 Actor=%s 行動=%s 前のターン数=%d 現在のターン数=%d"),
        sourceFunctionName,
        *GetNameSafe(actionActor),
        GetActionTypeLogText(actionType),
        previousTurnIndex,
        currentTurnIndex
    );

    return currentTurnIndex;
}
