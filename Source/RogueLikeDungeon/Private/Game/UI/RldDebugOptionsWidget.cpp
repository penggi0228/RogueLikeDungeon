// RldDebugOptionsWidget.cpp

#include "Game/UI/RldDebugOptionsWidget.h"

#include "Components/Button.h"
#include "Components/CheckBox.h"

#include "Common/Debug/CmnDebugCategories.h"
#include "Common/Debug/CmnDebugWorldSubsystem.h"
#include "Common/Input/CmnInputTypes.h"
#include "Game/Input/RldPlayerController.h"

DEFINE_LOG_CATEGORY_STATIC(LogRldDebugOptions, Log, All);

/** 初期化時のイベント登録を行う */
void URldDebugOptionsWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    // ----- 行ボタン -----

    // デバッグ描画全体行ボタン取得時はイベント登録
    if (ButtonDebugEnabledRow)
    {
        ButtonDebugEnabledRow->OnClicked.AddDynamic(this, &URldDebugOptionsWidget::OnDebugEnabledRowClicked);
    }
    else
    {
        UE_LOG(LogRldDebugOptions, Warning, TEXT("NativeOnInitialized: ButtonDebugEnabledRowが未設定です"));
    }

    // グリッド描画行ボタン取得時はイベント登録
    if (ButtonGridDebugDrawRow)
    {
        ButtonGridDebugDrawRow->OnClicked.AddDynamic(this, &URldDebugOptionsWidget::OnGridDebugDrawRowClicked);
    }
    else
    {
        UE_LOG(LogRldDebugOptions, Warning, TEXT("NativeOnInitialized: ButtonGridDebugDrawRowが未設定です"));
    }

    // フロア描画行ボタン取得時はイベント登録
    if (ButtonFloorDebugDrawRow)
    {
        ButtonFloorDebugDrawRow->OnClicked.AddDynamic(this, &URldDebugOptionsWidget::OnFloorDebugDrawRowClicked);
    }
    else
    {
        UE_LOG(LogRldDebugOptions, Warning, TEXT("NativeOnInitialized: ButtonFloorDebugDrawRowが未設定です"));
    }

    // AI描画行ボタン取得時はイベント登録
    if (ButtonAIDebugDrawRow)
    {
        ButtonAIDebugDrawRow->OnClicked.AddDynamic(this, &URldDebugOptionsWidget::OnAIDebugDrawRowClicked);
    }
    else
    {
        UE_LOG(LogRldDebugOptions, Warning, TEXT("NativeOnInitialized: ButtonAIDebugDrawRowが未設定です"));
    }

    // 当たり判定描画行ボタン取得時はイベント登録
    if (ButtonCollisionDebugDrawRow)
    {
        ButtonCollisionDebugDrawRow->OnClicked.AddDynamic(this, &URldDebugOptionsWidget::OnCollisionDebugDrawRowClicked);
    }
    else
    {
        UE_LOG(LogRldDebugOptions, Warning, TEXT("NativeOnInitialized: ButtonCollisionDebugDrawRowが未設定です"));
    }

    // ----- チェックボックス -----

    // デバッグ描画全体チェックボックス取得時はイベント登録
    if (CheckBoxDebugEnabled)
    {
        CheckBoxDebugEnabled->OnCheckStateChanged.AddDynamic(this, &URldDebugOptionsWidget::OnDebugEnabledChanged);
    }
    else
    {
        UE_LOG(LogRldDebugOptions, Warning, TEXT("NativeOnInitialized: CheckBoxDebugEnabledが未設定です"));
    }

    // グリッド描画チェックボックス取得時はイベント登録
    if (CheckBoxGridDebugDraw)
    {
        CheckBoxGridDebugDraw->OnCheckStateChanged.AddDynamic(this, &URldDebugOptionsWidget::OnGridDebugDrawChanged);
    }
    else
    {
        UE_LOG(LogRldDebugOptions, Warning, TEXT("NativeOnInitialized: CheckBoxGridDebugDrawが未設定です"));
    }

    // フロア描画チェックボックス取得時はイベント登録
    if (CheckBoxFloorDebugDraw)
    {
        CheckBoxFloorDebugDraw->OnCheckStateChanged.AddDynamic(this, &URldDebugOptionsWidget::OnFloorDebugDrawChanged);
    }
    else
    {
        UE_LOG(LogRldDebugOptions, Warning, TEXT("NativeOnInitialized: CheckBoxFloorDebugDrawが未設定です"));
    }

    // AI描画チェックボックス取得時はイベント登録
    if (CheckBoxAIDebugDraw)
    {
        CheckBoxAIDebugDraw->OnCheckStateChanged.AddDynamic(this, &URldDebugOptionsWidget::OnAIDebugDrawChanged);
    }
    else
    {
        UE_LOG(LogRldDebugOptions, Warning, TEXT("NativeOnInitialized: CheckBoxAIDebugDrawが未設定です"));
    }

    // 当たり判定描画チェックボックス取得時はイベント登録
    if (CheckBoxCollisionDebugDraw)
    {
        CheckBoxCollisionDebugDraw->OnCheckStateChanged.AddDynamic(this, &URldDebugOptionsWidget::OnCollisionDebugDrawChanged);
    }
    else
    {
        UE_LOG(LogRldDebugOptions, Warning, TEXT("NativeOnInitialized: CheckBoxCollisionDebugDrawが未設定です"));
    }

    // ----- 操作ボタン -----

    // 閉じるボタン取得時はイベント登録
    if (ButtonClose)
    {
        ButtonClose->OnClicked.AddDynamic(this, &URldDebugOptionsWidget::OnCloseClicked);
    }
    else
    {
        UE_LOG(LogRldDebugOptions, Warning, TEXT("NativeOnInitialized: ButtonCloseが未設定です"));
    }
}

/** 構築時に現在のデバッグ設定をUIへ反映する */
void URldDebugOptionsWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Subsystemの現在値をチェック状態へ反映
    RefreshUIFromDebugSettings();

    // 全体ON/OFFに応じてカテゴリ項目の操作可否を反映
    UpdateCategoryCheckBoxEnabledState();

    // UI表示時に最初の行へフォーカスを設定
    SetInitialDebugFocus();
}

/** デバッグ項目のフォーカス行を移動する */
void URldDebugOptionsWidget::MoveDebugFocus(int32 moveOffset)
{
    const TArray<UButton*> rowButtons = GetFocusableRowButtons();

    // フォーカス対象がない場合は移動しない
    if (rowButtons.Num() == 0)
    {
        UE_LOG(LogRldDebugOptions, Warning, TEXT("MoveDebugFocus: フォーカス対象が存在しないため移動しません"));
        return;
    }

    focusedRowIndex += moveOffset;

    // 上端を超えた場合は末尾へ回り込む
    if (focusedRowIndex < 0)
    {
        focusedRowIndex = rowButtons.Num() - 1;
    }
    // 下端を超えた場合は先頭へ回り込む
    else if (focusedRowIndex >= rowButtons.Num())
    {
        focusedRowIndex = 0;
    }

    ApplyCurrentDebugFocus();
}

/** 現在フォーカス中のデバッグ項目を決定する */
void URldDebugOptionsWidget::ConfirmFocusedDebugItem()
{
    const TArray<UButton*> rowButtons = GetFocusableRowButtons();

    // フォーカス対象がない場合は決定しない
    if (rowButtons.Num() == 0)
    {
        UE_LOG(LogRldDebugOptions, Warning, TEXT("ConfirmFocusedDebugItem: フォーカス対象が存在しないため決定できません"));
        return;
    }

    focusedRowIndex = FMath::Clamp(focusedRowIndex, 0, rowButtons.Num() - 1);

    UButton* focusedButton = rowButtons[focusedRowIndex];

    // 現在フォーカス行に対応する処理を実行
    if (focusedButton == ButtonDebugEnabledRow)
    {
        OnDebugEnabledRowClicked();
    }
    else if (focusedButton == ButtonGridDebugDrawRow)
    {
        OnGridDebugDrawRowClicked();
    }
    else if (focusedButton == ButtonFloorDebugDrawRow)
    {
        OnFloorDebugDrawRowClicked();
    }
    else if (focusedButton == ButtonAIDebugDrawRow)
    {
        OnAIDebugDrawRowClicked();
    }
    else if (focusedButton == ButtonCollisionDebugDrawRow)
    {
        OnCollisionDebugDrawRowClicked();
    }
    else if (focusedButton == ButtonClose)
    {
        CloseDebugOptions();
    }
    else
    {
        UE_LOG(LogRldDebugOptions, Warning, TEXT("ConfirmFocusedDebugItem: 対応する処理が見つかりません"));
    }
}

/** デバッグオプションを閉じる */
void URldDebugOptionsWidget::CloseDebugOptions()
{
    RemoveFromParent();

    // デバッグUIを閉じた後はゲーム操作モードへ戻す
    if (ARldPlayerController* playerController = Cast<ARldPlayerController>(GetOwningPlayer()))
    {
        playerController->SetCommonInputMode(ECmnInputMode::Game);
    }
    else
    {
        UE_LOG(LogRldDebugOptions, Warning, TEXT("CloseDebugOptions: OwningPlayerからRldPlayerControllerを取得できません"));
    }

    UE_LOG(LogRldDebugOptions, Log, TEXT("CloseDebugOptions: デバッグオプションを閉じました"));
}

/** デバッグ描画全体行クリック時の処理を行う */
void URldDebugOptionsWidget::OnDebugEnabledRowClicked()
{
    // 行クリック時はチェックボックスだけでなくSubsystemも直接更新する
    ToggleDebugEnabled();
}

/** グリッド描画行クリック時の処理を行う */
void URldDebugOptionsWidget::OnGridDebugDrawRowClicked()
{
    // 行クリック時はGridカテゴリの状態を反転する
    ToggleDebugCategory(CmnDebugCategories::Grid);
}

/** フロア描画行クリック時の処理を行う */
void URldDebugOptionsWidget::OnFloorDebugDrawRowClicked()
{
    // 行クリック時はFloorカテゴリの状態を反転する
    ToggleDebugCategory(CmnDebugCategories::Floor);
}

/** AI描画行クリック時の処理を行う */
void URldDebugOptionsWidget::OnAIDebugDrawRowClicked()
{
    // 行クリック時はAIカテゴリの状態を反転する
    ToggleDebugCategory(CmnDebugCategories::AI);
}

/** 当たり判定描画行クリック時の処理を行う */
void URldDebugOptionsWidget::OnCollisionDebugDrawRowClicked()
{
    // 行クリック時はCollisionカテゴリの状態を反転する
    ToggleDebugCategory(CmnDebugCategories::Collision);
}

/** デバッグ描画全体チェック変更時の処理を行う */
void URldDebugOptionsWidget::OnDebugEnabledChanged(bool bIsChecked)
{
    // UI反映処理によるチェック変更ではSubsystemを更新しない
    if (bRefreshingUI)
    {
        return;
    }

    UCmnDebugWorldSubsystem* debugSubsystem = GetDebugWorldSubsystem();

    // DebugWorldSubsystem未取得時は更新しない
    if (!debugSubsystem)
    {
        UE_LOG(LogRldDebugOptions, Warning, TEXT("OnDebugEnabledChanged: DebugWorldSubsystem未取得のため更新しません"));
        return;
    }

    // デバッグ描画全体の有効状態を更新
    debugSubsystem->SetDebugEnabled(bIsChecked);

    UE_LOG(
        LogRldDebugOptions,
        Log,
        TEXT("OnDebugEnabledChanged: デバッグ描画全体=%s"),
        bIsChecked ? TEXT("有効") : TEXT("無効")
    );

    // 全体ON/OFF変更後はカテゴリ項目の操作可否を更新
    UpdateCategoryCheckBoxEnabledState();
}

/** グリッド描画チェック変更時の処理を行う */
void URldDebugOptionsWidget::OnGridDebugDrawChanged(bool bIsChecked)
{
    // UI反映処理によるチェック変更ではSubsystemを更新しない
    if (bRefreshingUI)
    {
        return;
    }

    SetDebugCategoryEnabled(CmnDebugCategories::Grid, bIsChecked, TEXT("OnGridDebugDrawChanged"));
}

/** フロア描画チェック変更時の処理を行う */
void URldDebugOptionsWidget::OnFloorDebugDrawChanged(bool bIsChecked)
{
    // UI反映処理によるチェック変更ではSubsystemを更新しない
    if (bRefreshingUI)
    {
        return;
    }

    SetDebugCategoryEnabled(CmnDebugCategories::Floor, bIsChecked, TEXT("OnFloorDebugDrawChanged"));
}

/** AI描画チェック変更時の処理を行う */
void URldDebugOptionsWidget::OnAIDebugDrawChanged(bool bIsChecked)
{
    // UI反映処理によるチェック変更ではSubsystemを更新しない
    if (bRefreshingUI)
    {
        return;
    }

    SetDebugCategoryEnabled(CmnDebugCategories::AI, bIsChecked, TEXT("OnAIDebugDrawChanged"));
}

/** 当たり判定描画チェック変更時の処理を行う */
void URldDebugOptionsWidget::OnCollisionDebugDrawChanged(bool bIsChecked)
{
    // UI反映処理によるチェック変更ではSubsystemを更新しない
    if (bRefreshingUI)
    {
        return;
    }

    SetDebugCategoryEnabled(CmnDebugCategories::Collision, bIsChecked, TEXT("OnCollisionDebugDrawChanged"));
}

/** 閉じるボタンクリック時の処理を行う */
void URldDebugOptionsWidget::OnCloseClicked()
{
    CloseDebugOptions();
}

/** DebugWorldSubsystemを取得する */
UCmnDebugWorldSubsystem* URldDebugOptionsWidget::GetDebugWorldSubsystem() const
{
    UWorld* world = GetWorld();

    // World未取得時は取得しない
    if (!world)
    {
        return nullptr;
    }

    return world->GetSubsystem<UCmnDebugWorldSubsystem>();
}

/** 現在のデバッグ設定をUIへ反映する */
void URldDebugOptionsWidget::RefreshUIFromDebugSettings()
{
    UCmnDebugWorldSubsystem* debugSubsystem = GetDebugWorldSubsystem();

    // DebugWorldSubsystem未取得時は反映しない
    if (!debugSubsystem)
    {
        UE_LOG(LogRldDebugOptions, Warning, TEXT("RefreshUIFromDebugSettings: DebugWorldSubsystem未取得のため反映しません"));
        return;
    }

    // SetIsCheckedでOnCheckStateChangedが呼ばれてもSubsystemへ再反映しない
    bRefreshingUI = true;

    // デバッグ描画全体のチェック状態を反映
    if (CheckBoxDebugEnabled)
    {
        CheckBoxDebugEnabled->SetIsChecked(debugSubsystem->IsDebugEnabled());
    }

    // 各カテゴリのチェック状態を反映
    RefreshCategoryCheckBox(CheckBoxGridDebugDraw, CmnDebugCategories::Grid);
    RefreshCategoryCheckBox(CheckBoxFloorDebugDraw, CmnDebugCategories::Floor);
    RefreshCategoryCheckBox(CheckBoxAIDebugDraw, CmnDebugCategories::AI);
    RefreshCategoryCheckBox(CheckBoxCollisionDebugDraw, CmnDebugCategories::Collision);

    // UI反映完了後は通常のチェック変更処理を許可
    bRefreshingUI = false;
}

/** カテゴリチェックボックスの有効状態を更新する */
void URldDebugOptionsWidget::UpdateCategoryCheckBoxEnabledState()
{
    const UCmnDebugWorldSubsystem* debugSubsystem = GetDebugWorldSubsystem();

    // DebugWorldSubsystem未取得時は無効扱い
    const bool bDebugEnabled = debugSubsystem && debugSubsystem->IsDebugEnabled();

    // デバッグ描画全体が無効な場合は個別カテゴリ操作不可
    if (CheckBoxGridDebugDraw)
    {
        CheckBoxGridDebugDraw->SetIsEnabled(bDebugEnabled);
    }

    if (CheckBoxFloorDebugDraw)
    {
        CheckBoxFloorDebugDraw->SetIsEnabled(bDebugEnabled);
    }

    if (CheckBoxAIDebugDraw)
    {
        CheckBoxAIDebugDraw->SetIsEnabled(bDebugEnabled);
    }

    if (CheckBoxCollisionDebugDraw)
    {
        CheckBoxCollisionDebugDraw->SetIsEnabled(bDebugEnabled);
    }

    // 行クリックでもカテゴリ状態が変わらないよう行ボタンも同様に制御
    if (ButtonGridDebugDrawRow)
    {
        ButtonGridDebugDrawRow->SetIsEnabled(bDebugEnabled);
    }

    if (ButtonFloorDebugDrawRow)
    {
        ButtonFloorDebugDrawRow->SetIsEnabled(bDebugEnabled);
    }

    if (ButtonAIDebugDrawRow)
    {
        ButtonAIDebugDrawRow->SetIsEnabled(bDebugEnabled);
    }

    if (ButtonCollisionDebugDrawRow)
    {
        ButtonCollisionDebugDrawRow->SetIsEnabled(bDebugEnabled);
    }
}

/** カテゴリチェック状態をUIへ反映する */
void URldDebugOptionsWidget::RefreshCategoryCheckBox(UCheckBox* checkBox, FName categoryName) const
{
    // チェックボックス未取得時は処理しない
    if (!checkBox)
    {
        return;
    }

    const UCmnDebugWorldSubsystem* debugSubsystem = GetDebugWorldSubsystem();

    // DebugWorldSubsystem未取得時は反映しない
    if (!debugSubsystem)
    {
        return;
    }

    // Subsystem側のカテゴリ状態をチェックボックスへ反映
    checkBox->SetIsChecked(debugSubsystem->IsCategoryEnabled(categoryName));
}

/** デバッグ描画全体の有効状態を反転する */
void URldDebugOptionsWidget::ToggleDebugEnabled()
{
    UCmnDebugWorldSubsystem* debugSubsystem = GetDebugWorldSubsystem();

    // DebugWorldSubsystem未取得時は更新しない
    if (!debugSubsystem)
    {
        UE_LOG(LogRldDebugOptions, Warning, TEXT("ToggleDebugEnabled: DebugWorldSubsystem未取得のため更新しません"));
        return;
    }

    // Subsystemの現在値を基準に反転後の状態を決定
    const bool bNewEnabled = !debugSubsystem->IsDebugEnabled();

    // Subsystemを更新してからUIを同期する
    debugSubsystem->SetDebugEnabled(bNewEnabled);

    // チェック状態をSubsystemの最新状態へ揃える
    RefreshUIFromDebugSettings();

    // 全体ON/OFF変更後はカテゴリ項目の操作可否を更新
    UpdateCategoryCheckBoxEnabledState();

    UE_LOG(
        LogRldDebugOptions,
        Log,
        TEXT("ToggleDebugEnabled: デバッグ描画全体=%s"),
        bNewEnabled ? TEXT("有効") : TEXT("無効")
    );
}

/** デバッグカテゴリの有効状態を反転する */
void URldDebugOptionsWidget::ToggleDebugCategory(FName categoryName)
{
    UCmnDebugWorldSubsystem* debugSubsystem = GetDebugWorldSubsystem();

    // DebugWorldSubsystem未取得時は更新しない
    if (!debugSubsystem)
    {
        UE_LOG(LogRldDebugOptions, Warning, TEXT("ToggleDebugCategory: DebugWorldSubsystem未取得のため更新しません"));
        return;
    }

    // デバッグ描画全体が無効な場合はカテゴリを変更しない
    if (!debugSubsystem->IsDebugEnabled())
    {
        return;
    }

    // Subsystemの現在値を基準に反転後のカテゴリ状態を決定
    const bool bNewEnabled = !debugSubsystem->IsCategoryEnabled(categoryName);

    // Subsystemを更新してからUIを同期する
    debugSubsystem->SetCategoryEnabled(categoryName, bNewEnabled);

    // チェック状態をSubsystemの最新状態へ揃える
    RefreshUIFromDebugSettings();

    UE_LOG(
        LogRldDebugOptions,
        Log,
        TEXT("ToggleDebugCategory: カテゴリ=%s 状態=%s"),
        *categoryName.ToString(),
        bNewEnabled ? TEXT("有効") : TEXT("無効")
    );
}

/** デバッグカテゴリの有効状態を更新する */
void URldDebugOptionsWidget::SetDebugCategoryEnabled(
    FName categoryName,
    bool bIsChecked,
    const TCHAR* sourceFunctionName
)
{
    UCmnDebugWorldSubsystem* debugSubsystem = GetDebugWorldSubsystem();

    // DebugWorldSubsystem未取得時は更新しない
    if (!debugSubsystem)
    {
        UE_LOG(
            LogRldDebugOptions,
            Warning,
            TEXT("%s: DebugWorldSubsystem未取得のため更新しません"),
            sourceFunctionName
        );
        return;
    }

    // デバッグ描画全体が無効な場合はカテゴリを変更しない
    if (!debugSubsystem->IsDebugEnabled())
    {
        RefreshUIFromDebugSettings();
        return;
    }

    // チェックボックスの状態をカテゴリ状態として反映
    debugSubsystem->SetCategoryEnabled(categoryName, bIsChecked);

    UE_LOG(
        LogRldDebugOptions,
        Log,
        TEXT("%s: カテゴリ=%s 状態=%s"),
        sourceFunctionName,
        *categoryName.ToString(),
        bIsChecked ? TEXT("有効") : TEXT("無効")
    );
}

/** フォーカス対象行ボタン一覧を取得する */
TArray<UButton*> URldDebugOptionsWidget::GetFocusableRowButtons() const
{
    TArray<UButton*> rowButtons;

    if (ButtonDebugEnabledRow)
    {
        rowButtons.Add(ButtonDebugEnabledRow);
    }

    if (ButtonGridDebugDrawRow)
    {
        rowButtons.Add(ButtonGridDebugDrawRow);
    }

    if (ButtonFloorDebugDrawRow)
    {
        rowButtons.Add(ButtonFloorDebugDrawRow);
    }

    if (ButtonAIDebugDrawRow)
    {
        rowButtons.Add(ButtonAIDebugDrawRow);
    }

    if (ButtonCollisionDebugDrawRow)
    {
        rowButtons.Add(ButtonCollisionDebugDrawRow);
    }

    if (ButtonClose)
    {
        rowButtons.Add(ButtonClose);
    }

    return rowButtons;
}

/** 初期フォーカスを設定する */
void URldDebugOptionsWidget::SetInitialDebugFocus()
{
    focusedRowIndex = 0;

    ApplyCurrentDebugFocus();
}

/** 現在のフォーカス行へキーボードフォーカスを適用する */
void URldDebugOptionsWidget::ApplyCurrentDebugFocus()
{
    const TArray<UButton*> rowButtons = GetFocusableRowButtons();

    // フォーカス対象がない場合は適用しない
    if (rowButtons.Num() == 0)
    {
        UE_LOG(LogRldDebugOptions, Warning, TEXT("ApplyCurrentDebugFocus: フォーカス対象が存在しないため適用しません"));
        return;
    }

    focusedRowIndex = FMath::Clamp(focusedRowIndex, 0, rowButtons.Num() - 1);

    if (rowButtons[focusedRowIndex])
    {
        rowButtons[focusedRowIndex]->SetKeyboardFocus();
    }
}
