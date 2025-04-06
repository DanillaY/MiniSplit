import tkinter
from preferences.preferences import Minisplit_Preferences

def draw_a_line(mainframe: tkinter.Frame,pref: Minisplit_Preferences, row: int, columns: int, color:str):
    canvas = tkinter.Canvas(mainframe, width=pref.line_min_width, height=pref.line_height, bg=color, highlightthickness=0)
    canvas.grid(row=row, columnspan=columns, sticky=(tkinter.E, tkinter.W))