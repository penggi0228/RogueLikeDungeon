// RldFloorManager.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Game/Floor/RldFloorDefinition.h"
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

public:

    // ----- フロア進行 -----

    /**
     * 現在のフロア番号でフロア開始処理を行う
     */
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

    // ----- フロア反映 -----

    /** フロア状態をグリッドとプレイヤーと敵へ反映する */
    void ApplyFloorState();

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
};
