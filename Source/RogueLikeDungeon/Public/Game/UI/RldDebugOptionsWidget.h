// RldDebugOptionsWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RldDebugOptionsWidget.generated.h"

class UButton;
class UCheckBox;
class UCmnDebugWorldSubsystem;

/**
 * デバッグ専用オプション画面のロジックを行うWidget
 * ゲーム固有の表示項目から共通デバッグSubsystemを操作する
 */
UCLASS()
class ROGUELIKEDUNGEON_API URldDebugOptionsWidget : public UUserWidget
{
    GENERATED_BODY()

protected:

    // ----- UUserWidget -----

    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;

private:

    // ----- BindWidget: 行ボタン -----

    // デバッグ描画全体行ボタン
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> ButtonDebugEnabledRow;

    // グリッドデバッグ描画行ボタン
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> ButtonGridDebugDrawRow;

    // フロアデバッグ描画行ボタン
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> ButtonFloorDebugDrawRow;

    // AIデバッグ描画行ボタン
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> ButtonAIDebugDrawRow;

    // 当たり判定デバッグ描画行ボタン
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> ButtonCollisionDebugDrawRow;

private:

    // ----- BindWidget: チェックボックス -----

    // デバッグ描画全体チェックボックス
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UCheckBox> CheckBoxDebugEnabled;

    // グリッドデバッグ描画チェックボックス
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UCheckBox> CheckBoxGridDebugDraw;

    // フロアデバッグ描画チェックボックス
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UCheckBox> CheckBoxFloorDebugDraw;

    // AIデバッグ描画チェックボックス
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UCheckBox> CheckBoxAIDebugDraw;

    // 当たり判定デバッグ描画チェックボックス
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UCheckBox> CheckBoxCollisionDebugDraw;

private:

    // ----- BindWidget: 操作ボタン -----

    // 閉じるボタン
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> ButtonClose;

public:

    // ----- 外部入力操作 -----

    /**
     * デバッグ項目のフォーカス行を移動する
     *
     * @param moveOffset 移動方向
     */
    void MoveDebugFocus(int32 moveOffset);

    /** 現在フォーカス中のデバッグ項目を決定する */
    void ConfirmFocusedDebugItem();

    /** デバッグオプションを閉じる */
    void CloseDebugOptions();

private:

    // ----- Event Handlers: 行ボタン -----

    /** デバッグ描画全体行クリック時の処理を行う */
    UFUNCTION()
    void OnDebugEnabledRowClicked();

    /** グリッドデバッグ描画行クリック時の処理を行う */
    UFUNCTION()
    void OnGridDebugDrawRowClicked();

    /** フロアデバッグ描画行クリック時の処理を行う */
    UFUNCTION()
    void OnFloorDebugDrawRowClicked();

    /** AIデバッグ描画行クリック時の処理を行う */
    UFUNCTION()
    void OnAIDebugDrawRowClicked();

    /** 当たり判定デバッグ描画行クリック時の処理を行う */
    UFUNCTION()
    void OnCollisionDebugDrawRowClicked();

private:

    // ----- Event Handlers: チェックボックス -----

    /** デバッグ描画全体チェック変更時の処理を行う */
    UFUNCTION()
    void OnDebugEnabledChanged(bool bIsChecked);

    /** グリッドデバッグ描画チェック変更時の処理を行う */
    UFUNCTION()
    void OnGridDebugDrawChanged(bool bIsChecked);

    /** フロアデバッグ描画チェック変更時の処理を行う */
    UFUNCTION()
    void OnFloorDebugDrawChanged(bool bIsChecked);

    /** AIデバッグ描画チェック変更時の処理を行う */
    UFUNCTION()
    void OnAIDebugDrawChanged(bool bIsChecked);

    /** 当たり判定デバッグ描画チェック変更時の処理を行う */
    UFUNCTION()
    void OnCollisionDebugDrawChanged(bool bIsChecked);

private:

    // ----- Event Handlers: 操作ボタン -----

    /** 閉じるボタンクリック時の処理を行う */
    UFUNCTION()
    void OnCloseClicked();

private:

    // ----- 内部処理 -----

    /** DebugWorldSubsystemを取得する */
    UCmnDebugWorldSubsystem* GetDebugWorldSubsystem() const;

    /** 現在のデバッグ設定をUIへ反映する */
    void RefreshUIFromDebugSettings();

    /** デバッグ種別チェックボックスの有効状態を更新する */
    void UpdateCategoryCheckBoxEnabledState();

    /**
     * チェックボックスへカテゴリ状態を反映する
     *
     * @param checkBox 反映先チェックボックス
     * @param categoryName デバッグカテゴリ名
     */
    void RefreshCategoryCheckBox(UCheckBox* checkBox, FName categoryName) const;

    /** デバッグ描画全体の有効状態を反転する */
    void ToggleDebugEnabled();

    /**
     * デバッグカテゴリの有効状態を反転する
     *
     * @param categoryName デバッグカテゴリ名
     */
    void ToggleDebugCategory(FName categoryName);

    /**
     * カテゴリ有効状態を更新する
     *
     * @param categoryName デバッグカテゴリ名
     * @param bIsChecked 有効状態
     * @param sourceFunctionName 呼び出し元関数名
     */
    void SetDebugCategoryEnabled(
        FName categoryName,
        bool bIsChecked,
        const TCHAR* sourceFunctionName
    );

private:

    // ----- フォーカス制御 -----

    /** フォーカス対象行ボタン一覧を取得する */
    TArray<UButton*> GetFocusableRowButtons() const;

    /** 初期フォーカスを設定する */
    void SetInitialDebugFocus();

    /** 現在のフォーカス行へキーボードフォーカスを適用する */
    void ApplyCurrentDebugFocus();

private:

    // ----- UI更新状態 -----

    // UI反映中にチェック変更イベントを処理しないためのフラグ
    bool bRefreshingUI = false;

    // 現在フォーカス中の行Index
    int32 focusedRowIndex = 0;
};
