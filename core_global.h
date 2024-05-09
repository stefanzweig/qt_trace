#pragma once
#ifndef CORE_GLOBAL_H
#define CORE_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(CORE_LIBRARY)
#  define API_EXPORT Q_DECL_EXPORT
#else
#  define API_EXPORT
//#  define API_EXPORT Q_DECL_IMPORT
#endif

#ifdef Q_OS_WIN
#   define PATH_LIST_SEPARATOR ";"
#else
#   define PATH_LIST_SEPARATOR ":"
#endif

#endif // CORE_GLOBAL_H
