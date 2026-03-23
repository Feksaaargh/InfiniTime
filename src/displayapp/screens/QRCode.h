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

        // Creates an LVGL one-shot task so the QR code update happens later.
        // Used to improve app startup time at the cost of not showing the QR code during the slide-in animation.
        void UpdateQRCodeLater();
        static void UpdateQRCodeLaterCallback(lv_task_t* task);

        // Updates the QR code based on currentChosenEntry and foundQRCodeEntries
        void UpdateQRCode();

        void OpenMenu();
        void CloseMenu() const;

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

        struct ConfigFileEntryInfo {
          uint32_t nameStart;
          uint16_t nameLength;
          uint32_t contentStart;
          uint16_t contentLength;
        };

        // 400 chars:
        static constexpr char thingy[] =
          "hey what are you doing here? you really shouldn't be here you know. it's also kinda rude to be looking at test strings without permission, you know? although this is on a public github repo, so idk... ok whatever ig you can look. this is just to test excessively long strings on hardware anyway, so nothing sensitive. but still, you could've asked before poking around. also you just lost the game :3";
        // 28 chars:
        // static constexpr char thingy[] = "https://youtu.be/dQw4w9WgXcQ";

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

        // List of all found valid entries. *Start variables are byte offsets into the config file.
        ConfigFileEntryInfo foundQRCodeEntries[maxQREntries];

        // Number of valid entries found in the file
        unsigned int numFoundQREntries = 0;
        // Entry the user chose
        unsigned int currentChosenEntry = 0;

        // Maximum name size (entry names are truncated above this size)
        static constexpr unsigned int truncateAboveNameSize = 30;
        // If a QR code's contents are above this size, mark its name in red in the list (0 to disable) (default: 858)
        static constexpr unsigned int warnAboveContentSize = 858; // version 20, low error correction, byte mode
        // If a QR code's contents are above this size, ignore it entirely (default: 2953)
        static constexpr unsigned int ignoreAboveContentSize = 2953; // version 40, low error correction, byte mode

        bool isMenuOpen;

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