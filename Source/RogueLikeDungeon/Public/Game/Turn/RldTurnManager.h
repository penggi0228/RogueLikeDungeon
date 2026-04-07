// RldTurnManager.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RldTurnManager.generated.h"

class ARldEnemyManager;

/**
 * ターン進行を管理するActor
 * 現在のターン数の保持と更新を行う
 */
UCLASS()
class ROGUELIKEDUNGEON_API ARldTurnManager : public AActor
{
    GENERATED_BODY()

public:

    /** ターン管理Actorを初期化する */
    ARldTurnManager();

protected:

    // ----- AActor -----

    virtual void BeginPlay() override;

public:

    // ----- ターン操作 -----

    /**
     * ターンを1進める
     *
     * @return 更新後のターン数
     */
    UFUNCTION(BlueprintCallable, Category = "Rld|Turn")
    int32 AdvanceTurn();

    /**
     * ターン数を初期状態へ戻す
     *
     * @return リセット後のターン数
     */
    UFUNCTION(BlueprintCallable, Category = "Rld|Turn")
    int32 ResetTurn();

public:

    // ----- Getter -----

    /** 現在のターン数を取得する */
    UFUNCTION(BlueprintPure, Category = "Rld|Turn")
    int32 GetCurrentTurnIndex() const
    {
        return currentTurnIndex;
    }

private:

    // ----- 管理Actor取得 -----

    /** エネミー管理Actorを取得する */
    void ResolveEnemyManager();

private:

    // ----- 管理Actor参照 -----

    // エネミー管理Actor参照
    UPROPERTY(Transient)
    TObjectPtr<ARldEnemyManager> enemyManager = nullptr;

private:

    // ----- ターンの状態 -----

    // 現在のターン数(1始まり)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rld|Turn", meta = (AllowPrivateAccess = "true"))
    int32 currentTurnIndex = 1;
};
