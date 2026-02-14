// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Common/Input/CmnInputTypes.h"
#include "CmnInputRouter.generated.h"

class AplayerController;
class UEnhancedInputLocalPlayerSubsystem;
class UInputMappingContext;
class UInputAction;

/**
 * 共通入力ルータ
 * - IMC切替（Game / Menu / Dialog / Disabled）
 * - UIナビ（十字＋スティック）を4方向へ正規化
 * - スティック保持のUIリピート（初回遅延＋連続間隔）
 * - PlayerControllerからゲーム固有依存を切り離す
 */
UCLASS()
class ROGUELIKEDUNGEON_API UCmnInputRouter : public UObject
{
	GENERATED_BODY()
	
	public:
		/** 初期化（PCとSubsystemを保持） */
		void Initialize(APlayerController* InPC, UEnhancedInputLocalPlayerSubsystem* InSubsys);

		/** フレーム更新（UIリピート用） */
		void Tick(float DeltaSeconds);

		/** 入力モード設定（Game / Menu / Dialog / Disabled） */
		void SetInputMode(ECmnInputMode NewMode);

		/** 現在の入力モード取得 */
		ECmnInputMode GetInputMode() const {
			return CurrentMode;
		}

        bool IsGameMode() const { return CurrentMode == ECmnInputMode::Game; }
        bool IsMenuMode() const { return CurrentMode == ECmnInputMode::Menu; }
        bool IsDialogMode() const { return CurrentMode == ECmnInputMode::Dialog; }
        bool IsDisabled() const { return CurrentMode == ECmnInputMode::Disabled; }

        /** UIナビ：Axis2D入力を受け取り、方向イベントに変換（Menu/Dialogで有効） */
        void HandleUINavigateAxis(const FVector2D& Axis);

        /** ゲーム移動用：Axis2Dを4方向へスナップ（1回分の方向を返す） */
        bool GetMoveDirFromAxis(const FVector2D& Axis, FIntPoint& OutDir) const;

public:
    /** UI方向入力通知（Widget側でBindできる） */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUINavigation, FIntPoint, Direction);

    /** 入力モード変更通知（UI側の状態切替に使う） */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInputModeChanged, ECmnInputMode, NewMode);

    UPROPERTY(BlueprintAssignable, Category = "Common|Input")
    FOnUINavigation OnUINavigation;

    UPROPERTY(BlueprintAssignable, Category = "Common|Input")
    FOnInputModeChanged OnInputModeChanged;

public:
    /** IMC（コンテンツ側で設定） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Common|Input")
    UInputMappingContext* IMC_Game = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Common|Input")
    UInputMappingContext* IMC_UI = nullptr;

    /** UIナビ（Axis2D） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Common|Input")
    UInputAction* IA_UI_Navigate = nullptr;

    /** ゲーム移動（Axis2D） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Common|Input")
    UInputAction* IA_Move = nullptr;

    /** UIリピート設定 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Common|Input")
    float UIRepeatDelay = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Common|Input")
    float UIRepeatInterval = 0.12f;

    /** デッドゾーン（スティック倒し具合） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Common|Input")
    float DeadZone = 0.50f;

private:
    /** Axis2Dを4方向へスナップ */
    bool SnapToCardinal(const FVector2D& Axis, FIntPoint& OutDir) const;

    /** IMCを入れ替え（UI側を高優先度で追加） */
    void ApplyMappingContexts(bool bEnableUI);

    /** Menu/Dialogならtrue（UIナビ・リピート有効） */
    bool IsUIMode() const;

    /** UIリピート停止 */
    void StopUIRepeat();

private:
    UPROPERTY()
    TObjectPtr<APlayerController> PC;

    UPROPERTY()
    TObjectPtr<UEnhancedInputLocalPlayerSubsystem> Subsys;

    ECmnInputMode CurrentMode = ECmnInputMode::Game;

    // UIリピート制御
    bool bUIRepeating = false;
    float RepeatTimer = 0.0f;
    FIntPoint CurrentUIDir = FIntPoint::ZeroValue;
};
