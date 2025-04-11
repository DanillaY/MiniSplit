import threading
from tkinter import Label, Tk
from pynput import keyboard
from autosplitting.socket_server import close_connection
from hotkey_timer import Hotkeys
from vk_enum import vk_codes
from timer import Timer

def start_keyboard_listener(t : Timer, label: Label, root: Tk, hotkeys_config: Hotkeys):
    key_pressed_set = set()

    def on_press(key: keyboard.Key | keyboard.KeyCode):
        vk = key.vk if hasattr(key, 'vk') else key.value.vk
        key_pressed_set.add(vk)

        if key_pressed_set == hotkeys_config.start_key:
            pressed = key_pressed_set.copy()
            if pressed == hotkeys_config.start_key:
                threading.Thread(target=lambda: t.start_timer(label), daemon=True).start()

        if key_pressed_set == hotkeys_config.pause_key:
            t.pause_timer()
        
        if key_pressed_set == hotkeys_config.reset_key:
            t.reset_timer(label)
        
        if key_pressed_set == hotkeys_config.split_key:
            threading.Thread(target=lambda:  t.split(), daemon=True).start()
        
        if key_pressed_set == hotkeys_config.quit_key:
            t.stop()
            close_connection()
            return False
    
    def on_realease(key):
        try:
            vk = key.vk if hasattr(key, 'vk') else key.value.vk
            key_pressed_set.discard(vk)
            print(vk, key_pressed_set)
        except Exception as e:
            print(f'Error in keyboard listener: {str(e)}')
        
               
    with keyboard.Listener(on_press= on_press, on_release=on_realease) as listener:
        listener.join()
        root.quit()