import json
from pathlib import Path
import re

class Run_Info():
    def __init__(self, game_name:str, category:str,attempt_count:int,runner_src_account_url:str = ''):
        self.game_name = game_name
        self.category = category
        self.attempt_count = attempt_count
        self.runner_src_account = runner_src_account_url

class Splits_Manager():
    def __init__(self):
        #current_splits are not the same as splits that are beeing loaded from splits folder, current split means the one that wasnt saved in a file yet
        self.current_splits = {}
        self.loaded_splits = {}
        
        self.label_time_diff_list = list()
        self.label_prev_time_list = list()
        self.loaded_split_index = 0
        
        self.loaded_split_names = list()
        self.loaded_split_values = list()
        
        self.run_info_loaded = Run_Info('','',0)
        self.run_info_unsaved = Run_Info('','',0)

        self.sum_of_best = 0.0
        self.personal_best = 0.0
    
    def _set_class_values_from_json(self, json_loaded_splits):
        self.loaded_splits = json_loaded_splits['splits']
        self.current_splits = json_loaded_splits['splits']

        self.loaded_split_names.extend([list(split.keys())[0] for split in json_loaded_splits['splits']])
        self.loaded_split_values.extend([list(split.values())[0] for split in json_loaded_splits['splits']])
        
        self.run_info_loaded.game_name = json_loaded_splits['game_name']
        self.run_info_loaded.category = json_loaded_splits['category']
        self.run_info_loaded.attempt_count = int(json_loaded_splits['attempts'])
        self.run_info_loaded.runner_src_account = json_loaded_splits['runner_src_account_url']
        self.run_info_unsaved = self.run_info_loaded
        
        self.sum_of_best = float(json_loaded_splits['sum_of_best'])
        self.personal_best = float(json_loaded_splits['personal_best'])

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