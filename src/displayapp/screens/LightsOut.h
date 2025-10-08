#pragma once

#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "Symbols.h"

#include <vector>
#include <cstdlib>
#include <cstdio>

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class LightsOut : public Screen {
      public:
        LightsOut(Components::LittleVgl&);
        ~LightsOut() override;

        // bool OnButtonPushed() override;

        // bool OnTouchEvent(uint16_t x, uint16_t y) override;

      private:
        /// Reevaluate all table styles related to size
        void RealignTable();

        /// Evaluate the pressed array and populate the main table with the updated states
        void RelightTable();

        /// Check if user won (pressedArr is all false), and congratulate them if so
        void ProcessWin();

        /// Populate the pressedArr with random values, guaranteeing at least some buttons to be pressed
        void GenerateGame();

        int nRows;
        int nCols;
        std::vector<std::vector<bool>> pressedArr;

        unsigned int minPresses;
        unsigned int usedPresses;

        lv_style_t styleActive;
        lv_style_t styleInactive;
        lv_obj_t* lightDisplay;

        Components::LittleVgl& lvgl;

      };
    }
    
    template <>
    struct AppTraits<Apps::LightsOut> {
      static constexpr Apps app = Apps::LightsOut;
      static constexpr const char* icon = "L";  // TODO: Lightbulb icon
      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::LightsOut(controllers.lvgl);
      }

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      };
    };
  }
}