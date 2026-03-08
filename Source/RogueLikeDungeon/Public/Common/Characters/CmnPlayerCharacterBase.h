// CmnPlayerCharacterBase.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CmnPlayerCharacterBase.generated.h"

/**
 * 共通プレイヤーCharacter基底
 *
 * PlayerControllerなどの入力処理から受け取った要求を、
 * 各ゲーム固有Characterへ橋渡しするための共通窓口を提供する
 *
 * このクラス自体にはゲーム固有の移動ロジックは持たせず、
 * 実際の処理は派生クラス側で実装する
 */
UCLASS(Abstract)
class ROGUELIKEDUNGEON_API ACmnPlayerCharacterBase : public ACharacter
{
    GENERATED_BODY()

public:

    ACmnPlayerCharacterBase();

public:

    /**
     * 移動方向入力要求を受け取る
     *
     * @param Direction 確定した移動方向
     */
    UFUNCTION(BlueprintCallable, Category = "Cmn|Input")
    virtual void RequestMoveDirection(const FIntPoint& Direction);

    /**
     * 向き変更入力要求を受け取る
     *
     * @param Direction 確定した方向
     */
    UFUNCTION(BlueprintCallable, Category = "Cmn|Input")
    virtual void RequestFaceDirection(const FIntPoint& Direction);

    /**
     * カメラ視点入力要求を受け取る
     *
     * @param Axis 視点入力軸
     */
    UFUNCTION(BlueprintCallable, Category = "Cmn|Input")
    virtual void RequestLookInput(const FVector2D& Axis);

    /**
     * カメラズーム入力要求を受け取る
     *
     * @param Value ズーム入力値
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
     * 視点入力を受け付け可能か判定する
     *
     * @return 受付可能ならtrue
     */
    UFUNCTION(BlueprintCallable, Category = "Cmn|Input")
    virtual bool CanAcceptLookInput() const;

    /**
     * ズーム入力を受け付け可能か判定する
     *
     * @return 受付可能ならtrue
     */
    UFUNCTION(BlueprintCallable, Category = "Cmn|Input")
    virtual bool CanAcceptZoomInput() const;
};
