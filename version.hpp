// A++ Programming Language
// Author: OXOP, and probably more cuz of PR and stuff

#ifndef VERSION_HPP
#define VERSION_HPP

#define APP_VERSION_MAJOR 0
#define APP_VERSION_MINOR 1
#define APP_VERSION_PATCH 0
#define APP_VERSION_STRING "0.1.0"
#define APP_LANGUAGE_NAME "A++"
#define APP_BUILD_DATE __DATE__
#define APP_BUILD_TIME __TIME__

#include <string>

inline std::string getVersionString() {
    return std::string(APP_LANGUAGE_NAME) + " v" + APP_VERSION_STRING;
}

inline std::string getFullVersionString() {
    return std::string(APP_LANGUAGE_NAME) + " version " + APP_VERSION_STRING +
           " (built " + APP_BUILD_DATE + " " + APP_BUILD_TIME + ")";
}

#endif // VERSION_HPP
