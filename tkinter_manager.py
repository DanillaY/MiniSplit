import threading
import tkinter
from autosplitting.socket_server import close_connection, socket_server_wrap
from components.main_timer import draw_main_timer
from components.possible_time_save import draw_possible_time_save
from components.sum_of_best import draw_sum_of_best
from components.splits import draw_splits
from components.title import draw_title
from hotkey_timer import Hotkeys
from preferences.preferences import Minisplit_Preferences
from timer import Timer
from tkinter import *
from keyboard_listener import start_keyboard_listener

class Tkinter_manager:

    def __init__(self, root: Tk, preference_settings: Minisplit_Preferences, t:Timer):
        self.root = root
        self.preference_settings = preference_settings
        self.t = t
        
        self.label_main_timer = None

    def draw_all_minisplit_components(self, split_file_exist: bool, file_name:str):
        mainframe = tkinter.Frame(self.root, background=self.preference_settings.main_frame_color)
        mainframe.grid(column=0, row=0, sticky=(tkinter.N, tkinter.W, tkinter.E, tkinter.S))

        rows = draw_title(mainframe,self.t.split_manager,self.preference_settings,0,True)
        rows = draw_splits(mainframe, self.t, True,self.preference_settings , split_file_exist, file_name, rows)

        label_main_timer,rows = draw_main_timer(mainframe,self.preference_settings,rows,self.t)
        rows = draw_sum_of_best(mainframe, rows,self.preference_settings,self.t.split_manager,self.t)
        rows = draw_possible_time_save(mainframe, rows,self.preference_settings,self.t.split_manager,self.t)
        
        return label_main_timer

def tkinter_loop(t : Timer, hotkeys_config: Hotkeys):

    root = Tk()
    root.title('MiniSplit')
    root.attributes('-topmost', True)
    root.grid_rowconfigure(0, weight=1)
    root.grid_columnconfigure(0, weight=1)
    
    split_file_exist,file_name = t.split_manager.load_split_json()
    preference_settings = Minisplit_Preferences()
    tk_manager = Tkinter_manager(root, preference_settings, t)
    
    label_main_timer = tk_manager.draw_all_minisplit_components(split_file_exist, file_name)
    t.split_manager.label_main_timer = label_main_timer
    
    threading.Thread(target=socket_server_wrap,args=(t, t.split_manager.label_main_timer,),daemon=True).start()
    threading.Thread(target=start_keyboard_listener,args=(tk_manager,hotkeys_config,),daemon=True).start()

    def on_closing():
        close_connection()
        root.quit()

    root.protocol('WM_DELETE_WINDOW', on_closing)
    root.mainloop()
