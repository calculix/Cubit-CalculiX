#ifndef USEROPTIONSEXPORT_HPP
#define USEROPTIONSEXPORT_HPP

#include <QtCore/qglobal.h>

#if defined(calculix_useroptions_EXPORTS)
#define CALCULIX_USEROPTIONS_EXPORT Q_DECL_EXPORT
#else
#define CALCULIX_USEROPTIONS_EXPORT Q_DECL_IMPORT
#endif

#endif
