// RldTurnManager.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Game/Turn/RldActionTypes.h"
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
     * プレイヤー行動完了後にターンを1進める
     *
     * @param playerActor 行動したプレイヤーActor
     * @param actionType 完了した行動種別
     * @return 更新後のターン数
     */
    UFUNCTION(BlueprintCallable, Category = "Rld|Turn")
    int32 AdvanceTurnByPlayerAction(AActor* playerActor, ERldActionType actionType);

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

    // ----- ターン進行 -----

    /**
     * ターン進行本体を処理する
     *
     * @param actionActor 行動したActor
     * @param actionType 完了した行動種別
     * @param sourceFunctionName 呼び出し元関数名
     * @return 更新後のターン数
     */
    int32 AdvanceTurnInternal(
        AActor* actionActor,
        ERldActionType actionType,
        const TCHAR* sourceFunctionName
    );

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
