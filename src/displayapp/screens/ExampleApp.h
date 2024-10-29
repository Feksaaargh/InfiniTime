#pragma once

#include <algorithm>
#include <cstdlib>
#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "displayapp/LittleVgl.h"
#include "Symbols.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class WaterParticle {
      public:
        WaterParticle();
        void Step();
        float xPos;
        float yPos;
      private:
        void Reset();
        float xVelocity;
        float yVelocity;
        static constexpr float X_DAMPENING = 0.90;
        static constexpr float GRAVITY = 0.3;
      };

      class ExampleApp : public Screen {
      public:
        ExampleApp(Components::LittleVgl&);
        ~ExampleApp() override;
        void Refresh() override;
      private:
        Components::LittleVgl& lvgl;
        lv_task_t* taskRefresh;
        WaterParticle water[50];
        lv_color_t aquaBuffer[25];
        lv_color_t blackBuffer[25];
      };
    }
    
    template <>
    struct AppTraits<Apps::ExampleApp> {
      static constexpr Apps app = Apps::ExampleApp;
      static constexpr const char* icon = "E";
      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::ExampleApp(controllers.lvgl);
      }
    };
  }
}