import json
from pathlib import Path
from queue import Queue
import queue
import threading
from pynput import keyboard

from vk_enum import vk_codes

Key_or_vk_code = keyboard.Key | int

class Hotkeys:
    def __init__(self, split_key: Key_or_vk_code,
                 start_key: Key_or_vk_code ,
                 pause_key: Key_or_vk_code ,
                 reset_key: Key_or_vk_code):
        
        self.start_key = start_key
        self.pause_key = pause_key
        self.reset_key = reset_key
        self.split_key = split_key

def init_hotkeys_config(config_queue: Queue):

    config_path = Path('hotkeys.json')
    hotkey_config = Hotkeys(vk_codes.NUMPAD_NUM_0,
                            vk_codes.NUMPAD_NUM_1.value,
                            vk_codes.NUMPAD_NUM_2.value,
                            vk_codes.NUMPAD_NUM_3.value) #those are the default values

    if config_path.is_file() == False:
        json_hotkeys_default = {'split': 'NUMPAD_NUM_0','start/stop_timer':'NUMPAD_NUM_1', 'pause/unpause_timer':'NUMPAD_NUM_2', 'reset_timer':'NUMPAD_NUM_3'}
        with open('hotkeys.json', 'w') as hotkeys:
            json.dump(json_hotkeys_default, hotkeys, indent=4)
            
    else:
        try:
            with open('hotkeys.json', 'r') as hotkeys:
                json_hotkeys = json.load(hotkeys)

                hotkey_config.start_key = vk_codes[json_hotkeys['start/stop_timer']].value
                hotkey_config.pause_key = vk_codes[json_hotkeys['pause/unpause_timer']].value
                hotkey_config.reset_key = vk_codes[json_hotkeys['reset_timer']].value
                hotkey_config.split_key = vk_codes[json_hotkeys['split']].value
        except:
            print('Error while parsing the hotkey values, check if thers an incorrect key name or delete hotkeys.json to create a default config')    

    config_queue.put(hotkey_config) 

def start_hotkeys_thread() -> Hotkeys:
    config_queue = queue.Queue()
    config_thread = threading.Thread(target=init_hotkeys_config,args=(config_queue,))
    config_thread.start()
    config_thread.join()
    return config_queue.get()