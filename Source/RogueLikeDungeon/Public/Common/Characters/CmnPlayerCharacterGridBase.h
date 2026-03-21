// CmnPlayerCharacterGridBase.h

#pragma once

#include "CoreMinimal.h"
#include "Common/Characters/CmnPlayerCharacterBase.h"
#include "CmnPlayerCharacterGridBase.generated.h"

/**
 * 共通グリッド移動Characterのベースクラス
 *
 * マス移動ゲーム向けの共通機能を提供する。
 *
 * 主な役割
 * ・現在のグリッド座標保持
 * ・グリッド座標からワールド座標への変換
 * ・Actor位置への反映
 *
 * ゲーム固有の移動可否判定やターン進行は持たせず、
 * 実際の移動要求処理は派生クラス側で実装する。
 */
UCLASS(Abstract)
class ROGUELIKEDUNGEON_API ACmnPlayerCharacterGridBase : public ACmnPlayerCharacterBase
{
    GENERATED_BODY()

public:

    /** コンストラクタ */
    ACmnPlayerCharacterGridBase();

protected:

    // ----- AActor -----

    virtual void BeginPlay() override;

public:

    /**
     * 現在のグリッド座標を取得する
     *
     * @return 現在のグリッド座標
     */
    UFUNCTION(BlueprintCallable, Category = "Cmn|Grid")
    FIntPoint GetCurrentGridCoord() const
    {
        return CurrentGridCoord;
    }

    /**
     * 現在のグリッド座標を設定する
     * 設定後はワールド座標へ即時反映する
     *
     * @param NewGridCoord 新しいグリッド座標
     */
    UFUNCTION(BlueprintCallable, Category = "Cmn|Grid")
    virtual void SetCurrentGridCoord(const FIntPoint& NewGridCoord);

protected:

    /**
     * グリッド座標をワールド座標へ変換する
     *
     * 現段階では簡易的に1マス=CellSizeとして変換する
     * 将来的にはGridManagerなどへ委譲する想定
     *
     * @param GridCoord 変換対象のグリッド座標
     * @return 対応するワールド座標
     */
    UFUNCTION(BlueprintCallable, Category = "Cmn|Grid")
    virtual FVector GridToWorld(const FIntPoint& GridCoord) const;

    /**
     * 現在のグリッド座標をActor位置へ反映する
     */
    virtual void SyncActorLocationFromGridCoord();

protected:

    // ----- グリッド設定 -----

    /** 現在のグリッド座標 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cmn|Grid")
    FIntPoint CurrentGridCoord = FIntPoint::ZeroValue;

    /** 1マスのワールドサイズ */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cmn|Grid")
    float CellSize = 100.0f;

    /** ワールド座標へ変換するときの基準Z */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cmn|Grid")
    float GroundZ = 0.0f;

protected:

    /**
     * ワールド座標をグリッド座標へ変換する
     *
     * @param WorldLocation 変換対象のワールド座標
     * @return 対応するグリッド座標
     */
    virtual FIntPoint WorldToGrid(const FVector& WorldLocation) const;
};


