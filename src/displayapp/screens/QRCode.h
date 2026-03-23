#pragma once

#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "components/qrcode/QRCode.h"
#include "components/fs/FS.h"
#include "littlefs/lfs.h"
#include <nrf_log.h>

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class QRCode : public Screen {
      public:
        QRCode(Components::LittleVgl&, Controllers::FS& filesystem);
        ~QRCode() override;

      private:
        bool OnTouchEvent(TouchEvents event) override;
        bool OnButtonPushed() override;

        // Updates the QR code based on currentChosenEntry and foundQRCodeEntries
        void UpdateQRCode();

        // TODO: Look into not losing position with notifs and whatnot like Settings.h/cpp
        // TODO: Try using
        void OpenMenu();
        void CloseMenu();

        // Reads the data file containing available QR codes and populates foundQRCodeEntries
        // @return True if read succeeded, false otherwise.
        bool ReadDataFile();

        enum class ErrorMessageType : uint8_t {
          BadConfigFile,
          EmptyConfigFile,
          QRCodeGenFailed
        };

        void ShowErrorMessage(ErrorMessageType chosenMessage);
        void ShowErrorMessage(const char* errorString);
        void HideErrorMessage();

        // *Start variables are byte offsets into the config file
        struct ConfigFileEntryInfo {
          uint32_t nameStart;
          uint16_t nameLength;
          uint32_t contentStart;
          uint16_t contentLength;
        };

        // Path to the config file
        // Lines must follow following format to be valid:
        //
        // >>>listName
        // qrCodeContents
        // >>>listName
        // qrCodeContents
        //
        // e.g.:
        //
        // >>>Infinitime
        // https://github.com/InfiniTimeOrg/InfiniTime
        // >>>Important message
        // Hey!
        // Yeah, you!
        // You just lost the game!
        // >>>ISO 8859-1 help
        // https://en.wikipedia.org/wiki/ISO/IEC_8859-1#Code_page_layout
        //
        // Other notes:
        // QR code contents MUST be in ISO 8859-1 character encoding! Note that this standard is ASCII compatible (for printable chars)
        // QR code contents may have multiple lines
        // File must have LF endings (not CRLF) (carriage returns are allowed in QR code contents)
        // App opens with the first valid item in the file selected
        static constexpr char configPath[] = "/qrcodes.conf";

        // Maximum number of valid lines that can be read from the file
        static constexpr unsigned int maxQREntries = 20;

        // List of all found valid entries
        ConfigFileEntryInfo foundQRCodeEntries[maxQREntries];

        // Number of valid entries found in the file
        unsigned int numFoundQREntries = 0;
        bool areQREntriesValid = false;
        // Entry the user chose
        unsigned int currentChosenEntry;

        // Maximum name size (entry names are truncated above this size)
        static constexpr unsigned int truncateAboveNameSize = 30;
        // If a QR code's contents are above this size, mark its name in red in the list (0 to disable)
        // Default: 858 (Version 20, low error correction, byte mode)
        static constexpr unsigned int warnAboveContentSize = 858;
        // If a QR code's contents are above this size, ignore it entirely
        // Default: 2953 (Version 40, low error correction, byte mode)
        static constexpr unsigned int ignoreAboveContentSize = 2953;

        bool isMenuOpen;
        bool canCloseMenu;  // Set on app startup to not show a blank QR code

        lv_style_t qrCodeBGStyle;
        lv_obj_t* qrCode;
        lv_obj_t* errorText;
        lv_style_t errorTextStyle;

        Components::LittleVgl& lvgl;
        Controllers::FS& filesystem;
      };
    }

    template <>
    struct AppTraits<Apps::QRCode> {
      static constexpr Apps app = Apps::QRCode;
      // TODO: Change app icon to an actual qr code?
      static constexpr const char* icon = "Q";

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::QRCode(controllers.lvgl, controllers.filesystem);
      }

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      };
    };
  }
}