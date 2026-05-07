// CmnGridRoom.h

#pragma once

#include "CoreMinimal.h"
#include "CmnGridRoom.generated.h"

/**
 * グリッド上の部屋矩形情報
 */
USTRUCT(BlueprintType)
struct FCmnGridRoom
{
    GENERATED_BODY()

public:

    // 左上X座標
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cmn|ProcGen")
    int32 left = 0;

    // 左上Y座標
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cmn|ProcGen")
    int32 top = 0;

    // 横幅
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cmn|ProcGen")
    int32 width = 0;

    // 縦幅
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cmn|ProcGen")
    int32 height = 0;

public:

    /** デフォルトコンストラクタ */
    FCmnGridRoom() = default;

    /**
     * 部屋矩形を初期化する
     *
     * @param inLeft 左上X座標
     * @param inTop 左上Y座標
     * @param inWidth 横幅
     * @param inHeight 縦幅
     */
    FCmnGridRoom(int32 inLeft, int32 inTop, int32 inWidth, int32 inHeight)
        : left(inLeft)
        , top(inTop)
        , width(inWidth)
        , height(inHeight)
    {
    }

public:

    /** 右端X座標を取得する */
    int32 GetRight() const
    {
        return left + width - 1;
    }

    /** 下端Y座標を取得する */
    int32 GetBottom() const
    {
        return top + height - 1;
    }

    /** 中心座標を取得する */
    FIntPoint GetCenter() const
    {
        return FIntPoint(left + (width / 2), top + (height / 2));
    }

    /** 有効な部屋か判定する */
    bool IsValid() const
    {
        return width > 0 && height > 0;
    }

    /**
     * 指定余白込みで他部屋と重なっているか判定する
     *
     * @param other 比較対象の部屋
     * @param padding 判定時の余白
     * @return 重なっているならtrue
     */
    bool IntersectsWithPadding(const FCmnGridRoom& other, int32 padding) const
    {
        const int32 thisLeft = left - padding;
        const int32 thisTop = top - padding;
        const int32 thisRight = GetRight() + padding;
        const int32 thisBottom = GetBottom() + padding;

        const int32 otherLeft = other.left;
        const int32 otherTop = other.top;
        const int32 otherRight = other.GetRight();
        const int32 otherBottom = other.GetBottom();

        const bool bSeparatedX = (thisRight < otherLeft) || (otherRight < thisLeft);
        const bool bSeparatedY = (thisBottom < otherTop) || (otherBottom < thisTop);

        return !(bSeparatedX || bSeparatedY);
    }
};
