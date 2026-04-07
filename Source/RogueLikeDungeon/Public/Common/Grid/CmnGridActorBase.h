// CmnGridActorBase.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CmnGridActorBase.generated.h"

/**
 * 共通グリッドActor基底クラス
 * グリッド座標の保持と更新を行う
 */
UCLASS(Abstract)
class ROGUELIKEDUNGEON_API ACmnGridActorBase : public AActor
{
    GENERATED_BODY()

public:

    /** 共通グリッドActorを初期化する */
    ACmnGridActorBase();

public:

    // ----- Getter -----

    /**
     * 現在のグリッド座標を取得する
     *
     * @return 現在のグリッド座標
     */
    UFUNCTION(BlueprintCallable, Category = "Cmn|Grid")
    FIntPoint GetCurrentGridCoord() const
    {
        return currentGridCoord;
    }

public:

    // ----- Setter -----

    /**
     * 現在のグリッド座標を設定する
     *
     * @param newGridCoord 更新後のグリッド座標
     */
    UFUNCTION(BlueprintCallable, Category = "Cmn|Grid")
    virtual void SetCurrentGridCoord(const FIntPoint& newGridCoord);

protected:

    // ----- グリッド状態 -----

    // 現在のグリッド座標
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cmn|Grid")
    FIntPoint currentGridCoord = FIntPoint::ZeroValue;
};
