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

    // ----- InputMappingContext -----

    // ゲーム用IMC
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Common|Input|IMC")
    TObjectPtr<UInputMappingContext> IMC_Game = nullptr;

    // UI用IMC
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Common|Input|IMC")
    TObjectPtr<UInputMappingContext> IMC_UI = nullptr;

public:

    // ----- Game InputAction -----

    // ゲーム移動用InputAction
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Common|Input|IA|Game")
    TObjectPtr<UInputAction> IA_Move = nullptr;

    // 待機用InputAction
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Common|Input|IA|Game")
    TObjectPtr<UInputAction> IA_Wait = nullptr;

    // カメラ視点操作用InputAction
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Common|Input|IA|Game")
    TObjectPtr<UInputAction> IA_CameraLook = nullptr;

    // カメラズーム操作用InputAction
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Common|Input|IA|Game")
    TObjectPtr<UInputAction> IA_CameraZoom = nullptr;

public:

    // ----- UI InputAction -----

    // UI方向入力用InputAction
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Common|Input|IA|UI")
    TObjectPtr<UInputAction> IA_UI_Direction = nullptr;

    // UI決定用InputAction
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Common|Input|IA|UI")
    TObjectPtr<UInputAction> IA_UI_Confirm = nullptr;

    // UIクローズ用InputAction
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Common|Input|IA|UI")
    TObjectPtr<UInputAction> IA_UI_Close = nullptr;

    // UIスクロール用InputAction
    // マウスホイールや右スティックY軸入力を受け取る
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Common|Input|IA|UI")
    TObjectPtr<UInputAction> IA_UI_Scroll = nullptr;

public:

    // ----- Debug Command InputAction -----

    // デバッグコマンド開始用InputAction
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Common|Input|IA|Debug")
    TObjectPtr<UInputAction> IA_DebugCommandPrefix = nullptr;

    // キーボード用デバッグコマンド1InputAction
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Common|Input|IA|Debug|Keyboard")
    TObjectPtr<UInputAction> IA_DebugCommandKeyboard1 = nullptr;

    // キーボード用デバッグコマンド2InputAction
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Common|Input|IA|Debug|Keyboard")
    TObjectPtr<UInputAction> IA_DebugCommandKeyboard2 = nullptr;

    // キーボード用デバッグコマンド3InputAction
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Common|Input|IA|Debug|Keyboard")
    TObjectPtr<UInputAction> IA_DebugCommandKeyboard3 = nullptr;

    // キーボード用デバッグコマンド4InputAction
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Common|Input|IA|Debug|Keyboard")
    TObjectPtr<UInputAction> IA_DebugCommandKeyboard4 = nullptr;

    // キーボード用デバッグコマンド5InputAction
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Common|Input|IA|Debug|Keyboard")
    TObjectPtr<UInputAction> IA_DebugCommandKeyboard5 = nullptr;

    // ゲームパッド用デバッグコマンド1InputAction
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Common|Input|IA|Debug|Gamepad")
    TObjectPtr<UInputAction> IA_DebugCommandGamepad1 = nullptr;

    // ゲームパッド用デバッグコマンド2InputAction
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Common|Input|IA|Debug|Gamepad")
    TObjectPtr<UInputAction> IA_DebugCommandGamepad2 = nullptr;

public:

    // ----- Parameters -----

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
