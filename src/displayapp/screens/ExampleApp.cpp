#include "displayapp/screens/ExampleApp.h"


using namespace Pinetime::Applications::Screens;

WaterParticle::WaterParticle() {
  Reset();
}

void WaterParticle::Reset() {
  // yVelocity is 1 to 8 inclusive
  yVelocity = (float)(rand() % 700) / (float)100 + 1;
  // xVelocity is -15 to 15 inclusive
  xVelocity = (float)(rand() % 3000) / (float)100 - 15;
  xPos = 120;
  yPos = 0;
}

void WaterParticle::Step() {
  xVelocity *= X_DAMPENING;
  yVelocity += GRAVITY;

  xPos += xVelocity;
  yPos += yVelocity;

  // go back to the faucet once off screen (also to make drawing safe)
  if (xPos < 0 || xPos > 239 || yPos > 239) {
    Reset();
  }
}


ExampleApp::ExampleApp(Components::LittleVgl& lvgl) :
lvgl {lvgl} {
  // setup for water. buffer 0-24 is for printing water, buffer 25-49 is for clearing water.
  std::fill_n(aquaBuffer, 25, LV_COLOR_AQUA);
  std::fill_n(blackBuffer, 25, LV_COLOR_BLACK);

  // if it refreshes every frame then inptus (button, screen taps) take a concerningly long time to get processed...
  // so only update roughly every other frame.
  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD * 2, LV_TASK_PRIO_MID, this);
}

ExampleApp::~ExampleApp() {
  lv_obj_clean(lv_scr_act());
  lv_task_del(taskRefresh);
}

void ExampleApp::Refresh() {
  lv_area_t area;
  lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::None);
  for (WaterParticle& particle: water) {
    // clear water particle from old location
    area.x1 = std::max(0, (lv_coord_t)particle.xPos - 2);
    area.y1 = std::max(0, (lv_coord_t)particle.yPos - 2);
    area.x2 = std::min(240, (lv_coord_t)particle.xPos + 2);
    area.y2 = std::min(240, (lv_coord_t)particle.yPos + 2);
    lvgl.FlushDisplay(&area, blackBuffer);

    particle.Step();

    // draw water particle after it stepped
    area.x1 = std::max(0, (lv_coord_t)particle.xPos - 2);
    area.y1 = std::max(0, (lv_coord_t)particle.yPos - 2);
    area.x2 = std::min(240, (lv_coord_t)particle.xPos + 2);
    area.y2 = std::min(240, (lv_coord_t)particle.yPos + 2);
    lvgl.FlushDisplay(&area, aquaBuffer);
  }
}
