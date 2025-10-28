#pragma once

#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "displayapp/LittleVgl.h"
#include "systemtask/WakeLock.h"
#include "Symbols.h"

#include <vector>
#include <cstdlib>
#include <cstdio>
#include <cstdint>

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class LightsOut : public Screen {
      public:
        LightsOut(Components::LittleVgl& lvgl);
        ~LightsOut() override;

        bool OnButtonPushed() override;

        void UpdateSelected(const lv_obj_t* object, lv_event_t event);

      private:
        /// Populate the pressedArr with random values, guaranteeing at least some buttons to be pressed
        void GenerateGame();

        /// Reevaluate table styles related to grid size
        void RestyleTable();

        /// Evaluate the pressed array and populate the main table with the updated states
        void RelightTable();

        /// Check if a specific tile is lit or not
        bool IsLit(int row, int col);

        void LightDisplayEvent(lv_event_t event);
        void BtnSizeIncEvent(lv_event_t event);
        void BtnSizeDecEvent(lv_event_t event);

        void ShowWin();
        void ShowLoss();

        void ShowMenu();
        void HideMenu();

        int nRows;
        int nCols;
        std::vector<std::vector<bool>> pressedArr;

        unsigned int usedPresses;
        bool solnViewMode;

        lv_style_t styleActive;
        lv_style_t styleInactive;
        lv_style_t styleSolnActive;
        lv_style_t styleSolnInactive;
        lv_obj_t* lightDisplay;
        lv_obj_t* btnSizeIncrease;
        lv_obj_t* btnSizeDecrease;
        lv_obj_t* btnCloseMenu;
        lv_obj_t* btnToggleSolution;
        lv_obj_t* btnBoardSize;
        lv_obj_t* lblBoardSize;
        lv_obj_t* winScreenBG;
        lv_obj_t* lblWinScreenText;

        Components::LittleVgl& lvgl;

        enum class State { Playing, Won, Lost, InMenu } state;
      };
    }

    template <>
    struct AppTraits<Apps::LightsOut> {
      static constexpr Apps app = Apps::LightsOut;
      static constexpr const char* icon = Screens::Symbols::lightbulb;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::LightsOut(controllers.lvgl);
      }

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      };
    };
  }
}