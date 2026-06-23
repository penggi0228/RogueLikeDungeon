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
        UE_LOG(
            LogRldDebugOptions,
            Warning,
            TEXT("NativeOnInitialized: Widget=%s ButtonDebugEnabledRowが未設定です"),
            *GetNameSafe(this)
        );
    }

    // グリッド描画行ボタン取得時はイベント登録
    if (ButtonGridDebugDrawRow)
    {
        ButtonGridDebugDrawRow->OnClicked.AddDynamic(this, &URldDebugOptionsWidget::OnGridDebugDrawRowClicked);
    }
    else
    {
        UE_LOG(
            LogRldDebugOptions,
            Warning,
            TEXT("NativeOnInitialized: Widget=%s ButtonGridDebugDrawRowが未設定です"),
            *GetNameSafe(this)
        );
    }

    // フロア描画行ボタン取得時はイベント登録
    if (ButtonFloorDebugDrawRow)
    {
        ButtonFloorDebugDrawRow->OnClicked.AddDynamic(this, &URldDebugOptionsWidget::OnFloorDebugDrawRowClicked);
    }
    else
    {
        UE_LOG(
            LogRldDebugOptions,
            Warning,
            TEXT("NativeOnInitialized: Widget=%s ButtonFloorDebugDrawRowが未設定です"),
            *GetNameSafe(this)
        );
    }

    // AI描画行ボタン取得時はイベント登録
    if (ButtonAIDebugDrawRow)
    {
        ButtonAIDebugDrawRow->OnClicked.AddDynamic(this, &URldDebugOptionsWidget::OnAIDebugDrawRowClicked);
    }
    else
    {
        UE_LOG(
            LogRldDebugOptions,
            Warning,
            TEXT("NativeOnInitialized: Widget=%s ButtonAIDebugDrawRowが未設定です"),
            *GetNameSafe(this)
        );
    }

    // 当たり判定描画行ボタン取得時はイベント登録
    if (ButtonCollisionDebugDrawRow)
    {
        ButtonCollisionDebugDrawRow->OnClicked.AddDynamic(this, &URldDebugOptionsWidget::OnCollisionDebugDrawRowClicked);
    }
    else
    {
        UE_LOG(
            LogRldDebugOptions,
            Warning,
            TEXT("NativeOnInitialized: Widget=%s ButtonCollisionDebugDrawRowが未設定です"),
            *GetNameSafe(this)
        );
    }

    // ----- チェックボックス -----

    // デバッグ描画全体チェックボックス取得時はイベント登録
    if (CheckBoxDebugEnabled)
    {
        CheckBoxDebugEnabled->OnCheckStateChanged.AddDynamic(this, &URldDebugOptionsWidget::OnDebugEnabledChanged);
    }
    else
    {
        UE_LOG(
            LogRldDebugOptions,
            Warning,
            TEXT("NativeOnInitialized: Widget=%s CheckBoxDebugEnabledが未設定です"),
            *GetNameSafe(this)
        );
    }

    // グリッド描画チェックボックス取得時はイベント登録
    if (CheckBoxGridDebugDraw)
    {
        CheckBoxGridDebugDraw->OnCheckStateChanged.AddDynamic(this, &URldDebugOptionsWidget::OnGridDebugDrawChanged);
    }
    else
    {
        UE_LOG(
            LogRldDebugOptions,
            Warning,
            TEXT("NativeOnInitialized: Widget=%s CheckBoxGridDebugDrawが未設定です"),
            *GetNameSafe(this)
        );
    }

    // フロア描画チェックボックス取得時はイベント登録
    if (CheckBoxFloorDebugDraw)
    {
        CheckBoxFloorDebugDraw->OnCheckStateChanged.AddDynamic(this, &URldDebugOptionsWidget::OnFloorDebugDrawChanged);
    }
    else
    {
        UE_LOG(
            LogRldDebugOptions,
            Warning,
            TEXT("NativeOnInitialized: Widget=%s CheckBoxFloorDebugDrawが未設定です"),
            *GetNameSafe(this)
        );
    }

    // AI描画チェックボックス取得時はイベント登録
    if (CheckBoxAIDebugDraw)
    {
        CheckBoxAIDebugDraw->OnCheckStateChanged.AddDynamic(this, &URldDebugOptionsWidget::OnAIDebugDrawChanged);
    }
    else
    {
        UE_LOG(
            LogRldDebugOptions,
            Warning,
            TEXT("NativeOnInitialized: Widget=%s CheckBoxAIDebugDrawが未設定です"),
            *GetNameSafe(this)
        );
    }

    // 当たり判定描画チェックボックス取得時はイベント登録
    if (CheckBoxCollisionDebugDraw)
    {
        CheckBoxCollisionDebugDraw->OnCheckStateChanged.AddDynamic(this, &URldDebugOptionsWidget::OnCollisionDebugDrawChanged);
    }
    else
    {
        UE_LOG(
            LogRldDebugOptions,
            Warning,
            TEXT("NativeOnInitialized: Widget=%s CheckBoxCollisionDebugDrawが未設定です"),
            *GetNameSafe(this)
        );
    }

    // ----- 操作ボタン -----

    // 閉じるボタン取得時はイベント登録
    if (ButtonClose)
    {
        ButtonClose->OnClicked.AddDynamic(this, &URldDebugOptionsWidget::OnCloseClicked);
    }
    else
    {
        UE_LOG(
            LogRldDebugOptions,
            Warning,
            TEXT("NativeOnInitialized: Widget=%s ButtonCloseが未設定です"),
            *GetNameSafe(this)
        );
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
        UE_LOG(
            LogRldDebugOptions,
            Warning,
            TEXT("MoveDebugFocus: Widget=%s フォーカス対象が存在しないため移動しません"),
            *GetNameSafe(this)
        );

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
        UE_LOG(
            LogRldDebugOptions,
            Warning,
            TEXT("ConfirmFocusedDebugItem: Widget=%s フォーカス対象が存在しないため決定できません"),
            *GetNameSafe(this)
        );

        return;
    }

    focusedRowIndex = FMath::Clamp(focusedRowIndex, 0, rowButtons.Num() - 1);

    UButton* focusedButton = rowButtons[focusedRowIndex];

    // デバッグ描画全体行の場合は全体状態を反転する
    if (focusedButton == ButtonDebugEnabledRow)
    {
        OnDebugEnabledRowClicked();
    }
    // グリッド行の場合はGridカテゴリを反転する
    else if (focusedButton == ButtonGridDebugDrawRow)
    {
        OnGridDebugDrawRowClicked();
    }
    // フロア行の場合はFloorカテゴリを反転する
    else if (focusedButton == ButtonFloorDebugDrawRow)
    {
        OnFloorDebugDrawRowClicked();
    }
    // AI行の場合はAIカテゴリを反転する
    else if (focusedButton == ButtonAIDebugDrawRow)
    {
        OnAIDebugDrawRowClicked();
    }
    // 当たり判定行の場合はCollisionカテゴリを反転する
    else if (focusedButton == ButtonCollisionDebugDrawRow)
    {
        OnCollisionDebugDrawRowClicked();
    }
    // 閉じるボタンの場合はデバッグオプションを閉じる
    else if (focusedButton == ButtonClose)
    {
        CloseDebugOptions();
    }
    else
    {
        UE_LOG(
            LogRldDebugOptions,
            Warning,
            TEXT("ConfirmFocusedDebugItem: Widget=%s 対応する処理が見つかりません"),
            *GetNameSafe(this)
        );
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
        UE_LOG(
            LogRldDebugOptions,
            Warning,
            TEXT("CloseDebugOptions: Widget=%s OwningPlayerからRldPlayerControllerを取得できません"),
            *GetNameSafe(this)
        );
    }

    UE_LOG(
        LogRldDebugOptions,
        Log,
        TEXT("CloseDebugOptions: Widget=%s デバッグオプションを閉じました"),
        *GetNameSafe(this)
    );
}

/** デバッグ描画全体行クリック時の処理を行う */
void URldDebugOptionsWidget::OnDebugEnabledRowClicked()
{
    // 行クリック時はチェックボックスだけでなくSubsystemも直接更新する
    ToggleDebugEnabled();
}

/** グリッドデバッグ描画行クリック時の処理を行う */
void URldDebugOptionsWidget::OnGridDebugDrawRowClicked()
{
    // 行クリック時はGridカテゴリの状態を反転する
    ToggleDebugCategory(CmnDebugCategories::Grid);
}

/** フロアデバッグ描画行クリック時の処理を行う */
void URldDebugOptionsWidget::OnFloorDebugDrawRowClicked()
{
    // 行クリック時はFloorカテゴリの状態を反転する
    ToggleDebugCategory(CmnDebugCategories::Floor);
}

/** AIデバッグ描画行クリック時の処理を行う */
void URldDebugOptionsWidget::OnAIDebugDrawRowClicked()
{
    // 行クリック時はAIカテゴリの状態を反転する
    ToggleDebugCategory(CmnDebugCategories::AI);
}

/** 当たり判定デバッグ描画行クリック時の処理を行う */
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
        UE_LOG(
            LogRldDebugOptions,
            Warning,
            TEXT("OnDebugEnabledChanged: Widget=%s DebugWorldSubsystem未取得のため更新しません"),
            *GetNameSafe(this)
        );

        return;
    }

    // デバッグ描画全体の有効状態を更新
    debugSubsystem->SetDebugEnabled(bIsChecked);

    UE_LOG(
        LogRldDebugOptions,
        Log,
        TEXT("OnDebugEnabledChanged: Widget=%s デバッグ描画全体=%s"),
        *GetNameSafe(this),
        bIsChecked ? TEXT("有効") : TEXT("無効")
    );

    // 全体ON/OFF変更後はカテゴリ項目の操作可否を更新
    UpdateCategoryCheckBoxEnabledState();
}

/** グリッドデバッグ描画チェック変更時の処理を行う */
void URldDebugOptionsWidget::OnGridDebugDrawChanged(bool bIsChecked)
{
    // UI反映処理によるチェック変更ではSubsystemを更新しない
    if (bRefreshingUI)
    {
        return;
    }

    SetDebugCategoryEnabled(CmnDebugCategories::Grid, bIsChecked, TEXT("OnGridDebugDrawChanged"));
}

/** フロアデバッグ描画チェック変更時の処理を行う */
void URldDebugOptionsWidget::OnFloorDebugDrawChanged(bool bIsChecked)
{
    // UI反映処理によるチェック変更ではSubsystemを更新しない
    if (bRefreshingUI)
    {
        return;
    }

    SetDebugCategoryEnabled(CmnDebugCategories::Floor, bIsChecked, TEXT("OnFloorDebugDrawChanged"));
}

/** AIデバッグ描画チェック変更時の処理を行う */
void URldDebugOptionsWidget::OnAIDebugDrawChanged(bool bIsChecked)
{
    // UI反映処理によるチェック変更ではSubsystemを更新しない
    if (bRefreshingUI)
    {
        return;
    }

    SetDebugCategoryEnabled(CmnDebugCategories::AI, bIsChecked, TEXT("OnAIDebugDrawChanged"));
}

/** 当たり判定デバッグ描画チェック変更時の処理を行う */
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
        UE_LOG(
            LogRldDebugOptions,
            Warning,
            TEXT("RefreshUIFromDebugSettings: Widget=%s DebugWorldSubsystem未取得のため反映しません"),
            *GetNameSafe(this)
        );

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

/** デバッグ種別チェックボックスの有効状態を更新する */
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

    // デバッグ描画全体が無効な場合は個別カテゴリ操作不可
    if (CheckBoxFloorDebugDraw)
    {
        CheckBoxFloorDebugDraw->SetIsEnabled(bDebugEnabled);
    }

    // デバッグ描画全体が無効な場合は個別カテゴリ操作不可
    if (CheckBoxAIDebugDraw)
    {
        CheckBoxAIDebugDraw->SetIsEnabled(bDebugEnabled);
    }

    // デバッグ描画全体が無効な場合は個別カテゴリ操作不可
    if (CheckBoxCollisionDebugDraw)
    {
        CheckBoxCollisionDebugDraw->SetIsEnabled(bDebugEnabled);
    }

    // 行クリックでもカテゴリ状態が変わらないよう行ボタンも同様に制御
    if (ButtonGridDebugDrawRow)
    {
        ButtonGridDebugDrawRow->SetIsEnabled(bDebugEnabled);
    }

    // 行クリックでもカテゴリ状態が変わらないよう行ボタンも同様に制御
    if (ButtonFloorDebugDrawRow)
    {
        ButtonFloorDebugDrawRow->SetIsEnabled(bDebugEnabled);
    }

    // 行クリックでもカテゴリ状態が変わらないよう行ボタンも同様に制御
    if (ButtonAIDebugDrawRow)
    {
        ButtonAIDebugDrawRow->SetIsEnabled(bDebugEnabled);
    }

    // 行クリックでもカテゴリ状態が変わらないよう行ボタンも同様に制御
    if (ButtonCollisionDebugDrawRow)
    {
        ButtonCollisionDebugDrawRow->SetIsEnabled(bDebugEnabled);
    }
}

/** カテゴリチェックボックスへカテゴリ状態を反映する */
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
        UE_LOG(
            LogRldDebugOptions,
            Warning,
            TEXT("ToggleDebugEnabled: Widget=%s DebugWorldSubsystem未取得のため更新しません"),
            *GetNameSafe(this)
        );

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
        TEXT("ToggleDebugEnabled: Widget=%s デバッグ描画全体=%s"),
        *GetNameSafe(this),
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
        UE_LOG(
            LogRldDebugOptions,
            Warning,
            TEXT("ToggleDebugCategory: Widget=%s DebugWorldSubsystem未取得のため更新しません カテゴリ=%s"),
            *GetNameSafe(this),
            *categoryName.ToString()
        );

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
        TEXT("ToggleDebugCategory: Widget=%s カテゴリ=%s 状態=%s"),
        *GetNameSafe(this),
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
            TEXT("%s: Widget=%s DebugWorldSubsystem未取得のため更新しません カテゴリ=%s"),
            sourceFunctionName,
            *GetNameSafe(this),
            *categoryName.ToString()
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
        TEXT("%s: Widget=%s カテゴリ=%s 状態=%s"),
        sourceFunctionName,
        *GetNameSafe(this),
        *categoryName.ToString(),
        bIsChecked ? TEXT("有効") : TEXT("無効")
    );
}

/** フォーカス対象行ボタン一覧を取得する */
TArray<UButton*> URldDebugOptionsWidget::GetFocusableRowButtons() const
{
    TArray<UButton*> rowButtons;

    // デバッグ描画全体行がある場合はフォーカス対象へ追加
    if (ButtonDebugEnabledRow)
    {
        rowButtons.Add(ButtonDebugEnabledRow);
    }

    // グリッドデバッグ描画行がある場合はフォーカス対象へ追加
    if (ButtonGridDebugDrawRow)
    {
        rowButtons.Add(ButtonGridDebugDrawRow);
    }

    // フロアデバッグ描画行がある場合はフォーカス対象へ追加
    if (ButtonFloorDebugDrawRow)
    {
        rowButtons.Add(ButtonFloorDebugDrawRow);
    }

    // AIデバッグ描画行がある場合はフォーカス対象へ追加
    if (ButtonAIDebugDrawRow)
    {
        rowButtons.Add(ButtonAIDebugDrawRow);
    }

    // 当たり判定デバッグ描画行がある場合はフォーカス対象へ追加
    if (ButtonCollisionDebugDrawRow)
    {
        rowButtons.Add(ButtonCollisionDebugDrawRow);
    }

    // 閉じるボタンがある場合はフォーカス対象へ追加
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
        UE_LOG(
            LogRldDebugOptions,
            Warning,
            TEXT("ApplyCurrentDebugFocus: Widget=%s フォーカス対象が存在しないため適用しません"),
            *GetNameSafe(this)
        );

        return;
    }

    focusedRowIndex = FMath::Clamp(focusedRowIndex, 0, rowButtons.Num() - 1);

    // フォーカス行のボタンが有効な場合はキーボードフォーカスを適用
    if (rowButtons[focusedRowIndex])
    {
        rowButtons[focusedRowIndex]->SetKeyboardFocus();
    }
}
