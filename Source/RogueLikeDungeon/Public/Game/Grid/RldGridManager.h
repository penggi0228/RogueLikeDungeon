// RldGridManager.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Common/Debug/CmnDebugDrawTypes.h"
#include "Common/Grid/CmnGridDefinition.h"
#include "Common/ProcGen/CmnGridLayoutTypes.h"
#include "Game/Floor/RldFloorDefinition.h"
#include "RldGridManager.generated.h"

/**
 * RogueLikeDungeon用グリッド管理Actor
 *
 * レベル上のグリッド範囲・床マス・壁マス・特殊マス・占有情報を管理する
 * フロア定義と生成結果の反映後は、現在のフロアのグリッド状態を保持する
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
    virtual void Tick(float deltaSeconds) override;

public:

    // ----- フロア状態反映 -----

    /**
     * フロア定義と生成結果を現在のグリッド状態へ反映する
     *
     * @param floorDefinition 反映元フロア定義
     * @param floorLayout 反映元生成結果
     */
    UFUNCTION(BlueprintCallable, Category = "Rld|Grid")
    void ApplyFloorLayout(
        const FRldFloorDefinition& floorDefinition,
        const FCmnGridLayoutBuildResult& floorLayout
    );

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
     * 指定グリッド座標が床マスか判定する
     *
     * @param gridCoord 判定対象グリッド座標
     * @return 床マスならtrue
     */
    UFUNCTION(BlueprintCallable, Category = "Rld|Grid")
    bool IsFloorCell(const FIntPoint& gridCoord) const;

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

    /**
     * 移動ルールに応じて指定グリッド座標へ進入可能か判定する
     *
     * @param gridCoord 判定対象グリッド座標
     * @param bCanPassThroughWalls 壁マスを通過できるか
     * @return 進入可能ならtrue
     */
    UFUNCTION(BlueprintCallable, Category = "Rld|Grid")
    bool CanEnterCell(const FIntPoint& gridCoord, bool bCanPassThroughWalls) const;

    /**
     * 斜め移動や1マス近接攻撃で角の通過可否を判定する
     * 中距離攻撃、遠距離攻撃、範囲攻撃の到達判定には使用しない
     *
     * @param fromCoord 起点座標
     * @param direction 移動または攻撃方向
     * @return 角を通過可能な場合はtrue
     */
    UFUNCTION(BlueprintPure, Category = "Rld|Grid")
    bool CanPassDiagonalCorner(const FIntPoint& fromCoord, const FIntPoint& direction) const;

    /**
     * 移動ルールに応じて斜め移動や1マス近接攻撃で角の通過可否を判定する
     * 中距離攻撃、遠距離攻撃、範囲攻撃の到達判定には使用しない
     *
     * @param fromCoord 起点座標
     * @param direction 移動または攻撃方向
     * @param bCanPassThroughWalls 壁マスを通過可能か
     * @return 角を通過可能な場合はtrue
     */
    UFUNCTION(BlueprintPure, Category = "Rld|Grid")
    bool CanPassDiagonalCornerWithWallPass(
        const FIntPoint& fromCoord,
        const FIntPoint& direction,
        bool bCanPassThroughWalls
    ) const;

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
     * 移動ルールに応じてグリッド座標へ占有Actorを登録する
     *
     * @param gridCoord 登録先グリッド座標
     * @param occupantActor 登録するActor
     * @param bCanPassThroughWalls 壁マスへ登録可能か
     * @return 登録成功ならtrue
     */
    UFUNCTION(BlueprintCallable, Category = "Rld|Grid")
    bool RegisterOccupantWithWallPass(
        const FIntPoint& gridCoord,
        AActor* occupantActor,
        bool bCanPassThroughWalls
    );

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

    /**
     * 移動ルールに応じて占有Actorを別グリッド座標へ移動する
     *
     * @param fromGridCoord 移動前グリッド座標
     * @param toGridCoord 移動後グリッド座標
     * @param occupantActor 移動するActor
     * @param bCanPassThroughWalls 壁マスへ進入可能か
     * @return 移動成功ならtrue
     */
    UFUNCTION(BlueprintCallable, Category = "Rld|Grid")
    bool MoveOccupantWithWallPass(
        const FIntPoint& fromGridCoord,
        const FIntPoint& toGridCoord,
        AActor* occupantActor,
        bool bCanPassThroughWalls
    );

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

    // ----- デバッグ描画 -----

    /** 現在のフロア状態をデバッグ描画する */
    UFUNCTION(BlueprintCallable, Category = "Rld|Grid|Debug")
    void DrawDebugGridState() const;

    /** デバッグ描画凡例をログ出力する */
    UFUNCTION(BlueprintCallable, Category = "Rld|Grid|Debug")
    void LogDebugDrawInfo();

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
     * 床マス座標配列を取得する
     *
     * @return 床マス座標配列
     */
    UFUNCTION(BlueprintPure, Category = "Rld|Grid")
    const TArray<FIntPoint>& GetFloorCells() const
    {
        return floorCells;
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

    // ----- 内部状態更新 -----

    /** 配列から床マスと壁マスの検索用Setを再構築する */
    void RebuildCellSets();

    /** グリッドデバッグ描画を更新する */
    void UpdateContinuousDebugDraw(float deltaSeconds);

    /** グリッド常時デバッグ描画が有効か判定する */
    bool ShouldDrawContinuousDebug() const;

    /**
     * 現在のフロア状態をデバッグ描画する
     *
     * @param bOutputLog 描画ログを出力するか
     */
    void DrawDebugGridStateInternal(bool bOutputLog) const;

private:

    // ----- グリッド状態 -----

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rld|Grid", meta = (AllowPrivateAccess = "true"))
    int32 gridWidth = 20;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rld|Grid", meta = (AllowPrivateAccess = "true"))
    int32 gridHeight = 20;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rld|Grid", meta = (AllowPrivateAccess = "true"))
    FCmnGridDefinition gridDefinition;

private:

    // ----- マス状態 -----

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rld|Grid|Cell", meta = (AllowPrivateAccess = "true"))
    TArray<FIntPoint> floorCells;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rld|Grid|Cell", meta = (AllowPrivateAccess = "true"))
    TArray<FIntPoint> wallCells;

    TSet<FIntPoint> floorCellSet;
    TSet<FIntPoint> wallCellSet;

private:

    // ----- 特殊マス状態 -----

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rld|Grid|Special", meta = (AllowPrivateAccess = "true"))
    FIntPoint stairsGridCoord = FIntPoint(18, 18);

private:

    // ----- 占有情報 -----

    TMap<FIntPoint, TWeakObjectPtr<AActor>> occupantMap;

private:

    // ----- デバッグ描画設定 -----

    // フロア反映時に自動でデバッグ描画するか
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rld|Grid|Debug", meta = (AllowPrivateAccess = "true"))
    bool bDrawDebugOnApplyFloorLayout = true;

    // チェックON中に短時間DebugDrawを定期再描画するか
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rld|Grid|Debug", meta = (AllowPrivateAccess = "true"))
    bool bEnableContinuousDebugDraw = true;

    // 常時デバッグ描画の再描画間隔
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rld|Grid|Debug", meta = (AllowPrivateAccess = "true", ClampMin = "0.02"))
    float continuousDebugDrawInterval = 0.10f;

    // 床マス描画設定
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rld|Grid|Debug", meta = (AllowPrivateAccess = "true"))
    FCmnDebugDrawStyle floorDebugStyle;

    // 壁マス描画設定
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rld|Grid|Debug", meta = (AllowPrivateAccess = "true"))
    FCmnDebugDrawStyle wallDebugStyle;

    // 階段マス描画設定
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rld|Grid|Debug", meta = (AllowPrivateAccess = "true"))
    FCmnDebugDrawStyle stairsDebugStyle;

private:

    // ----- デバッグ描画Runtime状態 -----

    float continuousDebugDrawElapsed = 0.0f;
    bool bDebugInfoLogged = false;
};
