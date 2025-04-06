

import threading
import tkinter
from tkinter import ttk

from components.line import draw_a_line
from preferences.preferences import Minisplit_Preferences
from timer import Timer


def draw_main_timer(mainframe: tkinter.Frame,pref: Minisplit_Preferences ,rows: int, t:Timer) -> (tkinter.Label | int):
    draw_a_line(mainframe,pref,rows,3,pref.line_color)
    rows +=1
    label_main_timer = ttk.Label(mainframe,
                                text='00:00:000' if t.has_hours == False else '00:00:00:000',
                                font=(pref.main_timer_font, pref.main_timer_font_size),
                                foreground=pref.main_timer_foreground,
                                background=pref.main_timer_background,
                                anchor='center')
    label_main_timer.grid(row=rows, columnspan=3, sticky=(tkinter.S,tkinter.E,tkinter.W))
    rows +=1
    threading.Thread(target=t.reset_timer,args=(label_main_timer,),daemon=True).start()
    
    return (label_main_timer,rows)