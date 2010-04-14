#include "drishtislicer.h"

int main(int argv, char **args)
{
    QApplication app(argv, args);

    DrishtiSlicer mainWindow;
    mainWindow.show();

    return app.exec();
}
