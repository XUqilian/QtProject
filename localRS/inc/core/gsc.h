// global static config
#ifndef GSC_H
#define GSC_H

// #include <QtCore>
// #include <QtQml>
// #include <QtGui>

#include <QColor>
#include <QSettings>
// #include <QQmlEngine>
// #include <QFile>
// #include <QRegularExpression>
// #include <QDir>



namespace GSC{

    // theme模式
    extern const QStringList themeOptions;
    // theme模式颜色配置
    extern const QList<QColor> darkMod;
    extern const QList<QColor> lightMod;
    extern const QList<QColor>  highContrastMod;

    extern QString getSystemThemePreference();


    extern const QString DefaultLoadingPage;

    extern const QString DefaultLoadErrPage;

/*    c++不需要这种内容
    class Gsc{
    private:
        // 1. 禁止构造和拷贝（确保不能被实例化）
        Gsc() = delete;
        Gsc(const Gsc&) = delete;
        Gsc& operator=(const Gsc&) = delete;

    public:
    };
*/
}
#endif // GSC_H
