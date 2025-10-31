#pragma once

#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "components/datetime/DateTimeController.h"
#include "utility/DirtyValue.h"
#include <cstring>
#include <vector>

#define WATCHFACE_IMAGES_BASE_PATH "F:/images_watchface/"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class WatchFaceImages : public Screen {
      public:
        WatchFaceImages(Controllers::DateTime& dateTime, Controllers::FS& filesystem);
        ~WatchFaceImages() override;

        void Refresh() override;

      private:
        int FindNextHighestImage();

        lv_obj_t* mainImage;
        std::string imagePath;

        // If inErrorState is set, filesystem reading failed during construction.
        bool inErrorState;
        // In minutes, for each day. Each timestamp is in range [0,1399]. Is sorted in ascending order.
        std::vector<uint16_t> imageChangeTimestampsMins;
        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::minutes>> currentTime;

        Controllers::DateTime& dateTime;
        Controllers::FS& filesystem;
      };
    }

    template <>
    struct WatchFaceTraits<WatchFace::Images> {
      static constexpr WatchFace watchFace = WatchFace::Images;
      static constexpr const char* name = "Images";

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::WatchFaceImages(controllers.dateTimeController, controllers.filesystem);
      };

      static bool IsAvailable(Controllers::FS& filesystem);
    };
  }
}