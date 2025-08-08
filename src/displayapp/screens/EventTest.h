#pragma once

#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "Symbols.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class EventTest : public Screen {
      public:
        EventTest(Components::LittleVgl&);
        ~EventTest() override;
        bool OnTouchEvent(TouchEvents event) override;
        bool OnButtonPushed();
      private:
        void RefreshTexts();
        Components::LittleVgl& lvgl;
        lv_obj_t* lastEventType;
        lv_obj_t* numEventsText;
        int numEvents;
      };
    }
    
    template <>
    struct AppTraits<Apps::EventTest> {
      static constexpr Apps app = Apps::EventTest;
      static constexpr const char* icon = "E";
      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::EventTest(controllers.lvgl);
      }

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      };
    };
  }
}