#include "QRCode.h"

using namespace Pinetime::Applications::Screens;

QRCode::QRCode(Components::LittleVgl& lvgl, Controllers::FS& filesystem)
  : lvgl{lvgl},
    filesystem{filesystem} {

  isMenuOpen = false;
  areQREntriesValid = false;
  numFoundQREntries = 0;

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

  OpenMenu();
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

bool QRCode::OnButtonPushed() {
  if (isMenuOpen && canCloseMenu) {
    CloseMenu();
    return true;
  }
  return false;
}

void QRCode::UpdateQRCode() {
  assert(areQREntriesValid);
  if (!areQREntriesValid) {
    return;
  }

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

  char* qrContents = new char[chosenEntry.contentLength + 1];

  retval = filesystem.FileRead(&configHandle, reinterpret_cast<uint8_t*>(qrContents), chosenEntry.contentLength);
  if (retval < 0 || retval < chosenEntry.contentLength) {
    delete[] qrContents;
    filesystem.FileClose(&configHandle);
    ShowErrorMessage(ErrorMessageType::QRCodeGenFailed);
    return;
  }
  qrContents[chosenEntry.contentLength] = '\0';
  filesystem.FileClose(&configHandle);

  Pinetime::Tools::UpdateQRCodeCanvas(qrCode, qrContents);
  delete[] qrContents;
}

void QRCode::OpenMenu() {
  bool result = ReadDataFile();
  if (!result) {
    ShowErrorMessage(ErrorMessageType::BadConfigFile);
    return;
  }
  // TODO: Implement
  // Reference Settings.cpp/Settings.h for how to make screens of text

  isMenuOpen = true;
}

void QRCode::CloseMenu() {
  if (!isMenuOpen)
    return;
  // TODO: Implement
  isMenuOpen = false;
}

bool QRCode::ReadDataFile() {
  areQREntriesValid = false;
  numFoundQREntries = 0;

  int retval;
  lfs_file_t configHandle;

  retval = filesystem.FileOpen(&configHandle, configPath, LFS_O_RDONLY);
  if (retval < 0) {
    NRF_LOG_INFO("[QRCode] Failed to open data file\n");
    return false;
  }

  static constexpr unsigned int newEntryIndicatorSize = 4;
  static constexpr char newEntryIndicator[newEntryIndicatorSize + 1] = "\n>>>";
  char recentChars[newEntryIndicatorSize];

  // File must begin with a new entry
  retval = filesystem.FileRead(&configHandle, reinterpret_cast<uint8_t*>(recentChars + 1), newEntryIndicatorSize - 1);
  if (retval != newEntryIndicatorSize - 1 || memcmp(recentChars + 1, newEntryIndicator + 1, newEntryIndicatorSize - 1) != 0) {
    filesystem.FileClose(&configHandle);
    NRF_LOG_INFO("[QRCode] File didn't start with entry\n");
    return false;
  }

  // Loop over every byte in the file
  uint32_t curByteOffset = newEntryIndicatorSize - 1;
  bool readingEntryName = true;
  ConfigFileEntryInfo curEntryInfo = {curByteOffset, 0, 0, 0};
  while (true) {
    char curChar;

    // Read in a character
    retval = filesystem.FileRead(&configHandle, reinterpret_cast<uint8_t*>(&curChar), 1);
    if (retval < 0) {
      filesystem.FileClose(&configHandle);
      NRF_LOG_INFO("[QRCode] Read error\n");
      return false;
    }
    // Check if got EOF
    if (retval == 0) {
      filesystem.FileClose(&configHandle);
      // If ended during file name entry
      if (readingEntryName) {
        NRF_LOG_INFO("[QRCode] EOF while reading name\n");
        return false;
      }
      // Ended in a valid spot, finalize current unfinished entry
      // Disallow 0 size entries
      if (curEntryInfo.contentLength == 0 || curEntryInfo.nameLength == 0) {
        NRF_LOG_INFO("[QRCode] Found a 0 size entry (EOF)\n");
        return false;
      }
      if (curEntryInfo.contentLength <= ignoreAboveContentSize) {
        foundQRCodeEntries[numFoundQREntries] = curEntryInfo;
        numFoundQREntries++;
      }
      NRF_LOG_INFO("[QRCode] Everything succeeded (EOF)\n");
      areQREntriesValid = true;
      return true;
    }

    // Update history
    for (unsigned int i = 0; i < newEntryIndicatorSize - 1; i++)
      recentChars[i] = recentChars[i + 1];
    recentChars[newEntryIndicatorSize - 1] = curChar;

    // Add to current size
    if (readingEntryName) {
      if (curEntryInfo.nameLength < truncateAboveNameSize)
        curEntryInfo.nameLength++;
    } else {
      if (curEntryInfo.contentLength <= ignoreAboveContentSize)
        curEntryInfo.contentLength++;
    }

    // Check if on new entry
    if (!readingEntryName && memcmp(recentChars, newEntryIndicator, newEntryIndicatorSize) == 0) {
      // Finalize existing entry
      if (curEntryInfo.contentLength <= ignoreAboveContentSize) {
        curEntryInfo.contentLength -= newEntryIndicatorSize;
        // Disallow 0 size entries
        if (curEntryInfo.contentLength == 0 || curEntryInfo.nameLength == 0) {
          filesystem.FileClose(&configHandle);
          NRF_LOG_INFO("[QRCode] Found a 0 size entry\n");
          return false;
        }
        foundQRCodeEntries[numFoundQREntries] = curEntryInfo;
        numFoundQREntries++;
      }
      if (numFoundQREntries == maxQREntries) {
        filesystem.FileClose(&configHandle);
        NRF_LOG_INFO("[QRCode] Everything succeeded (max QR entries)\n");
        areQREntriesValid = true;
        return true;
      }
      curEntryInfo = {curByteOffset+1, 0, 0, 0};
      readingEntryName = true;
    }

    // Check if on newline
    if (curChar == '\n') {
      if (readingEntryName) {
        readingEntryName = false;
        curEntryInfo.nameLength--;
        curEntryInfo.contentStart = curByteOffset+1;
      }
    }

    curByteOffset++;
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