#ifndef __ppgmonitor_H__
#define __ppgmonitor_H__


#include <app.h>
#include <Elementary.h>
#include <system_settings.h>
#include <efl_extension.h>
#include <dlog.h>
#include <sensor.h>
#include <storage.h>
#include <privacy_privilege_manager.h>
#include <cairo.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <time.h>
#include <app_preference.h>

#ifdef  LOG_TAG
#undef  LOG_TAG
#endif
#define LOG_TAG "ppgmonitor"

#if !defined(PACKAGE)
#define PACKAGE "org.example.ppgmonitor"
#endif

#endif /* __ppgmonitor_H__ */
