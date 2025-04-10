import json
from pathlib import Path
import re
import tkinter

class Run_Info():
    def __init__(self, game_name:str, category:str,attempt_count:int,runner_src_account_url:str = ''):
        self.game_name = game_name
        self.category = category
        self.attempt_count = attempt_count
        self.runner_src_account = runner_src_account_url

class Splits_Manager():
    def __init__(self):
        #current_splits are not the same as splits that are beeing loaded from splits folder, current split means the one that wasnt saved in a file yet
        self.current_splits_sum = {}
        self.current_splits_segment = {}
        self.loaded_splits_sum = {}
        self.current_best_splits_sum = {}
        self.current_best_splits_segment = {}
        
        self.label_time_diff_list = list()
        self.label_prev_time_list = list()
        self.label_possible_time_save: tkinter.Label = None
        self.label_sum_of_best: tkinter.Label = None
        self.loaded_split_index = 0
        
        self.loaded_split_names_sum = list()
        self.loaded_split_values_sum = list()
        self.current_best_splits_names_sum = list()
        self.current_best_splits_values_sum = list()
        self.current_best_splits_names_segment = list()
        self.current_best_splits_values_segment = list()
        self.loaded_split_names_segment = list()
        self.loaded_split_values_segment = list()
        
        self.run_info_loaded = Run_Info('','',0)
        self.run_info_unsaved = Run_Info('','',0)

        self.sum_of_best = 0.0
        self.possible_time_save = 0.0
        
        self.loaded_sum_of_best = 0.0
        self.loaded_possible_time_save = 0.0
    
    def _set_class_values_from_json(self, json_loaded_splits):
        self.loaded_splits_sum = json_loaded_splits['splits_sum']
        self.current_splits_sum = json_loaded_splits['splits_sum']
        self.current_splits_segment = json_loaded_splits['splits_segments']
        self.current_best_splits_sum = json_loaded_splits['best_splits_sum']
        self.current_best_splits_segment = json_loaded_splits['best_splits_segments']

        self.loaded_split_names_sum.extend([list(split.keys())[0] for split in json_loaded_splits['splits_sum']])
        self.loaded_split_values_sum.extend([list(split.values())[0] for split in json_loaded_splits['splits_sum']])
        
        self.current_best_splits_names_sum.extend([list(split.keys())[0] for split in json_loaded_splits['best_splits_sum']])
        self.current_best_splits_values_sum.extend([list(split.values())[0] for split in json_loaded_splits['best_splits_sum']])
        
        self.current_best_splits_names_segment.extend([list(split.keys())[0] for split in json_loaded_splits['best_splits_segments']])
        self.current_best_splits_values_segment.extend([list(split.values())[0] for split in json_loaded_splits['best_splits_segments']])
        
        self.loaded_split_names_segment.extend([list(split.keys())[0] for split in json_loaded_splits['splits_segments']])
        self.loaded_split_values_segment.extend([list(split.values())[0] for split in json_loaded_splits['splits_segments']])
        
        self.run_info_loaded.game_name = json_loaded_splits['game_name']
        self.run_info_loaded.category = json_loaded_splits['category']
        self.run_info_loaded.attempt_count = int(json_loaded_splits['attempts'])
        self.run_info_loaded.runner_src_account = json_loaded_splits['runner_src_account_url']
        self.run_info_unsaved = self.run_info_loaded
        
    #this will load the first encountered _split.json file
    def splits_exist(self) -> (bool | str):
        dir = Path('./splits/')
        pattern = r'.*_splits.json$'

        try:
            for file in dir.iterdir():
                if file.is_file() and re.match(pattern, file.name):
                    
                    with open(dir.name+ '/' + file.name, 'r') as loaded_splits:
                        json_loaded_splits = json.load(loaded_splits)
                        self._set_class_values_from_json(json_loaded_splits)

                    return (True,file.name)
        except:
            print('Error while parsing the *_splits.json file')
        
        return (False,'')