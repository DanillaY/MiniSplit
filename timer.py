import json
import threading
import time
from tkinter import Label, filedialog, messagebox

from split_manager import Splits_Manager

class Timer:
    def __init__(self):
        self.start = 0.0
        self.end = 0.0
        self.pause_start = 0.0
        self.pause_end = 0.0
        self.paused_time_total = 0.0
        self.total_time_diff = 0.0

        self.is_pb = False
        self.has_hours = False
        self.running = False
        self.timer_thread = None
        self.split_manager = Splits_Manager()
        self._stop_event = threading.Event()
        self._pause_event = threading.Event()

    def reset_timer(self, label:Label):

        def redraw_loaded_values():
            for diff in self.split_manager.label_time_diff_list:
                diff.config(text='')
        
            for i in range(len(self.split_manager.label_prev_time_list)):
                prev_time = self.split_manager.loaded_split_values[i]
                self.split_manager.label_prev_time_list[i].config(text=self.format_time_with_diff(prev_time))
        
        def save_pb_splits():
            splits_path = filedialog.asksaveasfilename(defaultextension=".json", filetypes=[("JSON files", "*.json")])

            if splits_path:
                with open(splits_path, "w") as f:

                    run_info_dict = {'game_name': self.split_manager.run_info_unsaved.game_name,
                                     'category': self.split_manager.run_info_unsaved.category,
                                     'attempts': self.split_manager.run_info_unsaved.attempt_count,
                                     'runner_src_account_url': self.split_manager.run_info_unsaved.runner_src_account,
                                     'sum_of_best': self.split_manager.sum_of_best,
                                     'personal_best': self.split_manager.personal_best ,
                                     'splits': self.split_manager.current_splits}
                    
                    json_run_info_and_splits =  run_info_dict
                    json.dump(json_run_info_and_splits, f, indent=4)
        
        if self.timer_thread != None:
            self._stop_event.set() 
            self.timer_thread.join()

        if self.is_pb:
            response = messagebox.askyesno("New personal best!", "You got a new personal best!\nDo you want to save the splits?")
            if response:
                save_pb_splits()
            
        redraw_loaded_values()

        self.start = 0.0
        self.end = 0.0
        self.pause_start = 0.0
        self.pause_end = 0.0
        self.paused_time_total = 0.0
        self.total_time_diff = 0.0
        self.running = False
        self.is_pb = False

        self.split_manager.run_info_unsaved.attempt_count += 1
        self.split_manager.loaded_split_index = 0

        label.config(text='00:00:000' if self.has_hours == False else '00:00:00:000')
        self._stop_event.clear()
        self._pause_event.clear()
    
    def stop_timer(self):

        if self.timer_thread != None:
            self._stop_event.set() 
            self.timer_thread.join()

        self.end = time.time()
        self.pause_start = 0.0
        self.pause_end = 0.0
        self.paused_time_total = 0.0

        self.running = False
        self._stop_event.clear()
        self._pause_event.clear()

    def pause_timer(self):

        if self.running == False:
            return

        if self._pause_event.is_set() == False:
            self.pause_start = time.time()
            self._pause_event.set()

        else:
            self.pause_end = time.time()
            self.paused_time_total += self.pause_end - self.pause_start
            self._pause_event.clear()
    
    def split(self):

        if self.running == False:
            return
        
        split_i = self.split_manager.loaded_split_index

        def update_split_labels(split_i: int)-> str:
            
            old_time = self.split_manager.loaded_split_values[split_i]
            current_time = ((self.end - self.start) - self.paused_time_total)
            sign = ''

            if old_time >= current_time:
                sign = '-'
                self.total_time_diff += (old_time - current_time) - self.total_time_diff
            else:
                sign = '+'
                self.total_time_diff += (current_time - old_time) - self.total_time_diff

            self.split_manager.label_prev_time_list[split_i].config(text=self.format_time_without_diff())
            self.split_manager.label_time_diff_list[split_i].config(text=sign+self.format_time_with_diff(self.total_time_diff))

            return sign
        
        def change_curr_splits():
            for key in self.split_manager.current_splits[split_i]:
                self.split_manager.current_splits[split_i][key] = (self.end - self.start) - self.paused_time_total

        if split_i + 1 < len(self.split_manager.loaded_split_values):
            update_split_labels(split_i)
            change_curr_splits()

            self.split_manager.loaded_split_index += 1
        
        if split_i + 1 == len(self.split_manager.loaded_split_values):
            self._stop_event.set()
            self.is_pb = update_split_labels(split_i) == '-' #check if the last split was faster then in the loaded file
            change_curr_splits()

            self.split_manager.loaded_split_index = 0

    def update_main_timer(self, label:Label):

        while self.running:
            if  self._stop_event.is_set():
                break
            
            if self._pause_event.is_set():
                time.sleep(0.03)
                continue
                
            self.end = time.time()
            text_formated = self.format_time_without_diff()

            label.config(text=text_formated)
            time.sleep(0.02)

    def start_timer(self, label: Label):

        if self.running == False:
            self.start = time.time()
            self.running = True
            self.timer_thread = threading.Thread(target=self.update_main_timer,args=(label,),daemon=True)
            self.timer_thread.start()
        else:
            self.stop_timer()
    
    #function that calculates the difference between start and end of the timer to get seconds
    def format_time_without_diff(self) -> str:
        seconds = (self.end - self.start) - self.paused_time_total
        return self.format_time_with_diff(seconds)

    #main calculating function
    def format_time_with_diff(self, seconds:float) -> str:
        minutes = int(seconds // 60)
        hours = int(minutes // 60)
        remaining_seconds = int(seconds % 60)
        milliseconds = int((seconds - int(seconds)) * 1000)

        text_formated = ''

        if self.has_hours == False:
            text_formated = f'{minutes:02}:{remaining_seconds:02}:{milliseconds:03}'
        else:
            text_formated = f'{hours:02}:{minutes:02}:{remaining_seconds:02}:{milliseconds:03}'
        
        return text_formated
    
    def stop(self):
        self._stop_event.set()
        self.stop_timer() 
