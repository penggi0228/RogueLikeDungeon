// CmnGridLayoutBuilder.h

#pragma once

#include "CoreMinimal.h"
#include "Containers/Queue.h"
#include "Common/ProcGen/CmnGridRoom.h"

/**
 * グリッドレイアウト生成補助クラス
 * 床マスの掘削と壁マス構築を担当する
 */
class FCmnGridLayoutBuilder
{
public:

    /** ビルダーを初期化する */
    FCmnGridLayoutBuilder();

public:

    /** グリッドサイズを設定して内部状態を初期化する */
    void Initialize(int32 inGridWidth, int32 inGridHeight);

    /** 内部状態を初期化する */
    void Reset();

public:

    /** 指定座標がグリッド範囲内か判定する */
    bool IsInsideGrid(const FIntPoint& gridCoord) const;

    /** 指定座標が床マスか判定する */
    bool IsFloorCell(const FIntPoint& gridCoord) const;

public:

    /** 単一マスを床として掘る */
    void CarveCell(const FIntPoint& gridCoord);

    /** 部屋矩形を床として掘る */
    void CarveRoom(const FCmnGridRoom& room);

    /** 横通路を掘る */
    void CarveHorizontalTunnel(int32 startX, int32 endX, int32 y);

    /** 縦通路を掘る */
    void CarveVerticalTunnel(int32 startY, int32 endY, int32 x);

public:

    /** 現在の床マス一覧を取得する */
    TArray<FIntPoint> GetFloorCells() const;

    /** 床マスから壁マス一覧を構築する */
    TArray<FIntPoint> BuildWallCells() const;

    /**
     * 指定開始座標から最遠床マスを探索する
     *
     * @param startGridCoord 探索開始座標
     * @param outFarthestGridCoord 最遠床マス
     * @return 探索成功ならtrue
     */
    bool FindFarthestReachableCell(
        const FIntPoint& startGridCoord,
        FIntPoint& outFarthestGridCoord
    ) const;

private:

    /** 壁マス候補を重複なしで追加する */
    void AddWallCandidateUnique(TSet<FIntPoint>& wallCellSet, const FIntPoint& gridCoord) const;

    /** 座標一覧を安定順序へ並べ替える */
    void SortGridCoords(TArray<FIntPoint>& gridCoords) const;

private:

    int32 gridWidth = 0;
    int32 gridHeight = 0;

    TSet<FIntPoint> floorCellSet;
};
