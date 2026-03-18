#include "displayapp/screens/QRCode.h"

using namespace Pinetime::Applications::Screens;

QRCode::QRCode(Components::LittleVgl& lvgl) :
lvgl {lvgl} {
  lv_style_init(&qrcodeBGStyle);
  lv_style_set_bg_color(&qrcodeBGStyle, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_style_set_radius(&qrcodeBGStyle, LV_STATE_DEFAULT, 0);

  lv_obj_t* qrcodeBG = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_add_style(qrcodeBG, LV_OBJ_PART_MAIN, &qrcodeBGStyle);
  lv_obj_set_width(qrcodeBG, LV_HOR_RES);
  lv_obj_set_height(qrcodeBG, LV_VER_RES);
  lv_obj_align(qrcodeBG, nullptr, LV_ALIGN_CENTER, 0, 0);

  qrcode = Pinetime::Tools::CreateQRCodeCanvas(lv_scr_act(), 200, LV_COLOR_BLACK, LV_COLOR_WHITE);
  lv_obj_align(qrcode, nullptr, LV_ALIGN_CENTER, 0, 0);

  static constexpr char thingy[] = "testing uwu";
  Pinetime::Tools::UpdateQRCodeCanvas(qrcode, thingy, 11);
}

QRCode::~QRCode() {
  Pinetime::Tools::DeleteQRCodeCanvas(qrcode);
  lv_style_reset(&qrcodeBGStyle);
  lv_obj_clean(lv_scr_act());
}