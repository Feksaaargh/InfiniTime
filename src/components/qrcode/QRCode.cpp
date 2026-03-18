#include "QRCode.h"

lv_obj_t* Pinetime::Tools::CreateQRCodeCanvas(lv_obj_t* parent, lv_coord_t size, lv_color_t darkColor, lv_color_t lightColor) {
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
  // Generate QR code
  const QRCodeModules qrCodeModules = QRCodeGenerator::GenerateQRCode(data, dataLen);

  // Constants describing the qrcode object
  const lv_coord_t canvasWidth = lv_obj_get_width(qrcode);
  const lv_coord_t canvasHeight = lv_obj_get_height(qrcode);

  // Check if qr code failed to generate
  if (qrCodeModules.GetVersion() == 0) {
    // Fill bg
    lv_color_t bgColor;
    bgColor.full = 0; // light color
    lv_canvas_fill_bg(qrcode, bgColor, 0);
    // Draw text
    lv_draw_label_dsc_t label_dsc;
    lv_draw_label_dsc_init(&label_dsc);
    label_dsc.color.full = 1; // dark color
    lv_canvas_draw_text(qrcode,
                        canvasWidth / 2,
                        canvasHeight / 10,
                        canvasWidth * 8 / 10,
                        &label_dsc,
                        "QR code failed to generate",
                        LV_LABEL_ALIGN_CENTER);
    return;
  }

  // TODO: Make better (use lv_canvas_draw_rect)
  // Populate the canvas
  lv_color_t lightColor;
  lightColor.full = 0;
  lv_color_t darkColor;
  darkColor.full = 1;
  int modulesSize = qrCodeModules.GetSize();
  for (lv_coord_t y = 0; y < canvasHeight; y++) {
    for (lv_coord_t x = 0; x < canvasWidth; x++) {
      const bool moduleValue = qrCodeModules.GetModule(x * modulesSize / canvasWidth, y * modulesSize / canvasHeight);
      lv_canvas_set_px(qrcode, x, y, moduleValue ? darkColor : lightColor);
    }
  }
}

void Pinetime::Tools::DeleteQRCodeCanvas(lv_obj_t* qrcode) {
  lv_img_dsc_t* canvasData = lv_canvas_get_img(qrcode);
  const uint8_t* canvasBuffer = canvasData->data;
  lv_obj_del(qrcode);
  lv_mem_free(canvasBuffer);
}