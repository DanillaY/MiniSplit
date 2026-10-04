from enum import Enum

from pynput import keyboard


class vk_codes_xwayland(Enum):
    NUMPAD_NUM_0 = 65379
    NUMPAD_NUM_1 = 65367
    NUMPAD_NUM_2 = 65364
    NUMPAD_NUM_3 = 65366
    NUMPAD_NUM_4 = 65361
    NUMPAD_NUM_5 = 65437
    NUMPAD_NUM_6 = 65363
    NUMPAD_NUM_7 = 65360
    NUMPAD_NUM_8 = 65362
    NUMPAD_NUM_9 = 65365

    # NUMPAD_NUM_SLASH = not supported
    # NUMPAD_NUM_STAR = not supported
    # NUMPAD_NUM_MINUS = not supported
    # NUMPAD_NUM_PLUS = not supported
    # NUMPAD_NUM_DOT = not supported

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

    PLUS = 61
    MINUS = 45
    SLASH_RIGHT = 47
    DOT = 46
    COMMA = 44
    SEMICOLON = 44
    QUOTATION_MARK = 39
    SLASH_LEFT = 92

    OPEN_BRACKETS = 91
    CLOSE_BRACKETS = 93

    Q = 113
    W = 119
    E = 101
    R = 114
    T = 116
    Y = 121
    U = 117
    I = 105
    O = 111
    P = 112
    A = 97
    S = 115
    D = 100
    F = 102
    G = 103
    H = 104
    J = 106
    K = 107
    L = 108
    Z = 122
    X = 120
    C = 99
    V = 118
    B = 98
    N = 110
    M = 109

    # use these buttons with combination of shift (because pynput interprets uppercase letters as different keycodes)
    UPPER_Q = 1770
    UPPER_W = 1763
    UPPER_E = 1781
    UPPER_R = 1771
    UPPER_T = 1765
    UPPER_Y = 1774
    UPPER_U = 1767
    UPPER_I = 1787
    UPPER_O = 1789
    UPPER_P = 1786
    UPPER_A = 1766
    UPPER_S = 1785
    UPPER_D = 1783
    UPPER_F = 1761
    UPPER_G = 1776
    UPPER_H = 1778
    UPPER_J = 1775
    UPPER_K = 1772
    UPPER_L = 1764
    UPPER_Z = 1777
    UPPER_X = 1790
    UPPER_C = 1779
    UPPER_V = 1773
    UPPER_B = 1769
    UPPER_N = 1780
    UPPER_M = 1784

    # special keys section
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


SHIFT_ANY = "SHIFT_ANY"

UPPER_TO_BASE = {
    vk_codes_xwayland.UPPER_Q.value: vk_codes_xwayland.Q.value,
    vk_codes_xwayland.UPPER_W.value: vk_codes_xwayland.W.value,
    vk_codes_xwayland.UPPER_E.value: vk_codes_xwayland.E.value,
    vk_codes_xwayland.UPPER_R.value: vk_codes_xwayland.R.value,
    vk_codes_xwayland.UPPER_T.value: vk_codes_xwayland.T.value,
    vk_codes_xwayland.UPPER_Y.value: vk_codes_xwayland.Y.value,
    vk_codes_xwayland.UPPER_U.value: vk_codes_xwayland.U.value,
    vk_codes_xwayland.UPPER_I.value: vk_codes_xwayland.I.value,
    vk_codes_xwayland.UPPER_O.value: vk_codes_xwayland.O.value,
    vk_codes_xwayland.UPPER_P.value: vk_codes_xwayland.P.value,
    vk_codes_xwayland.UPPER_A.value: vk_codes_xwayland.A.value,
    vk_codes_xwayland.UPPER_S.value: vk_codes_xwayland.S.value,
    vk_codes_xwayland.UPPER_D.value: vk_codes_xwayland.D.value,
    vk_codes_xwayland.UPPER_F.value: vk_codes_xwayland.F.value,
    vk_codes_xwayland.UPPER_G.value: vk_codes_xwayland.G.value,
    vk_codes_xwayland.UPPER_H.value: vk_codes_xwayland.H.value,
    vk_codes_xwayland.UPPER_J.value: vk_codes_xwayland.J.value,
    vk_codes_xwayland.UPPER_K.value: vk_codes_xwayland.K.value,
    vk_codes_xwayland.UPPER_L.value: vk_codes_xwayland.L.value,
    vk_codes_xwayland.UPPER_Z.value: vk_codes_xwayland.Z.value,
    vk_codes_xwayland.UPPER_X.value: vk_codes_xwayland.X.value,
    vk_codes_xwayland.UPPER_C.value: vk_codes_xwayland.C.value,
    vk_codes_xwayland.UPPER_V.value: vk_codes_xwayland.V.value,
    vk_codes_xwayland.UPPER_B.value: vk_codes_xwayland.B.value,
    vk_codes_xwayland.UPPER_N.value: vk_codes_xwayland.N.value,
    vk_codes_xwayland.UPPER_M.value: vk_codes_xwayland.M.value,
}

SHIFT_L_VK = vk_codes_xwayland.SHIFT_L.value.value.vk
SHIFT_R_VK = vk_codes_xwayland.SHIFT_R.value.value.vk


def is_special_key(key: int | keyboard.Key):
    if isinstance(key.value, int):
        return False

    elif isinstance(key.value, keyboard.Key):
        return True


def normalize(vk):
    if vk in (SHIFT_L_VK, SHIFT_R_VK):
        return SHIFT_ANY
    return UPPER_TO_BASE.get(vk, vk)
