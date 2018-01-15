#Icons form https://www.iconfinder.com/iconsets/mayssam
#License: Creative Commons (Attribution 3.0 Unported) 

import urllib.request

icons = [[925909, "arrow_direction_previous_right_icon"],
[925894, "action_cog_gear_options_preferences_service_settings_icon"],
[925897, "arrow_direction_left_next_previous_return_icon"],
[926645, "arrow_direction_refresh_repeat_restart_icon"],
[925910, "arrow_direction_left_return_icon"],
[925927, "arrow_back_direction_reply_icon"],
[925898, "arrow_arrows_direction_next_previous_icon"],
[926649, "arrow_next_player_previous_icon"],
[925904, "arrow_arrows_direction_previous_icon"],
[925900, "arrow_play_player_record_right_start_icon"],
[926646, "arrow_media_next_player_previous_song_icon"]]

types = [
	["png", "32"],
	["png", "64"],
	["png", "128"],
	["svg", "128"]
]

# https://www.iconfinder.com/icons/925909/download/png/64

for ind, name in icons:
	print(str(ind) + " " + name)
	for imgtype, size in types[:]:
		urllib.request.urlretrieve("https://www.iconfinder.com/icons/" + str(ind) + "/download/"
			+ imgtype + "/" + size, "icons/" + name + "_" + size + "." + imgtype)

with open("demo.qrc", 'w') as f:
	f.write("<RCC>\n")
	f.write("	<qresource prefix=\"/animation/\">\n")
	for ind, name in icons:
		for imgtype, size in types:
			f.write("		<file>icons/" + name + "_" + size + "." + imgtype + "</file>\n")
	f.write("	</qresource>\n")
	f.write("</RCC>\n")

