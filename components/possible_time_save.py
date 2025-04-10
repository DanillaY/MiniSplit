from preferences.preferences import Minisplit_Preferences
from split_manager import Splits_Manager
from tkinter import ttk
import tkinter

def draw_possible_time_save(mainframe: tkinter.Frame, rows: int, pref: Minisplit_Preferences, s_manager: Splits_Manager, t) -> int:
    calculate_possible_time_save(s_manager)

    time_save_text_label =  ttk.Label(mainframe,
                          text= 'Possible time save: ',
                          font=(pref.sob_font, pref.sob_font_size),
                          foreground=pref.sob_foreground,
                          background=pref.sob_background,
                          anchor=(tkinter.W))
    time_save_text_label.grid(row=rows, column=0, columnspan=2, sticky=(tkinter.N,tkinter.S,tkinter.E,tkinter.W))
    
    time_save_label = ttk.Label(mainframe,
                          text= t.format_time_with_diff(s_manager.possible_time_save),
                          font=(pref.sob_font, pref.sob_font_size),
                          foreground=pref.sob_foreground,
                          background=pref.sob_background,
                          anchor=tkinter.E)
    time_save_label.grid(row=rows, column=2,columnspan=3, sticky=(tkinter.E))
    
    s_manager.label_possible_time_save = time_save_label
    s_manager.loaded_possible_time_save = s_manager.possible_time_save

    rows +=1
    return rows

def calculate_possible_time_save(s_manager: Splits_Manager):
    sum_of_current_segment_times = sum(current_split for loaded_segment in s_manager.current_splits_segment for _, current_split in loaded_segment.items())
    sob = sum(s_manager.current_best_splits_values_segment)
    s_manager.possible_time_save = max(0.0, sum_of_current_segment_times - sob)
    