#include "displayapp/screens/EventTest.h"

using namespace Pinetime::Applications::Screens;

EventTest::EventTest(Components::LittleVgl& lvgl) :
lvgl {lvgl} {
  lastEventType = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(lastEventType, "No events yet");
  lv_label_set_align(lastEventType, LV_LABEL_ALIGN_CENTER);

  numEventsText = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_align(numEventsText, LV_LABEL_ALIGN_CENTER);
  numEvents = 0;

  RefreshTexts();
}

EventTest::~EventTest() {
  lv_obj_clean(lv_scr_act());
}

bool EventTest::OnTouchEvent(TouchEvents event)
{
  switch (event)
  {
  case TouchEvents::None:
    lv_label_set_text_static(lastEventType, "None (bug?)");
    break;
  case TouchEvents::Tap:
    lv_label_set_text_static(lastEventType, "Tap");
    break;
  case TouchEvents::SwipeLeft:
    lv_label_set_text_static(lastEventType, "SwipeLeft");
    break;
  case TouchEvents::SwipeRight:
    lv_label_set_text_static(lastEventType, "SwipeRight");
    break;
  case TouchEvents::SwipeUp:
    lv_label_set_text_static(lastEventType, "SwipeUp");
    break;
  case TouchEvents::SwipeDown:
    lv_label_set_text_static(lastEventType, "SwipeDown");
    break;
  case TouchEvents::LongTap:
    lv_label_set_text_static(lastEventType, "LongTap");
    break;
  case TouchEvents::DoubleTap:
    lv_label_set_text_static(lastEventType, "DoubleTap");
    break;
  }
  numEvents++;
  RefreshTexts();
  return true;
}

bool EventTest::OnButtonPushed()
{
  lv_label_set_text_static(lastEventType, "Button pushed");
  numEvents++;
  RefreshTexts();
  return true;
}

void EventTest::RefreshTexts()
{
  lv_obj_align(lastEventType, lv_scr_act(), LV_ALIGN_CENTER, 0, -10);
  lv_label_set_text_fmt(numEventsText, "%d", numEvents);
  lv_obj_align(numEventsText, lv_scr_act(), LV_ALIGN_CENTER, 0, 10);
}
