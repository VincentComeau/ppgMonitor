#ifndef __myservice_H__
#define __myservice_H__

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
#include <time.h>
#include <stdio.h>
#include <app_common.h>
#include <glib.h>
#include <device/power.h>

#ifdef  LOG_TAG
#undef  LOG_TAG
#endif
#define LOG_TAG "myservice"

Eina_Bool timed_listener(void *data);

Eina_Bool recording_duration(void *data);


#endif /* __myservice_H__ */
