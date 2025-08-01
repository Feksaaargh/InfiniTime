#pragma once

#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "components/ble/BleController.h"
#include "components/datetime/DateTimeController.h"
#include <components/battery/BatteryController.h>
#include "displayapp/screens/BatteryIcon.h"
#include <utility/DirtyValue.h>
#include <displayapp/screens/Symbols.h>
#include <displayapp/screens/NotificationIcon.h>

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class WatchFaceAnalogHard : public Screen {
      public:
        WatchFaceAnalogHard(Controllers::DateTime& dateTimeController,
                            const Controllers::Battery& batteryController,
                            const Controllers::Ble& bleController,
                            Controllers::NotificationManager& notificationManager,
                            Controllers::Settings& settingsController);

        ~WatchFaceAnalogHard() override;

        void Refresh() override;

      private:
        uint8_t sHour, sMinute;

        Utility::DirtyValue<uint8_t> batteryPercentRemaining {0};
        Utility::DirtyValue<bool> isCharging {};
        Utility::DirtyValue<bool> bleState {};
        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>> currentDateTime;
        Utility::DirtyValue<bool> notificationState {false};
        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::days>> currentDate;

        // degree angles: [0,360]
        // (at least, I think _lv_atan2() can return 360 so handle that case too)
        // degree 0 means the hand is pointing straight up, 90 means straight right
        uint16_t offset_angle_minute;
        uint16_t offset_angle_hour;

        lv_obj_t* minor_scales_minute;
        lv_obj_t* major_scales_minute;
        lv_obj_t* large_scales_minute;
        lv_obj_t* twelve_minute;
        // TODO: rotatable 12?

        lv_obj_t* minor_scales_hour;
        lv_obj_t* major_scales_hour;
        lv_obj_t* large_scales_hour;
        lv_obj_t* twelve_hour;
        // TODO: rotatable 12?

        lv_obj_t* hour_body;
        lv_obj_t* hour_body_trace;
        lv_obj_t* minute_body;
        lv_obj_t* minute_body_trace;

        lv_point_t hour_point[2];
        lv_point_t hour_point_trace[2];
        lv_point_t minute_point[2];
        lv_point_t minute_point_trace[2];

        lv_style_t hour_line_style;
        lv_style_t hour_line_style_trace;
        lv_style_t minute_line_style;
        lv_style_t minute_line_style_trace;

        lv_obj_t* plugIcon;
        lv_obj_t* notificationIcon;
        lv_obj_t* bleIcon;

        BatteryIcon batteryIcon;

        Controllers::DateTime& dateTimeController;
        const Controllers::Battery& batteryController;
        const Controllers::Ble& bleController;
        Controllers::NotificationManager& notificationManager;
        Controllers::Settings& settingsController;

        void SetMinuteWheelAngle(int16_t angle);
        void SetHourWheelAngle(int16_t angle);
        void SetMinuteHandAngle(int16_t angle);
        void SetHourHandAngle(int16_t angle);

        void SetBatteryIcon();
        void UpdateClock();

        lv_task_t* taskRefresh;
      };
    }

    template <>
    struct WatchFaceTraits<WatchFace::AnalogHard> {
      static constexpr WatchFace watchFace = WatchFace::AnalogHard;
      static constexpr const char* name = "Analog-Hard";

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::WatchFaceAnalogHard(controllers.dateTimeController,
                                            controllers.batteryController,
                                            controllers.bleController,
                                            controllers.notificationManager,
                                            controllers.settingsController);
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      }
    };
  }
}