#ifndef IDOS_TREE_ACCESS_H
#define IDOS_TREE_ACCESS_H

#include <QObject>

#include "idos_core.h"

class CORE_EXPORT IDOSTreeAccess : public QObject
{
    Q_OBJECT
public:
    explicit IDOSTreeAccess(QObject* parent = nullptr);
    ~IDOSTreeAccess() override;
};

#endif
