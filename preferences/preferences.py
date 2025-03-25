import json


class Mini_Split_Preferences():
	
	def __init__(self):
		try:
			json_preferences = {}
			with open('./preferences/mini_split_preferences.json', 'r') as preferences:
				json_preferences = json.load(preferences)
			
			self.main_frame_color = json_preferences['main_frame_color']
			self.title_game_name_foreground = json_preferences['title_setting'][0]['game_name_foreground']
			self.title_game_name_background = json_preferences['title_setting'][1]['game_name_background']
			self.title_category_foreground = json_preferences['title_setting'][2]['category_foreground']
			self.title_category_background = json_preferences['title_setting'][3]['category_background']
			self.title_game_name_font = json_preferences['title_setting'][4]['game_name_font']
			self.title_category_font = json_preferences['title_setting'][5]['category_font']
			self.title_game_name_font_size =  json_preferences['title_setting'][6]['game_name_font_size']
			self.title_category_font_size = json_preferences['title_setting'][7]['category_font_size']

			self.line_min_width = json_preferences['line'][0]['min_width']
			self.line_height = json_preferences['line'][1]['height']
			self.line_color = json_preferences['line'][2]['color']
			
			self.main_timer_font = json_preferences['main_timer'][0]['font']
			self.main_timer_font_size = json_preferences['main_timer'][1]['font_size']
			self.main_timer_foreground = json_preferences['main_timer'][2]['timer_foreground']
			self.main_timer_background= json_preferences['main_timer'][3]['timer_background']

			self.splits_desc_name_column_foreground = json_preferences['splits'][0]['description_settings'][0]['name_column_foreground']
			self.splits_desc_name_column_background = json_preferences['splits'][0]['description_settings'][1]['name_column_background']
			self.splits_desc_name_column_font = json_preferences['splits'][0]['description_settings'][2]['name_column_font']
			self.splits_desc_name_column_font_size = json_preferences['splits'][0]['description_settings'][3]['name_column_font_size']
			
			self.splits_desc_diff_column_foreground = json_preferences['splits'][0]['description_settings'][4]['diff_column_foreground']
			self.splits_desc_diff_column_background = json_preferences['splits'][0]['description_settings'][5]['diff_column_background']
			self.splits_desc_diff_column_font = json_preferences['splits'][0]['description_settings'][6]['diff_column_font']
			self.splits_desc_diff_column_font_size = json_preferences['splits'][0]['description_settings'][7]['diff_column_font_size']

			self.splits_desc_prev_time_column_foreground = json_preferences['splits'][0]['description_settings'][8]['prev_time_column_foreground']
			self.splits_desc_prev_time_column_background = json_preferences['splits'][0]['description_settings'][9]['prev_time_column_background']
			self.splits_desc_prev_time_column_font = json_preferences['splits'][0]['description_settings'][10]['prev_time_column_font']
			self.splits_desc_prev_time_column_font_size = json_preferences['splits'][0]['description_settings'][11]['prev_time_column_font_size']

			self.splits_split_name_foreground =  json_preferences['splits'][1]['split_name_foreground']
			self.splits_split_name_background =  json_preferences['splits'][2]['split_name_background']
			self.splits_split_name_font = json_preferences['splits'][3]['split_name_font']
			self.splits_split_name_font_size = json_preferences['splits'][4]['split_name_font_size']
			self.splits_split_name_pady = json_preferences['splits'][5]['split_name_pady']

			self.splits_time_diff_foreground =  json_preferences['splits'][6]['time_diff_foreground']
			self.splits_time_diff_background =  json_preferences['splits'][7]['time_diff_background']
			self.splits_time_diff_font = json_preferences['splits'][8]['time_diff_font']
			self.splits_time_diff_font_size = json_preferences['splits'][9]['time_diff_font_size']
			self.splits_time_diff_pady = json_preferences['splits'][10]['time_diff_pady']

			self.splits_prev_time_foreground =  json_preferences['splits'][11]['prev_time_foreground']
			self.splits_prev_time_background =  json_preferences['splits'][12]['prev_time_background']
			self.splits_prev_time_font = json_preferences['splits'][13]['prev_time_font']
			self.splits_prev_time_font_size = json_preferences['splits'][14]['prev_time_font_size']
			self.splits_prev_time_pady = json_preferences['splits'][15]['prev_time_pady']
			
		except Exception:
			print('Error while parsing the preferences.json file')