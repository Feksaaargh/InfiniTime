#include "displayapp/screens/WatchFaceImages.h"

using namespace Pinetime::Applications::Screens;

WatchFaceImages::WatchFaceImages(Controllers::DateTime& dateTime, Controllers::FS& filesystem)
  : dateTime {dateTime}, filesystem {filesystem} {
  mainImage = lv_img_create(lv_scr_act(), nullptr);
  lv_img_set_auto_size(mainImage, false);
  lv_obj_set_width(mainImage, LV_HOR_RES);
  lv_obj_set_height(mainImage, LV_VER_RES);
  lv_obj_align(mainImage, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 0, 0);

  errorMessage = lv_label_create(lv_scr_act(), nullptr);

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);

  Refresh();
}

void WatchFaceImages::Refresh() {
  currentTime =
    std::chrono::time_point_cast<std::chrono::minutes, std::chrono::system_clock, std::chrono::nanoseconds>(dateTime.CurrentDateTime());
  if (!currentTime.IsUpdated()) {
    return;
  }

  // Take note of all timestamps where the image needs to change
  int retval = 0;
  // Open the images directory safely
  lfs_dir_t imagesDirHandle = {0};
  retval = filesystem.DirOpen(&WATCHFACE_IMAGES_BASE_PATH[2], &imagesDirHandle);
  if (retval < 0) {
    ShowError("DIR OPEN", retval);
    return;
  }

  char closestFileName[9] = "0000.bin";
  int fileDistance = 1339;
  bool foundZeroBin = false;
  const int currentMinute = dateTime.Hours() * 60 + dateTime.Minutes();
  // Go through every file in the directory and find the closest one still under the current minute
  while (true) {
    lfs_info itemInfo = {0};
    retval = filesystem.DirRead(&imagesDirHandle, &itemInfo);
    // If the read returns <0, an error occurred. If it returns 0, the end of the directory has been reached without finding 0000.bin.
    if (retval <= 0) {
      filesystem.DirClose(&imagesDirHandle);
      if (retval < 0) {
        ShowError("DIR STEP", retval);
      }
      break;
    }
    // Attempt to parse the filename as a timestamp
    if (strcmp(&itemInfo.name[4], ".bin") != 0) {
      continue;
    }
    bool isNameAllDigits = true;
    for (int i = 0; i < 4; i++) {
      if (itemInfo.name[i] < '0' || itemInfo.name[i] > '9')
        isNameAllDigits = false;
    }
    if (!isNameAllDigits) {
      continue;
    }
    // parse filename
    const int timestamp = std::atoi(itemInfo.name);
    const int minutes = timestamp % 100;
    const int hours = timestamp / 100;
    if (minutes == 0 && hours == 0) {
      foundZeroBin = true;
    }
    if (minutes >= 60 || hours >= 24) {
      continue;
    }
    // Figure out if this is closer than the previous closest item
    int tmpFileDistance = currentMinute - (hours * 60 + minutes);
    if (tmpFileDistance >= 0 && tmpFileDistance < fileDistance) {
      std::copy_n(itemInfo.name, 8, closestFileName);
      fileDistance = tmpFileDistance;
    }
  }
  // Finish up
  // Require zero bin since it's possible to be needed during execution
  if (!foundZeroBin) {
    filesystem.DirClose(&imagesDirHandle);
    ShowError("NO 0000.bin", -1);
  }
  imagePath = WATCHFACE_IMAGES_BASE_PATH;
  imagePath += closestFileName;
  lv_obj_set_hidden(mainImage, false);
  lv_obj_set_hidden(errorMessage, true);
  lv_img_set_src(mainImage, imagePath.c_str());
}

void WatchFaceImages::ShowError(const char* errorDesc, int errorNum) const {
  lv_label_set_text_fmt(errorMessage, "ERROR:\n%s\n%i", errorDesc, errorNum);
  lv_obj_align(errorMessage, nullptr, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_hidden(mainImage, true);
  lv_obj_set_hidden(errorMessage, false);
}


WatchFaceImages::~WatchFaceImages() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

bool Pinetime::Applications::WatchFaceTraits<Pinetime::Applications::WatchFace::Images>::IsAvailable(Controllers::FS& filesystem) {
  int retval = 0;
  // TODO: Fix on Infinisim?

  // Open the images directory safely
  lfs_dir_t imagesDirHandle = {0};
  retval = filesystem.DirOpen(&WATCHFACE_IMAGES_BASE_PATH[2], &imagesDirHandle);
  if (retval < 0) {
    printf("Failed to open: %i\n", retval);
    return false;
  }

  // Look for a file named 0000.bin. As long as that file exists, this application will function to some extent.
  while (true) {
    lfs_info itemInfo = {0};
    retval = filesystem.DirRead(&imagesDirHandle, &itemInfo);
    // If the read returns <0, an error occurred. If it returns 0, the end of the directory has been reached without finding 0000.bin.
    if (retval <= 0) {
      printf("Failed to read: %i\n", retval);
      filesystem.DirClose(&imagesDirHandle);
      return false;
    }
    printf("%s\n", itemInfo.name);
    if (strcmp(itemInfo.name, "0000.bin") == 0) {
      filesystem.DirClose(&imagesDirHandle);
      return true;
    }
  }
}