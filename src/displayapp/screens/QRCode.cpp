#include "displayapp/screens/QRCode.h"

using namespace Pinetime::Applications::Screens;

QRCode::QRCode(Components::LittleVgl& lvgl, Controllers::FS& filesystem)
  : lvgl{lvgl},
    filesystem{filesystem} {
  lv_style_init(&qrCodeBGStyle);
  lv_style_set_bg_color(&qrCodeBGStyle, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_style_set_radius(&qrCodeBGStyle, LV_STATE_DEFAULT, 0);

  lv_obj_t* qrCodeBG = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_add_style(qrCodeBG, LV_OBJ_PART_MAIN, &qrCodeBGStyle);
  lv_obj_set_width(qrCodeBG, LV_HOR_RES);
  lv_obj_set_height(qrCodeBG, LV_VER_RES);
  lv_obj_align(qrCodeBG, nullptr, LV_ALIGN_CENTER, 0, 0);

  qrCode = Pinetime::Tools::CreateQRCodeCanvas(lv_scr_act(), 200, LV_COLOR_BLACK, LV_COLOR_WHITE);
  lv_obj_align(qrCode, nullptr, LV_ALIGN_CENTER, 0, 0);

  // TODO: Call qr code generation properly
  // UpdateQRCode();
  UpdateQRCodeLater();
}

// TODO: Reference Settings.cpp/Settings.h for how to make screens of text

// TODO: Pull qrcode contents from files on disk

QRCode::~QRCode() {
  Pinetime::Tools::DeleteQRCodeCanvas(qrCode);
  lv_style_reset(&qrCodeBGStyle);
  lv_obj_clean(lv_scr_act());
}

bool QRCode::OnTouchEvent(TouchEvents event) {
  // TODO: Implement
  (void) event;
  return false;
}

void QRCode::UpdateQRCodeLater() {
  // TODO: Figure out why LV_TASK_PRIO_LOW is required
  lv_task_t* updateLaterTask = lv_task_create(UpdateQRCodeLaterCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_LOW, this);
  lv_task_set_repeat_count(updateLaterTask, 1);
}

void QRCode::UpdateQRCodeLaterCallback(lv_task_t* task) {
  static_cast<QRCode*>(task->user_data)->UpdateQRCode();
}

void QRCode::UpdateQRCode() {
  Pinetime::Tools::UpdateQRCodeCanvas(qrCode, thingy, sizeof(thingy) - 1);
  //Pinetime::Tools::UpdateQRCodeCanvas(qrCode, thingy, 2955);
}

bool QRCode::ReadDataFile() {
  return true;
}