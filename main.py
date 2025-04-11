from hotkey_timer import start_hotkeys_thread
from timer import Timer
from tkinter_manager import tkinter_loop

if __name__ == "__main__": 

    hotkey_config = start_hotkeys_thread()
    t: Timer = Timer()
    tkinter_loop(t, hotkey_config)
