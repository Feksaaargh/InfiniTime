#include "displayapp/screens/WatchFaceImages.h"

using namespace Pinetime::Applications::Screens;

WatchFaceImages::WatchFaceImages(Controllers::DateTime& dateTimeController) {
  // test object
  lv_obj_t* bgimg = lv_img_create(lv_scr_act(), nullptr);
  lv_img_set_auto_size(bgimg, false);
  lv_img_set_src(bgimg, "F:/clock_00.bin");
  lv_obj_set_width(bgimg, LV_HOR_RES);
  lv_obj_set_height(bgimg, LV_VER_RES);
  lv_obj_align(bgimg, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);

  dateTimeController.Day();
  // lv_obj_t* title = lv_label_create(lv_scr_act(), nullptr);
  // lv_label_set_text_static(title, "My test watchface");
  // lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
  // lv_obj_align(title, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
}

WatchFaceImages::~WatchFaceImages() {
  lv_obj_clean(lv_scr_act());
}