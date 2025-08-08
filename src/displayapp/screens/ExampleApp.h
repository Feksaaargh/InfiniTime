#pragma once

#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "Symbols.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class ExampleApp : public Screen {
      public:
        ExampleApp(Components::LittleVgl&);
        ~ExampleApp() override;
      private:
        Components::LittleVgl& lvgl;
      };
    }
    
    template <>
    struct AppTraits<Apps::Example> {
      static constexpr Apps app = Apps::Example;
      static constexpr const char* icon = "E";
      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::ExampleApp(controllers.lvgl);
      }

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      };
    };
  }
}