// CmnInputConfig.h

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CmnInputConfig.generated.h"

class UInputMappingContext;
class UInputAction;

/**
 * 共通入力設定DataAsset
 * ゲーム操作・UI操作・デバッグ操作で使用するInputActionとIMCを保持する
 */
UCLASS(BlueprintType)
class ROGUELIKEDUNGEON_API UCmnInputConfig : public UDataAsset
{
    GENERATED_BODY()

public:

    // ----- Input Mapping Context -----

    // ゲーム操作用IMC
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cmn|Input|IMC")
    TObjectPtr<UInputMappingContext> IMC_Game = nullptr;

    // UI操作用IMC
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cmn|Input|IMC")
    TObjectPtr<UInputMappingContext> IMC_UI = nullptr;

public:

    // ----- Game InputAction -----

    // ダンジョン移動用InputAction
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cmn|Input|Game")
    TObjectPtr<UInputAction> IA_Move = nullptr;

    // 足踏み用InputAction
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cmn|Input|Game")
    TObjectPtr<UInputAction> IA_StepInPlaceModifier = nullptr;

    // 通常攻撃用InputAction
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cmn|Input|Game")
    TObjectPtr<UInputAction> IA_Attack = nullptr;

    // インタラクト用InputAction
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cmn|Input|Game")
    TObjectPtr<UInputAction> IA_Interact = nullptr;

    // メニュー表示用InputAction
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cmn|Input|Game")
    TObjectPtr<UInputAction> IA_Menu = nullptr;

    // カメラ視点用InputAction
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cmn|Input|Game")
    TObjectPtr<UInputAction> IA_CameraLook = nullptr;

    // カメラズーム用InputAction
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cmn|Input|Game")
    TObjectPtr<UInputAction> IA_CameraZoom = nullptr;

public:

    // ----- UI InputAction -----

    // UI方向入力用InputAction
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cmn|Input|UI")
    TObjectPtr<UInputAction> IA_UI_Direction = nullptr;

    // UI決定用InputAction
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cmn|Input|UI")
    TObjectPtr<UInputAction> IA_UI_Confirm = nullptr;

    // UI閉じる用InputAction
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cmn|Input|UI")
    TObjectPtr<UInputAction> IA_UI_Close = nullptr;

    // UIスクロール用InputAction
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cmn|Input|UI")
    TObjectPtr<UInputAction> IA_UI_Scroll = nullptr;

public:

    // ----- Debug InputAction -----

    // デバッグコマンド開始用InputAction
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cmn|Input|Debug")
    TObjectPtr<UInputAction> IA_DebugCommandPrefix = nullptr;

    // キーボード用デバッグコマンド第1InputAction
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cmn|Input|Debug")
    TObjectPtr<UInputAction> IA_DebugCommandKeyboard1 = nullptr;

    // キーボード用デバッグコマンド第2InputAction
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cmn|Input|Debug")
    TObjectPtr<UInputAction> IA_DebugCommandKeyboard2 = nullptr;

    // キーボード用デバッグコマンド第3InputAction
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cmn|Input|Debug")
    TObjectPtr<UInputAction> IA_DebugCommandKeyboard3 = nullptr;

    // キーボード用デバッグコマンド第4InputAction
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cmn|Input|Debug")
    TObjectPtr<UInputAction> IA_DebugCommandKeyboard4 = nullptr;

    // キーボード用デバッグコマンド第5InputAction
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cmn|Input|Debug")
    TObjectPtr<UInputAction> IA_DebugCommandKeyboard5 = nullptr;

    // ゲームパッド用デバッグコマンド第1InputAction
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cmn|Input|Debug")
    TObjectPtr<UInputAction> IA_DebugCommandGamepad1 = nullptr;

    // ゲームパッド用デバッグコマンド第2InputAction
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cmn|Input|Debug")
    TObjectPtr<UInputAction> IA_DebugCommandGamepad2 = nullptr;

public:

    // ----- Input Parameters -----

    // UI入力リピート開始までの秒数
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cmn|Input|Parameter", meta = (ClampMin = "0.0"))
    float UIRepeatDelay = 0.25f;

    // UI入力リピート間隔
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cmn|Input|Parameter", meta = (ClampMin = "0.01"))
    float UIRepeatInterval = 0.12f;

    // 入力デッドゾーン
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cmn|Input|Parameter", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DeadZone = 0.50f;
};
