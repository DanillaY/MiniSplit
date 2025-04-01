from tkinter import Label, Tk
from pynput import keyboard
from hotkey_timer import Hotkeys
from split_manager import Splits_Manager
from vk_enum import vk_codes
from timer import Timer

def start_keyboard_listener(t : Timer, label: Label, root: Tk, hotkeys_config: Hotkeys):
    def on_press(key: keyboard.Key | keyboard.KeyCode):

        if hasattr(key,'vk'):

            if key.vk == hotkeys_config.start_key:
                t.start_timer(label)

            if key.vk == hotkeys_config.pause_key:
                t.pause_timer()
            
            if key.vk == hotkeys_config.reset_key:
                t.reset_timer(label)
            
            if key.vk == hotkeys_config.split_key:
                t.split()
                
        #close the whole program
        # if key == keyboard.Key.esc:
        #     t.stop()
        #     if t.timer_thread != None:
        #         t.timer_thread.join()
                
        #     return False TODO create keybinds

               
    with keyboard.Listener(on_press= on_press) as listener:
        listener.join()
        root.quit()