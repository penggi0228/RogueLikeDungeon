// RldFloorManager.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Common/Debug/CmnDebugDrawTypes.h"
#include "Common/ProcGen/CmnGridLayoutTypes.h"
#include "Game/Floor/RldFloorDefinition.h"
#include "Game/Floor/RldFloorGenerator.h"
#include "RldFloorManager.generated.h"

class UDataTable;
class ARldGridManager;
class ARldPlayerCharacter;
class ARldTurnManager;
class ARldEnemyManager;

/**
 * フロア進行管理Actor
 * フロア定義の読込とフロア開始処理を管理する
 */
UCLASS()
class ROGUELIKEDUNGEON_API ARldFloorManager : public AActor
{
    GENERATED_BODY()

public:

    /** フロア管理Actorを初期化する */
    ARldFloorManager();

protected:

    // ----- AActor -----

    virtual void BeginPlay() override;
    virtual void Tick(float deltaSeconds) override;

public:

    // ----- フロア進行 -----

    /** 現在のフロア番号でフロア開始処理を行う */
    UFUNCTION(BlueprintCallable, Category = "Rld|Floor")
    void StartFloor();

    /**
     * 指定フロア番号でフロア開始処理を行う
     *
     * @param floorIndex フロア番号
     */
    UFUNCTION(BlueprintCallable, Category = "Rld|Floor")
    void StartFloorAt(int32 floorIndex);

    /** 次フロアへ進む */
    UFUNCTION(BlueprintCallable, Category = "Rld|Floor")
    void GoToNextFloor();

public:

    // ----- デバッグ描画 -----

    /** 現在フロアのデバッグ描画を行う */
    UFUNCTION(BlueprintCallable, Category = "Rld|Floor|Debug")
    void DrawDebugFloorState() const;

public:

    // ----- Getter -----

    /** 現在のフロア番号を取得する */
    UFUNCTION(BlueprintPure, Category = "Rld|Floor")
    int32 GetCurrentFloorIndex() const
    {
        return currentFloorIndex;
    }

    /** 現在フロアの定義を取得する */
    UFUNCTION(BlueprintPure, Category = "Rld|Floor")
    FRldFloorDefinition GetCurrentFloorDefinition() const
    {
        return currentFloorDefinition;
    }

    /** 現在フロアの生成結果を取得する */
    UFUNCTION(BlueprintPure, Category = "Rld|Floor")
    FCmnGridLayoutBuildResult GetCurrentFloorLayout() const
    {
        return currentFloorLayout;
    }

public:

    // ----- 管理Actor取得 -----

    UFUNCTION(BlueprintCallable, Category = "Rld|Floor")
    void ResolveManagers();

private:

    // ----- 管理Actor取得 -----

    void ResolveGridManager();
    void ResolvePlayerCharacter();
    void ResolveTurnManager();
    void ResolveEnemyManager();

private:

    // ----- フロア定義読込 -----

    /**
     * 指定フロア番号に対応するRowNameを作成する
     *
     * @param floorIndex フロア番号
     * @return RowName
     */
    FName BuildFloorRowName(int32 floorIndex) const;

    /**
     * 指定フロア番号の定義を読込する
     *
     * @param floorIndex フロア番号
     * @param outFloorDefinition 読込結果
     * @return 読込成功ならtrue
     */
    bool TryLoadFloorDefinition(int32 floorIndex, FRldFloorDefinition& outFloorDefinition) const;

private:

    // ----- フロア生成 -----

    /**
     * 現在のフロア定義からレイアウトを生成する
     *
     * @param outFloorLayout 生成結果
     * @return 生成成功ならtrue
     */
    bool TryBuildFloorLayout(FCmnGridLayoutBuildResult& outFloorLayout);

private:

    // ----- フロア反映 -----

    /** フロア状態をグリッドとプレイヤーへ反映する */
    void ApplyFloorState();

private:

    // ----- デバッグ描画 -----

    /** フロアデバッグ描画の凡例をログ出力する */
    void LogDebugDrawLegend();

    /** フロアデバッグ描画を更新する */
    void UpdateContinuousDebugDraw(float deltaSeconds);

    /** フロア常時デバッグ描画が有効か判定する */
    bool ShouldDrawContinuousDebug() const;

    /**
     * 現在フロアのデバッグ描画を行う
     *
     * @param bOutputLog 描画ログを出力するか
     */
    void DrawDebugFloorStateInternal(bool bOutputLog) const;

private:

    // ----- 管理Actor参照 -----

    UPROPERTY(Transient)
    TObjectPtr<ARldGridManager> gridManager = nullptr;

    UPROPERTY(Transient)
    TObjectPtr<ARldPlayerCharacter> playerCharacter = nullptr;

    UPROPERTY(Transient)
    TObjectPtr<ARldTurnManager> turnManager = nullptr;

    UPROPERTY(Transient)
    TObjectPtr<ARldEnemyManager> enemyManager = nullptr;

private:

    // ----- フロア設定 -----

    // フロア定義DataTable
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rld|Floor", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UDataTable> floorDefinitionDataTable = nullptr;

    // BeginPlay時に自動開始するか
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rld|Floor", meta = (AllowPrivateAccess = "true"))
    bool bStartOnBeginPlay = true;

private:

    // ----- フロア状態 -----

    // 現在のフロア番号
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rld|Floor", meta = (AllowPrivateAccess = "true", ClampMin = "1"))
    int32 currentFloorIndex = 1;

    // 現在フロアの定義
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rld|Floor", meta = (AllowPrivateAccess = "true"))
    FRldFloorDefinition currentFloorDefinition;

    // 現在フロアの生成結果
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rld|Floor", meta = (AllowPrivateAccess = "true"))
    FCmnGridLayoutBuildResult currentFloorLayout;

private:

    // ----- フロア生成器 -----

    FRldFloorGenerator floorGenerator;

private:

    // ----- デバッグ描画設定 -----

    // フロア反映時に自動でデバッグ描画するか
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rld|Floor|Debug", meta = (AllowPrivateAccess = "true"))
    bool bDrawDebugOnApplyFloorState = true;

    // チェックON中に短時間DebugDrawを定期再描画するか
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rld|Floor|Debug", meta = (AllowPrivateAccess = "true"))
    bool bEnableContinuousDebugDraw = true;

    // 常時デバッグ描画の再描画間隔
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rld|Floor|Debug", meta = (AllowPrivateAccess = "true", ClampMin = "0.01"))
    float continuousDebugDrawInterval = 0.10f;

    // 部屋外枠を描画するか
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rld|Floor|Debug", meta = (AllowPrivateAccess = "true"))
    bool bDrawRoomBounds = true;

    // 開始位置を描画するか
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rld|Floor|Debug", meta = (AllowPrivateAccess = "true"))
    bool bDrawPlayerStartCell = true;

    // 階段位置を強調描画するか
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rld|Floor|Debug", meta = (AllowPrivateAccess = "true"))
    bool bDrawStairsHighlightCell = true;

    // 部屋外枠描画設定
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rld|Floor|Debug", meta = (AllowPrivateAccess = "true"))
    FCmnDebugDrawStyle roomBoundsDebugStyle;

    // 開始位置描画設定
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rld|Floor|Debug", meta = (AllowPrivateAccess = "true"))
    FCmnDebugDrawStyle playerStartDebugStyle;

    // 階段強調描画設定
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rld|Floor|Debug", meta = (AllowPrivateAccess = "true"))
    FCmnDebugDrawStyle stairsHighlightDebugStyle;

    // 常時デバッグ描画の経過時間
    float continuousDebugDrawElapsed = 0.0f;

    // デバッグ描画凡例を出力済みか
    bool bDebugLegendLogged = false;
};
