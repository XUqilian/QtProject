#include<core/gsc.h>

const QStringList GSC::themeOptions = { "auto" , "light" , "dark" , "highContrast" , "custom"};

const QList<QColor> GSC::darkMod = {
    QColor(0x12, 0x12, 0x12), // 背景色 #121212 (Background)
    QColor(0xE0, 0xE0, 0xE0), // 主要文字 #E0E0E0 (Text Primary)
    QColor(0xB0, 0xB0, 0xB0), // 次要文字 #B0B0B0 (Text Secondary)
    QColor(0x00, 0x78, 0xD7), // 主色调 #0078D7 (Primary Color)
    QColor(0xFF, 0x98, 0x00), // 强调色 #FF9800 (Accent)
    QColor(0x44, 0x44, 0x44), // 边框色 #444444 (Border)
    QColor(0x1E, 0x1E, 0x1E)  // 卡片背景 #1E1E1E (Card)
};

const QList<QColor> GSC::lightMod = {
    QColor(0xFF, 0xFF, 0xFF), // 背景色 #FFFFFF (Background)
    QColor(0x33, 0x33, 0x33), // 主要文字 #333333 (Text Primary)
    QColor(0x66, 0x66, 0x66), // 次要文字 #666666 (Text Secondary)
    QColor(0x00, 0x78, 0xD7), // 主色调 #0078D7 (Primary Color)
    QColor(0xFF, 0x57, 0x22), // 强调色 #FF5722 (Accent)
    QColor(0xDD, 0xDD, 0xDD), // 边框色 #DDDDDD (Border)
    QColor(0xF5, 0xF5, 0xF5)  // 卡片背景 #F5F5F5 (Card)
};

const QList<QColor> GSC::highContrastMod = {
    QColor(0x00, 0x00, 0x00), // 背景色 #000000 (Background)
    QColor(0xFF, 0xFF, 0xFF), // 主要文字 #FFFFFF (Text Primary)
    QColor(0xCC, 0xCC, 0xCC), // 次要文字 #CCCCCC (Text Secondary)
    QColor(0x00, 0xBF, 0xFF), // 主色调 #00BFFF (Primary Color)
    QColor(0xFF, 0xFF, 0x00), // 强调色 #FFFF00 (Accent)
    QColor(0xFF, 0xFF, 0xFF), // 边框色 #FFFFFF (Border)
    QColor(0x11, 0x11, 0x11)  // 卡片背景 #111111 (Card)
};


QString GSC::getSystemThemePreference()
{
#if defined(Q_OS_WIN)
    QSettings settings(R"(HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Themes\Personalize)", QSettings::NativeFormat);
    return settings.value("AppsUseLightTheme", 1).toInt() == 0 ? "dark" : "light";

#elif defined(Q_OS_MACOS)
    QString result = QProcess::execute("osascript -e 'tell application \"System Events\" to get dark mode of appearance preferences'");
    return result.trimmed() == "true" ? "dark" : "light";

#elif defined(Q_OS_LINUX)
    // 简化判断
    QSettings settings(QDir::homePath() + "/.config/gtk-3.0/settings.ini", QSettings::IniFormat);
    QString theme = settings.value("Settings/gtk-theme-name", "").toString().toLower();
    if (theme.contains("dark") || theme.contains("black") || theme.contains("night"))
        return "dark";
    return "light";

#else
    // 默认使用 QPalette 检测
    QPalette palette = qApp->palette();
    return palette.color(QPalette::Window).lightness() < 128 ? "dark" : "light";
#endif
}

const QString GSC::DefaultLoadingPage = R"(
        import QtQuick
        Item {
            Text {
                text: "Loading ..."
                anchors.centerIn: parent
                color: "gray"
                font.pixelSize: 16
            }
        }
    )";

const QString GSC::DefaultLoadErrPage = R"(
        import QtQuick
        Item {
            Text {
                text: "Loading Failed!"
                anchors.centerIn: parent
                color: "gray"
                font.pixelSize: 16
            }
        }
    )";
