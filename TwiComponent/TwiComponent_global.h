#ifndef TWICOMPONENT_GLOBAL_H
#define TWICOMPONENT_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(TWICOMPONENT_LIBRARY)
#  define TWICOMPONENTSHARED_EXPORT Q_DECL_EXPORT
#else
#  define TWICOMPONENTSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // TWICOMPONENT_GLOBAL_H
