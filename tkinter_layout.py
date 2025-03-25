import threading
import tkinter
from components.main_timer import draw_main_timer
from components.splits import draw_splits
from components.title import draw_title
from hotkey_timer import Hotkeys
from preferences.preferences import Mini_Split_Preferences
from timer import Timer
from tkinter import *
from keyboard_listener import start_keyboard_listener

def tkinter_loop(t : Timer, hotkeys_config: Hotkeys):

    root = Tk()
    root.title('MiniSplit')
    root.grid_rowconfigure(0, weight=1)
    root.grid_columnconfigure(0, weight=1)
    
    split_file_exist,file_name = t.split_manager.splits_exist()
    preference_settings = Mini_Split_Preferences()
    
    mainframe = tkinter.Frame(root, background=preference_settings.main_frame_color)
    mainframe.grid(column=0, row=0, sticky=(tkinter.N, tkinter.W, tkinter.E, tkinter.S))

    rows = draw_title(mainframe,t.split_manager,preference_settings,0,True)
    rows = draw_splits(mainframe, t, True,preference_settings , split_file_exist, file_name, rows)

    label_main_timer = draw_main_timer(mainframe,preference_settings,rows,t)

    threading.Thread(target=start_keyboard_listener,args=(t,label_main_timer,root,hotkeys_config,),daemon=True).start()

    def on_closing():
        t.stop()
        root.quit()

    root.protocol('WM_DELETE_WINDOW', on_closing)
    root.mainloop()