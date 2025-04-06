from preferences.preferences import Minisplit_Preferences
from split_manager import Splits_Manager
from timer import Timer
from tkinter import ttk
import tkinter

def draw_sum_of_best(mainframe: tkinter.Frame, rows: int, pref: Minisplit_Preferences, s_manager: Splits_Manager, t : Timer) -> int:
	
	sob_text_label =  ttk.Label(mainframe,
					      text= 'Sum of best: ',
                          font=(pref.sob_font, pref.sob_font_size),
                          foreground=pref.sob_foreground,
                          background=pref.sob_background,
                          anchor=(tkinter.W))
	sob_text_label.grid(row=rows, column=0, columnspan=2, sticky=(tkinter.N,tkinter.S,tkinter.E,tkinter.W))
	
	sob_label = ttk.Label(mainframe,
					      text= t.format_time_with_diff(s_manager.sum_of_best),
                          font=(pref.sob_font, pref.sob_font_size),
                          foreground=pref.sob_foreground,
                          background=pref.sob_background,
                          anchor=tkinter.E)
	sob_label.grid(row=rows, column=2,columnspan=3, sticky=(tkinter.E))

	rows +=1
	return rows