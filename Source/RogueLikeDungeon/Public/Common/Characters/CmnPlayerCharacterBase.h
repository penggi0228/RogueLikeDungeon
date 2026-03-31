// CmnPlayerCharacterBase.hPlayerCharacter

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CmnPlayerCharacterBase.generated.h"

/**
 * プレイヤーキャラクターの共通ベースクラス
 * 入力を受け取り、派生クラスへ処理を委譲する
 */
UCLASS(Abstract)
class ROGUELIKEDUNGEON_API ACmnPlayerCharacterBase : public ACharacter
{
    GENERATED_BODY()

public:

    /** プレイヤーキャラクターを初期化する */
    ACmnPlayerCharacterBase();

public:

    /**
     * 移動入力を受け取る
     *
     * @param Direction 移動方向
     */
    UFUNCTION(BlueprintCallable, Category = "Cmn|Input")
    virtual void RequestMoveDirection(const FIntPoint& Direction);

    /**
     * 向き変更入力を受け取る
     *
     * @param Direction プレイヤーキャラクターが正面を向く方向
     */
    UFUNCTION(BlueprintCallable, Category = "Cmn|Input")
    virtual void RequestFaceDirection(const FIntPoint& Direction);

    /**
     * カメラ視点入力を受け取る
     *
     * @param Axis カメラ視点入力値
     */
    UFUNCTION(BlueprintCallable, Category = "Cmn|Input")
    virtual void RequestLookInput(const FVector2D& Axis);

    /**
     * カメラズーム入力を受け取る
     *
     * @param Value カメラズーム入力値
     */
    UFUNCTION(BlueprintCallable, Category = "Cmn|Input")
    virtual void RequestZoomInput(float Value);

    /**
     * 移動入力を受け付け可能か判定する
     *
     * @return 受付可能ならtrue
     */
    UFUNCTION(BlueprintCallable, Category = "Cmn|Input")
    virtual bool CanAcceptMoveInput() const;

    /**
     * 向き変更入力を受け付け可能か判定する
     *
     * @return 受付可能ならtrue
     */
    UFUNCTION(BlueprintCallable, Category = "Cmn|Input")
    virtual bool CanAcceptFaceInput() const;

    /**
     * カメラ視点入力を受け付け可能か判定する
     *
     * @return 受付可能ならtrue
     */
    UFUNCTION(BlueprintCallable, Category = "Cmn|Input")
    virtual bool CanAcceptLookInput() const;

    /**
     * カメラズーム入力を受け付け可能か判定する
     *
     * @return 受付可能ならtrue
     */
    UFUNCTION(BlueprintCallable, Category = "Cmn|Input")
    virtual bool CanAcceptZoomInput() const;
};
