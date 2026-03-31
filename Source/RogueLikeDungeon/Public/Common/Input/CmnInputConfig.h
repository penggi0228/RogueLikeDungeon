// CmnInputConfig.h

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CmnInputConfig.generated.h"

class UInputMappingContext;
class UInputAction;

/**
 * 共通入力設定用DataAsset
 * 入力アセット参照と入力設定値を保持する
 */
UCLASS(BlueprintType)
class ROGUELIKEDUNGEON_API UCmnInputConfig : public UDataAsset
{
    GENERATED_BODY()

public:

    // ゲーム用IMC
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Common|Input|IMC")
    TObjectPtr<UInputMappingContext> IMC_Game = nullptr;

    // UI用IMC
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Common|Input|IMC")
    TObjectPtr<UInputMappingContext> IMC_UI = nullptr;

    // ゲーム移動用InputAction
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Common|Input|IA")
    TObjectPtr<UInputAction> IA_Move = nullptr;

    // カメラ視点操作用InputAction
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Common|Input|IA")
    TObjectPtr<UInputAction> IA_CameraLook = nullptr;

    // カメラズーム操作用InputAction
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Common|Input|IA")
    TObjectPtr<UInputAction> IA_CameraZoom = nullptr;

    // UI方向入力用InputAction
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Common|Input|IA")
    TObjectPtr<UInputAction> IA_UI_Direction = nullptr;

    // UIスクロール用InputAction
    // マウスホイールや右スティックY軸入力を受け取る
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Common|Input|IA")
    TObjectPtr<UInputAction> IA_UI_Scroll = nullptr;

    // UIリピートの初回遅延時間
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Common|Input|UI")
    float UIRepeatDelay = 0.25f;

    // UIリピートの間隔
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Common|Input|UI")
    float UIRepeatInterval = 0.12f;

    // スティック入力のデッドゾーン
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Common|Input|Axis")
    float DeadZone = 0.50f;
};
