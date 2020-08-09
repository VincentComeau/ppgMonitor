#include "ppgmonitor.h"



typedef struct appdata {
	Evas_Object *win;
	Evas_Object *conform;
	Evas_Object *label;
	Evas_Object *list;
	Evas_Object *check;
	Evas_Object *main_box;
	Evas_Object *settings_box;
	Evas_Object *csv_box;
	Evas_Object *csv_list;
	Evas_Object *nf;
	Evas_Object *view_csv_data_btn;
	Evas_Object *close_btn;
	Evas_Object *settings_btn;
	Evas_Object *back_btn;
	int file_count;

} appdata_s;



void app_request_response_cb(ppm_call_cause_e cause,
		ppm_request_result_e result, const char *privilege, void *user_data) {
	if (cause == PRIVACY_PRIVILEGE_MANAGER_CALL_CAUSE_ERROR) {
		/* Log and handle errors */

		return;
	}

	switch (result) {
	case PRIVACY_PRIVILEGE_MANAGER_REQUEST_RESULT_ALLOW_FOREVER:
		dlog_print(DLOG_INFO, LOG_TAG, "Permission ALLOWED FOREVER");
		break;
	case PRIVACY_PRIVILEGE_MANAGER_REQUEST_RESULT_DENY_FOREVER:
		/* Show a message and terminate the application */

		dlog_print(DLOG_INFO, LOG_TAG, "Permission Denied Forever");
		break;
	case PRIVACY_PRIVILEGE_MANAGER_REQUEST_RESULT_DENY_ONCE:
		/* Show a message with explanation */
		dlog_print(DLOG_INFO, LOG_TAG, "Permission Denied once");
		break;
	}
}


void printdir(char *dir, int depth, appdata_s *ad)
{
    DIR *dp;
    struct dirent *entry;
    struct stat statbuf;
    if((dp = opendir(dir)) == NULL) {
        fprintf(stderr,"cannot open directory: %s\n", dir);
        return;
    }
    chdir(dir);
    while((entry = readdir(dp)) != NULL) {
        lstat(entry->d_name,&statbuf);
        if(S_ISDIR(statbuf.st_mode)) {
            /* Found a directory, but ignore . and .. */
            if(strcmp(".",entry->d_name) == 0 ||
                strcmp("..",entry->d_name) == 0)
                continue;

            char search[] = ".csv";
            char *ptr = strstr(entry->d_name, search);

            if (ptr != NULL) /* Substring found */
            {
            	dlog_print(DLOG_INFO, LOG_TAG, "CSV %*s%s/\n",depth,"",entry->d_name);
            	elm_list_item_append(ad->list, entry->d_name, NULL, NULL, NULL, NULL);
            }
            else /* Substring not found */
            {
            	dlog_print(DLOG_INFO, LOG_TAG, "NOT CSV %*s%s/\n",depth,"",entry->d_name);
            }

            /* Recurse at a new indent level */
            printdir(entry->d_name,depth+4, ad);
        }
        else
        {
		char search[] = ".csv";
        char *ptr = strstr(entry->d_name, search);

        if (ptr != NULL) /* Substring found */
        {
        	dlog_print(DLOG_INFO, LOG_TAG, "ELSE CSV %*s%s/\n",depth,"",entry->d_name);
        	if(elm_list_item_append(ad->list, entry->d_name, NULL, NULL, NULL, NULL) == NULL){

        		dlog_print(DLOG_INFO, LOG_TAG, "Element not appended");
        	}

        }
        else /* Substring not found */
        {
        	dlog_print(DLOG_INFO, LOG_TAG, "ELSE NOT CSV %*s%s/\n",depth,"",entry->d_name);
        }
        }
    }
    chdir("..");
    closedir(dp);
}





void delete_csv() {

	char * data_path = app_get_data_path();
	int size = strlen(data_path) + 10;
	char * path = malloc(sizeof(char) * size);

	strcpy(path, data_path);
	strncat(path, "PPGtest", size);

	if (remove(path) == 0)

		dlog_print(DLOG_INFO, LOG_TAG, "CSV Deleted successfully");
	else
		dlog_print(DLOG_INFO, LOG_TAG, "CSV Unable to delete file");

	return;
}





void app_check_and_request_permission() {
	ppm_check_result_e result;
	const char *privilege = "http://tizen.org/privilege/healthinfo";

	int ret = ppm_check_permission(privilege, &result);

	if (ret == PRIVACY_PRIVILEGE_MANAGER_ERROR_NONE) {

		switch (result) {

		case PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_ALLOW:
			dlog_print(DLOG_INFO, LOG_TAG, "Check result ALLOWED");
			break;
		case PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_DENY:
			/* Show a message and terminate the application */

			dlog_print(DLOG_INFO, LOG_TAG, "Check result DENIED");
			ret = ppm_request_permission(privilege, app_request_response_cb,
					NULL);
			break;

		case PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_ASK:
			dlog_print(DLOG_INFO, LOG_TAG, "Check result ASK");
			ret = ppm_request_permission(privilege, app_request_response_cb,
					NULL);
			/* Log and handle errors */
			break;

		}
	}
}

static void win_delete_request_cb(void *data, Evas_Object *obj,
		void *event_info) {
	ui_app_exit();
}

static void win_back_cb(void *data, Evas_Object *obj, void *event_info) {

	ui_app_exit();
	/* Let window go to hide state. */
	//elm_win_lower(ad->win);
}

static void close_app_cb(void *data, Evas_Object *obj, void *event_info) {
	ui_app_exit();
}

static void launch_service() {
	app_control_h app_control = NULL;
	if (app_control_create(&app_control) == APP_CONTROL_ERROR_NONE) {
		if ((app_control_set_app_id(app_control, "org.example.myservice")
				== APP_CONTROL_ERROR_NONE)
				&& (app_control_send_launch_request(app_control, NULL, NULL)
						== APP_CONTROL_ERROR_NONE)) {
			dlog_print(DLOG_INFO, LOG_TAG, "App launch request sent!");
		} else {
			dlog_print(DLOG_ERROR, LOG_TAG,
					"App launch request sending failed!");
		}
	} else {
		dlog_print(DLOG_ERROR, LOG_TAG, "App control creation failed!");
	}
}

static void stop_service() {
	app_control_h app_control = NULL;
	if (app_control_create(&app_control) == APP_CONTROL_ERROR_NONE) {
		if ((app_control_set_app_id(app_control, "org.example.myservice")
				== APP_CONTROL_ERROR_NONE)
				&& (app_control_add_extra_data(app_control, "service_action",
						"stop") == APP_CONTROL_ERROR_NONE)
				&& (app_control_send_launch_request(app_control, NULL, NULL)
						== APP_CONTROL_ERROR_NONE)) {
			dlog_print(DLOG_INFO, LOG_TAG, "App stop request sent!");

		} else {
			dlog_print(DLOG_ERROR, LOG_TAG,
					"App stop request sending failed!");
		}

	} else {
		dlog_print(DLOG_ERROR, LOG_TAG, "App control creation failed!");
	}
}





static void view_csv_data_cb(void *data, Evas_Object *obj, void *event_info) {
	dlog_print(DLOG_INFO, LOG_TAG, "View CSV data clicked");

	appdata_s *ad = data;

	char *data_path = "/opt/usr/home/owner/media/Documents/";
	printdir(data_path, 0, ad);
	elm_naviframe_item_push(ad->nf, "Files", NULL, NULL, ad->list, NULL);



}




static void check_cb(void *data, Evas_Object *obj, void *event_info) {
	dlog_print(DLOG_INFO, LOG_TAG, "Check callback");
	appdata_s *ad = data;


	switch(elm_check_state_get(ad->check))
	{
	case 1:
		launch_service();
		preference_set_boolean("check_value", 1 );
		elm_object_text_set(ad->label, _("<align=center>Recording..</align>"));

		break;
	case 0:
		stop_service();
		preference_set_boolean("check_value", 0 );
		elm_object_text_set(ad->label, _("<align=center>Press to record</align>"));
		break;
	default:return;
	}


}


static void settings_cb(void *data, Evas_Object *obj, void *event_info) {

	appdata_s *ad = data;

	elm_naviframe_item_push(ad->nf, "Settings", NULL, NULL, ad->settings_box, NULL);



}


static void back_btn_cb(void *data, Evas_Object *obj, void *event_info) {

	appdata_s *ad = data;

	elm_naviframe_item_push(ad->nf, "PPG Recorder", NULL, NULL, ad->main_box, NULL);



}




/*static void delete_csv_cb(void *data, Evas_Object *obj, void *event_info) {

	dlog_print(DLOG_INFO, LOG_TAG, "Delete csv data clicked");
	char * dataPath = app_get_data_path();
	int size = strlen(dataPath) + 10;
	char * path = malloc(sizeof(char) * size);
	strcpy(path, dataPath);
	strncat(path, CSV_FILE_NAME, size);

	remove(path);
	dlog_print(DLOG_INFO, LOG_TAG, "removed %s from %s", CSV_FILE_NAME, dataPath);
}*/
static void create_base_gui(appdata_s *ad) {
	/* Window */
	/* Create and initialize elm_win.
	 elm_win is mandatory to manipulate window. */
	ad->win = elm_win_util_standard_add(PACKAGE, PACKAGE);
	elm_win_autodel_set(ad->win, EINA_TRUE);

	if (elm_win_wm_rotation_supported_get(ad->win)) {
		int rots[4] = { 0, 90, 180, 270 };
		elm_win_wm_rotation_available_rotations_set(ad->win,
				(const int *) (&rots), 4);
	}

	evas_object_smart_callback_add(ad->win, "delete,request",
			win_delete_request_cb, NULL);
	eext_object_event_callback_add(ad->win, EEXT_CALLBACK_BACK, win_back_cb,
			ad);

	ad->conform = elm_conformant_add(ad->win);
	elm_win_indicator_mode_set(ad->win, ELM_WIN_INDICATOR_SHOW);
	elm_win_indicator_opacity_set(ad->win, ELM_WIN_INDICATOR_OPAQUE);
	evas_object_size_hint_weight_set(ad->conform, EVAS_HINT_EXPAND,
			EVAS_HINT_EXPAND);
	elm_win_resize_object_add(ad->win, ad->conform);
	evas_object_show(ad->conform);

	ad->nf = elm_naviframe_add(ad->conform);
	evas_object_show(ad->nf);
	elm_naviframe_prev_btn_auto_pushed_set(ad->nf, EINA_TRUE);
	elm_object_content_set(ad->conform, ad->nf);
	elm_naviframe_prev_btn_auto_pushed_set(ad->nf, EINA_TRUE);
	//eext_object_event_callback_add(ad->nf, EEXT_CALLBACK_BACK, naviframe_back_cb, ad);

	ad->main_box = elm_box_add(ad->nf);
	evas_object_show(ad->main_box);
	elm_naviframe_item_push(ad->nf, "PPG Recorder", NULL, NULL, ad->main_box, NULL);

	ad->settings_box = elm_box_add(ad->nf);
	evas_object_show(ad->settings_box);

	ad->csv_box = elm_box_add(ad->nf);
	evas_object_show(ad->csv_box);


	ad->check = elm_check_add(ad->main_box);
	elm_object_style_set(ad->check, "on&off");
	elm_object_part_text_set(ad->check, NULL, "Start");
	elm_box_pack_end(ad->main_box, ad->check);
	evas_object_smart_callback_add(ad->check, "changed", check_cb, ad);
	evas_object_show(ad->check);



	ad->label = elm_label_add(ad->main_box);
	evas_object_size_hint_weight_set(ad->label, 0.5, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(ad->label, EVAS_HINT_FILL, 0.5);
	elm_object_text_set(ad->label, _("<align=center>Press to record</align>"));
	elm_box_pack_end(ad->main_box, ad->label);
	evas_object_show(ad->label);


	ad->settings_btn = elm_button_add(ad->main_box);
	evas_object_size_hint_weight_set(ad->settings_btn, 0.5, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(ad->settings_btn, EVAS_HINT_FILL, 0.5);
	evas_object_show(ad->settings_btn);
	elm_object_text_set(ad->settings_btn, "Settings");
	elm_box_pack_end(ad->main_box, ad->settings_btn);
	evas_object_smart_callback_add(ad->settings_btn, "clicked", settings_cb, ad);

	ad->close_btn = elm_button_add(ad->main_box);
	evas_object_size_hint_weight_set(ad->close_btn, 0.5, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(ad->close_btn, EVAS_HINT_FILL, 0.5);
	evas_object_show(ad->close_btn);
	elm_object_text_set(ad->close_btn, "Close");
	elm_box_pack_end(ad->main_box, ad->close_btn);
	evas_object_smart_callback_add(ad->close_btn, "clicked", close_app_cb, ad);




	ad->view_csv_data_btn = elm_button_add(ad->settings_box);
	evas_object_size_hint_weight_set(ad->view_csv_data_btn, 0.5, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(ad->view_csv_data_btn, EVAS_HINT_FILL, 0.5);
	evas_object_show(ad->view_csv_data_btn);
	elm_object_text_set(ad->view_csv_data_btn, "View CSV Data");
	elm_box_pack_end(ad->settings_box, ad->view_csv_data_btn);
	evas_object_smart_callback_add(ad->view_csv_data_btn, "clicked", view_csv_data_cb, ad);


	ad->back_btn = elm_button_add(ad->settings_box);
	elm_object_style_set(ad->back_btn, "bottom");
	evas_object_size_hint_weight_set(ad->back_btn, 0.2, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(ad->back_btn, EVAS_HINT_FILL, 0.5);
	evas_object_show(ad->back_btn);
	elm_object_text_set(ad->back_btn, "Back");
	elm_box_pack_end(ad->settings_box, ad->back_btn);
	evas_object_smart_callback_add(ad->back_btn, "clicked", back_btn_cb, ad);










	ad->list = elm_list_add(ad->nf);
	elm_list_item_append(ad->list, "Back", NULL, NULL, back_btn_cb, NULL);

	evas_object_show(ad->list);



	/* Show window after base gui is set up */
	evas_object_show(ad->win);
}

static bool app_create(void *data) {
	/* Hook to take necessary actions before main event loop starts
	 Initialize UI resources and application's data
	 If this function returns true, the main loop of application starts
	 If this function returns false, the application is terminated */
	appdata_s *ad = data;

	bool check_state;
	bool existing;
	//ad->file_count = count_files(ad);
	//dlog_print(DLOG_INFO, LOG_TAG, "File Count: %d", ad->file_count);

	preference_is_existing("check_value", &existing);
	if(existing){

		preference_get_boolean("check_value", &check_state);
		if(check_state){

			elm_check_state_set(ad->check, 1);
			elm_object_text_set(ad->label, _("<align=center>Recording..</align>"));
		}else{
			elm_check_state_set(ad->check, 0);
			elm_object_text_set(ad->label, _("<align=center>Press to record</align>"));
		}
	}else{
		dlog_print(DLOG_INFO, LOG_TAG, "Preference doesnt exist");
	}
	create_base_gui(ad);
	app_check_and_request_permission();




	return true;
}

static void app_control(app_control_h app_control, void *data) {
	/* Handle the launch request. */
}

static void app_pause(void *data) {
	/* Take necessary actions when application becomes invisible. */
	appdata_s *ad = data;

	switch(elm_check_state_get(ad->check))
		{
		case 1:

			preference_set_boolean("check_value", 1 );
			break;
		case 0:

			preference_set_boolean("check_value", 0 );
			break;
		default:return;
		}


}

static void app_resume(void *data) {
	/* Take necessary actions when application becomes visible. */

	appdata_s *ad = data;
	bool check_state;
	bool existing;
	preference_is_existing("check_value", &existing);
	if(existing){
		preference_get_boolean("check_value", &check_state);
		if(check_state){

			elm_check_state_set(ad->check, 1);
			elm_object_text_set(ad->label, _("<align=center>Recording..</align>"));

		}else{
			elm_check_state_set(ad->check, 0);
			elm_object_text_set(ad->label, _("<align=center>Press to record</align>"));
		}
	}

}

static void app_terminate(void *data) {

	appdata_s *ad = data;
	switch(elm_check_state_get(ad->check))
			{
			case 1:
				preference_set_boolean("check_value", 1 );
				break;
			case 0:
				preference_set_boolean("check_value", 0 );
				break;
			default:
				return;
			}
}

static void ui_app_lang_changed(app_event_info_h event_info, void *user_data) {
	/*APP_EVENT_LANGUAGE_CHANGED*/
	char *locale = NULL;
	system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE,
			&locale);
	elm_language_set(locale);
	free(locale);
	return;
}

static void ui_app_orient_changed(app_event_info_h event_info, void *user_data) {
	/*APP_EVENT_DEVICE_ORIENTATION_CHANGED*/
	return;
}

static void ui_app_region_changed(app_event_info_h event_info, void *user_data) {
	/*APP_EVENT_REGION_FORMAT_CHANGED*/
}

static void ui_app_low_battery(app_event_info_h event_info, void *user_data) {
	/*APP_EVENT_LOW_BATTERY*/
}

static void ui_app_low_memory(app_event_info_h event_info, void *user_data) {
	/*APP_EVENT_LOW_MEMORY*/
}

int main(int argc, char *argv[]) {
	appdata_s ad = { 0, };
	int ret = 0;

	ui_app_lifecycle_callback_s event_callback = { 0, };
	app_event_handler_h handlers[5] = { NULL, };

	event_callback.create = app_create;
	event_callback.terminate = app_terminate;
	event_callback.pause = app_pause;
	event_callback.resume = app_resume;
	event_callback.app_control = app_control;

	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY],
			APP_EVENT_LOW_BATTERY, ui_app_low_battery, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY],
			APP_EVENT_LOW_MEMORY, ui_app_low_memory, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_DEVICE_ORIENTATION_CHANGED],
			APP_EVENT_DEVICE_ORIENTATION_CHANGED, ui_app_orient_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED],
			APP_EVENT_LANGUAGE_CHANGED, ui_app_lang_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED],
			APP_EVENT_REGION_FORMAT_CHANGED, ui_app_region_changed, &ad);

	ret = ui_app_main(argc, argv, &event_callback, &ad);
	if (ret != APP_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "app_main() is failed. err = %d", ret);
	}

	return ret;
}
