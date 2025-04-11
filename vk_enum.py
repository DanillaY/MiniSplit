from enum import Enum
from pynput import keyboard

class vk_codes(Enum):

    NUMPAD_NUM_0 = 96 
    NUMPAD_NUM_1 = 97
    NUMPAD_NUM_2 = 98
    NUMPAD_NUM_3 = 99
    NUMPAD_NUM_4 = 100
    NUMPAD_NUM_5 = 101
    NUMPAD_NUM_6 = 102
    NUMPAD_NUM_7 = 103
    NUMPAD_NUM_8 = 104
    NUMPAD_NUM_9 = 105
    NUMPAD_NUM_SLASH = 111
    NUMPAD_NUM_STAR = 106
    NUMPAD_NUM_MINUS = 109
    NUMPAD_NUM_PLUS = 107
    NUMPAD_NUM_DOT = 110

    NUM_0 = 48 
    NUM_1 = 49
    NUM_2 = 50
    NUM_3 = 51
    NUM_4 = 52
    NUM_5 = 53
    NUM_6 = 54
    NUM_7 = 55
    NUM_8 = 56
    NUM_9 = 57

    PLUS = 187
    MINUS = 189
    SLASH_RIGHT = 191
    DOT = 190
    COMMA = 188
    SEMICOLON = 186
    QUOTATION_MARK = 222
    SLASH_LEFT = 220

    OPEN_BRACKETS = 219
    CLOSE_BRACKETS = 221

    Q = 81
    W = 87
    E = 69
    R = 82
    T = 84
    Y = 89
    U = 85
    I = 73
    O = 79
    P = 80
    A = 65
    S = 83
    D = 68
    F = 70
    G = 71
    H = 72
    J = 74
    K = 75
    L = 76
    Z = 90
    X = 88
    C = 67
    V = 86
    B = 66
    N = 78
    M = 77

    #special keys section
    F1 = keyboard.Key.f1
    F2 = keyboard.Key.f2
    F3 = keyboard.Key.f3
    F4 = keyboard.Key.f4
    F5 = keyboard.Key.f5
    F6 = keyboard.Key.f6
    F7 = keyboard.Key.f7
    F8 = keyboard.Key.f8
    F9 = keyboard.Key.f9
    F10 = keyboard.Key.f10
    F11 = keyboard.Key.f11
    F12 = keyboard.Key.f12
    ESC = keyboard.Key.esc
    TAB = keyboard.Key.tab
    CAPS = keyboard.Key.caps_lock
    SHIFT_L = keyboard.Key.shift_l
    SHIFT_R = keyboard.Key.shift_r
    CTRL_L = keyboard.Key.ctrl_l
    CTRL_R = keyboard.Key.ctrl_r
    
def is_special_key(key: int | keyboard.Key):
    if isinstance(key.value, int):
        return False
    
    elif isinstance(key.value, keyboard.Key):
        return True