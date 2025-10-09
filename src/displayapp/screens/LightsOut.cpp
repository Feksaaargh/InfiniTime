#include "displayapp/screens/LightsOut.h"

using namespace Pinetime::Applications::Screens;

LightsOut::LightsOut(Components::LittleVgl& lvgl, System::SystemTask& systemTask) : lvgl {lvgl}, wakeLock {systemTask} {
  wakeLock.Lock();

  lightDisplay = lv_table_create(lv_scr_act(), nullptr);
  // Table sizing and positioning is handled in RestyleTable() to account for changing table sizes

  lv_style_init(&styleActive);
  lv_style_set_bg_color(&styleActive, LV_STATE_DEFAULT, LV_COLOR_MAKE(0xFF, 0xFF, 0x00));
  lv_style_set_bg_opa(&styleActive, LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_style_set_border_width(&styleActive, LV_STATE_DEFAULT, 1);
  lv_style_set_border_color(&styleActive, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_obj_add_style(lightDisplay, LV_TABLE_PART_CELL1, &styleActive);

  lv_style_init(&styleInactive);
  lv_style_set_bg_color(&styleInactive, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x40, 0x30, 0x00));
  lv_style_set_bg_opa(&styleInactive, LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_style_set_border_width(&styleInactive, LV_STATE_DEFAULT, 1);
  lv_style_set_border_color(&styleInactive, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_obj_add_style(lightDisplay, LV_TABLE_PART_CELL2, &styleInactive);

  lv_style_init(&styleHintActive);
  lv_style_set_bg_color(&styleHintActive, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x00, 0x80, 0xFF));
  lv_style_set_bg_opa(&styleHintActive, LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_style_set_border_width(&styleHintActive, LV_STATE_DEFAULT, 1);
  lv_style_set_border_color(&styleHintActive, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_obj_add_style(lightDisplay, LV_TABLE_PART_CELL3, &styleHintActive);

  lv_style_init(&styleHintInactive);
  lv_style_set_bg_color(&styleHintInactive, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x00, 0x20, 0x40));
  lv_style_set_bg_opa(&styleHintInactive, LV_STATE_DEFAULT, LV_OPA_COVER);
  lv_style_set_border_width(&styleHintInactive, LV_STATE_DEFAULT, 1);
  lv_style_set_border_color(&styleHintInactive, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_obj_add_style(lightDisplay, LV_TABLE_PART_CELL4, &styleHintInactive);

  nRows = 3;
  nCols = 3;
  solutionViewMode = false;
  state = State::Playing;

  GenerateGame();
  RestyleTable();
  RelightTable();
}

LightsOut::~LightsOut() {
  wakeLock.Release();
  lv_obj_clean(lv_scr_act());
  lv_style_reset(&styleActive);
  lv_style_reset(&styleInactive);
  lv_style_reset(&styleHintActive);
  lv_style_reset(&styleHintInactive);
}

bool LightsOut::OnButtonPushed() {
  if (state == State::InMenu) {
    CloseMenu();
    return true;
  }
  return false;
}

bool LightsOut::OnTouchEvent(TouchEvents event) {
  if (event == TouchEvents::Tap && state == State::Playing) {
    lv_indev_data_t tapData;
    lvgl.GetTouchPadInfo(&tapData);
    const uint16_t tappedCol = tapData.point.x * nCols / LV_HOR_RES;
    const uint16_t tappedRow = tapData.point.y * nRows / LV_VER_RES;
    pressedArr[tappedRow][tappedCol] = !pressedArr[tappedRow][tappedCol];
    usedPresses++;
    int numLit = RelightTable();
    if (numLit == 0) {
      ShowWin();
      state = State::Won;
    }
    return true;
  }
  if (event == TouchEvents::Tap && state == State::Won) {
    HideWin();
    GenerateGame();
    RelightTable();
    state = State::Playing;
    return true;
  }
  if (event == TouchEvents::LongTap && state == State::Playing) {
    OpenMenu();
    return true;
  }
  return false;
}

// Randomize which buttons are pressed or not, and prevent a too boring game
void LightsOut::GenerateGame() {
  pressedArr = std::vector<std::vector<bool>>(nCols);
  for (int col = 0; col < nCols; col++) {
    pressedArr[col] = std::vector<bool>(nRows);
  }
  int totalPressed = 0;
  for (int row = 0; row < nRows; row++) {
    for (int col = 0; col < nCols; col++) {
      pressedArr[row][col] = (bool) (std::rand() & 1);
      totalPressed += (int) pressedArr[row][col];
    }
  }
  // Prevent too few (<20%) buttons being pressed
  while (totalPressed < nRows * nCols / 5) {
    int chosenLightIdx = std::rand() % (nRows * nCols - totalPressed);
    for (int row = 0; row < nRows && chosenLightIdx >= 0; row++) {
      for (int col = 0; col < nCols && chosenLightIdx >= 0; col++) {
        if (!pressedArr[row][col]) {
          if (chosenLightIdx == 0) {
            pressedArr[row][col] = true;
            totalPressed++;
          }
          chosenLightIdx--;
        }
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
    lv_table_set_col_width(lightDisplay, col, LV_HOR_RES / nCols);
  }
  // Modified from https://docs.lvgl.io/7.11/widgets/table.html#simple-table
  lv_obj_set_height(lightDisplay, LV_VER_RES);  // Needed else part of the table may be cut off
  const lv_table_ext_t* ext = (lv_table_ext_t*)lv_obj_get_ext_attr(lightDisplay);
  for (int row = 0; row < nRows; row++) {
    ext->row_h[row] = LV_VER_RES / nRows;
  }
  lv_obj_align(lightDisplay, nullptr, LV_ALIGN_IN_TOP_MID, 0, 0);
}

// Reevaluate all the lights on the table
int LightsOut::RelightTable() {
  int nLitLights = 0;
  for (int row = 0; row < nRows; row++) {
    for (int col = 0; col < nCols; col++) {
      bool isLit = pressedArr[row][col];
      if (row > 0)
        isLit = isLit != pressedArr[row - 1][col];
      if (row < nRows - 1)
        isLit = isLit != pressedArr[row + 1][col];
      if (col > 0)
        isLit = isLit != pressedArr[row][col - 1];
      if (col < nCols - 1)
        isLit = isLit != pressedArr[row][col + 1];
      if (solutionViewMode && pressedArr[row][col])
        lv_table_set_cell_type(lightDisplay, row, col, isLit ? 3 : 4);
      else
        lv_table_set_cell_type(lightDisplay, row, col, isLit ? 1 : 2);
      nLitLights += (int)isLit;
    }
  }
  return nLitLights;
}

void LightsOut::ShowWin() {
  state = State::Won;
  // TODO: Text (+low opacity bg)
}

void LightsOut::HideWin() {
  state = State::Playing;
}

void LightsOut::OpenMenu() {
  // TODO: Lift from magic8ball
}

void LightsOut::CloseMenu() {
}
