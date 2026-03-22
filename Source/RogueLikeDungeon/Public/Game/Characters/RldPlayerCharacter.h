// RldPlayerCharacter.h

#pragma once

#include "CoreMinimal.h"
#include "Common/Characters/CmnPlayerCharacterGridBase.h"
#include "RldPlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class ARldGridManager;

/**
 * RogueLikeDungeon用プレイヤーCharacter
 *
 * 共通グリッド移動Characterのベースクラスを継承し、
 * ゲーム固有の移動処理とカメラ処理を実装する
 *
 * 調整系の値はBlueprint派生クラス側で変更しやすいように
 * UPROPERTYで公開している
 */
UCLASS()
class ROGUELIKEDUNGEON_API ARldPlayerCharacter : public ACmnPlayerCharacterGridBase
{
    GENERATED_BODY()

public:

    /** コンストラクタ */
    ARldPlayerCharacter();

protected:

    // ----- AActor -----

    virtual void BeginPlay() override;

public:

    // ----- ACmnPlayerCharacterBase -----

    /**
     * 移動方向入力を受け取る
     *
     * @param Direction 確定した移動方向
     */
    virtual void RequestMoveDirection(const FIntPoint& Direction) override;

    /**
     * カメラ視点入力を受け取る
     *
     * @param Axis 視点入力軸
     */
    virtual void RequestLookInput(const FVector2D& Axis) override;

    /**
     * カメラズーム入力を受け取る
     *
     * @param Value ズーム入力値
     */
    virtual void RequestZoomInput(float Value) override;

public:

    // ----- カメラ方向取得 -----

    /**
     * カメラの平面前方向を取得する
     *
     * Z成分を除外した正規化ベクトルを返す
     *
     * @return カメラの平面前方向
     */
    FVector GetCameraPlanarForward() const;

    /**
     * カメラの平面右方向を取得する
     *
     * Z成分を除外した正規化ベクトルを返す
     *
     * @return カメラの平面右方向
     */
    FVector GetCameraPlanarRight() const;

private:

    // ----- グリッド管理取得 -----

    /**
     * レベル上のグリッド管理Actorを取得する
     *
     * 現段階ではレベル上に1個だけ配置する前提で検索する
     * 将来的にはGameModeや生成処理から明示的に受け取る形へ拡張可能
     */
    void ResolveGridManager();

private:

    // ----- 移動処理 -----

    /**
     * 移動入力を処理する
     *
     * 現段階では
     * ・次グリッド座標を計算
     * ・グリッド管理へ通行可否を問い合わせ
     * ・通行可能なら座標を更新
     *
     * 将来的にはTurnManagerや占有判定もここから委譲する想定
     *
     * @param Direction 確定した移動方向
     */
    void HandleMoveRequest(const FIntPoint& Direction);

private:

    // ----- カメラ初期設定 -----

    /**
     * 初期カメラ設定を適用する
     *
     * Blueprint派生クラスで調整した初期値を
     * 実行時に確実にSpringArmへ反映するために使用する
     */
    void ApplyInitialCameraSettings();

private:

    // ----- グリッド管理参照 -----

    /** レベル上のグリッド管理Actor参照 */
    UPROPERTY(Transient)
    TObjectPtr<ARldGridManager> gridManager = nullptr;

private:

    // ----- カメラコンポーネント -----

    /** カメラ用スプリングアーム */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rld|Camera", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USpringArmComponent> springArm = nullptr;

    /** プレイヤーカメラ */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rld|Camera", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCameraComponent> playerCamera = nullptr;

private:

    // ----- カメラ回転設定 -----

    /** カメラ水平回転速度 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rld|Camera|Rotate", meta = (AllowPrivateAccess = "true"))
    float cameraYawSpeed = 2.0f;

    /** カメラ垂直回転速度 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rld|Camera|Rotate", meta = (AllowPrivateAccess = "true"))
    float cameraPitchSpeed = 2.0f;

    /** カメラ最小ピッチ */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rld|Camera|Rotate", meta = (AllowPrivateAccess = "true"))
    float minCameraPitch = -80.0f;

    /** カメラ最大ピッチ */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rld|Camera|Rotate", meta = (AllowPrivateAccess = "true"))
    float maxCameraPitch = -20.0f;

private:

    // ----- カメラズーム設定 -----

    /** ズーム速度 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rld|Camera|Zoom", meta = (AllowPrivateAccess = "true"))
    float zoomSpeed = 100.0f;

    /** 最小アーム長 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rld|Camera|Zoom", meta = (AllowPrivateAccess = "true"))
    float minTargetArmLength = 300.0f;

    /** 最大アーム長 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rld|Camera|Zoom", meta = (AllowPrivateAccess = "true"))
    float maxTargetArmLength = 1200.0f;

private:

    // ----- カメラ初期設定値 -----

    /** 初期アーム長 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rld|Camera|Initial", meta = (AllowPrivateAccess = "true"))
    float initialTargetArmLength = 800.0f;

    /** 初期ピッチ */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rld|Camera|Initial", meta = (AllowPrivateAccess = "true"))
    float initialCameraPitch = -60.0f;

    /** 初期ヨー */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rld|Camera|Initial", meta = (AllowPrivateAccess = "true"))
    float initialCameraYaw = 0.0f;

    /** スプリングアームの衝突判定を使うか */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rld|Camera|Initial", meta = (AllowPrivateAccess = "true"))
    bool bEnableCameraCollisionTest = false;
};
