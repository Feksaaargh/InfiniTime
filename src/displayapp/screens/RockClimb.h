#pragma once

#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "Symbols.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class RockClimb : public Screen {
      public:
        RockClimb(Components::LittleVgl&);
        ~RockClimb() override;
      private:
        Components::LittleVgl& lvgl;
      };
    }
    
    template <>
    struct AppTraits<Apps::RockClimb> {
      static constexpr Apps app = Apps::RockClimb;
      static constexpr const char* icon = "C";
      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::RockClimb(controllers.lvgl);
      }
    };
  }
}