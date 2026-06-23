// CmnDebugWorldSubsystem.h

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Common/Debug/CmnDebugDrawTypes.h"
#include "Common/Grid/CmnGridDefinition.h"
#include "Common/ProcGen/CmnGridSection.h"
#include "CmnDebugWorldSubsystem.generated.h"

/**
 * 共通デバッグ描画WorldSubsystem
 * デバッグ描画の全体状態とカテゴリ状態を管理する
 */
UCLASS()
class ROGUELIKEDUNGEON_API UCmnDebugWorldSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:

    // ----- UWorldSubsystem -----

    virtual bool ShouldCreateSubsystem(UObject* outer) const override;
    virtual void Initialize(FSubsystemCollectionBase& collection) override;

public:

    // ----- 全体制御 -----

    /** デバッグ描画全体の有効状態を設定する */
    UFUNCTION(BlueprintCallable, Category = "Cmn|Debug")
    void SetDebugEnabled(bool bEnabled);

    /** デバッグ描画全体が有効か判定する */
    UFUNCTION(BlueprintPure, Category = "Cmn|Debug")
    bool IsDebugEnabled() const
    {
        return bDebugEnabled;
    }

public:

    // ----- カテゴリ制御 -----

    /**
     * デバッグカテゴリの有効状態を設定する
     *
     * @param categoryName デバッグカテゴリ名
     * @param bEnabled 有効状態
     */
    UFUNCTION(BlueprintCallable, Category = "Cmn|Debug")
    void SetCategoryEnabled(FName categoryName, bool bEnabled);

    /**
     * デバッグカテゴリが有効か判定する
     *
     * @param categoryName デバッグカテゴリ名
     * @return 有効ならtrue
     */
    UFUNCTION(BlueprintPure, Category = "Cmn|Debug")
    bool IsCategoryEnabled(FName categoryName) const;

public:

    // ----- グリッド描画 -----

    /**
     * 単一グリッドマスを描画する
     *
     * @param gridDefinition グリッド定義
     * @param gridCoord 描画対象グリッド座標
     * @param drawStyle 描画設定
     */
    UFUNCTION(BlueprintCallable, Category = "Cmn|Debug")
    void DrawGridCell(
        const FCmnGridDefinition& gridDefinition,
        const FIntPoint& gridCoord,
        const FCmnDebugDrawStyle& drawStyle
    ) const;

    /**
     * 複数グリッドマスを描画する
     *
     * @param gridDefinition グリッド定義
     * @param gridCoords 描画対象グリッド座標一覧
     * @param drawStyle 描画設定
     */
    UFUNCTION(BlueprintCallable, Category = "Cmn|Debug")
    void DrawGridCells(
        const FCmnGridDefinition& gridDefinition,
        const TArray<FIntPoint>& gridCoords,
        const FCmnDebugDrawStyle& drawStyle
    ) const;

    /**
     * セクション矩形外枠を描画する
     *
     * @param gridDefinition グリッド定義
     * @param section 描画対象セクション
     * @param drawStyle 描画設定
     */
    UFUNCTION(BlueprintCallable, Category = "Cmn|Debug")
    void DrawGridSectionBounds(
        const FCmnGridDefinition& gridDefinition,
        const FCmnGridSection& section,
        const FCmnDebugDrawStyle& drawStyle
    ) const;

private:

    // ----- 内部処理 -----

    /** デバッグ描画を実行可能か判定する */
    bool CanDrawDebug() const;

    /** デフォルトのデバッグカテゴリ状態を初期化する */
    void InitializeDefaultCategories();

private:

    // ----- 共通設定 -----

    // デバッグ描画全体の有効フラグ
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cmn|Debug", meta = (AllowPrivateAccess = "true"))
    bool bDebugEnabled = false;

    // デバッグカテゴリごとの有効状態
    UPROPERTY(Transient)
    TMap<FName, bool> categoryEnabledMap;
};
