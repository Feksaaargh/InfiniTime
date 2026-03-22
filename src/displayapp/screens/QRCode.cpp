#include "QRCode.h"

using namespace Pinetime::Applications::Screens;

QRCode::QRCode(Components::LittleVgl& lvgl, Controllers::FS& filesystem)
  : lvgl{lvgl},
    filesystem{filesystem} {

  isMenuOpen = false;

  lv_style_init(&qrCodeBGStyle);
  lv_style_set_bg_color(&qrCodeBGStyle, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_style_set_radius(&qrCodeBGStyle, LV_STATE_DEFAULT, 0);

  lv_obj_t* qrCodeBG = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_add_style(qrCodeBG, LV_OBJ_PART_MAIN, &qrCodeBGStyle);
  lv_obj_set_width(qrCodeBG, LV_HOR_RES);
  lv_obj_set_height(qrCodeBG, LV_VER_RES);
  lv_obj_align(qrCodeBG, nullptr, LV_ALIGN_CENTER, 0, 0);

  lv_style_init(&errorTextStyle);
  lv_style_set_text_color(&errorTextStyle, LV_STATE_DEFAULT, LV_COLOR_WHITE);

  errorText = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_add_style(errorText, LV_LABEL_PART_MAIN, &errorTextStyle);
  lv_label_set_long_mode(errorText, LV_LABEL_LONG_BREAK);
  lv_label_set_align(errorText, LV_LABEL_ALIGN_CENTER);
  lv_obj_set_hidden(errorText, true);

  // Canvas is 5/6 the size of the screen on its shortest axis (200px on a 240px screen). The rest acts as the quiet zone.
  qrCode = Pinetime::Tools::CreateQRCodeCanvas(lv_scr_act(),
                                               std::min(LV_HOR_RES, LV_VER_RES) * 5 / 6,
                                               LV_COLOR_BLACK,
                                               LV_COLOR_WHITE);
  lv_obj_align(qrCode, nullptr, LV_ALIGN_CENTER, 0, 0);

  // Read file and populate QR code (either now or later if the first entry is too long)
  const bool readSuccess = ReadDataFile();
  if (!readSuccess) {
    ShowErrorMessage(ErrorMessageType::BadConfigFile);
  } else if (numFoundQREntries == 0) {
    ShowErrorMessage(ErrorMessageType::EmptyConfigFile);
  } else {
    // Read succeeded
    for (unsigned int i = 0; i < numFoundQREntries; i++) {
      const auto entry = foundQRCodeEntries[i];
      printf("IDX %u\nSTART: %lu (%u)\nCONTENT: %lu (%u)\n\n", i, entry.nameStart, entry.nameLength, entry.contentStart, entry.contentLength);
    }

    currentChosenEntry = 0;
    if (foundQRCodeEntries[currentChosenEntry].contentLength > warnAboveContentSize) {
      UpdateQRCodeLater();
    } else {
      UpdateQRCode();
    }
  }
}

QRCode::~QRCode() {
  Pinetime::Tools::DeleteQRCodeCanvas(qrCode);
  lv_style_reset(&qrCodeBGStyle);
  lv_style_reset(&errorTextStyle);
  lv_obj_clean(lv_scr_act());
}

bool QRCode::OnTouchEvent(TouchEvents event) {
  if (event == TouchEvents::LongTap && !isMenuOpen) {
    OpenMenu();
    return true;
  }
  return false;
}

void QRCode::UpdateQRCodeLater() {
  // TODO: Figure out why LV_TASK_PRIO_LOW is required (or if it even is)
  lv_task_t* updateLaterTask = lv_task_create(UpdateQRCodeLaterCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_LOW, this);
  lv_task_set_repeat_count(updateLaterTask, 1);
}

void QRCode::UpdateQRCodeLaterCallback(lv_task_t* task) {
  static_cast<QRCode*>(task->user_data)->UpdateQRCode();
}

void QRCode::UpdateQRCode() {
  int retval;
  lfs_file_t configHandle;

  const ConfigFileEntryInfo chosenEntry = foundQRCodeEntries[currentChosenEntry];

  retval = filesystem.FileOpen(&configHandle, configPath, LFS_O_RDONLY);
  if (retval < 0) {
    ShowErrorMessage(ErrorMessageType::BadConfigFile);
    return;
  }

  retval = filesystem.FileSeek(&configHandle, chosenEntry.contentStart);
  if (retval < 0) {
    ShowErrorMessage(ErrorMessageType::QRCodeGenFailed);
    return;
  }

  auto qrContents = std::make_shared<char>(chosenEntry.contentLength + 1);
  qrContents.get()[chosenEntry.contentLength] = '\0';
  retval = filesystem.FileRead(&configHandle, reinterpret_cast<uint8_t*>(qrContents.get()), chosenEntry.contentLength);
  if (retval < 0 || retval < chosenEntry.contentLength) {
    ShowErrorMessage(ErrorMessageType::QRCodeGenFailed);
    return;
  }

  Pinetime::Tools::UpdateQRCodeCanvas(qrCode, qrContents.get());
}

void QRCode::OpenMenu() {
  bool result = ReadDataFile();
  if (!result) {
    ShowErrorMessage(ErrorMessageType::BadConfigFile);
    return;
  }
  // TODO: Implement
  // Reference Settings.cpp/Settings.h for how to make screens of text
}

void QRCode::CloseMenu() const {
  if (!isMenuOpen)
    return;
  // TODO: Implement
}

bool QRCode::ReadDataFile() {
  int retval;
  lfs_file_t configHandle;

  retval = filesystem.FileOpen(&configHandle, configPath, LFS_O_RDONLY);
  if (retval < 0)
    return false;

  static constexpr unsigned int newEntryIndicatorSize = 4;
  static constexpr char newEntryIndicator[newEntryIndicatorSize + 1] = "\n>>>";
  char recentChars[newEntryIndicatorSize];

  // File must begin with a new entry
  retval = filesystem.FileRead(&configHandle, reinterpret_cast<uint8_t*>(recentChars + 1), newEntryIndicatorSize - 1);
  if (retval != newEntryIndicatorSize - 1 || memcmp(recentChars + 1, newEntryIndicator + 1, newEntryIndicatorSize - 1) != 0) {
    filesystem.FileClose(&configHandle);
    return false;
  }

  // Loop over every byte in the file
  unsigned int curLine = 0;
  uint32_t curByteOffset = newEntryIndicatorSize - 1;
  bool readingEntryName = true;
  ConfigFileEntryInfo curEntryInfo = {curByteOffset, 0, 0, 0};
  while (true) {
    char curChar;

    // Read in a character
    retval = filesystem.FileRead(&configHandle, reinterpret_cast<uint8_t*>(&curChar), 1);
    if (retval < 0) {
      filesystem.FileClose(&configHandle);
      return false;
    }
    // Check if got EOF
    if (retval == 0) {
      filesystem.FileClose(&configHandle);
      // If ended during file name entry
      if (readingEntryName) {
        return false;
      }
      // Ended in a valid spot, finalize current unfinished entry
      // Disallow 0 size entries
      if (curEntryInfo.contentLength == 0) {
        return false;
      }
      if (curEntryInfo.contentLength <= ignoreAboveContentSize) {
        foundQRCodeEntries[numFoundQREntries] = curEntryInfo;
        numFoundQREntries++;
      }
      return true;
    }
    // Read succeeded
    curByteOffset++;

    // Update history
    for (unsigned int i = 0; i < newEntryIndicatorSize - 1; i++)
      recentChars[i] = recentChars[i + 1];
    recentChars[newEntryIndicatorSize - 1] = curChar;

    // Check if on newline
    if (curChar == '\n') {
      if (readingEntryName) {
        readingEntryName = false;
        curEntryInfo.contentStart = curByteOffset+1;
      }
      continue;
    }

    // Check if on new entry
    if (!readingEntryName && memcmp(recentChars, newEntryIndicator, newEntryIndicatorSize) == 0) {
      // Finalize existing entry
      // Disallow 0 size entries
      if (curEntryInfo.contentLength == 0) {
        filesystem.FileClose(&configHandle);
        return false;
      }
      // Add complete entry and make new one
      if (curEntryInfo.contentLength <= ignoreAboveContentSize) {
        curEntryInfo.contentLength -= newEntryIndicatorSize;
        // Disallow entries with no contents
        if (curEntryInfo.contentLength == 0) {
          filesystem.FileClose(&configHandle);
          return false;
        }
        foundQRCodeEntries[numFoundQREntries] = curEntryInfo;
        numFoundQREntries++;
      }
      if (numFoundQREntries == maxQREntries) {
        filesystem.FileClose(&configHandle);
        return true;
      }
      curEntryInfo = {curByteOffset, 0, 0, 0};
      readingEntryName = true;
      continue;
    }

    // On data character
    if (readingEntryName) {
      if (curEntryInfo.nameLength < truncateAboveNameSize)
        curEntryInfo.nameLength++;
    } else {
      if (curEntryInfo.contentLength <= ignoreAboveContentSize)
        curEntryInfo.contentLength++;
    }
  }
}

void QRCode::ShowErrorMessage(const ErrorMessageType chosenMessage) {
  switch (chosenMessage) {
    case ErrorMessageType::BadConfigFile: {
      std::string errorMessage = "Failed to read config. Is there a valid config file at '";
      errorMessage += configPath;
      errorMessage += "'?";
      ShowErrorMessage(errorMessage.c_str());
      break;
    }
    case ErrorMessageType::EmptyConfigFile:
      ShowErrorMessage("Config file has no valid lines");
      break;
    case ErrorMessageType::QRCodeGenFailed:
      ShowErrorMessage("QR code creation failed");
      break;
  }
}

void QRCode::ShowErrorMessage(const char* errorString) {
  lv_style_set_bg_color(&qrCodeBGStyle, LV_STATE_DEFAULT, LV_COLOR_BLACK);

  lv_label_set_text(errorText, errorString);
  lv_obj_set_width(errorText, LV_HOR_RES * 9 / 10);

  // Put error message at top center of screen, pushed down by 1/10 of the screen
  lv_obj_align(errorText, nullptr, LV_ALIGN_CENTER, 0, 0);

  lv_obj_set_hidden(qrCode, true);
  lv_obj_set_hidden(errorText, false);
}

void QRCode::HideErrorMessage() {
  lv_style_set_bg_color(&qrCodeBGStyle, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_obj_set_hidden(errorText, true);
  lv_obj_set_hidden(qrCode, false);
}