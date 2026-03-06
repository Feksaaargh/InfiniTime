#include "QRCode.h"


lv_obj_t* Pinetime::Tools::CreateQRCodeCanvas(lv_obj_t* parent,
                               lv_coord_t size,
                               lv_color_t darkColor,
                               lv_color_t lightColor) {
  // temporarily silence unused parameter warnings
  (void)parent;
  (void)size;
  (void)darkColor;
  (void)lightColor;
  return nullptr;
}

int Pinetime::Tools::UpdateQRCodeCanvas(lv_obj_t* qrcode, const char* data, uint16_t dataLen) {
  // temporarily silence unused parameter warnings
  (void)qrcode;
  (void)data;
  (void)dataLen;
  return 0;
}

void Pinetime::Tools::DeleteQRCodeCanvas(lv_obj_t* qrcode) {
  // temporarily silence unused parameter warnings
  (void)qrcode;
}