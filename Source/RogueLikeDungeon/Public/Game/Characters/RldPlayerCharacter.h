// RldPlayerCharacter.h

#pragma once

#include "CoreMinimal.h"
#include "Common/Characters/CmnPlayerCharacterGridBase.h"
#include "Game/Status/RldStatusTypes.h"
#include "RldPlayerCharacter.generated.h"

class UDataTable;
class USpringArmComponent;
class UCameraComponent;
class UCmnHealthComponent;
class UCmnManaComponent;
class URldStatusEffectComponent;

class ARldGridManager;
class ARldTurnManager;
class ARldFloorManager;
class ARldEnemyManager;
class ARldEnemyBase;

/**
 * RogueLikeDungeon用プレイヤーキャラクター
 * ゲーム固有の移動処理とカメラ処理を行う
 */
UCLASS()
class ROGUELIKEDUNGEON_API ARldPlayerCharacter : public ACmnPlayerCharacterGridBase
{
    GENERATED_BODY()

public:

    /** プレイヤーキャラクターを初期化する */
    ARldPlayerCharacter();

protected:

    // ----- AActor -----

    virtual void BeginPlay() override;

public:

    // ----- ACmnPlayerCharacterBase -----

    /**
     * 移動方向入力を受け取る
     *
     * @param Direction 移動方向
     */
    virtual void RequestMoveDirection(const FIntPoint& Direction) override;

    /**
     * 向き変更入力を受け取る
     *
     * @param Direction 向き変更方向
     */
    virtual void RequestFaceDirection(const FIntPoint& Direction) override;

    /**
     * カメラ視点入力を受け取る
     *
     * @param Axis 視点入力値
     */
    virtual void RequestLookInput(const FVector2D& Axis) override;

    /**
     * カメラズーム入力を受け取る
     *
     * @param Value ズーム入力値
     */
    virtual void RequestZoomInput(float Value) override;

public:

    // ----- Getter -----

    /**
     * HP管理コンポーネントを取得する
     *
     * @return HP管理コンポーネント
     */
    UFUNCTION(BlueprintPure, Category = "Rld|Player")
    UCmnHealthComponent* GetHealthComponent() const
    {
        return healthComponent;
    }

    /**
     * MP管理コンポーネントを取得する
     *
     * @return MP管理コンポーネント
     */
    UFUNCTION(BlueprintPure, Category = "Rld|Player")
    UCmnManaComponent* GetManaComponent() const
    {
        return manaComponent;
    }

    /**
     * 状態異常管理コンポーネントを取得する
     *
     * @return 状態異常管理コンポーネント
     */
    UFUNCTION(BlueprintPure, Category = "Rld|Player")
    URldStatusEffectComponent* GetStatusEffectComponent() const
    {
        return statusEffectComponent;
    }

    /**
     * 現在の戦闘ステータスを取得する
     *
     * @return 現在の戦闘ステータス
     */
    UFUNCTION(BlueprintPure, Category = "Rld|Player|Status")
    const FRldBattleStatus& GetCurrentBattleStatus() const
    {
        return currentBattleStatus;
    }

    /**
     * 現在の向きを取得する
     *
     * @return 現在の向き
     */
    UFUNCTION(BlueprintPure, Category = "Rld|Player|Grid")
    FIntPoint GetCurrentFacingGridDir() const
    {
        return currentFacingGridDir;
    }

public:

    // ----- ゲーム固有行動 -----

    /**
     * 足踏み行動を実行する
     *
     * @param Direction 足踏み時に向く方向
     */
    UFUNCTION(BlueprintCallable, Category = "Rld|Action")
    void RequestStepInPlaceAction(const FIntPoint& Direction);

    /** 通常攻撃行動を実行する */
    UFUNCTION(BlueprintCallable, Category = "Rld|Action")
    void RequestAttackAction();

    /** インタラクト行動を実行する */
    UFUNCTION(BlueprintCallable, Category = "Rld|Action")
    void RequestInteractAction();

public:

    // ----- カメラ方向取得 -----

    /**
     * カメラの平面前方向を取得する
     *
     * @return カメラの平面前方向
     */
    FVector GetCameraPlanarForward() const;

    /**
     * カメラの平面右方向を取得する
     *
     * @return カメラの平面右方向
     */
    FVector GetCameraPlanarRight() const;

private:

    // ----- 管理Actor取得 -----

    /** グリッド管理Actorを取得する */
    void ResolveGridManager();

    /** ターン管理Actorを取得する */
    void ResolveTurnManager();

    /** フロア管理Actorを取得する */
    void ResolveFloorManager();

    /** エネミー管理Actorを取得する */
    void ResolveEnemyManager();

private:

    // ----- 移動処理 -----

    /**
     * 移動入力を処理する
     *
     * @param Direction 移動方向
     */
    void HandleMoveRequest(const FIntPoint& Direction);

private:

    // ----- 向き処理 -----

    /**
     * 現在の向きを設定する
     *
     * @param newFacingGridDir 更新後の向き
     */
    void SetCurrentFacingGridDir(const FIntPoint& newFacingGridDir);

    /**
     * グリッド方向が8方向として有効か判定する
     *
     * @param gridDir 判定対象方向
     * @return 有効ならtrue
     */
    bool IsValidGridDir(const FIntPoint& gridDir) const;

private:

    // ----- 足踏み処理 -----

    /**
     * 足踏み行動を処理する
     *
     * @param Direction 足踏み時に向く方向
     */
    void HandleStepInPlaceRequest(const FIntPoint& Direction);

private:

    // ----- 行動処理 -----

    /** 通常攻撃行動を処理する */
    void HandleAttackRequest();

    /**
     * 攻撃対象Actorを取得する
     *
     * @param targetGridCoord 攻撃対象座標
     * @param bOutTargetCoordInGrid 攻撃対象座標が範囲内か
     * @return 攻撃対象Actor
     */
    AActor* ResolveAttackTargetActor(const FIntPoint& targetGridCoord, bool& bOutTargetCoordInGrid) const;

    /**
     * 仮攻撃でエネミーを破棄する
     *
     * @param enemyActor 破棄対象エネミー
     * @param targetGridCoord 攻撃対象座標
     * @return 破棄成功ならtrue
     */
    bool TryDestroyEnemyByTemporaryAttack(ARldEnemyBase* enemyActor, const FIntPoint& targetGridCoord);

    /**
     * 攻撃空振りログを出力する
     *
     * @param targetGridCoord 攻撃対象座標
     * @param targetActor 攻撃対象Actor
     * @param bTargetCoordInGrid 攻撃対象座標が範囲内か
     */
    void LogAttackMiss(const FIntPoint& targetGridCoord, AActor* targetActor, bool bTargetCoordInGrid) const;

    /** インタラクト行動を処理する */
    void HandleInteractRequest();

private:

    // ----- ステータス -----

    /** プレイヤーステータス定義を読み込む */
    void LoadPlayerStatusDefinition();

    /** 戦闘ステータスをコンポーネントへ反映する */
    void ApplyBattleStatusToComponents();

private:

    // ----- カメラ初期設定 -----

    /** 初期カメラ設定を適用する */
    void ApplyInitialCameraSettings();

private:

    // ----- ステータス設定 -----

    // プレイヤーステータスのDataTable
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rld|Player|Status", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UDataTable> playerStatusDataTable = nullptr;

    // プレイヤーステータスのRowName
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rld|Player|Status", meta = (AllowPrivateAccess = "true"))
    FName playerStatusRowName = TEXT("Player_000");

    // 現在の戦闘ステータス
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rld|Player|Status", meta = (AllowPrivateAccess = "true"))
    FRldBattleStatus currentBattleStatus;

private:

    // ----- 管理Actor参照 -----

    // グリッド管理Actor参照
    UPROPERTY(Transient)
    TObjectPtr<ARldGridManager> gridManager = nullptr;

    // ターン管理Actor参照
    UPROPERTY(Transient)
    TObjectPtr<ARldTurnManager> turnManager = nullptr;

    // フロア管理Actor参照
    UPROPERTY(Transient)
    TObjectPtr<ARldFloorManager> floorManager = nullptr;

    // エネミー管理Actor参照
    UPROPERTY(Transient)
    TObjectPtr<ARldEnemyManager> enemyManager = nullptr;

private:

    // ----- グリッド行動状態 -----

    // 現在の向き
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rld|Player|Grid", meta = (AllowPrivateAccess = "true"))
    FIntPoint currentFacingGridDir = FIntPoint(0, 1);

private:

    // ----- コンポーネント -----

    // HP管理コンポーネント
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rld|Player", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCmnHealthComponent> healthComponent = nullptr;

    // MP管理コンポーネント
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rld|Player", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCmnManaComponent> manaComponent = nullptr;

    // 状態異常管理コンポーネント
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rld|Player", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<URldStatusEffectComponent> statusEffectComponent = nullptr;

private:

    // ----- カメラコンポーネント -----

    // カメラ用スプリングアーム
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rld|Camera", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USpringArmComponent> springArm = nullptr;

    // プレイヤーカメラ
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rld|Camera", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCameraComponent> playerCamera = nullptr;

private:

    // ----- カメラ回転設定 -----

    // カメラ水平回転速度
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rld|Camera|Rotate", meta = (AllowPrivateAccess = "true"))
    float cameraYawSpeed = 2.0f;

    // カメラ垂直回転速度
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rld|Camera|Rotate", meta = (AllowPrivateAccess = "true"))
    float cameraPitchSpeed = 2.0f;

    // カメラ最小Pitch
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rld|Camera|Rotate", meta = (AllowPrivateAccess = "true"))
    float minCameraPitch = -80.0f;

    // カメラ最大Pitch
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rld|Camera|Rotate", meta = (AllowPrivateAccess = "true"))
    float maxCameraPitch = -20.0f;

private:

    // ----- カメラズーム設定 -----

    // ズーム速度
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rld|Camera|Zoom", meta = (AllowPrivateAccess = "true"))
    float zoomSpeed = 100.0f;

    // 最小アーム長
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rld|Camera|Zoom", meta = (AllowPrivateAccess = "true"))
    float minTargetArmLength = 300.0f;

    // 最大アーム長
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rld|Camera|Zoom", meta = (AllowPrivateAccess = "true"))
    float maxTargetArmLength = 1200.0f;

private:

    // ----- カメラ初期設定値 -----

    // 初期アーム長
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rld|Camera|Initial", meta = (AllowPrivateAccess = "true"))
    float initialTargetArmLength = 800.0f;

    // 初期Pitch
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rld|Camera|Initial", meta = (AllowPrivateAccess = "true"))
    float initialCameraPitch = -60.0f;

    // 初期Yaw
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rld|Camera|Initial", meta = (AllowPrivateAccess = "true"))
    float initialCameraYaw = 0.0f;

    // スプリングアームの衝突判定使用フラグ
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rld|Camera|Initial", meta = (AllowPrivateAccess = "true"))
    bool bEnableCameraCollisionTest = false;
};
