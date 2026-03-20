#pragma once

#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "components/qrcode/QRCode.h"
#include <vector>

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

        void UpdateQRCode();

        // Reads the data file containing available QR codes and populates fileLineStarts
        // @return True if read succeeded, false otherwise.
        bool ReadDataFile();


        // 400 chars:
        // TODO: Fix extremely large codes??
        // Off by one error somewhere?
        static constexpr char thingy[2954] = "hey what are you doing here? you really shouldn't be here you know. it's also kinda rude to be looking at test strings without permission, you know? although this is on a public github repo, so idk... ok whatever ig you can look. this is just to test excessively long strings on hardware anyway, so nothing sensitive. but still, you could've asked before poking around. also you just lost the game :3";
        // 28 chars:
        // static constexpr char thingy[] = "https://youtu.be/dQw4w9WgXcQ";
        // Path to the config file
        // Compatible with both CRLF and LF line endings
        // Lines starting with a # will be ignored
        // Empty lines will be ignored
        // Lines must follow following format to be valid:
        // listName:::qrCodeContents
        // e.g.:
        // Rick roll:::https://www.youtube.com/watch?v=dQw4w9WgXcQ
        // App opens with the first valid item in the file selected
        static constexpr char configPath[] = "/qrcodes.conf";
        // Byte offsets inside the file to each qr code contents
        // Capped size to prevent too much data being loaded at once
        static constexpr unsigned int maxQREntries = 50;
        unsigned int fileLineStarts[maxQREntries];
        unsigned int numFoundQREntries = 0;
        unsigned int currentChosenEntry = 0;

        // If a QR code's contents are above this size, mark its name in red in the list (0 to disable) (default: 861)
        static constexpr unsigned int warnAboveContentSize = 861;  // version 20 code
        // If a QR code's contents are above this size, ignore it entirely (default: 2956)
        static constexpr unsigned int ignoreAboveContentSize = 2956;  // version 40 code

        lv_style_t qrCodeBGStyle;
        lv_obj_t* qrCode;

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