// CmnGridCoordFunctionLibrary.h

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Common/Grid/CmnGridDefinition.h"
#include "CmnGridCoordFunctionLibrary.generated.h"

/**
 * グリッド座標変換用FunctionLibrary
 * グリッド座標とワールド座標の相互変換を行う
 */
UCLASS()
class ROGUELIKEDUNGEON_API UCmnGridCoordFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    /**
     * グリッド座標をワールド座標へ変換する
     *
     * @param gridDefinition グリッド定義
     * @param gridCoord グリッド座標
     * @return ワールド座標
     */
    UFUNCTION(BlueprintPure, Category = "Cmn|Grid")
    static FVector GridToWorld(const FCmnGridDefinition& gridDefinition, const FIntPoint& gridCoord);

    /**
     * ワールド座標をグリッド座標へ変換する
     *
     * @param gridDefinition グリッド定義
     * @param worldLocation ワールド座標
     * @return グリッド座標
     */
    UFUNCTION(BlueprintPure, Category = "Cmn|Grid")
    static FIntPoint WorldToGrid(const FCmnGridDefinition& gridDefinition, const FVector& worldLocation);
};
