
import tkinter

from components.line import draw_a_line
from preferences.prederences import Mini_Split_Preferences
from split_manager import Splits_Manager


def draw_title(mainframe: tkinter.Frame, split_manager:Splits_Manager , pref:Mini_Split_Preferences ,rows_count: int, with_attempts: bool) -> int:
    
    #draw title row
    label_game_title = tkinter.Label(mainframe,
                                    text= split_manager.run_info_loaded.game_name,
                                    font=(pref.title_game_name_font, pref.title_game_name_font_size),
                                    foreground=pref.title_game_name_foreground,
                                    background=pref.title_game_name_background,
                                    anchor='center')
    
    label_game_title.grid(row=rows_count, columnspan=3, sticky=(tkinter.N,tkinter.S,tkinter.E,tkinter.W))
    rows_count += 1

    #draw category with/without attempts row
    label_category_text = split_manager.run_info_loaded.category + ' ('+ str(split_manager.run_info_loaded.attempt_count) +')' if with_attempts else split_manager.run_info_loaded.category
    label_category = tkinter.Label(mainframe,
                                   text=label_category_text,
                                   font=(pref.title_category_font, pref.title_category_font_size),
                                   foreground=pref.title_category_foreground,
                                   background=pref.title_category_background,
                                   anchor='center')
    
    label_category.grid(row=rows_count, columnspan=3,  sticky=(tkinter.N,tkinter.S,tkinter.E,tkinter.W))
    rows_count += 1

    draw_a_line(mainframe,pref,rows_count,3,pref.line_color)
    rows_count +=1

    return rows_count