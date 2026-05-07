// RldEnemyManager.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Common/ProcGen/CmnGridLayoutTypes.h"
#include "Game/Floor/RldFloorDefinition.h"
#include "RldEnemyManager.generated.h"

class ARldEnemyBase;
class ARldGridManager;

/**
 * RogueLikeDungeon用エネミー管理Actor
 * レベル上のエネミー一覧を保持し、ターン時に一括実行する
 */
UCLASS()
class ROGUELIKEDUNGEON_API ARldEnemyManager : public AActor
{
    GENERATED_BODY()

public:

    /** エネミー管理Actorを初期化する */
    ARldEnemyManager();

protected:

    // ----- AActor -----

    virtual void BeginPlay() override;

public:

    // ----- エネミー管理 -----

    /** レベル上のエネミー一覧を再取得する */
    UFUNCTION(BlueprintCallable, Category = "Rld|Enemy")
    void RefreshEnemyList();

    /** 全エネミーのターン行動を実行する */
    UFUNCTION(BlueprintCallable, Category = "Rld|Enemy")
    void ExecuteEnemyTurn();

    /** フロア開始時に全エネミーを初期状態へ戻す */
    UFUNCTION(BlueprintCallable, Category = "Rld|Enemy")
    void ResetAllEnemiesToInitialState();

    /**
     * 自動生成フロア用エネミーをスポーンする
     *
     * @param floorDefinition フロア定義
     * @param floorLayout フロア生成結果
     * @param gridManager グリッド管理Actor
     */
    UFUNCTION(BlueprintCallable, Category = "Rld|Enemy")
    void SpawnEnemiesForProceduralFloor(
        const FRldFloorDefinition& floorDefinition,
        const FCmnGridLayoutBuildResult& floorLayout,
        ARldGridManager* gridManager
    );

    /** 自動生成したエネミーをすべて破棄する */
    UFUNCTION(BlueprintCallable, Category = "Rld|Enemy")
    void DestroyAllRuntimeSpawnedEnemies();

public:

    // ----- Getter -----

    /**
     * 現在保持しているエネミー数を取得する
     *
     * @return エネミー数
     */
    UFUNCTION(BlueprintPure, Category = "Rld|Enemy")
    int32 GetEnemyCount() const
    {
        return enemyList.Num();
    }

private:

    /** 自動生成フロア用スポーン数を解決する */
    int32 ResolveProceduralEnemySpawnCount(const FRldFloorDefinition& floorDefinition) const;

    /** 自動生成フロア用スポーンSeedを解決する */
    int32 ResolveProceduralEnemySpawnSeed(const FRldFloorDefinition& floorDefinition) const;

private:

    // ----- エネミー一覧 -----

    // レベル上のエネミー一覧
    UPROPERTY(Transient)
    TArray<TObjectPtr<ARldEnemyBase>> enemyList;

    // 自動生成でスポーンしたエネミー一覧
    UPROPERTY(Transient)
    TArray<TObjectPtr<ARldEnemyBase>> runtimeSpawnedEnemies;
};
