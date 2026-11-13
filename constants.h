#ifndef CONSTANTS_H
#define CONSTANTS_H
#include <QColor>

namespace Constants {
    // Download limits
    inline constexpr const char * RATE_LIMIT = "3M";
    inline constexpr int SLEEP_MIN = 3;
    inline constexpr int SLEEP_MAX = 10;

    // UI colors
    inline constexpr QColor DARK_ERROR_COLOR = QColor(255, 100, 100);
    inline constexpr QColor DARK_DOWNLOAD_COLOR = QColor(255, 214, 102);
    inline constexpr QColor DARK_FINISHED_COLOR = QColor(144, 238, 144);

    inline constexpr QColor LIGHT_ERROR_COLOR = QColor(200, 0, 0);
    inline constexpr QColor LIGHT_DOWNLOAD_COLOR = QColor(184, 134, 11);
    inline constexpr QColor LIGHT_FINISHED_COLOR = QColor(34, 139, 34);
}

#endif // CONSTANTS_H
