// RldGridManager.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RldGridManager.generated.h"

/**
 * RogueLikeDungeon用グリッド管理Actor
 *
 * このクラスはレベル上のグリッド範囲と通行可否判定を管理する
 * 現段階では最小構成として、以下を扱う
 *
 * ・グリッド範囲管理
 * ・範囲内判定
 * ・壁マス判定
 * ・通行可否判定
 *
 * 将来的には以下の責務を段階的に追加する想定
 * ・占有情報管理
 * ・イベントマス管理
 * ・ダンジョン生成結果の保持
 */
UCLASS()
class ROGUELIKEDUNGEON_API ARldGridManager : public AActor
{
    GENERATED_BODY()

public:

    /** コンストラクタ */
    ARldGridManager();

public:

    // ----- 判定処理 -----

    /**
     * 指定グリッド座標が有効範囲内か判定する
     *
     * @param gridCoord 判定対象グリッド座標
     * @return 範囲内ならtrue
     */
    UFUNCTION(BlueprintCallable, Category = "Rld|Grid")
    bool IsInsideGrid(const FIntPoint& gridCoord) const;

    /**
     * 指定グリッド座標が壁マスか判定する
     *
     * @param gridCoord 判定対象グリッド座標
     * @return 壁マスならtrue
     */
    UFUNCTION(BlueprintCallable, Category = "Rld|Grid")
    bool IsWallCell(const FIntPoint& gridCoord) const;

    /**
     * 指定グリッド座標へ通行可能か判定する
     *
     * 範囲外は通行不可とし、範囲内でも壁マスなら通行不可とする。
     *
     * @param gridCoord 判定対象グリッド座標
     * @return 通行可能ならtrue
     */
    UFUNCTION(BlueprintCallable, Category = "Rld|Grid")
    bool IsWalkable(const FIntPoint& gridCoord) const;

public:

    // ----- Getter -----

    /**
     * グリッド横幅を取得する
     *
     * @return グリッド横幅
     */
    UFUNCTION(BlueprintPure, Category = "Rld|Grid")
    int32 GetGridWidth() const
    {
        return gridWidth;
    }

    /**
     * グリッド縦幅を取得する
     *
     * @return グリッド縦幅
     */
    UFUNCTION(BlueprintPure, Category = "Rld|Grid")
    int32 GetGridHeight() const
    {
        return gridHeight;
    }

    /**
     * 壁マス座標配列を取得する
     *
     * @return 壁マス座標配列
     */
    UFUNCTION(BlueprintPure, Category = "Rld|Grid")
    const TArray<FIntPoint>& GetWallCells() const
    {
        return wallCells;
    }

private:

    // ----- グリッド設定 -----

    /** グリッド横幅 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rld|Grid", meta = (ClampMin = "1", AllowPrivateAccess = "true"))
    int32 gridWidth = 20;

    /** グリッド縦幅 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rld|Grid", meta = (ClampMin = "1", AllowPrivateAccess = "true"))
    int32 gridHeight = 20;

private:

    // ----- 壁マス設定 -----

    /**
     * 壁マス座標配列
     *
     * 現段階では手動設定用として配列で保持する。
     * 将来的にはダンジョン生成結果から一括設定する想定。
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rld|Grid|Wall", meta = (AllowPrivateAccess = "true"))
    TArray<FIntPoint> wallCells;
};
