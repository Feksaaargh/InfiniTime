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

  // 400 chars:
  static constexpr char thingy[] = "hey what are you doing here? you really shouldn't be here you know. it's also kinda rude to be looking at test strings without permission, you know? although this is on a public github repo, so idk... ok whatever ig you can look. this is just to test excessively long strings on hardware anyway, so nothing sensitive. but still, you could've asked before poking around. also you just lost the game :3";
  // 28 chars:
  // static constexpr char thingy[] = "https://youtu.be/dQw4w9WgXcQ";
  Pinetime::Tools::UpdateQRCodeCanvas(qrcode, thingy, sizeof(thingy) - 1);  // -1 since null terminator doesn't need to be incode
}

QRCode::~QRCode() {
  Pinetime::Tools::DeleteQRCodeCanvas(qrcode);
  lv_style_reset(&qrcodeBGStyle);
  lv_obj_clean(lv_scr_act());
}