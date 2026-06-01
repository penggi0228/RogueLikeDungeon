// RldPlayerCharacter.h

#pragma once

#include "CoreMinimal.h"
#include "Common/Characters/CmnPlayerCharacterGridBase.h"
#include "RldPlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class ARldGridManager;
class ARldTurnManager;
class ARldFloorManager;

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

    // ----- ゲーム固有行動 -----

    /** 待機行動を実行する */
    UFUNCTION(BlueprintCallable, Category = "Rld|Turn")
    void RequestWaitAction();

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

private:

    // ----- 移動処理 -----

    /**
     * 移動入力を処理する
     *
     * @param Direction 移動方向
     */
    void HandleMoveRequest(const FIntPoint& Direction);

private:

    // ----- 待機処理 -----

    /** 待機行動を処理する */
    void HandleWaitRequest();

private:

    // ----- 行動処理 -----

    /** 通常攻撃行動を処理する */
    void HandleAttackRequest();

    /** インタラクト行動を処理する */
    void HandleInteractRequest();

private:

    // ----- カメラ初期設定 -----

    /** 初期カメラ設定を適用する */
    void ApplyInitialCameraSettings();

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

    // カメラ最小ピッチ
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rld|Camera|Rotate", meta = (AllowPrivateAccess = "true"))
    float minCameraPitch = -80.0f;

    // カメラ最大ピッチ
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

    // 初期ピッチ
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rld|Camera|Initial", meta = (AllowPrivateAccess = "true"))
    float initialCameraPitch = -60.0f;

    // 初期ヨー
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rld|Camera|Initial", meta = (AllowPrivateAccess = "true"))
    float initialCameraYaw = 0.0f;

    // スプリングアームの衝突判定使用フラグ
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rld|Camera|Initial", meta = (AllowPrivateAccess = "true"))
    bool bEnableCameraCollisionTest = false;
};
