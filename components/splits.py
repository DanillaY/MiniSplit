import json
import tkinter
from tkinter import ttk

from components.line import draw_a_line
from preferences.prederences import Mini_Split_Preferences
from split_manager import Splits_Manager
from timer import Timer


def draw_splits(mainframe: tkinter.Frame, t: Timer, draw_with_split_desc: bool, pref: Mini_Split_Preferences, split_file_exist:bool, file_name:str, rows_count: int) -> int:

    def draw_splits_description(rows_count: int) -> int:
        label_name_column = ttk.Label(mainframe,
                                      text= 'split name',
                                      font=(pref.splits_desc_name_column_font, pref.splits_desc_name_column_font_size),
                                      foreground=pref.splits_desc_name_column_foreground,
                                      background=pref.splits_desc_name_column_background,
                                      anchor='center')
        
        label_name_column.grid(row=rows_count, column=0, sticky=(tkinter.N,tkinter.S,tkinter.E, tkinter.W))
        mainframe.grid_columnconfigure(0, minsize=pref.line_min_width/3)
            
        label_diff_column = ttk.Label(mainframe,
                                      text= '+/-',
                                      font=(pref.splits_desc_diff_column_font, pref.splits_desc_diff_column_font_size),
                                      foreground=pref.splits_desc_diff_column_foreground,
                                      background=pref.splits_desc_diff_column_background,
                                      anchor='center')
        
        label_diff_column.grid(row=rows_count, column=1, sticky=(tkinter.N,tkinter.S,tkinter.E, tkinter.W))
        mainframe.grid_columnconfigure(1, minsize=pref.line_min_width/3)
            
        label_prev_time_column = ttk.Label(mainframe,
                                          text= 'compare to',
                                          font=(pref.splits_desc_prev_time_column_font, pref.splits_desc_prev_time_column_font_size),
                                          foreground=pref.splits_desc_prev_time_column_foreground,
                                          background=pref.splits_desc_prev_time_column_background,
                                          anchor='center')
        
        label_prev_time_column.grid(row=rows_count, column=2, sticky=(tkinter.N,tkinter.S,tkinter.E, tkinter.W))
        mainframe.grid_columnconfigure(2, minsize=pref.line_min_width/3)

        draw_a_line(mainframe,pref,rows_count+1,3,pref.line_color)
        rows_count += 2
        
        return rows_count

    def configure_rows(json_splits: dict, rows_count: int) -> int:
        
        if draw_with_split_desc:
            rows_count = draw_splits_description(rows_count)
            
        for split in json_splits['splits']:
            
            for split_name,split_time in split.items():
                column = 0
                
                mainframe.grid_rowconfigure(rows_count, weight=1)

                for col in range(3):
                    mainframe.grid_columnconfigure(col, weight=1, uniform='equal')

                #fill the first column with the split name
                label_split_name = ttk.Label(mainframe,
                                            text= split_name,
                                            font=(pref.splits_split_name_font, pref.splits_split_name_font_size),
                                            foreground=pref.splits_split_name_foreground,
                                            background=pref.splits_split_name_background,
                                            anchor='center')
                label_split_name.grid(row=rows_count, column=column, sticky=(tkinter.N,tkinter.S,tkinter.E, tkinter.W))

                #fill the second column with the timesave/timelose (empty from the start)
                column += 1
                label_time_diff = ttk.Label(mainframe,
                                            text= '',
                                            font=(pref.splits_time_diff_font, pref.splits_time_diff_font_size),
                                            foreground=pref.splits_time_diff_foreground,
                                            background=pref.splits_time_diff_background,
                                            anchor='e')
                label_time_diff.grid(row=rows_count, column=column, sticky=(tkinter.E))
                t.split_manager.label_time_diff_list.append(label_time_diff)

                #fill the third column with the loaded time from _splits.json file
                column += 1
                label_prev_time = ttk.Label(mainframe,
                                            text= t.format_time_with_diff(float(split_time)),
                                            font=(pref.splits_prev_time_font, pref.splits_prev_time_font_size),
                                            foreground=pref.splits_prev_time_foreground,
                                            background=pref.splits_prev_time_background,
                                            anchor='center')
                label_prev_time.grid(row=rows_count, column=column, sticky=(tkinter.N,tkinter.S,tkinter.E, tkinter.W))
                t.split_manager.label_prev_time_list.append(label_prev_time)

                rows_count+= 1

        return rows_count
    

    try:
        if split_file_exist:
            with open(f'./splits/{file_name}', 'r') as splits:
                json_splits = json.load(splits)
                rows_count = configure_rows(json_splits,rows_count)

        else:
            with open(f'./splits/example_splits.json', 'w') as splits:
                json_splits_default = { 'game_name': 'thebestgameever','category': '100%','attempts': 10,'runner_src_account_url': '','sum_of_best': 99.89995,'personal_best': 115.0035,'splits':[{'split_name1': 88.35235},{'split_name2': 99.35235},{'split_name3': 100.3511},{'split_name4': 110.37535},{'split_name5': 115.0035}]}   
                rows_count = configure_rows(json_splits_default,rows_count)
                json.dump(json_splits_default, splits, indent=4)
    except Exception as e:
        print('Error while drawing the labels from splits\n', str(e))
    
    return rows_count