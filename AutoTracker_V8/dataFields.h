#ifndef DATAFIELDS_H
#define DATAFIELDS_H

#include <stdlib.h>

#include <QObject>

namespace DATAFIELDS
{
Q_NAMESPACE
enum DATA_FIELDS
{
    X_ERR = 0,
    X_DIR,
    Y_ERR,
    Y_DIR,
    AT_ON,
    SIGHT_VOLT,
    AT_GATE,
    SHAPPING_SIG,
    INP_CURRENT,

    _data_field_count
};

Q_ENUM_NS(DATA_FIELDS)
}


#endif // DATAFIELDS_H
