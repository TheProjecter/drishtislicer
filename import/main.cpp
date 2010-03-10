#include "drishtiimport.h"

#include <magick/ImageMagick.h>

int main(int argv, char **args)
{
    QApplication app(argv, args);

    DrishtiImport mainWindow;
    mainWindow.show();

#if defined(Q_OS_MAC)
    //
    // Under OSX, tell imagemagick where its plugins are by setting the relevant
    // environment variables.
    //
    QDir appd=QCoreApplication::applicationDirPath();
    appd.cdUp();
    appd.cdUp();
    appd.cdUp();
    appd.cd("Shared");
    appd.cd("Plugins");
    appd.cd("ImageMagick-6.4.9");
    appd.cd("modules-Q16");

    QString coders = appd.absoluteFilePath("coders");
    QString filters = appd.absoluteFilePath("filters");

    setenv("MAGICK_CODER_MODULE_PATH", coders.toAscii().data(), 1);
    setenv("MAGICK_CODER_FILTER_PATH", filters.toAscii().data(), 1);
#elif defined(Q_OS_LINUX)
    QDir appd=QCoreApplication::applicationDirPath();
    appd.cdUp();
    appd.cd("lib");
    appd.cd("ImageMagick-6.5.4");
    appd.cd("modules-Q16");

    QString coders = appd.absoluteFilePath("coders");
    QString filters = appd.absoluteFilePath("filters");

    setenv("MAGICK_CODER_MODULE_PATH", coders.toAscii().data(), 1);
    setenv("MAGICK_CODER_FILTER_PATH", filters.toAscii().data(), 1);
#endif // defined(Q_OS_MAC)
    return app.exec();
}
