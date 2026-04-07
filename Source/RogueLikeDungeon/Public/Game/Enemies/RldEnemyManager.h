// RldEnemyManager.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RldEnemyManager.generated.h"

class ARldEnemyBase;

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

    // ----- エネミー一覧 -----

    // レベル上のエネミー一覧
    UPROPERTY(Transient)
    TArray<TObjectPtr<ARldEnemyBase>> enemyList;
};
