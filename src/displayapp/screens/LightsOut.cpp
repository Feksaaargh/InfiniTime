#include "displayapp/screens/LightsOut.h"

using namespace Pinetime::Applications::Screens;

LightsOut::LightsOut(Components::LittleVgl& lvgl) :
lvgl {lvgl} {
  lightDisplay = lv_table_create(lv_scr_act(), nullptr);
  lv_table_set_col_cnt(lightDisplay, nCols);
  lv_table_set_row_cnt(lightDisplay, nRows);
  lv_obj_clean_style_list(lightDisplay, LV_TABLE_PART_BG);  // TODO: Check if this is needed?
  // Table sizing and positioning is handled in RelightTable() to account for different table sizes

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

  nRows = 5;
  nCols = 5;

  GenerateGame();
  RealignTable();
  RelightTable();
}

LightsOut::~LightsOut() {
  lv_obj_clean(lv_scr_act());
  lv_style_reset(&styleActive);
  lv_style_reset(&styleInactive);
}

void LightsOut::RealignTable() {
  lv_table_set_row_cnt(lightDisplay, nRows);
  lv_table_set_col_cnt(lightDisplay, nCols);

  for (int col = 0; col < nCols; col++) {
    lv_table_set_col_width(lightDisplay, col, LV_HOR_RES / nCols);
  }
  lv_style_set_pad_top(&styleActive, LV_STATE_DEFAULT, (LV_VER_RES / nRows) / 2 - 1);
  lv_style_set_pad_top(&styleInactive, LV_STATE_DEFAULT, (LV_VER_RES / nRows) / 2 - 1);
  lv_obj_refresh_style(lightDisplay, LV_STATE_DEFAULT, LV_STYLE_PROP_ALL);

  lv_obj_align(lightDisplay, nullptr, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
}

void LightsOut::RelightTable() {
  // Evaluate which lights are enabled
  for (int row = 0; row < nRows; row++) {
    for (int col = 0; col < nCols; col++) {
      bool isLit = pressedArr[row][col];
      if (row > 0)
        isLit = isLit != pressedArr[row-1][col];
      if (row < nRows - 1)
        isLit = isLit != pressedArr[row+1][col];
      if (col > 0)
        isLit = isLit != pressedArr[row][col-1];
      if (col < nCols - 1)
        isLit = isLit != pressedArr[row][col+1];
      if (isLit)
        lv_table_set_cell_type(lightDisplay, row, col, 1);
      else
        lv_table_set_cell_type(lightDisplay, row, col, 2);
    }
  }
}

void LightsOut::ProcessWin() {
  // TODO
}

void LightsOut::GenerateGame() {
  pressedArr = std::vector<std::vector<bool>>(nCols);
  for (int col = 0; col < nCols; col++) {
    pressedArr[col] = std::vector<bool>(nRows);
  }
  for (int row = 0; row < nRows; row++) {
    for (int col = 0; col < nCols; col++) {
      pressedArr[row][col] = bool(std::rand() & 1);
    }
  }
}