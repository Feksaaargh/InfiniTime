#pragma once

#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "components/qrcode/QRCode.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class QRCode : public Screen {
      public:
        QRCode(Components::LittleVgl&, Controllers::FS& filesystem);
        ~QRCode() override;

      private:
        lv_style_t qrCodeBGStyle;
        lv_obj_t* qrCode;

        Components::LittleVgl& lvgl;
        Controllers::FS& filesystem;
      };
    }

    template <>
    struct AppTraits<Apps::QRCode> {
      static constexpr Apps app = Apps::QRCode;
      static constexpr const char* icon = "Q";

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::QRCode(controllers.lvgl, controllers.filesystem);
      }

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      };
    };
  }
}