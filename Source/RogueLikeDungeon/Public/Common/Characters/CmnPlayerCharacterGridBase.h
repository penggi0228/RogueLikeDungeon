// CmnPlayerCharacterGridBase.h

#pragma once

#include "CoreMinimal.h"
#include "Common/Characters/CmnPlayerCharacterBase.h"
#include "CmnPlayerCharacterGridBase.generated.h"

/**
 * グリッド移動用キャラクターの共通ベースクラス
 * グリッド座標の保持と更新を行う
 */
UCLASS(Abstract)
class ROGUELIKEDUNGEON_API ACmnPlayerCharacterGridBase : public ACmnPlayerCharacterBase
{
    GENERATED_BODY()

public:

    /** グリッド移動用キャラクターを初期化する */
    ACmnPlayerCharacterGridBase();

protected:

    virtual void BeginPlay() override;

public:

    // ----- グリッド座標操作 -----

    /**
     * 現在のグリッド座標を取得する
     *
     * @return グリッド座標
     */
    UFUNCTION(BlueprintCallable, Category = "Cmn|Grid")
    FIntPoint GetCurrentGridCoord() const
    {
        return currentGridCoord;
    }

    /**
     * グリッド座標を設定する
     *
     * @param newGridCoord 更新後のグリッド座標
     */
    UFUNCTION(BlueprintCallable, Category = "Cmn|Grid")
    virtual void SetCurrentGridCoord(const FIntPoint& newGridCoord);

protected:

    // ----- グリッドの状態 -----

    // 現在のグリッド座標
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cmn|Grid")
    FIntPoint currentGridCoord = FIntPoint::ZeroValue;
};
