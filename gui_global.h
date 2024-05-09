#pragma once
#ifndef GUI_GLOBAL_H
#define GUI_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(GUI_LIBRARY)
#  define GUI_API_EXPORT
#else
#  define GUI_API_EXPORT
#endif

#endif // GUI_GLOBAL_H
