// CmnInputConfig.h

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CmnInputConfig.generated.h"

class UInputMappingContext;
class UInputAction;

/**
 * 共通入力設定用DataAsset
 *
 * ・IMC/IAの参照をまとめて管理
 * ・UIリピートやデッドゾーンなどの入力パラメータを保持
 *
 * ※ロジックは持たず、入力設定データのみを保持する
 */
UCLASS(BlueprintType)
class ROGUELIKEDUNGEON_API UCmnInputConfig : public UDataAsset
{
    GENERATED_BODY()

public:
    /** ゲーム用IMC */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Common|Input|IMC")
    TObjectPtr<UInputMappingContext> IMC_Game = nullptr;

    /** UI用IMC */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Common|Input|IMC")
    TObjectPtr<UInputMappingContext> IMC_UI = nullptr;

    /** ゲーム移動用Axis2D */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Common|Input|IA")
    TObjectPtr<UInputAction> IA_Move = nullptr;

    /** カメラ操作:視点回転(Axis2D) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Common|Input|IA")
    TObjectPtr<UInputAction> IA_CameraLook = nullptr;

    /** カメラ操作:ズーム(Axis1D) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Common|Input|IA")
    TObjectPtr<UInputAction> IA_CameraZoom = nullptr;

    /** UI方向入力用Axis2D */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Common|Input|IA")
    TObjectPtr<UInputAction> IA_UI_Direction = nullptr;

    /** UIスクロール用Axis1D(ホイール/右スティックY軸など) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Common|Input|IA")
    TObjectPtr<UInputAction> IA_UI_Scroll = nullptr;

    /** UIリピート初回遅延 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Common|Input|UI")
    float UIRepeatDelay = 0.25f;

    /** UIリピート間隔 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Common|Input|UI")
    float UIRepeatInterval = 0.12f;

    /** スティックデッドゾーン */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Common|Input|Axis")
    float DeadZone = 0.50f;
};
