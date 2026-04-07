// RldEnemyBase.h

#pragma once

#include "CoreMinimal.h"
#include "Common/Grid/CmnGridActorBase.h"
#include "Common/Turn/CmnTurnActorInterface.h"
#include "RldEnemyBase.generated.h"

class UCmnHealthComponent;
class ARldGridManager;
class ARldPlayerCharacter;

/**
 * RogueLikeDungeon用エネミー基底Actor
 * グリッド座標・HP・ターン行動を持つ
 */
UCLASS()
class ROGUELIKEDUNGEON_API ARldEnemyBase
    : public ACmnGridActorBase
    , public ICmnTurnActorInterface
{
    GENERATED_BODY()

public:

    /** エネミーActorを初期化する */
    ARldEnemyBase();

protected:

    // ----- AActor -----

    virtual void BeginPlay() override;

public:

    // ----- ICmnTurnActorInterface -----

    /**
     * 1ターン分の行動を実行する
     */
    virtual void ExecuteTurn_Implementation() override;

public:

    // ----- Getter -----

    /**
     * HP管理Componentを取得する
     *
     * @return HP管理Component
     */
    UFUNCTION(BlueprintPure, Category = "Rld|Enemy")
    UCmnHealthComponent* GetHealthComponent() const
    {
        return healthComponent;
    }

    /**
     * 初期グリッド座標を取得する
     *
     * @return 初期グリッド座標
     */
    UFUNCTION(BlueprintPure, Category = "Rld|Enemy")
    FIntPoint GetInitialGridCoord() const
    {
        return initialGridCoord;
    }

public:

    // ----- 初期状態管理 -----

    /**
     * 初期グリッド座標を設定する
     *
     * @param newInitialGridCoord 更新後の初期グリッド座標
     */
    UFUNCTION(BlueprintCallable, Category = "Rld|Enemy")
    void SetInitialGridCoord(const FIntPoint& newInitialGridCoord);

    /** 初期状態へ戻す */
    UFUNCTION(BlueprintCallable, Category = "Rld|Enemy")
    void ResetToInitialState();

private:

    // ----- 管理Actor取得 -----

    /** グリッド管理Actorを取得する */
    void ResolveGridManager();

    /** プレイヤーキャラクターを取得する */
    void ResolvePlayerCharacter();

private:

    // ----- 行動処理 -----

    /**
     * プレイヤー方向への移動候補を求める
     *
     * @param outTargetGridCoord 次に移動したいグリッド座標
     * @return 移動候補を決定できた場合はtrue
     */
    bool TryBuildNextMoveTarget(FIntPoint& outTargetGridCoord) const;

private:

    // ----- 管理Actor参照 -----

    // グリッド管理Actor参照
    UPROPERTY(Transient)
    TObjectPtr<ARldGridManager> gridManager = nullptr;

    // プレイヤーキャラクター参照
    UPROPERTY(Transient)
    TObjectPtr<ARldPlayerCharacter> playerCharacter = nullptr;

private:

    // ----- Component -----

    // HP管理Component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rld|Enemy", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCmnHealthComponent> healthComponent = nullptr;

private:

    // ----- 初期状態 -----

    // 初期グリッド座標
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rld|Enemy", meta = (AllowPrivateAccess = "true"))
    FIntPoint initialGridCoord = FIntPoint::ZeroValue;
};
