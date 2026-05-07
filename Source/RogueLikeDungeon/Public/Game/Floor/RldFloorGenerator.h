// RldFloorGenerator.h

#pragma once

#include "CoreMinimal.h"
#include "Common/ProcGen/CmnGridDungeonGeneratorBase.h"
#include "Common/ProcGen/CmnGridLayoutTypes.h"
#include "Game/Floor/RldFloorDefinition.h"

/**
 * RogueLikeDungeon用フロア自動生成クラス
 */
class FRldFloorGenerator : public FCmnGridDungeonGeneratorBase
{
public:

    /**
     * フロア定義からレイアウトを生成する
     *
     * @param floorDefinition 生成元フロア定義
     * @param outBuildResult 生成結果
     * @return 生成成功ならtrue
     */
    bool GenerateFloorLayout(
        const FRldFloorDefinition& floorDefinition,
        FCmnGridLayoutBuildResult& outBuildResult
    );

private:

    /** 固定レイアウトを生成する */
    bool GenerateFixedLayout(
        const FRldFloorDefinition& floorDefinition,
        FCmnGridLayoutBuildResult& outBuildResult
    );

    /** 部屋生成レイアウトを生成する */
    bool GenerateProceduralLayout(
        const FRldFloorDefinition& floorDefinition,
        FCmnGridLayoutBuildResult& outBuildResult
    );

    /** 部屋一覧を生成する */
    void GenerateRooms(
        const FRldFloorDefinition& floorDefinition,
        TArray<FCmnGridRoom>& outRooms
    );

    /** 指定部屋候補が配置可能か判定する */
    bool CanPlaceRoom(
        const FCmnGridRoom& candidateRoom,
        const TArray<FCmnGridRoom>& existingRooms,
        int32 padding
    ) const;

    /** 部屋同士を通路で接続する */
    void ConnectRooms(const TArray<FCmnGridRoom>& rooms);

    /** 固定フロアの特殊マスが有効か判定する */
    bool ValidateFixedSpecialCells(
        const FRldFloorDefinition& floorDefinition,
        const FCmnGridLayoutBuildResult& buildResult
    ) const;

    /** 生成結果のメタ情報を設定する */
    bool FinalizeBuildResult(
        const TArray<FCmnGridRoom>& rooms,
        FCmnGridLayoutBuildResult& outBuildResult
    ) const;

    /** 使用Seedを解決する */
    int32 ResolveSeed(const FRldFloorDefinition& floorDefinition) const;
};
