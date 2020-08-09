#include <tizen.h>
#include <service_app.h>
#include <sensor.h>
#include <stdlib.h>

#include "ppgService.h"


#define CSV_FILE_NAME "ppgData.csv"
#define CSV_TITLE "PPG, PPG_TIME, MS, ACC_X, ACC_Y, ACC_Z, ACC_TIME, COUNT\n"
#define PPGMONITOR_APP_ID "org.example.ppgmonitor" // an ID of the UI application of our package
#define BUFSIZE 500
#define BIG_NUM 100000
#define STRNCMP_LIMIT 500

#define TIME_BETWEEN_RECORDINGS 300 //In seconds
#define RECORDING_TIME 60          //In seconds
#define SENSOR_INTERVAL 40         //In milliseconds
#define ACCELEROMETER_INTERVAL 40  //In milliseconds


struct ppg_data {

	int ppg_time;
	int ppg_value;
	int ppg_count;
	unsigned long long millisec;

} ppg[BIG_NUM];

struct acc_data {

	float acc_x;
	float acc_y;
	float acc_z;
	int acc_count;
	int acc_time;

} acc[BIG_NUM];



typedef struct appdata {

	sensor_h PPG_Sensor;
	sensor_h Acc_Sensor;
	sensor_listener_h PPG_Listener;
	sensor_listener_h Acc_Listener;
	sensor_recorder_option_h Sensor_option;
	sensor_event_cb sensor_cb;
	Ecore_Timer *main_timer;
	Ecore_Timer *recording_timer;
	sensor_event_s *ppgData;
	FILE* fp;

	unsigned long long start_time;
	int ppg_count;
	int acc_count;
	int rec_count;


} appdata_s;


void open_csv_file(appdata_s *ad){


	char *data_path = "/opt/usr/home/owner/media/Documents/";
	int size = strlen(data_path) + 20;
	char * path = malloc(sizeof(char) * size);
	char temp[1024];

	strcpy(path, data_path);

	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	sprintf(temp, "ppg_%02d_%02d_%02dh%02d.csv",tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min);
	dlog_print(DLOG_INFO, LOG_TAG, "%s", temp);
	//printf("now: %d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

	strncat(path, temp, size);

	dlog_print(DLOG_INFO, LOG_TAG, "Path: %s", path);

	ad->fp = fopen(path, "w");
	if (ad->fp == NULL) {
		dlog_print(DLOG_INFO, LOG_TAG, "Unable to create file. Error: %d\n",
		errno);
		return;
	}
	dlog_print(DLOG_INFO, LOG_TAG, "File opened");

	fprintf(ad->fp, "%s", CSV_TITLE);
	free(path);

}


static void sensor_cb(sensor_h sensor, sensor_event_s *event, void *data) {

	sensor_type_e type;
	sensor_get_type(sensor, &type);
	struct timeval tv;

	appdata_s *ad = data;

	switch (type) {

	case SENSOR_HRM_LED_GREEN:

		gettimeofday(&tv, NULL);

		unsigned long long millisecondsSinceEpoch =
		 (unsigned long long)(tv.tv_sec) * 1000 +
		 (unsigned long long)(tv.tv_usec) / 1000; //Unix with milliseconds

		unsigned long long millisec = millisecondsSinceEpoch-ad->start_time;

		int timestamp = (int) time(NULL);  //Unix
		int value = (int) event->values[0];



		ppg[ad->ppg_count].ppg_value = value;
		ppg[ad->ppg_count].ppg_time = timestamp;
		ppg[ad->ppg_count].ppg_count = ad->rec_count;
		ppg[ad->ppg_count].millisec = millisec;
		ad->ppg_count++;


		break;

	case SENSOR_ACCELEROMETER:

		gettimeofday(&tv, NULL);

		int Accelerometer_timestamp = (int) time(NULL);  //Unix
		float Xvalue = event->values[0];
		float Yvalue = event->values[1];
		float Zvalue = event->values[2];

		acc[ad->acc_count].acc_x = Xvalue;
		acc[ad->acc_count].acc_y = Yvalue;
		acc[ad->acc_count].acc_z = Zvalue;
		acc[ad->acc_count].acc_time = Accelerometer_timestamp;
		acc[ad->acc_count].acc_count = ad->rec_count;

		ad->acc_count++;
		ad->rec_count++;

		break;
	default:
		break;

	}

}


void init_sensors(appdata_s *ad) {

	bool supported = false;

	int error = sensor_is_supported(SENSOR_HRM_LED_GREEN, &supported);
	if (error != SENSOR_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "GREEN NOT SUPPORTED %d", error);
		return;
	}

	if (supported) {

		error = sensor_get_default_sensor(SENSOR_HRM_LED_GREEN,
				&ad->PPG_Sensor);
		if (error != SENSOR_ERROR_NONE) {
			dlog_print(DLOG_ERROR, LOG_TAG, "COULDNT GET SENSOR %d", error);
			return;
		}

		error = sensor_create_listener(ad->PPG_Sensor, &ad->PPG_Listener);
		if (error != SENSOR_ERROR_NONE) {
			dlog_print(DLOG_ERROR, LOG_TAG,
					"PPG sensor_create_listener error: %d", error);
			return;
		}

		error = sensor_listener_set_event_cb(ad->PPG_Listener, SENSOR_INTERVAL,
				sensor_cb, ad);
		if (error != SENSOR_ERROR_NONE) {
			dlog_print(DLOG_ERROR, LOG_TAG,
					"PPG sensor_listener_set_event_cb error: %d", error);
			return;
		}

		error = sensor_listener_set_interval(ad->PPG_Listener, SENSOR_INTERVAL);
		if (error != SENSOR_ERROR_NONE) {
			dlog_print(DLOG_ERROR, LOG_TAG,
					"PPG sensor_listener_set_interval error: %d", error);
			return;
		}

		error = sensor_listener_set_option(ad->PPG_Listener,
				SENSOR_OPTION_ALWAYS_ON);
		if (error != SENSOR_ERROR_NONE) {
			dlog_print(DLOG_ERROR, LOG_TAG,
					"PPG sensor_listener_set_option error: %d", error);
			return;
		}

		error = sensor_listener_set_attribute_int(ad->PPG_Listener,
				SENSOR_ATTRIBUTE_PAUSE_POLICY, SENSOR_PAUSE_NONE);
		if (error != SENSOR_ERROR_NONE) {
			dlog_print(DLOG_ERROR, LOG_TAG,
					"PPG sensor_listener_set_atribute error: %d", error);
			return;
		}


	} else {
		dlog_print(DLOG_INFO, LOG_TAG, "PPG not supported");
	}


	 supported = false;
	 error = sensor_is_supported(SENSOR_ACCELEROMETER, &supported);
	 if (error != SENSOR_ERROR_NONE) {
	 dlog_print(DLOG_ERROR, LOG_TAG, "Accelerometer NOT SUPPORTED %d", error);
	 return;
	 }

	 if (supported) {


	 error = sensor_get_default_sensor(SENSOR_ACCELEROMETER, &ad->Acc_Sensor);
	 if (error != SENSOR_ERROR_NONE) {
	 dlog_print(DLOG_ERROR, LOG_TAG, "COULDNT GET SENSOR %d", error);
	 return;
	 }

	 error = sensor_create_listener(ad->Acc_Sensor, &ad->Acc_Listener);
	 if (error != SENSOR_ERROR_NONE) {
	 dlog_print(DLOG_ERROR, LOG_TAG,
	 "Acc sensor_create_listener error: %d", error);
	 return;
	 }

	 error = sensor_listener_set_event_cb(ad->Acc_Listener, ACCELEROMETER_INTERVAL, sensor_cb, ad);
	 if (error != SENSOR_ERROR_NONE) {
	 dlog_print(DLOG_ERROR, LOG_TAG,
	 "Acc sensor_listener_set_event_cb error: %d", error);
	 return;
	 }

	 error = sensor_listener_set_option(ad->Acc_Listener,
	 SENSOR_OPTION_ALWAYS_ON);
	 if (error != SENSOR_ERROR_NONE) {
	 dlog_print(DLOG_ERROR, LOG_TAG,
	 "Acc sensor_listener_set_option error: %d", error);
	 return;
	 }

	 error = sensor_listener_set_attribute_int(ad->Acc_Listener,
	 SENSOR_ATTRIBUTE_PAUSE_POLICY, SENSOR_PAUSE_NONE);
	 if (error != SENSOR_ERROR_NONE) {
	 dlog_print(DLOG_ERROR, LOG_TAG,
	 "Acc sensor_listener_set_atribute error: %d", error);
	 return;
	 }

	 } else {
	 dlog_print(DLOG_INFO, LOG_TAG, "Acc not supported");
	 }
}

void start_sensor(appdata_s *ad) {





	int error = sensor_listener_start(ad->PPG_Listener);
	if (error == SENSOR_ERROR_NONE) {
		dlog_print(DLOG_INFO, LOG_TAG, "PPG sensor started");

	} else {
		dlog_print(DLOG_INFO, LOG_TAG, "PPG sensor couldn't be started");
	}

	error = sensor_listener_start(ad->Acc_Listener);
	if (error == SENSOR_ERROR_NONE) {
		dlog_print(DLOG_INFO, LOG_TAG, "ACC sensor started");
		struct timeval tv;
		gettimeofday(&tv, NULL);
		ad->start_time =
				 (unsigned long long)(tv.tv_sec) * 1000 +
				 (unsigned long long)(tv.tv_usec) / 1000; //Unix with milliseconds

	} else {
		dlog_print(DLOG_INFO, LOG_TAG, "ACC sensor couldn't be started");
	}


}

void stop_sensor(appdata_s *ad) {

	int error = sensor_listener_stop(ad->PPG_Listener);
	if (error == SENSOR_ERROR_NONE) {
		dlog_print(DLOG_INFO, LOG_TAG, "PPG sensor stopped");

	} else {
		dlog_print(DLOG_INFO, LOG_TAG, "PPG sensor couldn't be stopped");
	}
	error = sensor_listener_stop(ad->Acc_Listener);
	if (error == SENSOR_ERROR_NONE) {
		dlog_print(DLOG_INFO, LOG_TAG, "ACC sensor stopped");
		//ad->start_time = 0;







	} else {
		dlog_print(DLOG_INFO, LOG_TAG, "ACC sensor couldn't be stopped");
	}
	dlog_print(DLOG_INFO, LOG_TAG, "Writing to file, PPGCOUNT: %d",
			ad->rec_count);

	sleep(1);

	int i = 0;
	while (i < ad->acc_count) {

		dlog_print(DLOG_INFO, LOG_TAG, "%d, %d, %llu, %f, %f, %f, %d, %d\n",
				ppg[i].ppg_value, ppg[i].ppg_time, ppg[i].millisec,
				acc[i].acc_x, acc[i].acc_y, acc[i].acc_z, acc[i].acc_time,
				acc[i].acc_count);
		fprintf(ad->fp, "%d, %d, %llu, %f, %f, %f, %d, %d\n", ppg[i].ppg_value,
				ppg[i].ppg_time, ppg[i].millisec, acc[i].acc_x, acc[i].acc_y,
				acc[i].acc_z, acc[i].acc_time, acc[i].acc_count);
		i++;
	}
	memset(ppg, 0, sizeof(ppg));
	memset(acc, 0, sizeof(acc));
	ad->acc_count = 0;
	ad->ppg_count = 0;
	dlog_print(DLOG_INFO, LOG_TAG, "Written successfully to file");


}

Eina_Bool recording_duration(void *data) {

	appdata_s *ad = data;

	stop_sensor(ad);
	ecore_timer_del(ad->recording_timer);

	return ECORE_CALLBACK_CANCEL;
}

Eina_Bool main_timer(void *data) {

	appdata_s* ad = data;

	start_sensor(ad);
	ad->recording_timer = ecore_timer_add(RECORDING_TIME, recording_duration,
			ad);

	return ECORE_CALLBACK_RENEW;
}

bool service_app_create(void *data) {

	appdata_s* ad = data;


	dlog_print(DLOG_INFO, LOG_TAG, "Service Created.");
	if (!ecore_init()) {
		dlog_print(DLOG_INFO, LOG_TAG, "CANNOT INIT ECORE.");
		return -1;
	}
	if (device_power_request_lock(POWER_LOCK_CPU, 0) != DEVICE_ERROR_NONE) {
		dlog_print(DLOG_INFO, LOG_TAG, "CANNOT LOCK CPU.");

	}





	ad->rec_count = 0;
	ad->ppg_count = 0;
	ad->acc_count = 0;

	init_sensors(ad);
	open_csv_file(ad);

	start_sensor(ad);

	ad->recording_timer = ecore_timer_add(RECORDING_TIME, recording_duration,
			ad);
	ad->main_timer = ecore_timer_add(TIME_BETWEEN_RECORDINGS, main_timer, ad);
	return true;
}

void service_app_terminate(void *data) {

	appdata_s* ad = data;
	dlog_print(DLOG_INFO, LOG_TAG, "Service Terminated.");

	ecore_timer_freeze(ad->main_timer);
	ecore_timer_freeze(ad->recording_timer);
	ecore_timer_del(ad->main_timer);
	ecore_timer_del(ad->recording_timer);
	//ecore_main_loop_quit();
	if ((sensor_destroy_listener(ad->PPG_Listener)) == SENSOR_ERROR_NONE) {
		dlog_print(DLOG_INFO, LOG_TAG, "Sensor destroyed");
	}
	if (device_power_release_lock(POWER_LOCK_CPU) != DEVICE_ERROR_NONE) {
		dlog_print(DLOG_INFO, LOG_TAG, "CANNOT UNLOCK CPU.");
	}


	fclose(ad->fp);
	free(data);

	return;
}

void service_app_control(app_control_h app_control, void *data) {

	char *caller_id = NULL, *action_value = NULL;
	if ((app_control_get_caller(app_control, &caller_id)
			== APP_CONTROL_ERROR_NONE)
			&& (app_control_get_extra_data(app_control, "service_action",
					&action_value) == APP_CONTROL_ERROR_NONE)) {
		if ((caller_id != NULL) && (action_value != NULL)
				&& (!strncmp(caller_id, PPGMONITOR_APP_ID, STRNCMP_LIMIT))
				&& (!strncmp(action_value, "stop", STRNCMP_LIMIT))) {
			dlog_print(DLOG_INFO, LOG_TAG, "Stopping MyService!");
			free(caller_id);
			free(action_value);
			service_app_exit();
			return;
		} else {
			dlog_print(DLOG_INFO, LOG_TAG,
					"Unsupported action! Doing nothing...");
			free(caller_id);
			free(action_value);
			caller_id = NULL;
			action_value = NULL;
		}
	} else {

		dlog_print(DLOG_INFO, LOG_TAG, "Can't get caller");

	}
}

static void service_app_lang_changed(app_event_info_h event_info,
		void *user_data) {
	/*APP_EVENT_LANGUAGE_CHANGED*/
	return;
}

static void service_app_region_changed(app_event_info_h event_info,
		void *user_data) {
	/*APP_EVENT_REGION_FORMAT_CHANGED*/
}

static void service_app_low_battery(app_event_info_h event_info,
		void *user_data) {
	/*APP_EVENT_LOW_BATTERY*/
}

static void service_app_low_memory(app_event_info_h event_info, void *user_data) {
	/*APP_EVENT_LOW_MEMORY*/
}

int main(int argc, char* argv[]) {
	char ad[50] = { 0, };
	service_app_lifecycle_callback_s event_callback;
	app_event_handler_h handlers[5] = { NULL, };

	event_callback.create = service_app_create;
	event_callback.terminate = service_app_terminate;
	event_callback.app_control = service_app_control;

	service_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY],
			APP_EVENT_LOW_BATTERY, service_app_low_battery, &ad);
	service_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY],
			APP_EVENT_LOW_MEMORY, service_app_low_memory, &ad);
	service_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED],
			APP_EVENT_LANGUAGE_CHANGED, service_app_lang_changed, &ad);
	service_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED],
			APP_EVENT_REGION_FORMAT_CHANGED, service_app_region_changed, &ad);

	return service_app_main(argc, argv, &event_callback, ad);
}
