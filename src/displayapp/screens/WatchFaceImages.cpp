#include "displayapp/screens/WatchFaceImages.h"

using namespace Pinetime::Applications::Screens;

WatchFaceImages::WatchFaceImages(Controllers::DateTime& dateTime, Controllers::FS& filesystem)
  : dateTime {dateTime}, filesystem {filesystem} {
  mainImage = lv_img_create(lv_scr_act(), nullptr);
  lv_img_set_auto_size(mainImage, false);
  lv_obj_set_width(mainImage, LV_HOR_RES);
  lv_obj_set_height(mainImage, LV_VER_RES);
  lv_obj_align(mainImage, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 0, 0);

  // Take note of all timestamps where the image needs to change
  // The vector can theoretically take ~2.8kb, since there can be maximum 1440 valid images here
  int retval = 0;
  // Open the images directory safely
  lfs_dir_t imagesDirHandle = {0};
  retval = filesystem.DirOpen(WATCHFACE_IMAGES_BASE_PATH, &imagesDirHandle);
  if (retval < 0) {
    inErrorState = true;
  }

  // Go through every file in the directory and log their values to the imageChangeTimestampsMins vector
  while (!inErrorState) {
    lfs_info itemInfo = {0};
    retval = filesystem.DirRead(&imagesDirHandle, &itemInfo);
    // If the read returns <0, an error occurred. If it returns 0, the end of the directory has been reached without finding 0000.bin.
    if (retval <= 0) {
      filesystem.DirClose(&imagesDirHandle);
      inErrorState = retval < 0;
      break;
    }
    // Attempt to parse the filename as a timestamp
    if (strcmp(&itemInfo.name[4], ".bin") == 0) {
      bool isNameAllDigits = true;
      for (int i = 0; i < 4; i++) {
        if (itemInfo.name[i] < '0' || itemInfo.name[i] > '9')
          isNameAllDigits = false;
      }
      if (!isNameAllDigits)
        continue;
      // handling --##.bin
      int minutes = std::atoi(&itemInfo.name[2]);
      // handling ##--.bin, destroy third char to read only first two chars
      itemInfo.name[2] = '.';
      int hours = std::atoi(itemInfo.name);
      if (minutes >= 60 || hours >= 24)
        continue;
      // Add the timestamp to the list of timestamps
      imageChangeTimestampsMins.push_back(hours * 60 + minutes);
    }
  }

  imageChangeTimestampsMins.shrink_to_fit();
  sort(imageChangeTimestampsMins.begin(), imageChangeTimestampsMins.end());

  if (inErrorState) {
    // TODO: Add a label saying what happened
  }

  // TODO: Add refresh properly

  Refresh();
}

void WatchFaceImages::Refresh() {
  if (inErrorState) {
    return;
  }

  currentTime =
    std::chrono::time_point_cast<std::chrono::minutes, std::chrono::system_clock, std::chrono::nanoseconds>(dateTime.CurrentDateTime());
  if (currentTime.IsUpdated()) {
    // Minute has changed, need to reevaluate the time
    unsigned int currentMinute = dateTime.Hours() * 60 + dateTime.Minutes();
    // Find largest timestamp that is still below the current timestamp
    uint16_t targetImageTime = 0;
    for (auto n : imageChangeTimestampsMins) {
      if (n > currentMinute)
        break;
      targetImageTime = n;
    }
    // Construct filename from minute timestamp
    imagePath = WATCHFACE_IMAGES_BASE_PATH;
    char imageTimeStr[5];
    imageTimeStr[0] = '0' + targetImageTime / 600;
    imageTimeStr[1] = '0' + targetImageTime / 60 % 10;
    imageTimeStr[2] = '0' + targetImageTime % 60 / 10;
    imageTimeStr[3] = '0' + targetImageTime % 60 % 10;
    imageTimeStr[4] = '\0';
    imagePath += imageTimeStr;
    imagePath += ".bin";
    lv_img_set_src(mainImage, imagePath.c_str());
  }
}

WatchFaceImages::~WatchFaceImages() {
  lv_obj_clean(lv_scr_act());
}

bool Pinetime::Applications::WatchFaceTraits<Pinetime::Applications::WatchFace::Images>::IsAvailable(Controllers::FS& filesystem) {
  int retval = 0;

  // Open the images directory safely
  lfs_dir_t imagesDirHandle = {0};
  retval = filesystem.DirOpen(WATCHFACE_IMAGES_BASE_PATH, &imagesDirHandle);
  if (retval < 0) {
    return false;
  }

  // Look for a file named 0000.bin. As long as that file exists, this application will function to some extent.
  while (true) {
    lfs_info itemInfo = {0};
    retval = filesystem.DirRead(&imagesDirHandle, &itemInfo);
    // If the read returns <0, an error occurred. If it returns 0, the end of the directory has been reached without finding 0000.bin.
    if (retval <= 0) {
      filesystem.DirClose(&imagesDirHandle);
      return false;
    }
    if (strcmp(itemInfo.name, "0000.bin") == 0) {
      filesystem.DirClose(&imagesDirHandle);
      return true;
    }
  }
}