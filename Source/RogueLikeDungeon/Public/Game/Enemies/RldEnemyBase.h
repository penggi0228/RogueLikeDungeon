// RldEnemyBase.h

#pragma once

#include "CoreMinimal.h"
#include "Common/Grid/CmnGridActorBase.h"
#include "Common/Turn/CmnTurnActorInterface.h"
#include "Game/Status/RldStatusTypes.h"
#include "RldEnemyBase.generated.h"

class UDataTable;
class UCmnHealthComponent;
class UCmnManaComponent;
class URldStatusEffectComponent;

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
     * HP管理コンポーネントを取得する
     *
     * @return HP管理コンポーネント
     */
    UFUNCTION(BlueprintPure, Category = "Rld|Enemy")
    UCmnHealthComponent* GetHealthComponent() const
    {
        return healthComponent;
    }

    /**
     * MP管理コンポーネントを取得する
     *
     * @return MP管理コンポーネント
     */
    UFUNCTION(BlueprintPure, Category = "Rld|Enemy")
    UCmnManaComponent* GetManaComponent() const
    {
        return manaComponent;
    }

    /**
     * 状態異常管理コンポーネントを取得する
     *
     * @return 状態異常管理コンポーネント
     */
    UFUNCTION(BlueprintPure, Category = "Rld|Enemy")
    URldStatusEffectComponent* GetStatusEffectComponent() const
    {
        return statusEffectComponent;
    }

    /**
     * 現在の戦闘ステータスを取得する
     *
     * @return 現在の戦闘ステータス
     */
    UFUNCTION(BlueprintPure, Category = "Rld|Enemy|Status")
    const FRldBattleStatus& GetCurrentBattleStatus() const
    {
        return currentBattleStatus;
    }

    /**
     * 壁マスの通過可否を取得する
     *
     * @return 壁マスを通過できる場合はtrue
     */
    UFUNCTION(BlueprintPure, Category = "Rld|Enemy|Movement")
    bool CanPassThroughWalls() const
    {
        return bCanPassThroughWalls;
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

    // ----- ステータス -----

    /** エネミーステータス定義を読み込む */
    void LoadEnemyStatusDefinition();

    /** 戦闘ステータスをコンポーネントへ反映する */
    void ApplyBattleStatusToComponents();

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

    // ----- ステータス設定 -----

    // エネミーステータスのDataTable
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rld|Enemy|Status", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UDataTable> enemyStatusDataTable = nullptr;

    // エネミーステータスのRowName
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rld|Enemy|Status", meta = (AllowPrivateAccess = "true"))
    FName enemyStatusRowName = TEXT("Enemy_000");

    // 現在の戦闘ステータス
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rld|Enemy|Status", meta = (AllowPrivateAccess = "true"))
    FRldBattleStatus currentBattleStatus;

    // 壁マスの通過可否
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rld|Enemy|Movement", meta = (AllowPrivateAccess = "true"))
    bool bCanPassThroughWalls = false;

private:

    // ----- 管理Actor参照 -----

    // グリッド管理Actor参照
    UPROPERTY(Transient)
    TObjectPtr<ARldGridManager> gridManager = nullptr;

    // プレイヤーキャラクター参照
    UPROPERTY(Transient)
    TObjectPtr<ARldPlayerCharacter> playerCharacter = nullptr;

private:

    // ----- コンポーネント -----

    // HP管理コンポーネント
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rld|Enemy", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCmnHealthComponent> healthComponent = nullptr;

    // MP管理コンポーネント
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rld|Enemy", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCmnManaComponent> manaComponent = nullptr;

    // 状態異常管理コンポーネント
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rld|Enemy", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<URldStatusEffectComponent> statusEffectComponent = nullptr;

private:

    // ----- 初期状態 -----

    // 初期グリッド座標
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rld|Enemy", meta = (AllowPrivateAccess = "true"))
    FIntPoint initialGridCoord = FIntPoint::ZeroValue;
};
