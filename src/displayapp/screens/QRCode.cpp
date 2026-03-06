#include "displayapp/screens/QRCode.h"

using namespace Pinetime::Applications::Screens;

QRCode::QRCode(Components::LittleVgl& lvgl) :
lvgl {lvgl} {
  qrcode = Pinetime::Tools::CreateQRCodeCanvas(lv_scr_act(), 200);
  lv_obj_align(qrcode, nullptr, LV_ALIGN_CENTER, 0, 0);

  static constexpr char thingy[] = "testing uwu";
  Pinetime::Tools::UpdateQRCodeCanvas(qrcode, thingy, 7);
}

QRCode::~QRCode() {
  Pinetime::Tools::DeleteQRCodeCanvas(qrcode);
  lv_obj_clean(lv_scr_act());
}