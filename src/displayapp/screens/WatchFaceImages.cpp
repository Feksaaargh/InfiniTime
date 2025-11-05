#include "displayapp/screens/WatchFaceImages.h"

namespace {
  // Must include LVGL disk and trailing slash
  const char* watchfaceImagesBasePath = "F:/images_watchface/";
}

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

  int retval;
  std::string configLocation = &watchfaceImagesBasePath[2];
  configLocation += "config.txt";
  lfs_file_t configFileHandle = {0};
  retval = filesystem.FileOpen(&configFileHandle, configLocation.c_str(), LFS_O_RDONLY);
  if (retval < 0) {
    ShowError("CONFIG OPEN", retval);
    return;
  }

  // Default distance is 1 higher than maximum possible distance
  int fileDistance = 1440;
  char closestFileName[LFS_NAME_MAX + 1] = {0};
  uint8_t buffer[LFS_NAME_MAX + 1] = {0};
  const int currentMinute = (dateTime.Hours() * 60) + dateTime.Minutes();
  uint16_t lineNum = 1;
  int configFileError = 0;
  bool foundTimestampZero = false;
  while (true) {
    // Format:
    // 0000 midnight.bin
    // 1330 halfpast1pm.bin
    retval = filesystem.FileRead(&configFileHandle, buffer, 5);
    if (retval == 0) {
      break;
    }
    // if retval is <0 then read failed, if retval is 1-4 then didn't get enough data
    if (retval < 5) {
      configFileError = retval < 0 ? retval : 1;
      break;
    }
    for (int i = 0; i < 4; i++) {
      if (buffer[i] < '0' || buffer[i] > '9') {
        configFileError = 2;
        break;
      }
    }
    if (buffer[4] != ' ') {
      configFileError = 3;
      break;
    }
    // Buffer now definitely has contents in format "1234 \0"
    const uint16_t entryTimestamp = std::atoi(reinterpret_cast<const char*>(&buffer));
    const uint16_t minutes = entryTimestamp % 100;
    const uint16_t hours = entryTimestamp / 100;
    if (minutes == 0 && hours == 0) {
      foundTimestampZero = true;
    }
    if (minutes >= 60 || hours >= 24) {
      configFileError = 4;
      break;
    }
    // Figure out if this is closer than the previous closest item
    const int tmpFileDistance = currentMinute - (hours * 60 + minutes);
    int nameLen = 0;
    // Read filename from file, exiting if name is too long
    while (true) {
      retval = filesystem.FileRead(&configFileHandle, &buffer[nameLen], 1);
      if (retval == 0 || buffer[nameLen] == '\n') {
        buffer[nameLen] = '\0';
        break;
      }
      if (retval < 0 || nameLen == LFS_NAME_MAX) {
        configFileError = retval < 0 ? retval : 5;
        break;
      }
      nameLen++;
    }
    if (nameLen == 0)
      configFileError = 6;
    if (configFileError != 0)
      break;
    if (tmpFileDistance >= 0 && tmpFileDistance <= fileDistance) {
      strncpy(closestFileName, reinterpret_cast<const char*>(&buffer), nameLen+1);
      fileDistance = tmpFileDistance;
    }
    lineNum++;
  }
  filesystem.FileClose(&configFileHandle);
  if (configFileError == 0 && !foundTimestampZero) {
    configFileError = 7;
  }

  if (configFileError != 0) {
    // <0 = LittleFS error
    // 1 = Too few chars in timestamp
    // 2 = Timestamp had non-digit characters
    // 3 = Timestamp was not followed by a space
    // 4 = Minute or hour was too high
    // 5 = File name was too long
    // 6 = Empty filename for entry
    // 7 = Did not find any entry for timestamp 0000
    std::string configErrorMsg = "BAD CONFIG: L";
    configErrorMsg += std::to_string(lineNum);
    ShowError(configErrorMsg.c_str(), configFileError);
    return;
  }

  // Confirm file exists before using it
  imagePath = watchfaceImagesBasePath;
  imagePath += closestFileName;
  lfs_file_t fileHandle = {0};
  retval = filesystem.FileOpen(&fileHandle, &imagePath.c_str()[2], LFS_O_RDONLY);
  filesystem.FileClose(&fileHandle);
  if (retval != 0) {
    ShowError("IMAGE OPENING", retval);
    return;
  }
  lv_obj_set_hidden(mainImage, false);
  lv_obj_set_hidden(errorMessage, true);
  lv_img_set_src(mainImage, imagePath.c_str());
}

void WatchFaceImages::ShowError(const char* errorDesc, int errorNum) const {
  lv_label_set_text_fmt(errorMessage, "--ERROR--\n%s\n%i", errorDesc, errorNum);
  lv_obj_align(errorMessage, nullptr, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_hidden(mainImage, true);
  lv_obj_set_hidden(errorMessage, false);
}


WatchFaceImages::~WatchFaceImages() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

bool Pinetime::Applications::WatchFaceTraits<Pinetime::Applications::WatchFace::Images>::IsAvailable(Controllers::FS& filesystem) {
  std::string configLocation = &watchfaceImagesBasePath[2];
  configLocation += "config.txt";
  lfs_file_t configFileHandle = {0};
  const int retval = filesystem.FileOpen(&configFileHandle, configLocation.c_str(), LFS_O_RDONLY);
  if (retval < 0) {
    printf("Failed to open config file: %i\n", retval);
    return false;
  }
  filesystem.FileClose(&configFileHandle);
  return true;
}