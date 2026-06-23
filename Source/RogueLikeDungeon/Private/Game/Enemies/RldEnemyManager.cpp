// RldEnemyManager.cpp

#include "Game/Enemies/RldEnemyManager.h"

#include "Kismet/GameplayStatics.h"

#include "Common/ProcGen/CmnGridSpawnHelper.h"
#include "Common/ProcGen/CmnGridSpawnTypes.h"
#include "Game/Enemies/RldEnemyBase.h"
#include "Game/Grid/RldGridManager.h"

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
        Verbose,
        TEXT("RefreshEnemyList: エネミー一覧再取得完了 エネミー数=%d 自動生成エネミー数=%d"),
        enemyList.Num(),
        runtimeSpawnedEnemies.Num()
    );
}

/** 指定エネミーを管理一覧から除外する */
bool ARldEnemyManager::RemoveEnemyFromLists(ARldEnemyBase* enemy)
{
    // 除外対象エネミー未取得時は処理しない
    if (!enemy)
    {
        UE_LOG(
            LogRldEnemyManager,
            Warning,
            TEXT("RemoveEnemyFromLists: Enemyがnullのため除外しません")
        );

        return false;
    }

    const int32 removedEnemyListCount = enemyList.RemoveAll(
        [enemy](const TObjectPtr<ARldEnemyBase>& listedEnemy)
        {
            return listedEnemy.Get() == enemy;
        }
    );

    const int32 removedRuntimeSpawnedCount = runtimeSpawnedEnemies.RemoveAll(
        [enemy](const TObjectPtr<ARldEnemyBase>& listedEnemy)
        {
            return listedEnemy.Get() == enemy;
        }
    );

    const bool bRemoved = (removedEnemyListCount > 0) || (removedRuntimeSpawnedCount > 0);

    UE_LOG(
        LogRldEnemyManager,
        Verbose,
        TEXT("RemoveEnemyFromLists: Actor=%s 管理一覧から除外しました 除外あり=%s エネミー一覧除外数=%d 自動生成一覧除外数=%d エネミー数=%d 自動生成エネミー数=%d"),
        *GetNameSafe(enemy),
        bRemoved ? TEXT("true") : TEXT("false"),
        removedEnemyListCount,
        removedRuntimeSpawnedCount,
        enemyList.Num(),
        runtimeSpawnedEnemies.Num()
    );

    return bRemoved;
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
        TEXT("ExecuteEnemyTurn: エネミーターン開始 エネミー数=%d"),
        enemyList.Num()
    );

    int32 executedEnemyCount = 0;

    for (ARldEnemyBase* enemy : enemyList)
    {
        if (!enemy)
        {
            continue;
        }

        // BlueprintNativeEventのInterface関数は直接呼ぶと落ちるため、Execute_関数経由で呼び出す
        ICmnTurnActorInterface::Execute_ExecuteTurn(enemy);
        ++executedEnemyCount;
    }

    UE_LOG(
        LogRldEnemyManager,
        Log,
        TEXT("ExecuteEnemyTurn: エネミーターン終了 実行数=%d エネミー数=%d"),
        executedEnemyCount,
        enemyList.Num()
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
        TEXT("ResetAllEnemiesToInitialState: 全エネミー初期化開始 エネミー数=%d"),
        enemyList.Num()
    );

    int32 resetEnemyCount = 0;

    for (ARldEnemyBase* enemy : enemyList)
    {
        if (!enemy)
        {
            continue;
        }

        enemy->ResetToInitialState();
        ++resetEnemyCount;
    }

    UE_LOG(
        LogRldEnemyManager,
        Log,
        TEXT("ResetAllEnemiesToInitialState: 全エネミー初期化終了 初期化数=%d エネミー数=%d"),
        resetEnemyCount,
        enemyList.Num()
    );
}

/** 自動生成フロア用エネミーをスポーンする */
void ARldEnemyManager::SpawnEnemiesForProceduralFloor(
    const FRldFloorDefinition& floorDefinition,
    const FCmnGridLayoutBuildResult& floorLayout,
    ARldGridManager* gridManager
)
{
    // GridManager未取得時はスポーンしない
    if (!gridManager)
    {
        UE_LOG(
            LogRldEnemyManager,
            Warning,
            TEXT("SpawnEnemiesForProceduralFloor: GridManagerがnullのためスポーンしません")
        );

        return;
    }

    // エネミークラス未設定時はスポーンしない
    if (!floorDefinition.proceduralEnemyClass)
    {
        UE_LOG(
            LogRldEnemyManager,
            Log,
            TEXT("SpawnEnemiesForProceduralFloor: 自動生成エネミークラスが未設定のためスポーンしません")
        );

        return;
    }

    // フロア定義からスポーン数を決定
    const int32 targetSpawnCount = ResolveProceduralEnemySpawnCount(floorDefinition);

    // スポーン数が0以下なら処理しない
    if (targetSpawnCount <= 0)
    {
        UE_LOG(
            LogRldEnemyManager,
            Log,
            TEXT("SpawnEnemiesForProceduralFloor: スポーン対象がないためスポーンしません スポーン数=%d"),
            targetSpawnCount
        );

        return;
    }

    TArray<FCmnGridSpawnExclusionRule> exclusionRules;

    // プレイヤー開始位置周辺をスポーン候補から除外
    exclusionRules.Add(
        FCmnGridSpawnExclusionRule(
            floorLayout.playerStartGridCoord,
            floorDefinition.minDistanceFromPlayerStartForEnemySpawn
        )
    );

    // 階段位置周辺をスポーン候補から除外
    exclusionRules.Add(
        FCmnGridSpawnExclusionRule(
            floorLayout.stairsGridCoord,
            floorDefinition.minDistanceFromStairsForEnemySpawn
        )
    );

    TArray<FIntPoint> candidateCells;

    // 床マス一覧からスポーン可能な候補マスを抽出
    FCmnGridSpawnHelper::BuildCandidateCells(
        floorLayout.floorCells,
        exclusionRules,
        candidateCells
    );

    // 候補マスがない場合はスポーンしない
    if (candidateCells.Num() == 0)
    {
        UE_LOG(
            LogRldEnemyManager,
            Warning,
            TEXT("SpawnEnemiesForProceduralFloor: スポーン候補が存在しないためスポーンしません 目標数=%d"),
            targetSpawnCount
        );

        return;
    }

    // 候補数を超えない範囲で実スポーン数を確定
    const int32 actualSpawnCount = FMath::Min(targetSpawnCount, candidateCells.Num());

    FRandomStream randomStream;
    randomStream.Initialize(ResolveProceduralEnemySpawnSeed(floorDefinition));

    UE_LOG(
        LogRldEnemyManager,
        Log,
        TEXT("SpawnEnemiesForProceduralFloor: スポーン開始 目標数=%d 実スポーン数=%d 候補数=%d"),
        targetSpawnCount,
        actualSpawnCount,
        candidateCells.Num()
    );

    int32 spawnedEnemyCount = 0;

    for (int32 spawnIndex = 0; spawnIndex < actualSpawnCount; ++spawnIndex)
    {
        // 候補マスからランダムに1つ選択
        const int32 candidateIndex = randomStream.RandRange(0, candidateCells.Num() - 1);
        const FIntPoint spawnGridCoord = candidateCells[candidateIndex];

        // 同じ候補マスを再利用しない
        candidateCells.RemoveAtSwap(candidateIndex);

        const FVector spawnWorldLocation = gridManager->GridToWorld(spawnGridCoord);

        FActorSpawnParameters spawnParameters;
        spawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        // 選択した候補マスへエネミーを生成
        ARldEnemyBase* spawnedEnemy = GetWorld()->SpawnActor<ARldEnemyBase>(
            floorDefinition.proceduralEnemyClass,
            spawnWorldLocation,
            FRotator::ZeroRotator,
            spawnParameters
        );

        // 生成失敗時は次の候補へ進む
        if (!spawnedEnemy)
        {
            UE_LOG(
                LogRldEnemyManager,
                Warning,
                TEXT("SpawnEnemiesForProceduralFloor: エネミー生成に失敗しました グリッド座標=(%d,%d)"),
                spawnGridCoord.X,
                spawnGridCoord.Y
            );

            continue;
        }

        // フロア遷移時に破棄できるよう自動生成エネミーとして保持
        runtimeSpawnedEnemies.Add(spawnedEnemy);
        ++spawnedEnemyCount;

        UE_LOG(
            LogRldEnemyManager,
            Verbose,
            TEXT("SpawnEnemiesForProceduralFloor: エネミー生成完了 Actor=%s グリッド座標=(%d,%d)"),
            *GetNameSafe(spawnedEnemy),
            spawnGridCoord.X,
            spawnGridCoord.Y
        );
    }

    // 生成後のエネミー一覧を再取得
    RefreshEnemyList();

    UE_LOG(
        LogRldEnemyManager,
        Log,
        TEXT("SpawnEnemiesForProceduralFloor: スポーン完了 生成数=%d 自動生成エネミー数=%d エネミー数=%d"),
        spawnedEnemyCount,
        runtimeSpawnedEnemies.Num(),
        enemyList.Num()
    );
}

/** 自動生成したエネミーをすべて破棄する */
void ARldEnemyManager::DestroyAllRuntimeSpawnedEnemies()
{
    if (runtimeSpawnedEnemies.Num() == 0)
    {
        UE_LOG(
            LogRldEnemyManager,
            Verbose,
            TEXT("DestroyAllRuntimeSpawnedEnemies: 自動生成エネミーが存在しないため処理しません")
        );

        return;
    }

    const int32 destroyTargetCount = runtimeSpawnedEnemies.Num();

    UE_LOG(
        LogRldEnemyManager,
        Log,
        TEXT("DestroyAllRuntimeSpawnedEnemies: 自動生成エネミー破棄開始 対象数=%d"),
        destroyTargetCount
    );

    int32 destroyedEnemyCount = 0;

    for (ARldEnemyBase* enemy : runtimeSpawnedEnemies)
    {
        if (!enemy)
        {
            continue;
        }

        UE_LOG(
            LogRldEnemyManager,
            Verbose,
            TEXT("DestroyAllRuntimeSpawnedEnemies: 自動生成エネミーを破棄します Actor=%s"),
            *GetNameSafe(enemy)
        );

        enemy->Destroy();
        ++destroyedEnemyCount;
    }

    runtimeSpawnedEnemies.Empty();
    RefreshEnemyList();

    UE_LOG(
        LogRldEnemyManager,
        Log,
        TEXT("DestroyAllRuntimeSpawnedEnemies: 自動生成エネミー破棄終了 対象数=%d 破棄数=%d エネミー数=%d"),
        destroyTargetCount,
        destroyedEnemyCount,
        enemyList.Num()
    );
}

/** 自動生成フロア用スポーン数を解決する */
int32 ARldEnemyManager::ResolveProceduralEnemySpawnCount(const FRldFloorDefinition& floorDefinition) const
{
    const int32 minEnemyCount = FMath::Max(0, floorDefinition.minProceduralEnemyCount);
    const int32 maxEnemyCount = FMath::Max(minEnemyCount, floorDefinition.maxProceduralEnemyCount);

    if (maxEnemyCount <= 0)
    {
        return 0;
    }

    if (floorDefinition.bUseRandomSeed)
    {
        return FMath::RandRange(minEnemyCount, maxEnemyCount);
    }

    FRandomStream randomStream;
    randomStream.Initialize(ResolveProceduralEnemySpawnSeed(floorDefinition));

    return randomStream.RandRange(minEnemyCount, maxEnemyCount);
}

/** 自動生成フロア用スポーンSeedを解決する */
int32 ARldEnemyManager::ResolveProceduralEnemySpawnSeed(const FRldFloorDefinition& floorDefinition) const
{
    if (floorDefinition.bUseRandomSeed)
    {
        return FMath::Rand();
    }

    return floorDefinition.randomSeed + 1000;
}
