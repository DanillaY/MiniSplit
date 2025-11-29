from hotkey_timer import start_hotkeys_thread
from timer import Timer
from display_detector import detect_display_backend, display, DisplayBackend
from tkinter_manager import tkinter_loop

if __name__ == "__main__": 

    display = detect_display_backend()

    #current support is only for windows and xwayland (could work for x11 but i didnt test it)
    if display != DisplayBackend.X11 and display != DisplayBackend.UNKNOWN and display != DisplayBackend.WAYLAND:
        hotkey_config = start_hotkeys_thread(display)
        t: Timer = Timer()
        tkinter_loop(t, hotkey_config)
    else:
        print("display client is not supported :(")
