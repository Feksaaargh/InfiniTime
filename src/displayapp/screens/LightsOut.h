#pragma once

#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "Symbols.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class LightsOut : public Screen {
      public:
        LightsOut(Components::LittleVgl&);
        ~LightsOut() override;
      private:
        Components::LittleVgl& lvgl;
      };
    }
    
    template <>
    struct AppTraits<Apps::LightsOut> {
      static constexpr Apps app = Apps::LightsOut;
      static constexpr const char* icon = "E";  // TODO: Lightbulb icon
      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::LightsOut(controllers.lvgl);
      }

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      };
    };
  }
}