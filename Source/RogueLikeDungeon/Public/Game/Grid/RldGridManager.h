// RldGridManager.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Common/Grid/CmnGridDefinition.h"
#include "Game/Floor/RldFloorDefinition.h"
#include "RldGridManager.generated.h"

/**
 * RogueLikeDungeon用グリッド管理Actor
 *
 * レベル上のグリッド範囲・通行可否・占有情報を管理する
 * フロア定義の反映後は、現在フロアのグリッド状態を保持する
 */
UCLASS()
class ROGUELIKEDUNGEON_API ARldGridManager : public AActor
{
    GENERATED_BODY()

public:

    /** コンストラクタ */
    ARldGridManager();

protected:

    // ----- AActor -----

    virtual void BeginPlay() override;

public:

    // ----- フロア定義反映 -----

    /**
     * フロア定義を現在のグリッド状態へ反映する
     *
     * @param floorDefinition 反映するフロア定義
     */
    UFUNCTION(BlueprintCallable, Category = "Rld|Grid")
    void ApplyFloorDefinition(const FRldFloorDefinition& floorDefinition);

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
     * 指定グリッド座標が占有されているか判定する
     *
     * @param gridCoord 判定対象グリッド座標
     * @return 占有されているならtrue
     */
    UFUNCTION(BlueprintCallable, Category = "Rld|Grid")
    bool IsOccupied(const FIntPoint& gridCoord) const;

    /**
     * 指定グリッド座標へ通行可能か判定する
     *
     * @param gridCoord 判定対象グリッド座標
     * @return 通行可能ならtrue
     */
    UFUNCTION(BlueprintCallable, Category = "Rld|Grid")
    bool IsWalkable(const FIntPoint& gridCoord) const;

public:

    // ----- 占有情報操作 -----

    /**
     * 指定グリッド座標の占有Actorを取得する
     *
     * @param gridCoord 取得対象グリッド座標
     * @return 占有Actor
     */
    UFUNCTION(BlueprintCallable, Category = "Rld|Grid")
    AActor* GetOccupyingActor(const FIntPoint& gridCoord) const;

    /**
     * グリッド座標へ占有Actorを登録する
     *
     * @param gridCoord 登録先グリッド座標
     * @param occupantActor 登録するActor
     * @return 登録成功ならtrue
     */
    UFUNCTION(BlueprintCallable, Category = "Rld|Grid")
    bool RegisterOccupant(const FIntPoint& gridCoord, AActor* occupantActor);

    /**
     * グリッド座標から占有Actorを解除する
     *
     * @param gridCoord 解除対象グリッド座標
     * @param occupantActor 解除するActor
     * @return 解除成功ならtrue
     */
    UFUNCTION(BlueprintCallable, Category = "Rld|Grid")
    bool UnregisterOccupant(const FIntPoint& gridCoord, AActor* occupantActor);

    /**
     * 占有Actorを別グリッド座標へ移動する
     *
     * @param fromGridCoord 移動前グリッド座標
     * @param toGridCoord 移動後グリッド座標
     * @param occupantActor 移動するActor
     * @return 移動成功ならtrue
     */
    UFUNCTION(BlueprintCallable, Category = "Rld|Grid")
    bool MoveOccupant(const FIntPoint& fromGridCoord, const FIntPoint& toGridCoord, AActor* occupantActor);

    /** すべての占有情報をクリアする */
    UFUNCTION(BlueprintCallable, Category = "Rld|Grid")
    void ClearAllOccupants();

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

    // ----- 初期化補助 -----

    /** 必要に応じて外周壁を自動生成する */
    void GenerateOuterWallCells();

    /**
     * 壁マスを重複なしで追加する
     *
     * @param wallCoord 追加したい壁マス座標
     */
    void AddWallCellUnique(const FIntPoint& wallCoord);

private:

    // ----- グリッド状態 -----

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rld|Grid", meta = (AllowPrivateAccess = "true"))
    int32 gridWidth = 20;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rld|Grid", meta = (AllowPrivateAccess = "true"))
    int32 gridHeight = 20;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rld|Grid", meta = (AllowPrivateAccess = "true"))
    FCmnGridDefinition gridDefinition;

private:

    // ----- 壁マス状態 -----

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rld|Grid|Wall", meta = (AllowPrivateAccess = "true"))
    TArray<FIntPoint> wallCells;

    // 現在フロア反映時に外周壁を自動生成するか
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rld|Grid|Wall", meta = (AllowPrivateAccess = "true"))
    bool bGenerateOuterWallsOnBeginPlay = true;

private:

    // ----- 特殊マス状態 -----

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rld|Grid|Special", meta = (AllowPrivateAccess = "true"))
    FIntPoint stairsGridCoord = FIntPoint(18, 18);

private:

    // ----- 占有情報 -----

    TMap<FIntPoint, TWeakObjectPtr<AActor>> occupantMap;
};
