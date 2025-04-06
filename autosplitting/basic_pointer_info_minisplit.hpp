#include <cstdint>
#include <vector>
#include <map>
#include "signals_minisplit.hpp"

#pragma once


template <typename T>
class Basic_Pointer_Info {

    public:
        unsigned int offsets_len;
        std::vector<uintptr_t> offsets;
        bool with_change_back;  //with_change_back param is used to send the command twice. once it detects that the current value has changed to the desired one, it sends the signal again after the current value changes, this is useful for tracking things like in-game time, where you can send a PAUSE signal and then unpause
        bool with_compare_prev;
        T buffer;
        T compared_to; 
        T compared_to_prev;

        //use T compared_to or std::vector<T> compared_to_vec, compared_to_vec is used for comparing multiple current and previous values so that you wont have to initialize a new pointer class for each value but if you need to compare only 1 value then use T buffer instead 
        std::unordered_set<std::string> compared_to_set;
        std::unordered_set<std::string> compared_to_prev_set;

        Signal_split sig;

        Basic_Pointer_Info(int offsets_len, std::vector<uintptr_t> offsets,T buffer,T compared_to, T compared_to_prev, bool with_change_back, bool with_compare_prev, Signal_split sig) {
            this->offsets_len = offsets_len;
            this->offsets = offsets;
            this->buffer = buffer;
            this->compared_to = compared_to;
            this->compared_to_prev = compared_to_prev;
            this->with_change_back  =with_change_back;
            this->with_compare_prev = with_compare_prev;
            this->sig = sig;
        }

        
        Basic_Pointer_Info(int offsets_len, std::vector<uintptr_t> offsets,T buffer,std::unordered_set<T> compared_to, std::unordered_set<T> compared_to_prev, bool with_change_back, bool with_compare_prev, Signal_split sig) {
            
            this->offsets_len = offsets_len;
            this->offsets = offsets;
            this->buffer = buffer;
            this->compared_to_set = compared_to;
            this->compared_to_prev_set = compared_to_prev;
            this->with_change_back  =with_change_back;
            this->with_compare_prev = with_compare_prev;
            this->sig = sig;
        }
        
};