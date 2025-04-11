import threading
from tkinter import Label, filedialog
from pynput import keyboard
from autosplitting.socket_server import close_connection
from hotkey_timer import Hotkeys

def start_keyboard_listener(tk_manager, hotkeys_config: Hotkeys):
    key_pressed_set = set()

    def on_press(key: keyboard.Key | keyboard.KeyCode):
        vk = key.vk if hasattr(key, 'vk') else key.value.vk
        key_pressed_set.add(vk)

        if key_pressed_set == hotkeys_config.start_key:
            #we have to start a start_timer function in a new thread so that if the run is a pb run the tkinter messagebox would not prevent the on_realease call 
            pressed = key_pressed_set.copy()
            if pressed == hotkeys_config.start_key:
                threading.Thread(target=lambda: tk_manager.t.start_timer(), daemon=True).start()

        if key_pressed_set == hotkeys_config.pause_key:
            tk_manager.t.pause_timer()
        
        if key_pressed_set == hotkeys_config.reset_key:
            threading.Thread(target=lambda: tk_manager.t.reset_timer(), daemon=True).start()
        
        if key_pressed_set == hotkeys_config.split_key:
            threading.Thread(target=lambda:  tk_manager.t.split(), daemon=True).start()
        
        if key_pressed_set == hotkeys_config.load_split_key:
            def load_split():
                path_str = filedialog.askopenfilename()
                path_str = path_str.split('/splits/')[-1]

                split_file_exist, file_name = tk_manager.t.split_manager.load_split_json(path_str)
                label_main_timer = tk_manager.draw_all_minisplit_components(split_file_exist, file_name)
                tk_manager.label_main_timer = label_main_timer

            pressed = key_pressed_set.copy()
            if pressed == hotkeys_config.load_split_key:
                threading.Thread(target=lambda:load_split(), daemon=True).start()
        
        if key_pressed_set == hotkeys_config.quit_key:
            tk_manager.t.stop()
            close_connection()
            return False
    
    def on_realease(key):
        try:
            vk = key.vk if hasattr(key, 'vk') else key.value.vk
            key_pressed_set.discard(vk)
        except Exception as e:
            print(f'Error in keyboard listener: {str(e)}')
        
               
    with keyboard.Listener(on_press= on_press, on_release=on_realease) as listener:
        listener.join()
        tk_manager.root.quit()