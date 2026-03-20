#include "displayapp/screens/QRCode.h"

using namespace Pinetime::Applications::Screens;

QRCode::QRCode(Components::LittleVgl& lvgl, Controllers::FS& filesystem) :
lvgl {lvgl},
filesystem {filesystem} {
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

  // 400 chars:
  static constexpr char thingy[] = "hey what are you doing here? you really shouldn't be here you know. it's also kinda rude to be looking at test strings without permission, you know? although this is on a public github repo, so idk... ok whatever ig you can look. this is just to test excessively long strings on hardware anyway, so nothing sensitive. but still, you could've asked before poking around. also you just lost the game :3";
  // 28 chars:
  // static constexpr char thingy[] = "https://youtu.be/dQw4w9WgXcQ";
  Pinetime::Tools::UpdateQRCodeCanvas(qrCode, thingy, sizeof(thingy) - 1);  // -1 since null terminator doesn't need to be incode
}

// TODO: Move qr code generation to be generated in the update function if longer than, say, 100 chars (for app opening speed)

// TODO: Reference Settings.cpp/Settings.h for how to make screens of text

// TODO: Pull qrcode contents from files on disk

QRCode::~QRCode() {
  Pinetime::Tools::DeleteQRCodeCanvas(qrCode);
  lv_style_reset(&qrCodeBGStyle);
  lv_obj_clean(lv_scr_act());
}