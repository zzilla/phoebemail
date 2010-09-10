
#ifndef DVR_EXPORT_H
#define DVR_EXPORT_H


#ifdef WIN32

#ifdef DVR_EXPORTS
#define DVR_API __declspec(dllexport)
#else	//#ifdef DVR_EXPORTS
#define DVR_API __declspec(dllimport)
#endif

#else

#define DVR_API

#endif


#include "./kernel/afkplugin.h"

extern "C" DVR_API afk_plugin_info_t plugin_info;

#endif /* DVR_EXPORT_H */

