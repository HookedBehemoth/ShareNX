#pragma once

#include <string>
#include <vector>

namespace upload {

    class HosterConfig {
      private:
        bool img, vid;
        std::string url;

      public:
        HosterConfig(const std::string &name);
    };

    std::vector<std::string> GetHosterNameList();

    std::string GenerateImgurConfig();;

}
