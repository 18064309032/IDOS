#ifndef IDOS_APPLICATION_H
#define IDOS_APPLICATION_H

#include <QApplication>
#include "idos_app.h"

class APP_EXPORT IDOSApplication : public QApplication
{
    Q_OBJECT
public:
    IDOSApplication(int &argc, char **argv);
    ~IDOSApplication();
};


#endif //IDOS_APPLICATION_H
