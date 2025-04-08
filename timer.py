from decimal import Decimal
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
        
        self.last_split_start = 0.0
        self.last_split_end = 0.0
        self.last_split_pause_start = 0.0
        self.last_split_pause_end = 0.0
        self.last_split_pause_total = 0.0
        self.last_split_pause_index = 0

        self.is_pb = False
        self.has_gold_splits = False
        self.has_hours = False
        self.running = False
        self.timer_thread = None
        self.split_manager = Splits_Manager()

        self._stop_event = threading.Event()
        self._pause_event = threading.Event()
        
    def save_pb_splits(self):
            #calculate sum of best
            sob: Decimal = 0.0
            for split in self.split_manager.current_best_splits_segment:
                sob += Decimal(list(split.values())[0])
            self.split_manager.sum_of_best = sob

            splits_path = filedialog.asksaveasfilename(defaultextension=".json", filetypes=[("JSON files", "*.json")])

            if splits_path:
                with open(splits_path, "w") as f:

                    run_info_dict = {'game_name': self.split_manager.run_info_unsaved.game_name,
                                     'category': self.split_manager.run_info_unsaved.category,
                                     'attempts': self.split_manager.run_info_unsaved.attempt_count,
                                     'runner_src_account_url': self.split_manager.run_info_unsaved.runner_src_account,
                                     'sum_of_best': self.split_manager.sum_of_best,
                                     'splits_sum': self.split_manager.current_splits_sum,
                                     'best_splits_sum': self.split_manager.current_best_splits_sum,
                                     'splits_segments': self.split_manager.current_splits_segment,
                                     'best_splits_segments': self.split_manager.current_best_splits_segment}
                    
                    json_run_info_and_splits =  run_info_dict
                    json.dump(json_run_info_and_splits, f, indent=4)
    
    def redraw_loaded_values(self):
            for diff in self.split_manager.label_time_diff_list:
                diff.config(text='')
        
            for i in range(len(self.split_manager.label_prev_time_list)):
                prev_time = self.split_manager.loaded_split_values_sum[i]
                self.split_manager.label_prev_time_list[i].config(text=self.format_time_with_diff(prev_time))
    
    def save_json_splits(self):
        if (self.is_pb | self.has_gold_splits) & self._stop_event.is_set() == True:
            response = messagebox.askyesno("Got new records!", "You have beaten some of your records!\nDo you want to save the splits?")
            if response:
                self.save_pb_splits()

    def reset_timer(self, label:Label):

        if self.timer_thread != None:
            self._stop_event.set() 
            self.timer_thread.join()

        self.save_json_splits()
        self.redraw_loaded_values()

        self.start = 0.0
        self.end = 0.0
        self.pause_start = 0.0
        self.pause_end = 0.0
        self.last_split_pause_start = 0.0
        self.last_split_pause_end = 0.0
        self.paused_time_total = 0.0
        self.total_time_diff = 0.0
        self.last_split_start = 0.0
        self.last_split_end = 0.0
        self.last_split_pause_total = 0.0
        self.last_split_pause_index = 0
        self.last_split_paused_split_index = 0
        self.running = False
        self.is_pb = False
        self.has_gold_splits = False

        self.split_manager.run_info_unsaved.attempt_count += 1
        self.split_manager.loaded_split_index = 0

        label.config(text='00:00:000' if self.has_hours == False else '00:00:00:000')
        self._stop_event.clear()
        self._pause_event.clear()
    
    def stop_timer(self):

        if self.timer_thread != None:
            self._stop_event.set() 
            self.timer_thread.join()
        
        self.split_manager.current_splits_sum[-1]
        current_time = ((self.end - self.start) - self.paused_time_total)

        for key in self.split_manager.current_splits_sum[-1]:
            if self.split_manager.current_splits_sum[-1][key] > current_time:
                self.is_pb = True
                self.split_manager.current_splits_sum[-1][key] = current_time

        self.save_json_splits()

        self.end = time.time()
        self.pause_start = 0.0
        self.pause_end = 0.0
        self.last_split_start = 0.0
        self.last_split_end = 0.0
        self.last_split_pause_start = 0.0
        self.last_split_pause_end = 0.0
        self.last_split_pause_total = 0.0
        self.last_split_pause_index = 0
        self.last_split_paused_split_index = 0
        self.paused_time_total = 0.0

        self.running = False
        self.is_pb = False
        self.has_gold_splits = False
        self._stop_event.clear()
        self._pause_event.clear()

    def pause_timer(self):

        if self.running == False:
            return

        if self._pause_event.is_set() == False:
            if (self.last_split_pause_index != self.split_manager.loaded_split_index) | (self.last_split_pause_index == 0):
                self.last_split_pause_start = time.time()

            self.pause_start = time.time()
            self._pause_event.set()

        else:
            self.last_split_pause_end = time.time()
            self.pause_end = time.time()

            self.last_split_pause_total += self.last_split_pause_end - self.last_split_pause_start
            self.paused_time_total += self.pause_end - self.pause_start
            self._pause_event.clear()
    
    def split(self):

        if self.running == False | self._stop_event.is_set() == True:
            return
        
        split_i = self.split_manager.loaded_split_index
        self.last_split_end = time.time()

        def update_split_labels(split_i: int)-> str:
            
            old_time = self.split_manager.loaded_split_values_sum[split_i]
            current_time = ((self.end - self.start) - self.paused_time_total)
            sign = ''

            if old_time >= current_time:
                sign = '-'
                self.total_time_diff += (old_time - current_time) - self.total_time_diff
                
                if self.split_manager.current_best_splits_values_sum[split_i] > current_time:
                    self.has_gold_splits = True
                    for key  in self.split_manager.current_best_splits_sum[split_i]:
                        self.split_manager.current_best_splits_sum[split_i][key] = current_time

            else:
                sign = '+'
                self.total_time_diff += (current_time - old_time) - self.total_time_diff

            self.split_manager.label_prev_time_list[split_i].config(text=self.format_time_without_diff())
            self.split_manager.label_time_diff_list[split_i].config(text=sign+self.format_time_with_diff(self.total_time_diff))

            return sign
        
        def change_curr_splits():
            for key in self.split_manager.current_splits_sum[split_i]:
                self.split_manager.current_splits_sum[split_i][key] = (self.end - self.start) - self.paused_time_total
            
            for key in self.split_manager.current_splits_segment[split_i]:
                self.split_manager.current_splits_segment[split_i][key] = (self.last_split_end - self.last_split_start) - self.last_split_pause_total
            
            for key in self.split_manager.current_best_splits_segment[split_i]:
               segment_time = (self.last_split_end - self.last_split_start) - self.last_split_pause_total
               if self.split_manager.current_best_splits_segment[split_i][key] > segment_time:
                   self.split_manager.current_best_splits_segment[split_i][key] = segment_time

        if split_i + 1 < len(self.split_manager.loaded_split_values_sum):
            
            update_split_labels(split_i)
            change_curr_splits()

            self.last_split_start = time.time()
            self.last_split_pause_end = 0.0
            self.last_split_pause_total = 0.0
            self.split_manager.loaded_split_index += 1
            self.last_split_paused_split_index = self.split_manager.loaded_split_index
            
        if split_i + 1 == len(self.split_manager.loaded_split_values_sum):
            self._stop_event.set()
            self.is_pb = update_split_labels(split_i) == '-' #check if the last split was faster then in the loaded file
            change_curr_splits()

            self.split_manager.loaded_split_index = 0
            self.last_split_paused_split_index = 0
            self.last_split_pause_total = 0.0

    def update_main_timer(self, label:Label):

        while self.running:
            if  self._stop_event.is_set():
                break
            
            if self._pause_event.is_set():
                time.sleep(0.004)
                continue

            self.end = time.time()
            text_formated = self.format_time_without_diff()

            label.config(text=text_formated)
            time.sleep(0.04)

    def start_timer(self, label: Label):

        if self.running == False:
            self.start = time.time()
            self.last_split_start = time.time()
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
        remaining_seconds = int(seconds % 60)
        milliseconds = int((seconds - int(seconds)) * 1000)

        text_formatted = ''

        if not self.has_hours:  # Assuming this flag controls whether hours are included or not
            text_formatted = f'{minutes:02}:{remaining_seconds:02}:{milliseconds:03}'
        else:
            hours = int(minutes // 60)  # Calculate hours properly
            minutes = minutes % 60      # Correct the minutes for hours overflows
            text_formatted = f'{hours:02}:{minutes:02}:{remaining_seconds:02}:{milliseconds:03}'

        return text_formatted
    
    def stop(self):
        self._stop_event.set()
        self.stop_timer() 
