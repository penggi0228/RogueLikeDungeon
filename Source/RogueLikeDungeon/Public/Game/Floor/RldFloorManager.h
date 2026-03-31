// RldFloorManager.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RldFloorManager.generated.h"

class ARldGridManager;
class ARldPlayerCharacter;
class ARldTurnManager;

/**
 * フロア進行管理Actor
 * フロア開始と次フロア遷移を管理する
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

public:

    // ----- フロア進行 ----- 

    /**
     * フロア開始処理を行う
     * 現在のフロア番号で開始する
     */
    UFUNCTION(BlueprintCallable, Category = "Rld|Floor")
    void StartFloor();

    /**
     * 指定フロアの開始処理を行う
     *
     * @param floorIndex フロア番号
     */
    UFUNCTION(BlueprintCallable, Category = "Rld|Floor")
    void StartFloorAt(int32 floorIndex);

    /**
     * 次フロアへ進む
     */
    UFUNCTION(BlueprintCallable, Category = "Rld|Floor")
    void GoToNextFloor();

public:

    // ----- Getter -----

    /** 現在のフロア番号を取得する */
    UFUNCTION(BlueprintPure, Category = "Rld|Floor")
    int32 GetCurrentFloorIndex() const
    {
        return currentFloorIndex;
    }

    /** プレイヤー開始グリッド座標を取得する */
    UFUNCTION(BlueprintPure, Category = "Rld|Floor")
    FIntPoint GetPlayerStartGridCoord() const
    {
        return playerStartGridCoord;
    }

    /** 階段グリッド座標を取得する */
    UFUNCTION(BlueprintPure, Category = "Rld|Floor")
    FIntPoint GetStairsGridCoord() const
    {
        return stairsGridCoord;
    }

public:

    // ----- Setter -----

    /** プレイヤー開始グリッド座標を設定する */
    UFUNCTION(BlueprintCallable, Category = "Rld|Floor")
    void SetPlayerStartGridCoord(const FIntPoint& newGridCoord);

    /** 階段グリッド座標を設定する */
    UFUNCTION(BlueprintCallable, Category = "Rld|Floor")
    void SetStairsGridCoord(const FIntPoint& newGridCoord);

private:

    // ----- 管理Actor取得 -----

    void ResolveGridManager();
    void ResolvePlayerCharacter();
    void ResolveTurnManager();

private:

    // ----- フロア反映 -----

    /** フロア状態をグリッドとプレイヤーへ反映する */
    void ApplyFloorState();

private:

    // ----- 管理Actor参照 -----

    UPROPERTY(Transient)
    TObjectPtr<ARldGridManager> gridManager = nullptr;

    UPROPERTY(Transient)
    TObjectPtr<ARldPlayerCharacter> playerCharacter = nullptr;

    UPROPERTY(Transient)
    TObjectPtr<ARldTurnManager> turnManager = nullptr;

private:

    // ----- フロア状態 -----

    // 現在のフロア番号
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rld|Floor", meta = (AllowPrivateAccess = "true", ClampMin = "1"))
    int32 currentFloorIndex = 1;

    // プレイヤー開始位置
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rld|Floor", meta = (AllowPrivateAccess = "true"))
    FIntPoint playerStartGridCoord = FIntPoint(1, 1);

    // 階段位置
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rld|Floor", meta = (AllowPrivateAccess = "true"))
    FIntPoint stairsGridCoord = FIntPoint(18, 18);

    // BeginPlay時に自動開始するか
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rld|Floor", meta = (AllowPrivateAccess = "true"))
    bool bStartOnBeginPlay = true;
};
