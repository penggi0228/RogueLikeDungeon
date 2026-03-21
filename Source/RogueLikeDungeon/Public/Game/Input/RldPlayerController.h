// RldPlayerController.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "TimerManager.h"

#include "Common/Input/CmnInputTypes.h"
#include "RldPlayerController.generated.h"

class UEnhancedInputComponent;
class UEnhancedInputLocalPlayerSubsystem;
class UCmnInputRouter;
class UCmnInputConfig;
class UCmnSettingsSubsystem;
class ARldPlayerCharacter;
struct FInputRuntimeSettings;

/**
 * ゲーム固有PlayerController
 *
 * EnhancedInputのBindのみ担当
 * 入力解釈はゲーム固有のPlayerController側で行う
 * 入力設定はUCmnSettingsSubsystemから通知で受け取る
 */
UCLASS()
class ROGUELIKEDUNGEON_API ARldPlayerController : public APlayerController
{
    GENERATED_BODY()

public:

    ARldPlayerController();

    // ----- AActor -----

    virtual void PostInitializeComponents() override;
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // ----- AController -----

    virtual void SetupInputComponent() override;

public:

    /**
     * 共通入力モードを切り替える
     * IMC適用およびInputMode制御はRouterへ委譲する
     */
    UFUNCTION(BlueprintCallable, Category = "Rld|Input")
    void SetCommonInputMode(ECmnInputMode Mode);

    /**
     * Routerを取得する(UI側参照用)
     */
    UFUNCTION(BlueprintCallable, Category = "Rld|Input")
    UCmnInputRouter* GetInputRouter() const;

private:

    // ----- 設定通知 -----

    /**
     * Subsystemからの入力設定変更通知を受け取る
     * Controller内のローカルキャッシュを更新する
     */
    void HandleInputSettingsChanged(const FInputRuntimeSettings& NewSettings);

private:

    // ----- ゲーム操作の入力 -----

    /**
     * 移動入力開始時の処理
     * 1入力=1ターンの確定処理を行う
     */
    void OnMoveStarted(const FInputActionValue& Value);

    /**
     * 移動入力の生値ログ出力用
     * デバッグ確認のためTriggeredでも受け取る
     */
    void OnMoveTriggered(const FInputActionValue& Value);

    /**
     * カメラ視点入力処理
     */
    void OnCameraLookTriggered(const FInputActionValue& Value);

    /**
     * カメラズーム入力処理
     */
    void OnCameraZoomTriggered(const FInputActionValue& Value);

private:

    // ----- UI操作の入力 -----

    /**
     * UI方向入力処理
     */
    void OnUIDirectionTriggered(const FInputActionValue& Value);

    /**
     * UIスクロール入力処理
     */
    void OnUIScrollTriggered(const FInputActionValue& Value);

private:

    // ----- ゲーム固有入力変換 -----

    /**
     * 入力軸をカメラ基準のグリッド4方向へ変換する
     *
     * @param Axis 入力軸
     * @param OutDirection 変換後のグリッド方向
     * @return 方向確定できた場合はtrueを返す
     */
    bool TryConvertMoveAxisToGridDir(const FVector2D& Axis, FIntPoint& OutDirection) const;

    /**
     * 入力軸をカメラ基準のワールド平面方向へ変換する
     *
     * @param Axis 入力軸
     * @param OutWorldDirection 変換後のワールド平面方向
     * @return 有効な方向が得られた場合はtrueを返す
     */
    bool TryConvertInputAxisToCameraRelativeWorldDir(
        const FVector2D& Axis,
        FVector& OutWorldDirection
    ) const;

    /**
     * ワールド平面方向をグリッド4方向へ変換する
     *
     * @param WorldDirection ワールド平面方向
     * @param OutGridDirection 変換後のグリッド方向
     * @return 方向確定できた場合はtrueを返す
     */
    bool TryConvertWorldDirToGridDir(
        const FVector& WorldDirection,
        FIntPoint& OutGridDirection
    ) const;

private:

    // ----- 左スティック専用処理 -----

    /**
     * 左スティック入力かどうかを判定する
     *
     * @param Axis 入力軸
     * @return 左スティック入力とみなせる場合はtrueを返す
     */
    bool IsLikelyLeftStickInput(const FVector2D& Axis) const;

    /**
     * 左スティックの1入力=1ターン確定処理
     * Triggeredから呼び出し、しきい値を超えた瞬間のみ移動を確定する
     *
     * @param Axis 入力軸
     */
    void HandleLeftStickMoveTriggered(const FVector2D& Axis);

private:

    // ----- Character取得 -----

    /**
     * 現在操作中のRogueLikeDungeon用プレイヤーCharacterを取得する
     *
     * @return 取得成功時はARldPlayerCharacter、失敗時はnullptr
     */
    ARldPlayerCharacter* GetRldPlayerCharacter() const;

private:

    // ----- 左スティックの状態 -----

    /** 左スティック入力が確定済みならtrue */
    bool bLeftStickMoveConsumed = false;

private:

    // ----- デバッグ補助 -----

    /**
     * 現在の移動入力元をデバッグ文字列として取得する
     * キーボード、十字キー、左スティックの状態を見て推定する
     */
    FString BuildMoveInputSourceDebugText() const;
    
private:

    /**
     * 確定した移動方向を処理する
     *
     * @param Direction 確定した移動方向
     * @param InputSourceText デバッグ表示用の入力元文字列
     * @param Axis デバッグ表示用の入力軸
     */
    void ProcessResolvedMoveDirection(
        const FIntPoint& Direction,
        const FString& InputSourceText,
        const FVector2D& Axis
    );

private:

    // ----- 移動リピート制御 -----

    /**
     * 押しっぱなし移動のリピートを開始する
     *
     * @param Direction 確定した移動方向
     * @param InputSourceText デバッグ表示用の入力元文字列
     * @param Axis デバッグ表示用の入力軸
     */
    void StartMoveRepeat(
        const FIntPoint& Direction,
        const FString& InputSourceText,
        const FVector2D& Axis
    );

    /**
     * 押しっぱなし移動のリピートを停止する
     */
    void StopMoveRepeat();

    /**
     * 押しっぱなし移動の初回遅延後に呼ばれる
     */
    void BeginMoveRepeat();

    /**
     * 押しっぱなし移動の定期実行
     */
    void TickMoveRepeat();

    /**
     * 現在の入力状態から、押しっぱなし移動を継続すべきか判定する
     */
    bool ShouldContinueMoveRepeat() const;

private:

    // ----- 移動リピート状態 -----

    /** 初回遅延Timer */
    FTimerHandle MoveRepeatStartTimerHandle;

    /** 継続リピートTimer */
    FTimerHandle MoveRepeatTickTimerHandle;

    /** 現在のリピート方向 */
    FIntPoint RepeatingMoveDirection = FIntPoint::ZeroValue;

    /** 現在のリピート入力元 */
    FString RepeatingMoveInputSourceText;

    /** 現在のリピート入力軸 */
    FVector2D RepeatingMoveAxis = FVector2D::ZeroVector;

    /** 押しっぱなし移動の初回遅延秒数 */
    float MoveRepeatInitialDelay = 0.25f;

    /** 押しっぱなし移動の継続間隔秒数 */
    float MoveRepeatInterval = 0.10f;

    /** 押しっぱなし移動中ならtrue */
    bool bMoveRepeatActive = false;

private:

    // ----- 内部処理 -----

    /**
     * 入力設定のロード&ルータへ適用
     * SetupInputComponent前にIA参照を確定させるために使用する
     */
    void LoadAndApplyInputConfig();

    /**
     * EnhancedInputComponentへ差し替える
     * InputComponentClass依存を排除するための安全措置
     */
    void EnsureEnhancedInputComponent();

private:

    // ----- 入力設定DataAsset -----

    UPROPERTY(EditDefaultsOnly, Category = "Rld|Input")
    TSoftObjectPtr<UCmnInputConfig> InputConfigAsset;

    UPROPERTY(Transient)
    TObjectPtr<UCmnInputConfig> LoadedInputConfig = nullptr;

    UPROPERTY(VisibleAnywhere, Category = "Rld|Input")
    TObjectPtr<UCmnInputRouter> InputRouter = nullptr;

private:

    // ----- 設定ローカルキャッシュ -----

    bool bInvertCameraX = false;
    bool bInvertCameraY = false;

    /** 設定Subsystem参照(GameInstance管理のためUPROPERTY不要) */
    UCmnSettingsSubsystem* SettingsSubsystem = nullptr;
};
