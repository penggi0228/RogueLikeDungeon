// RldFloorGenerator.cpp

#include "Game/Floor/RldFloorGenerator.h"

DEFINE_LOG_CATEGORY_STATIC(LogRldFloorGenerator, Log, All);

/** フロア定義からレイアウトを生成する */
bool FRldFloorGenerator::GenerateFloorLayout(
    const FRldFloorDefinition& floorDefinition,
    FCmnGridLayoutBuildResult& outBuildResult
)
{
    outBuildResult.Reset();

    const int32 resolvedSeed = ResolveSeed(floorDefinition);

    InitializeGeneration(
        floorDefinition.gridWidth,
        floorDefinition.gridHeight,
        resolvedSeed
    );

    // 有効な内側領域を作れないサイズでは生成しない
    if (gridWidth < 3 || gridHeight < 3)
    {
        UE_LOG(
            LogRldFloorGenerator,
            Warning,
            TEXT("GenerateFloorLayout: グリッドサイズが小さすぎるため生成しません サイズ=(%d,%d)"),
            gridWidth,
            gridHeight
        );

        return false;
    }

    if (floorDefinition.bUseProceduralLayout)
    {
        return GenerateProceduralLayout(floorDefinition, outBuildResult);
    }

    return GenerateFixedLayout(floorDefinition, outBuildResult);
}

/** 固定レイアウトを生成する */
bool FRldFloorGenerator::GenerateFixedLayout(
    const FRldFloorDefinition& floorDefinition,
    FCmnGridLayoutBuildResult& outBuildResult
)
{
    TSet<FIntPoint> blockedCellSet;

    // 外周を壁扱いにする
    for (int32 x = 0; x < gridWidth; ++x)
    {
        blockedCellSet.Add(FIntPoint(x, 0));
        blockedCellSet.Add(FIntPoint(x, gridHeight - 1));
    }

    for (int32 y = 0; y < gridHeight; ++y)
    {
        blockedCellSet.Add(FIntPoint(0, y));
        blockedCellSet.Add(FIntPoint(gridWidth - 1, y));
    }

    // 定義済み内壁を反映
    for (const FIntPoint& wallCell : floorDefinition.wallCells)
    {
        if (wallCell.X < 0 || wallCell.X >= gridWidth || wallCell.Y < 0 || wallCell.Y >= gridHeight)
        {
            continue;
        }

        blockedCellSet.Add(wallCell);
    }

    // 壁以外の内側を床にする
    for (int32 y = 1; y < gridHeight - 1; ++y)
    {
        for (int32 x = 1; x < gridWidth - 1; ++x)
        {
            const FIntPoint cell(x, y);

            if (blockedCellSet.Contains(cell))
            {
                continue;
            }

            layoutBuilder.CarveCell(cell);
        }
    }

    outBuildResult.gridWidth = gridWidth;
    outBuildResult.gridHeight = gridHeight;
    outBuildResult.floorCells = layoutBuilder.GetFloorCells();
    outBuildResult.wallCells = layoutBuilder.BuildWallCells();
    outBuildResult.playerStartGridCoord = floorDefinition.playerStartGridCoord;
    outBuildResult.stairsGridCoord = floorDefinition.stairsGridCoord;

    if (!ValidateFixedSpecialCells(floorDefinition, outBuildResult))
    {
        UE_LOG(
            LogRldFloorGenerator,
            Warning,
            TEXT("GenerateFixedLayout: 開始座標または階段座標が無効なため生成に失敗しました 開始座標=(%d,%d) 階段座標=(%d,%d)"),
            floorDefinition.playerStartGridCoord.X,
            floorDefinition.playerStartGridCoord.Y,
            floorDefinition.stairsGridCoord.X,
            floorDefinition.stairsGridCoord.Y
        );

        return false;
    }

    UE_LOG(
        LogRldFloorGenerator,
        Log,
        TEXT("GenerateFixedLayout: 固定レイアウト生成完了 床マス数=%d 壁マス数=%d 開始座標=(%d,%d) 階段座標=(%d,%d)"),
        outBuildResult.floorCells.Num(),
        outBuildResult.wallCells.Num(),
        outBuildResult.playerStartGridCoord.X,
        outBuildResult.playerStartGridCoord.Y,
        outBuildResult.stairsGridCoord.X,
        outBuildResult.stairsGridCoord.Y
    );

    return true;
}

/** セクション生成レイアウトを生成する */
bool FRldFloorGenerator::GenerateProceduralLayout(
    const FRldFloorDefinition& floorDefinition,
    FCmnGridLayoutBuildResult& outBuildResult
)
{
    TArray<FCmnGridSection> sections;
    GenerateSections(floorDefinition, sections);

    // 有効なセクションが1つもない場合は失敗
    if (sections.Num() == 0)
    {
        UE_LOG(
            LogRldFloorGenerator,
            Warning,
            TEXT("GenerateProceduralLayout: 有効なセクションを生成できませんでした 試行回数=%d"),
            floorDefinition.sectionPlacementAttempts
        );

        return false;
    }

    // セクションを掘る
    for (const FCmnGridSection& section : sections)
    {
        layoutBuilder.CarveSection(section);
    }

    // セクション同士を接続
    ConnectSections(sections);

    // 結果化
    if (!FinalizeBuildResult(sections, outBuildResult))
    {
        UE_LOG(
            LogRldFloorGenerator,
            Warning,
            TEXT("GenerateProceduralLayout: 生成結果の確定に失敗しました セクション数=%d"),
            sections.Num()
        );

        return false;
    }

    UE_LOG(
        LogRldFloorGenerator,
        Log,
        TEXT("GenerateProceduralLayout: 自動生成レイアウト生成完了 セクション数=%d 床マス数=%d 壁マス数=%d 開始座標=(%d,%d) 階段座標=(%d,%d)"),
        sections.Num(),
        outBuildResult.floorCells.Num(),
        outBuildResult.wallCells.Num(),
        outBuildResult.playerStartGridCoord.X,
        outBuildResult.playerStartGridCoord.Y,
        outBuildResult.stairsGridCoord.X,
        outBuildResult.stairsGridCoord.Y
    );

    return true;
}

/** セクション一覧を生成する */
void FRldFloorGenerator::GenerateSections(
    const FRldFloorDefinition& floorDefinition,
    TArray<FCmnGridSection>& outSections
)
{
    outSections.Reset();

    const int32 minSectionCount = FMath::Max(1, floorDefinition.minSectionCount);
    const int32 maxSectionCount = FMath::Max(minSectionCount, floorDefinition.maxSectionCount);

    const int32 minSectionWidth = FMath::Max(3, floorDefinition.minSectionWidth);
    const int32 maxSectionWidth = FMath::Max(minSectionWidth, floorDefinition.maxSectionWidth);

    const int32 minSectionHeight = FMath::Max(3, floorDefinition.minSectionHeight);
    const int32 maxSectionHeight = FMath::Max(minSectionHeight, floorDefinition.maxSectionHeight);

    const int32 targetSectionCount = RandRange(minSectionCount, maxSectionCount);

    for (int32 attemptIndex = 0; attemptIndex < floorDefinition.sectionPlacementAttempts; ++attemptIndex)
    {
        if (outSections.Num() >= targetSectionCount)
        {
            break;
        }

        const int32 sectionWidth = RandRange(minSectionWidth, maxSectionWidth);
        const int32 sectionHeight = RandRange(minSectionHeight, maxSectionHeight);

        // 外周壁を残すため1マス内側から配置する
        const int32 minLeft = 1;
        const int32 minTop = 1;
        const int32 maxLeft = gridWidth - sectionWidth - 1;
        const int32 maxTop = gridHeight - sectionHeight - 1;

        if (maxLeft < minLeft || maxTop < minTop)
        {
            continue;
        }

        const FCmnGridSection candidateSection(
            RandRange(minLeft, maxLeft),
            RandRange(minTop, maxTop),
            sectionWidth,
            sectionHeight
        );

        if (!CanPlaceSection(candidateSection, outSections, floorDefinition.sectionSeparationPadding))
        {
            continue;
        }

        outSections.Add(candidateSection);
    }

    UE_LOG(
        LogRldFloorGenerator,
        Verbose,
        TEXT("GenerateSections: 目標セクション数=%d 生成セクション数=%d 試行回数=%d"),
        targetSectionCount,
        outSections.Num(),
        floorDefinition.sectionPlacementAttempts
    );
}

/** 指定セクション候補が配置可能か判定する */
bool FRldFloorGenerator::CanPlaceSection(
    const FCmnGridSection& candidateSection,
    const TArray<FCmnGridSection>& existingSections,
    int32 padding
) const
{
    if (!candidateSection.IsValid())
    {
        return false;
    }

    // 外周へ接触するセクションは許可しない
    if (candidateSection.left <= 0 || candidateSection.top <= 0)
    {
        return false;
    }

    if (candidateSection.GetRight() >= (gridWidth - 1) || candidateSection.GetBottom() >= (gridHeight - 1))
    {
        return false;
    }

    for (const FCmnGridSection& existingSection : existingSections)
    {
        if (candidateSection.IntersectsWithPadding(existingSection, padding))
        {
            return false;
        }
    }

    return true;
}

/** セクション同士を通路で接続する */
void FRldFloorGenerator::ConnectSections(const TArray<FCmnGridSection>& sections)
{
    if (sections.Num() <= 1)
    {
        return;
    }

    for (int32 sectionIndex = 1; sectionIndex < sections.Num(); ++sectionIndex)
    {
        const FIntPoint previousCenter = sections[sectionIndex - 1].GetCenter();
        const FIntPoint currentCenter = sections[sectionIndex].GetCenter();

        // L字接続
        const bool bHorizontalFirst = (RandRange(0, 1) == 0);

        if (bHorizontalFirst)
        {
            layoutBuilder.CarveHorizontalTunnel(previousCenter.X, currentCenter.X, previousCenter.Y);
            layoutBuilder.CarveVerticalTunnel(previousCenter.Y, currentCenter.Y, currentCenter.X);
        }
        else
        {
            layoutBuilder.CarveVerticalTunnel(previousCenter.Y, currentCenter.Y, previousCenter.X);
            layoutBuilder.CarveHorizontalTunnel(previousCenter.X, currentCenter.X, currentCenter.Y);
        }
    }
}

/** 固定フロアの特殊マスが有効か判定する */
bool FRldFloorGenerator::ValidateFixedSpecialCells(
    const FRldFloorDefinition& floorDefinition,
    const FCmnGridLayoutBuildResult& buildResult
) const
{
    const bool bStartInside =
        floorDefinition.playerStartGridCoord.X >= 0 &&
        floorDefinition.playerStartGridCoord.X < buildResult.gridWidth &&
        floorDefinition.playerStartGridCoord.Y >= 0 &&
        floorDefinition.playerStartGridCoord.Y < buildResult.gridHeight;

    const bool bStairsInside =
        floorDefinition.stairsGridCoord.X >= 0 &&
        floorDefinition.stairsGridCoord.X < buildResult.gridWidth &&
        floorDefinition.stairsGridCoord.Y >= 0 &&
        floorDefinition.stairsGridCoord.Y < buildResult.gridHeight;

    if (!bStartInside || !bStairsInside)
    {
        return false;
    }

    const TSet<FIntPoint> floorCellSet(buildResult.floorCells);

    if (!floorCellSet.Contains(floorDefinition.playerStartGridCoord))
    {
        return false;
    }

    if (!floorCellSet.Contains(floorDefinition.stairsGridCoord))
    {
        return false;
    }

    return true;
}

/** 生成結果のメタ情報を設定する */
bool FRldFloorGenerator::FinalizeBuildResult(
    const TArray<FCmnGridSection>& sections,
    FCmnGridLayoutBuildResult& outBuildResult
) const
{
    if (sections.Num() == 0)
    {
        return false;
    }

    outBuildResult.gridWidth = gridWidth;
    outBuildResult.gridHeight = gridHeight;
    outBuildResult.sections = sections;
    outBuildResult.floorCells = layoutBuilder.GetFloorCells();
    outBuildResult.wallCells = layoutBuilder.BuildWallCells();
    outBuildResult.playerStartGridCoord = sections[0].GetCenter();

    FIntPoint farthestCell = outBuildResult.playerStartGridCoord;

    if (!layoutBuilder.FindFarthestReachableCell(outBuildResult.playerStartGridCoord, farthestCell))
    {
        return false;
    }

    outBuildResult.stairsGridCoord = farthestCell;
    return outBuildResult.IsValid();
}

/** 使用Seedを解決する */
int32 FRldFloorGenerator::ResolveSeed(const FRldFloorDefinition& floorDefinition) const
{
    if (floorDefinition.bUseRandomSeed)
    {
        return FMath::Rand();
    }

    return floorDefinition.randomSeed;
}
