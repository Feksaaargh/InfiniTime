#pragma once
#include "lvgl/src/lv_core/lv_obj.h"
//#include "QRCodeGenerator.h"

namespace Pinetime::Tools {
  // Create an empty QRCode object (an lv_canvas)
  // @param parent The parent of the QRCode
  // @param size The edge length, in pixels, of the displayed QRCode
  // @param dark_color The color of dark modules in the code
  // @param light_color The color of the light modules in the code
  // @return A QRCode object
  lv_obj_t* CreateQRCodeCanvas(lv_obj_t* parent,
                                 lv_coord_t size,
                                 lv_color_t darkColor = LV_COLOR_BLACK,
                                 lv_color_t lightColor = LV_COLOR_WHITE);

  // Updates a QRCode object with the provided data
  // @param qrcode A pointer to the qrcode object to update
  // @param data A pointer to a char array containing the data to use
  // @param dataLen The length of the passed data
  int UpdateQRCodeCanvas(lv_obj_t* qrcode, const char* data, uint16_t dataLen);

  // Destroys a QRCode object
  // @param qrcode The QRCode object to destroy
  void DeleteQRCodeCanvas(lv_obj_t* qrcode);
}