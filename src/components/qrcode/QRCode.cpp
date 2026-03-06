#include "QRCode.h"


lv_obj_t* Pinetime::Tools::CreateQRCodeCanvas(lv_obj_t* parent,
                                              lv_coord_t size,
                                              lv_color_t darkColor,
                                              lv_color_t lightColor) {
  const uint16_t bufferSize = LV_CANVAS_BUF_SIZE_INDEXED_1BIT(size, size);
  auto* buffer = static_cast<lv_color_t*>(lv_mem_alloc(bufferSize));
  if (buffer == nullptr) {
    return nullptr;
  }

  lv_obj_t* qrCanvas = lv_canvas_create(parent, nullptr);
  lv_canvas_set_buffer(qrCanvas, buffer, size, size, LV_IMG_CF_INDEXED_1BIT);
  lv_canvas_set_palette(qrCanvas, 0, lightColor);
  lv_canvas_set_palette(qrCanvas, 1, darkColor);

  lv_color_t fillColor;
  fillColor.full = 0;
  lv_canvas_fill_bg(qrCanvas, fillColor, 0);

  return qrCanvas;
}

void Pinetime::Tools::UpdateQRCodeCanvas(lv_obj_t* qrcode, const char* data, uint16_t dataLen) {
  // temporarily silence unused parameter warnings
  (void) qrcode;
  (void) data;
  (void) dataLen;
}

void Pinetime::Tools::DeleteQRCodeCanvas(lv_obj_t* qrcode) {
  lv_img_dsc_t* canvasData = lv_canvas_get_img(qrcode);
  const uint8_t* canvasBuffer = canvasData->data;
  lv_obj_del(qrcode);
  lv_mem_free(canvasBuffer);
}