#include "displayapp/screens/LightsOut.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void EventHandler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<LightsOut*>(obj->user_data);
    screen->UpdateSelected(obj, event);
  }
}

LightsOut::LightsOut(Components::LittleVgl& lvgl) : lvgl {lvgl} {
  std::srand(xTaskGetTickCount());

  nRows = 5;
  nCols = 5;
  solnViewMode = false;
  state = State::Playing;

  lightDisplay = lv_table_create(lv_scr_act(), nullptr);
  lightDisplay->user_data = this;
  lv_obj_set_event_cb(lightDisplay, EventHandler);
  // Table sizing and positioning is handled in RestyleTable() to account for changing table sizes

  // Yellow for lit buttons
  lv_style_init(&styleActive);
  lv_style_set_bg_color(&styleActive, LV_STATE_DEFAULT, LV_COLOR_MAKE(0xFF, 0xFF, 0x00));
  lv_style_set_bg_opa(&styleActive, LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_style_set_border_width(&styleActive, LV_STATE_DEFAULT, 1);
  lv_style_set_border_color(&styleActive, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_obj_add_style(lightDisplay, LV_TABLE_PART_CELL1, &styleActive);

  // Brown for unlit buttons
  lv_style_init(&styleInactive);
  lv_style_set_bg_color(&styleInactive, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x40, 0x30, 0x00));
  lv_style_set_bg_opa(&styleInactive, LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_style_set_border_width(&styleInactive, LV_STATE_DEFAULT, 1);
  lv_style_set_border_color(&styleInactive, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_obj_add_style(lightDisplay, LV_TABLE_PART_CELL2, &styleInactive);

  // Blue for lit buttons that need to be pressed during solution mode
  lv_style_init(&styleSolnActive);
  lv_style_set_bg_color(&styleSolnActive, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x00, 0x80, 0xFF));
  lv_style_set_bg_opa(&styleSolnActive, LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_style_set_border_width(&styleSolnActive, LV_STATE_DEFAULT, 1);
  lv_style_set_border_color(&styleSolnActive, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_obj_add_style(lightDisplay, LV_TABLE_PART_CELL3, &styleSolnActive);

  // Dark blue for unlit buttons that need to be pressed during solution mode
  lv_style_init(&styleSolnInactive);
  lv_style_set_bg_color(&styleSolnInactive, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x00, 0x20, 0x40));
  lv_style_set_bg_opa(&styleSolnInactive, LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_style_set_border_width(&styleSolnInactive, LV_STATE_DEFAULT, 1);
  lv_style_set_border_color(&styleSolnInactive, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_obj_add_style(lightDisplay, LV_TABLE_PART_CELL4, &styleSolnInactive);

  // Close menu button
  btnCloseMenu = lv_btn_create(lv_scr_act(), nullptr);
  btnCloseMenu->user_data = this;
  lv_obj_set_size(btnCloseMenu, 76, 60);
  lv_obj_align(btnCloseMenu, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 0, 0);
  lv_obj_set_style_local_bg_opa(btnCloseMenu, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_70);
  lv_obj_t* lblClose = lv_label_create(btnCloseMenu, nullptr);
  lv_label_set_text_static(lblClose, "X");
  lv_obj_set_event_cb(btnCloseMenu, EventHandler);
  lv_obj_set_hidden(btnCloseMenu, true);

  // Toggle solution mode button
  btnToggleSolution = lv_btn_create(lv_scr_act(), nullptr);
  btnToggleSolution->user_data = this;
  lv_obj_set_size(btnToggleSolution, 76, 60);
  lv_obj_align(btnToggleSolution, lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, 0, 0);
  lv_obj_set_style_local_bg_opa(btnToggleSolution, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_70);
  lv_obj_t* lblSolution = lv_label_create(btnToggleSolution, nullptr);
  lv_label_set_text_static(lblSolution, "?");
  lv_obj_set_event_cb(btnToggleSolution, EventHandler);
  lv_obj_set_hidden(btnToggleSolution, true);

  // Button to decrease board size
  btnSizeDecrease = lv_btn_create(lv_scr_act(), nullptr);
  btnSizeDecrease->user_data = this;
  lv_obj_set_size(btnSizeDecrease, 70, 60);
  lv_obj_align(btnSizeDecrease, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
  lv_obj_set_style_local_bg_opa(btnSizeDecrease, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_70);
  lv_obj_set_style_local_bg_opa(btnSizeDecrease, LV_BTN_PART_MAIN, LV_STATE_DISABLED, LV_OPA_20);
  lv_obj_t* lblDecrease = lv_label_create(btnSizeDecrease, nullptr);
  lv_label_set_text_static(lblDecrease, "-");
  lv_obj_set_event_cb(btnSizeDecrease, EventHandler);
  lv_obj_set_hidden(btnSizeDecrease, true);

  // Button to increase board size
  btnSizeIncrease = lv_btn_create(lv_scr_act(), nullptr);
  btnSizeIncrease->user_data = this;
  lv_obj_set_size(btnSizeIncrease, 70, 60);
  lv_obj_align(btnSizeIncrease, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
  lv_obj_set_style_local_bg_opa(btnSizeIncrease, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_70);
  lv_obj_set_style_local_bg_opa(btnSizeIncrease, LV_BTN_PART_MAIN, LV_STATE_DISABLED, LV_OPA_20);
  lv_obj_t* lblIncrease = lv_label_create(btnSizeIncrease, nullptr);
  lv_label_set_text_static(lblIncrease, "+");
  lv_obj_set_event_cb(btnSizeIncrease, EventHandler);
  lv_obj_set_hidden(btnSizeIncrease, true);

  // Button which shows current board size (button since it can be long clicked to regenerate board)
  btnBoardSize = lv_btn_create(lv_scr_act(), nullptr);
  btnBoardSize->user_data = this;
  lv_obj_set_size(btnBoardSize, 86, 60);
  lv_obj_align(btnBoardSize, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, 0);
  lv_obj_set_style_local_bg_opa(btnBoardSize, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_90);
  lv_obj_set_style_local_bg_color(btnBoardSize, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
  lblBoardSize = lv_label_create(btnBoardSize, nullptr);
  lv_label_set_text_fmt(lblBoardSize, "%i x %i", nCols, nRows);
  lv_obj_set_event_cb(btnBoardSize, EventHandler);
  lv_obj_set_hidden(btnBoardSize, true);

  // Background for the win screen
  winScreenBG = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_size(winScreenBG, LV_HOR_RES * 8 / 10, LV_VER_RES * 8 / 10);
  lv_obj_align(winScreenBG, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_local_bg_opa(winScreenBG, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_80);
  lv_obj_set_style_local_bg_color(winScreenBG, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
  lblWinScreenText = lv_label_create(winScreenBG, nullptr);
  lv_obj_set_state(winScreenBG, LV_STATE_DISABLED);
  lv_label_set_align(lblWinScreenText, LV_LABEL_ALIGN_CENTER);
  lv_obj_set_hidden(winScreenBG, true);

  GenerateGame();
  RestyleTable();
  RelightTable();
}

LightsOut::~LightsOut() {
  lv_obj_clean(lv_scr_act());
  lv_style_reset(&styleActive);
  lv_style_reset(&styleInactive);
  lv_style_reset(&styleSolnActive);
  lv_style_reset(&styleSolnInactive);
}

bool LightsOut::OnButtonPushed() {
  if (state == State::InMenu) {
    HideMenu();
    return true;
  }
  return false;
}

void LightsOut::UpdateSelected(const lv_obj_t* object, lv_event_t event) {
  // Handle main table click
  if (object == lightDisplay) {
    LightDisplayEvent(event);
  }
  // Close menu
  if (object == btnCloseMenu && event == LV_EVENT_CLICKED) {
    HideMenu();
    return;
  }
  // Toggle solution view mode
  if (object == btnToggleSolution && event == LV_EVENT_CLICKED) {
    solnViewMode = !solnViewMode;
    RelightTable();
    return;
  }
  // Update size increase button
  if (object == btnSizeIncrease) {
    BtnSizeIncEvent(event);
  }
  // Update size decrease button
  if (object == btnSizeDecrease) {
    BtnSizeDecEvent(event);
  }
  // Only update board state
  if (object == btnBoardSize && event == LV_EVENT_LONG_PRESSED) {
    GenerateGame();
    RestyleTable();
    RelightTable();
    lv_label_set_text_fmt(lblBoardSize, "%i x %i", nCols, nRows);
  }
}

// Randomize which buttons are pressed or not, and prevent a too boring game
void LightsOut::GenerateGame() {
  pressedArr = std::vector<std::vector<bool>>(nCols);
  for (int col = 0; col < nCols; col++) {
    pressedArr[col] = std::vector<bool>(nRows);
  }
  // Require a game to have at least 1/5 pressed buttons and at least one lit tile
  int totalPressed = 0;
  bool anyLit = false;
  while (totalPressed < nRows * nCols / 5 || !anyLit) {
    totalPressed = 0;
    for (int row = 0; row < nRows; row++) {
      for (int col = 0; col < nCols; col++) {
        pressedArr[col][row] = std::rand() & 1;
        totalPressed += pressedArr[col][row];
      }
    }
    anyLit = false;
    for (int row = 0; row < nRows && !anyLit; row++) {
      for (int col = 0; col < nCols && !anyLit; col++) {
        anyLit = IsLit(row, col);
      }
    }
  }
  usedPresses = 0;
}

// Set all styles on the table relating to size
void LightsOut::RestyleTable() {
  lv_table_set_row_cnt(lightDisplay, nRows);
  lv_table_set_col_cnt(lightDisplay, nCols);
  for (int col = 0; col < nCols; col++) {
    // This approach for width fills out the screen width more correctly than simply LV_HOR_RES / nCols.
    lv_table_set_col_width(lightDisplay, col, (LV_HOR_RES * col / nCols) - (LV_HOR_RES * (col - 1) / nCols));
  }
  // Modified from https://docs.lvgl.io/7.11/widgets/table.html#simple-table
  lv_obj_set_height(lightDisplay, LV_VER_RES); // Needed else part of the table may be cut off
  const lv_table_ext_t* ext = (lv_table_ext_t*) lv_obj_get_ext_attr(lightDisplay);
  for (int row = 0; row < nRows; row++) {
    // This approach for height fills out the screen height more correctly than simply LV_VER_RES / nRows.
    ext->row_h[row] = (LV_VER_RES * row / nRows) - (LV_VER_RES * (row - 1) / nRows);
  }
  lv_obj_align(lightDisplay, nullptr, LV_ALIGN_IN_TOP_MID, 0, 0);
}

// Reevaluate all the lights on the table
void LightsOut::RelightTable() {
  for (int row = 0; row < nRows; row++) {
    for (int col = 0; col < nCols; col++) {
      if (solnViewMode && pressedArr[col][row])
        lv_table_set_cell_type(lightDisplay, row, col, IsLit(row, col) ? 3 : 4);
      else
        lv_table_set_cell_type(lightDisplay, row, col, IsLit(row, col) ? 1 : 2);
    }
  }
}

// Evaluate if an index is lit or not
bool LightsOut::IsLit(int row, int col) {
  if (row < 0 || row >= nRows || col < 0 || col >= nCols) {
    return false;
  }
  bool isLit = pressedArr[col][row];
  if (row > 0)
    isLit = isLit != pressedArr[col][row - 1];
  if (row < nRows - 1)
    isLit = isLit != pressedArr[col][row + 1];
  if (col > 0)
    isLit = isLit != pressedArr[col - 1][row];
  if (col < nCols - 1)
    isLit = isLit != pressedArr[col + 1][row];
  return isLit;
}

// Click to toggle lights, long click to open menu, and click on win/loss screen to continue to game.
void LightsOut::LightDisplayEvent(lv_event_t event) {
  if (event == LV_EVENT_SHORT_CLICKED && state == State::Playing) {
    // Toggle lights
    lv_indev_data_t tapData;
    lvgl.GetTouchPadInfo(&tapData);
    const uint16_t tappedCol = tapData.point.x * nCols / LV_HOR_RES;
    const uint16_t tappedRow = tapData.point.y * nRows / LV_VER_RES;
    pressedArr[tappedCol][tappedRow] = !pressedArr[tappedCol][tappedRow];
    usedPresses++;
    RelightTable();
    int numLit = 0;
    for (int row = 0; row < nRows; row++) {
      for (int col = 0; col < nCols; col++) {
        numLit += (int) IsLit(row, col);
      }
    }
    if (numLit == 0) {
      ShowWin();
    } else if (numLit == nRows * nCols) {
      ShowLoss();
    }
  } else if (event == LV_EVENT_LONG_PRESSED && state == State::Playing) {
    ShowMenu();
  } else if (event == LV_EVENT_SHORT_CLICKED && state == State::Won) {
    solnViewMode = false;
    lv_obj_set_hidden(winScreenBG, true);
    GenerateGame();
    RelightTable();
    state = State::Playing;
  } else if (event == LV_EVENT_SHORT_CLICKED && state == State::Lost) {
    lv_obj_set_hidden(winScreenBG, true);
    state = State::Playing;
  }
}

// Increase size up to a maximum of 9. Long click to only increase column count to a maximum of min(9, nRows+3).
void LightsOut::BtnSizeIncEvent(lv_event_t event) {
  if (event == LV_EVENT_SHORT_CLICKED) {
    nCols++;
    nRows++;
  } else if (event == LV_EVENT_LONG_PRESSED) {
    if (nCols >= nRows + 3)
      return;
    nCols++;
  } else {
    return;
  }
  lv_btn_set_state(btnSizeDecrease, LV_BTN_STATE_RELEASED);
  if (std::max(nRows, nCols) >= 9) {
    lv_btn_set_state(btnSizeIncrease, LV_BTN_STATE_DISABLED);
  }
  GenerateGame();
  RestyleTable();
  RelightTable();
  lv_label_set_text_fmt(lblBoardSize, "%i x %i", nCols, nRows);
}

// Decrease size down to a minimum of 3. Long click to only decrease column count to a minimum of max(3, nRows-3).
void LightsOut::BtnSizeDecEvent(lv_event_t event) {
  if (event == LV_EVENT_SHORT_CLICKED) {
    nCols--;
    nRows--;
  } else if (event == LV_EVENT_LONG_PRESSED) {
    if (nCols <= nRows - 3)
      return;
    nCols--;
  } else {
    return;
  }
  lv_btn_set_state(btnSizeIncrease, LV_BTN_STATE_RELEASED);
  if (std::min(nRows, nCols) <= 3) {
    lv_btn_set_state(btnSizeDecrease, LV_BTN_STATE_DISABLED);
  }
  GenerateGame();
  RestyleTable();
  RelightTable();
  lv_label_set_text_fmt(lblBoardSize, "%i x %i", nCols, nRows);
}

void LightsOut::ShowWin() {
  lv_label_set_text_fmt(lblWinScreenText, "You win!\n\nMoves: %i", usedPresses);
  lv_obj_align(lblWinScreenText, nullptr, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_hidden(winScreenBG, false);
  state = State::Won;
}

void LightsOut::ShowLoss() {
  lv_label_set_text_fmt(lblWinScreenText, "Good job!\n\nNow try turning\nthe lights off\ninstead!", usedPresses);
  lv_obj_align(lblWinScreenText, nullptr, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_hidden(winScreenBG, false);
  state = State::Lost;
}

void LightsOut::ShowMenu() {
  lv_obj_set_hidden(btnSizeIncrease, false);
  lv_obj_set_hidden(btnSizeDecrease, false);
  lv_obj_set_hidden(btnCloseMenu, false);
  lv_obj_set_hidden(btnToggleSolution, false);
  lv_obj_set_hidden(btnBoardSize, false);
  state = State::InMenu;
}

void LightsOut::HideMenu() {
  lv_obj_set_hidden(btnSizeIncrease, true);
  lv_obj_set_hidden(btnSizeDecrease, true);
  lv_obj_set_hidden(btnCloseMenu, true);
  lv_obj_set_hidden(btnToggleSolution, true);
  lv_obj_set_hidden(btnBoardSize, true);
  state = State::Playing;
}
