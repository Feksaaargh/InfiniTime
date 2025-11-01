#pragma once

#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "components/datetime/DateTimeController.h"
#include "utility/DirtyValue.h"
#include <cstring>
#include <vector>

// Must include trailing slash
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
        void ShowError(const char* errorDesc, int errorNum) const;

        lv_obj_t* mainImage;
        lv_obj_t* errorMessage;
        std::string imagePath;

        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::minutes>> currentTime;

        lv_task_t* taskRefresh;
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