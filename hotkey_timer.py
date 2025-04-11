import json
from pathlib import Path
from queue import Queue
import queue
import threading

from vk_enum import is_special_key, vk_codes

class Hotkeys:
    def __init__(self, split_keys: set,
                 start_keys: set ,
                 pause_keys: set ,
                 reset_keys: set, 
                 quit_keys: set,
                 load_key: set):
        
        self.start_key = start_keys
        self.pause_key = pause_keys
        self.reset_key = reset_keys
        self.split_key = split_keys
        self.quit_key = quit_keys
        self.load_split_key = load_key

def parse_combination_to_keys(json_hotkeys: dict, key_command: str):
        result_combination = set()
        key_names = json_hotkeys[key_command].split(',') 

        for key in key_names:
            key = key.upper()
            if key in vk_codes.__members__:
                result_combination.add(vk_codes[key].value.value.vk if is_special_key(vk_codes[key]) else vk_codes[key].value)

        return result_combination

def init_hotkeys_config(config_queue: Queue):

    config_path = Path('hotkeys.json')
    hotkey_config = Hotkeys([vk_codes.NUMPAD_NUM_0],
                            [vk_codes.NUMPAD_NUM_1],
                            [vk_codes.NUMPAD_NUM_2],
                            [vk_codes.NUMPAD_NUM_3],
                            [vk_codes.SHIFT_L, vk_codes.Q],
                            [vk_codes.SHIFT_L,vk_codes.L]) #those are the default values

    if config_path.is_file() == False:
        json_hotkeys_default = {'split': 'NUMPAD_NUM_0','start/stop_timer':'NUMPAD_NUM_1', 'pause/unpause_timer':'NUMPAD_NUM_2', 'reset_timer':'NUMPAD_NUM_3', 'quit':'SHIFT_L,Q', 'load_split':'SHIFT_L,L'} 
        with open('hotkeys.json', 'w') as hotkeys:
            json.dump(json_hotkeys_default, hotkeys, indent=4)
            
    else:
        try:
            with open('hotkeys.json', 'r') as hotkeys:
                json_hotkeys = json.load(hotkeys)

                hotkey_config.start_key = parse_combination_to_keys(json_hotkeys,'start/stop_timer')
                hotkey_config.pause_key = parse_combination_to_keys(json_hotkeys,'pause/unpause_timer')
                hotkey_config.reset_key = parse_combination_to_keys(json_hotkeys,'reset_timer')
                hotkey_config.split_key = parse_combination_to_keys(json_hotkeys,'split')
                hotkey_config.quit_key = parse_combination_to_keys(json_hotkeys,'quit')
                hotkey_config.load_split_key = parse_combination_to_keys(json_hotkeys,'load_split')
        except:
            print('Error while parsing the hotkey values, check if thers an incorrect key name or delete hotkeys.json to create a default config')    

    config_queue.put(hotkey_config) 

def start_hotkeys_thread() -> Hotkeys:
    config_queue = queue.Queue()
    config_thread = threading.Thread(target=init_hotkeys_config,args=(config_queue,))
    config_thread.start()
    config_thread.join()
    return config_queue.get()