#Icons form https://www.iconfinder.com/iconsets/mayssam
#License: Creative Commons (Attribution 3.0 Unported)

import urllib.request

icons = [[925909, "arrow_direction_previous_right_icon"],
[925901, "account_avatar_people_profile_user_icon"],
[925908, "content_network_share_social_icon"],
[925919, "location_map_maps_marker_pin_icon"],
[925894, "action_cog_gear_options_preferences_service_settings_icon"],
[925916, "call_communication_contact_phone_icon"],
[925897, "arrow_direction_left_next_previous_return_icon"],
[925890, "account_add_create_new_profile_user_icon"],
[925924, "cloud_cloudy_server_sky_weather_icon"],
[925922, "arrow_bottom_direction_down_icon"],
[926640, "admiration_admire_favorite_like_rate_rating_star_icon"],
[925921, "contact_email_envelope_letter_message_send_icon"],
[926645, "arrow_direction_refresh_repeat_restart_icon"],
[925923, "close_cross_danger_delete_remove_trash_icon"],
[925926, "bubble_chat_conversation_friends_talk_icon"],
[925920, "admiration_favorite_health_heart_like_love_rating_icon"],
[925911, "discover_explore_find_magnifier_search_icon"],
[925914, "add_create_new_plus_icon"],
[925910, "arrow_direction_left_return_icon"],
[925906, "arrow_direction_top_up_icon"],
[925927, "arrow_back_direction_reply_icon"],
[925892, "basket_buy_goods_market_online_shop_shopping_icon"],
[925898, "arrow_arrows_direction_next_previous_icon"],
[925907, "full_noise_sound_volume_icon"],
[926652, "list_menu_needs_player_to_do_icon"],
[925905, "account_delete_remove_user_icon"],
[925918, "account_lock_password_protect_save_saving_security_icon"],
[926636, "off_on_player_power_start_stop_icon"],
[926633, "danger_forbidden_not_off_stop_icon"],
[925896, "full_movie_screen_video_watch_icon"],
[926651, "micro_microphone_on_phone_radio_recording_icon"],
[925929, "arrow_direction_next_previous_right_icon"],
[925895, "code_coding_dev_development_embed_programming_tag_icon"],
[925893, "calendar_date_day_event_month_year_icon"],
[926649, "arrow_next_player_previous_icon"],
[925899, "design_drawing_education_pen_pencil_school_study_icon"],
[925912, "face_happy_ok_smile_smiley_smiling_icon"],
[925904, "arrow_arrows_direction_previous_icon"],
[926639, "full_media_player_resize_screen_icon"],
[925891, "arrow_direction_left_next_previous_icon"],
[925900, "arrow_play_player_record_right_start_icon"],
[926642, "audio_media_mute_off_player_sound_volume_icon"],
[926637, "media_next_player_previous_song_track_icon"],
[926650, "arrows_media_next_player_previous_recording_right_icon"],
[925917, "film_movie_pause_player_sound_icon"],
[925903, "heat_hot_nature_sky_summer_sun_weather_icon"],
[925925, "check_checkmark_good_improve_improved_ok_success_icon"],
[925930, "arrow_bottom_direction_down_icon"],
[926648, "more_options_player_preferences_settings_icon"],
[925902, "cloud_rain_rainy_weather_icon"],
[926646, "arrow_media_next_player_previous_song_icon"],
[926638, "arrows_media_player_repeat_song_sound_video_icon"],
[926641, "multimedia_off_recording_station_stop_icon"],
[926634, "micro_microphone_mute_off_radio_icon"],
[925928, "arrow_direction_top_up_icon"],
[925913, "cloud_sky_thunder_weather_icon"],
[926635, "arrow_next_player_previous_recording_right_icon"],
[1063861, "lock_open_opened_protection_safety_security_unlocked_icon"],
[926644, "arrows_direction_media_player_repeat_restart_icon"],
[926653, "audio_call_communication_headphone_talk_icon"],
[926647, "media_next_player_previous_song_track_icon"],
[926643, "media_monitor_player_resize_screen_icon"],
[925931, "alert_attention_danger_error_message_warning_icon"],
[925915, "current_location_map_maps_marker_note_pin_icon"],
[2625630, "basket_delete_garbage_trash_waste_icon"],
[2625638, "eye_look_search_view_icon"],
]

types = [
	["png", "32"],
	["png", "64"],
	["png", "128"],
	["svg", "128"]
]

# https://www.iconfinder.com/icons/925909/download/png/64

for ind, name in icons:
	print(str(ind) + " " + name)
	for imgtype, size in types:
		urllib.request.urlretrieve("https://www.iconfinder.com/icons/" + str(ind) + "/download/"
			+ imgtype + "/" + size, "icons/" + name + "_" + size + "." + imgtype)

with open("animation.qrc", 'w') as f:
	f.write("<RCC>\n")
	f.write("	<qresource prefix=\"/animation/\">\n")
	for ind, name in icons:
		for imgtype, size in types:
			f.write("		<file>icons/" + name + "_" + size + "." + imgtype + "</file>\n")
	f.write("	</qresource>\n")
	f.write("</RCC>\n")
