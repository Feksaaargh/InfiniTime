#include "displayapp/screens/WatchFaceAnalogHard.h"

using namespace Pinetime::Applications::Screens;

namespace {
  constexpr int16_t HourLength = 60;
  constexpr int16_t MinuteLength = 90;

  // LVGL sin isn't constexpr (though it could be if it were C++) so fix size here
  // All the types are hardcoded anyway and would need changing if the size changed
  // sin(90) = 1 so the value of _lv_trigo_sin(90) is the scaling factor
  constexpr int16_t LV_TRIG_SCALE = std::numeric_limits<int16_t>::max(); // = _lv_trigo_sin(90)

  int16_t Cosine(int16_t angle) {
    return _lv_trigo_sin(angle + 90);
  }

  int16_t Sine(int16_t angle) {
    return _lv_trigo_sin(angle);
  }

  int16_t CoordinateXRelocate(int16_t x) {
    return (x + LV_HOR_RES / 2);
  }

  int16_t CoordinateYRelocate(int16_t y) {
    return std::abs(y - LV_HOR_RES / 2);
  }

  lv_point_t CoordinateRelocate(int16_t radius, int16_t angle) {
    return lv_point_t {.x = CoordinateXRelocate(radius * static_cast<int32_t>(Sine(angle)) / LV_TRIG_SCALE),
                       .y = CoordinateYRelocate(radius * static_cast<int32_t>(Cosine(angle)) / LV_TRIG_SCALE)};
  }

}

WatchFaceAnalogHard::WatchFaceAnalogHard(Controllers::DateTime& dateTimeController,
                                 const Controllers::Battery& batteryController,
                                 const Controllers::Ble& bleController,
                                 Controllers::NotificationManager& notificationManager,
                                 Controllers::Settings& settingsController)
  : currentDateTime {{}},
    batteryIcon(true),
    dateTimeController {dateTimeController},
    batteryController {batteryController},
    bleController {bleController},
    notificationManager {notificationManager},
    settingsController {settingsController} {

  sHour = 99;
  sMinute = 99;

  // TODO: load from settings
  offset_angle_minute = 18;
  offset_angle_hour = 225;

  // Minutes wheel

  minor_scales_minute = lv_linemeter_create(lv_scr_act(), nullptr);
  lv_linemeter_set_scale(minor_scales_minute, 300, 51);
  lv_obj_set_size(minor_scales_minute, 240, 240);
  lv_obj_align(minor_scales_minute, nullptr, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_local_bg_opa(minor_scales_minute, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_style_local_scale_width(minor_scales_minute, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, 4);
  lv_obj_set_style_local_scale_end_line_width(minor_scales_minute, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, 1);
  lv_obj_set_style_local_scale_end_color(minor_scales_minute, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);

  major_scales_minute = lv_linemeter_create(lv_scr_act(), nullptr);
  lv_linemeter_set_scale(major_scales_minute, 300, 11);
  lv_obj_set_size(major_scales_minute, 240, 240);
  lv_obj_align(major_scales_minute, nullptr, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_local_bg_opa(major_scales_minute, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_style_local_scale_width(major_scales_minute, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, 6);
  lv_obj_set_style_local_scale_end_line_width(major_scales_minute, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, 3);
  lv_obj_set_style_local_scale_end_color(major_scales_minute, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);

  large_scales_minute = lv_linemeter_create(lv_scr_act(), nullptr);
  lv_linemeter_set_scale(large_scales_minute, 180, 3);
  lv_obj_set_size(large_scales_minute, 240, 240);
  lv_obj_align(large_scales_minute, nullptr, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_local_bg_opa(large_scales_minute, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_style_local_scale_width(large_scales_minute, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, 20);
  lv_obj_set_style_local_scale_end_line_width(large_scales_minute, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, 4);
  lv_obj_set_style_local_scale_end_color(large_scales_minute, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_AQUA);

  twelve_minute = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_align(twelve_minute, LV_LABEL_ALIGN_CENTER);
  lv_label_set_text_static(twelve_minute, "12");
  lv_obj_set_style_local_text_color(twelve_minute, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_AQUA);

  // Hours wheel

  minor_scales_hour = lv_linemeter_create(lv_scr_act(), nullptr);
  lv_linemeter_set_scale(minor_scales_hour, 300, 51);
  lv_obj_set_size(minor_scales_hour, 180, 180);
  lv_obj_align(minor_scales_hour, nullptr, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_local_bg_opa(minor_scales_hour, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_style_local_scale_width(minor_scales_hour, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, 4);
  lv_obj_set_style_local_scale_end_line_width(minor_scales_hour, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, 1);
  lv_obj_set_style_local_scale_end_color(minor_scales_hour, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);

  major_scales_hour = lv_linemeter_create(lv_scr_act(), nullptr);
  lv_linemeter_set_scale(major_scales_hour, 300, 11);
  lv_obj_set_size(major_scales_hour, 180, 180);
  lv_obj_align(major_scales_hour, nullptr, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_local_bg_opa(major_scales_hour, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_style_local_scale_width(major_scales_hour, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, 4);
  lv_obj_set_style_local_scale_end_line_width(major_scales_hour, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, 3);
  lv_obj_set_style_local_scale_end_color(major_scales_hour, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);

  large_scales_hour = lv_linemeter_create(lv_scr_act(), nullptr);
  lv_linemeter_set_scale(large_scales_hour, 180, 3);
  lv_obj_set_size(large_scales_hour, 180, 180);
  lv_obj_align(large_scales_hour, nullptr, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_local_bg_opa(large_scales_hour, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_style_local_scale_width(large_scales_hour, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, 10);
  lv_obj_set_style_local_scale_end_line_width(large_scales_hour, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, 4);
  lv_obj_set_style_local_scale_end_color(large_scales_hour, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLUE);

  twelve_hour = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_align(twelve_hour, LV_LABEL_ALIGN_CENTER);
  lv_label_set_text_static(twelve_hour, "12");
  lv_obj_set_style_local_text_color(twelve_hour, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLUE);

  // Icons and watch hands

  batteryIcon.Create(lv_scr_act());
  lv_obj_align(batteryIcon.GetObject(), nullptr, LV_ALIGN_IN_TOP_RIGHT, 0, 0);

  plugIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(plugIcon, Symbols::plug);
  lv_obj_align(plugIcon, nullptr, LV_ALIGN_IN_TOP_RIGHT, 0, 0);

  bleIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(bleIcon, "");
  lv_obj_align(bleIcon, nullptr, LV_ALIGN_IN_TOP_RIGHT, -30, 0);

  notificationIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(notificationIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_LIME);
  lv_label_set_text_static(notificationIcon, NotificationIcon::GetIcon(false));
  lv_obj_align(notificationIcon, nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 0);

  minute_body = lv_line_create(lv_scr_act(), nullptr);
  minute_body_trace = lv_line_create(lv_scr_act(), nullptr);
  hour_body = lv_line_create(lv_scr_act(), nullptr);
  hour_body_trace = lv_line_create(lv_scr_act(), nullptr);

  // Watch hand styles

  lv_style_init(&minute_line_style);
  lv_style_set_line_width(&minute_line_style, LV_STATE_DEFAULT, 7);
  lv_style_set_line_color(&minute_line_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_style_set_line_rounded(&minute_line_style, LV_STATE_DEFAULT, true);
  lv_obj_add_style(minute_body, LV_LINE_PART_MAIN, &minute_line_style);

  lv_style_init(&minute_line_style_trace);
  lv_style_set_line_width(&minute_line_style_trace, LV_STATE_DEFAULT, 3);
  lv_style_set_line_color(&minute_line_style_trace, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_style_set_line_rounded(&minute_line_style_trace, LV_STATE_DEFAULT, false);
  lv_obj_add_style(minute_body_trace, LV_LINE_PART_MAIN, &minute_line_style_trace);

  SetMinuteHandAngle(offset_angle_minute);

  lv_style_init(&hour_line_style);
  lv_style_set_line_width(&hour_line_style, LV_STATE_DEFAULT, 7);
  lv_style_set_line_color(&hour_line_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_style_set_line_rounded(&hour_line_style, LV_STATE_DEFAULT, true);
  lv_obj_add_style(hour_body, LV_LINE_PART_MAIN, &hour_line_style);

  lv_style_init(&hour_line_style_trace);
  lv_style_set_line_width(&hour_line_style_trace, LV_STATE_DEFAULT, 3);
  lv_style_set_line_color(&hour_line_style_trace, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_style_set_line_rounded(&hour_line_style_trace, LV_STATE_DEFAULT, false);
  lv_obj_add_style(hour_body_trace, LV_LINE_PART_MAIN, &hour_line_style_trace);

  SetHourHandAngle(offset_angle_hour);

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);

  Refresh();
}

WatchFaceAnalogHard::~WatchFaceAnalogHard() {
  lv_task_del(taskRefresh);

  lv_style_reset(&hour_line_style);
  lv_style_reset(&hour_line_style_trace);
  lv_style_reset(&minute_line_style);
  lv_style_reset(&minute_line_style_trace);

  lv_obj_clean(lv_scr_act());
}

/// Sets the angle of the outer clock wheel
void WatchFaceAnalogHard::SetMinuteWheelAngle(int16_t angle) {
  while (angle < 0) {
    angle += 360;
  }
  const uint16_t real_angle = (angle + 180) % 360;
  lv_linemeter_set_angle_offset(minor_scales_minute, real_angle);
  lv_linemeter_set_angle_offset(major_scales_minute, real_angle);
  lv_linemeter_set_angle_offset(large_scales_minute, real_angle);

  const lv_point_t twelve_loc = CoordinateRelocate(100, angle);
  lv_obj_set_pos(twelve_minute, twelve_loc.x - 10, twelve_loc.y - 7);
}

/// Sets the angle of the inner clock wheel
void WatchFaceAnalogHard::SetHourWheelAngle(int16_t angle) {
  while (angle < 0) {
    angle += 360;
  }
  const int16_t real_angle = (angle + 180) % 360;
  lv_linemeter_set_angle_offset(minor_scales_hour, real_angle);
  lv_linemeter_set_angle_offset(major_scales_hour, real_angle);
  lv_linemeter_set_angle_offset(large_scales_hour, real_angle);

  const lv_point_t twelve_loc = CoordinateRelocate(65, angle);
  lv_obj_set_pos(twelve_hour, twelve_loc.x - 10, twelve_loc.y - 7);
}

void WatchFaceAnalogHard::SetMinuteHandAngle(int16_t angle) {
  minute_point[0] = CoordinateRelocate(30, angle);
  minute_point[1] = CoordinateRelocate(MinuteLength, angle);

  minute_point_trace[0] = CoordinateRelocate(5, angle);
  minute_point_trace[1] = CoordinateRelocate(31, angle);

  lv_line_set_points(minute_body, minute_point, 2);
  lv_line_set_points(minute_body_trace, minute_point_trace, 2);
}

void WatchFaceAnalogHard::SetHourHandAngle(int16_t angle) {
  hour_point[0] = CoordinateRelocate(30, angle);
  hour_point[1] = CoordinateRelocate(HourLength, angle);

  hour_point_trace[0] = CoordinateRelocate(5, angle);
  hour_point_trace[1] = CoordinateRelocate(31, angle);

  lv_line_set_points(hour_body, hour_point, 2);
  lv_line_set_points(hour_body_trace, hour_point_trace, 2);
}

void WatchFaceAnalogHard::UpdateClock() {
  const uint8_t hour = dateTimeController.Hours();
  const uint8_t minute = dateTimeController.Minutes();

  if (sMinute != minute) {
    const int16_t angle = (minute * -6) + offset_angle_minute;
    SetMinuteWheelAngle(angle);
  }

  if (sHour != hour || sMinute != minute) {
    sHour = hour;
    sMinute = minute;
    const int16_t angle = -(hour * 30 + minute / 2) + offset_angle_hour;
    SetHourWheelAngle(angle);
  }
}

void WatchFaceAnalogHard::SetBatteryIcon() {
  auto batteryPercent = batteryPercentRemaining.Get();
  batteryIcon.SetBatteryPercentage(batteryPercent);
}

void WatchFaceAnalogHard::Refresh() {
  isCharging = batteryController.IsCharging();
  if (isCharging.IsUpdated()) {
    if (isCharging.Get()) {
      lv_obj_set_hidden(batteryIcon.GetObject(), true);
      lv_obj_set_hidden(plugIcon, false);
    } else {
      lv_obj_set_hidden(batteryIcon.GetObject(), false);
      lv_obj_set_hidden(plugIcon, true);
      SetBatteryIcon();
    }
  }
  if (!isCharging.Get()) {
    batteryPercentRemaining = batteryController.PercentRemaining();
    if (batteryPercentRemaining.IsUpdated()) {
      SetBatteryIcon();
    }
  }

  bleState = bleController.IsConnected();
  if (bleState.IsUpdated()) {
    if (bleState.Get()) {
      lv_label_set_text_static(bleIcon, Symbols::bluetooth);
    } else {
      lv_label_set_text_static(bleIcon, "");
    }
  }

  notificationState = notificationManager.AreNewNotificationsAvailable();
  if (notificationState.IsUpdated()) {
    lv_label_set_text_static(notificationIcon, NotificationIcon::GetIcon(notificationState.Get()));
  }

  currentDateTime = dateTimeController.CurrentDateTime();
  if (currentDateTime.IsUpdated()) {
    UpdateClock();
  }
}
