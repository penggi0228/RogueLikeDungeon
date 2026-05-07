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
        Log,
        TEXT("RefreshEnemyList: エネミー数=%d 自動生成エネミー数=%d"),
        enemyList.Num(),
        runtimeSpawnedEnemies.Num()
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

        // BlueprintNativeEventのInterface関数は直接呼ぶと落ちるため、Execute_関数経由で呼び出す
        ICmnTurnActorInterface::Execute_ExecuteTurn(enemy);
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

/** 自動生成フロア用エネミーをスポーンする */
void ARldEnemyManager::SpawnEnemiesForProceduralFloor(
    const FRldFloorDefinition& floorDefinition,
    const FCmnGridLayoutBuildResult& floorLayout,
    ARldGridManager* gridManager
)
{
    // 前フロアで自動生成したエネミーを先に破棄
    DestroyAllRuntimeSpawnedEnemies();

    // GridManager未取得時はスポーンできない
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
            TEXT("SpawnEnemiesForProceduralFloor: proceduralEnemyClass未設定のためスポーンしません")
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
            TEXT("SpawnEnemiesForProceduralFloor: スポーン数が0のためスポーンしません")
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
            TEXT("SpawnEnemiesForProceduralFloor: スポーン候補が存在しないためスポーンしません")
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

        UE_LOG(
            LogRldEnemyManager,
            Log,
            TEXT("SpawnEnemiesForProceduralFloor: エネミーを生成しました グリッド座標=(%d,%d) 名前=%s"),
            spawnGridCoord.X,
            spawnGridCoord.Y,
            *spawnedEnemy->GetName()
        );
    }

    // 生成後のエネミー一覧を再取得
    RefreshEnemyList();
}

/** 自動生成したエネミーをすべて破棄する */
void ARldEnemyManager::DestroyAllRuntimeSpawnedEnemies()
{
    if (runtimeSpawnedEnemies.Num() == 0)
    {
        UE_LOG(
            LogRldEnemyManager,
            Log,
            TEXT("DestroyAllRuntimeSpawnedEnemies: 自動生成エネミーが存在しないため処理しません")
        );
        return;
    }

    UE_LOG(
        LogRldEnemyManager,
        Log,
        TEXT("DestroyAllRuntimeSpawnedEnemies: 自動生成エネミー破棄を開始します 件数=%d"),
        runtimeSpawnedEnemies.Num()
    );

    for (ARldEnemyBase* enemy : runtimeSpawnedEnemies)
    {
        if (!enemy)
        {
            continue;
        }

        enemy->Destroy();
    }

    runtimeSpawnedEnemies.Empty();
    RefreshEnemyList();

    UE_LOG(
        LogRldEnemyManager,
        Log,
        TEXT("DestroyAllRuntimeSpawnedEnemies: 自動生成エネミー破棄を終了しました")
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
