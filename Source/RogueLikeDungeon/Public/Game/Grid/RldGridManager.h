// RldGridManager.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Common/Grid/CmnGridDefinition.h"
#include "RldGridManager.generated.h"

/**
 * RogueLikeDungeon用グリッド管理Actor
 *
 * レベル上のグリッド範囲と通行可否判定を管理する
 * 座標変換ルール自体は共通関数を利用し、
 * ゲーム側では定義値の保持と判定責務を担当する
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
     * 指定グリッド座標が階段マスか判定する
     *
     * @param gridCoord 判定対象グリッド座標
     * @return 階段マスならtrue
     */
    UFUNCTION(BlueprintCallable, Category = "Rld|Grid")
    bool IsStairsCell(const FIntPoint& gridCoord) const;

    /**
     * 指定グリッド座標へ通行可能か判定する
     *
     * @param gridCoord 判定対象グリッド座標
     * @return 通行可能ならtrue
     */
    UFUNCTION(BlueprintCallable, Category = "Rld|Grid")
    bool IsWalkable(const FIntPoint& gridCoord) const;

public:

    // ----- 座標変換 -----

    /**
     * グリッド座標をワールド座標へ変換する
     *
     * @param gridCoord 変換対象グリッド座標
     * @return 対応するワールド座標
     */
    UFUNCTION(BlueprintPure, Category = "Rld|Grid")
    FVector GridToWorld(const FIntPoint& gridCoord) const;

    /**
     * ワールド座標をグリッド座標へ変換する
     *
     * @param worldLocation 変換対象ワールド座標
     * @return 対応するグリッド座標
     */
    UFUNCTION(BlueprintPure, Category = "Rld|Grid")
    FIntPoint WorldToGrid(const FVector& worldLocation) const;

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
     * グリッド定義を取得する
     *
     * @return グリッド定義
     */
    UFUNCTION(BlueprintPure, Category = "Rld|Grid")
    FCmnGridDefinition GetGridDefinition() const
    {
        return gridDefinition;
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

    /**
     * 階段マス座標を取得する
     *
     * @return 階段マス座標
     */
    UFUNCTION(BlueprintPure, Category = "Rld|Grid")
    FIntPoint GetStairsGridCoord() const
    {
        return stairsGridCoord;
    }

public:

    // ----- Setter -----

    /**
     * 階段マス座標を設定する
     *
     * @param newGridCoord 更新後の階段マス座標
     */
    UFUNCTION(BlueprintCallable, Category = "Rld|Grid")
    void SetStairsGridCoord(const FIntPoint& newGridCoord);

private:

    // ----- グリッド設定 -----

    /** グリッド横幅 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rld|Grid", meta = (ClampMin = "1", AllowPrivateAccess = "true"))
    int32 gridWidth = 20;

    /** グリッド縦幅 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rld|Grid", meta = (ClampMin = "1", AllowPrivateAccess = "true"))
    int32 gridHeight = 20;

    /**
     * 共通グリッド定義
     *
     * 変換式ではなく定義値を持たせることで、
     * ロジックの共通化とゲーム固有値の分離を両立する
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rld|Grid", meta = (AllowPrivateAccess = "true"))
    FCmnGridDefinition gridDefinition;

private:

    // ----- 壁マス設定 -----

    /**
     * 壁マス座標配列
     *
     * 現段階では手動設定で十分なため配列で保持する
     * 将来的にはダンジョン生成結果から一括設定する想定
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rld|Grid|Wall", meta = (AllowPrivateAccess = "true"))
    TArray<FIntPoint> wallCells;

private:

    // ----- 特殊マス設定 -----

    /**
     * 階段マス座標
     *
     * 現段階では1フロアに1つだけ配置する前提で保持する
     * 将来的に上下階段や複数特殊マスへ拡張する場合は管理方法を見直す
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rld|Grid|Special", meta = (AllowPrivateAccess = "true"))
    FIntPoint stairsGridCoord = FIntPoint(18, 18);
};
