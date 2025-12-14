#include "../includes/io.h"
#include "../includes/isr.h"
#include "../includes/printk.h"
#include "../includes/tty.h"

typedef enum KB_STATE
{
    KB_NORMAL_MODE,
    KB_EXTENDED_MODE
} keyboard_state_t;

static keyboard_state_t kb_state = KB_NORMAL_MODE;

typedef enum
{
    KEY_UNKNOWN = 0,
    KEY_ARROW_UP = 128,
    KEY_ARROW_DOWN = 129,
    KEY_ARROW_LEFT = 130,
    KEY_ARROW_RIGHT = 131,
    KEY_HOME = 132,
    KEY_END = 133,
    KEY_PAGE_UP = 134,
    KEY_PAGE_DOWN = 135,
    KEY_INSERT = 136,
    KEY_DELETE = 137,
    KEY_F1 = 138,
    KEY_F2 = 139,
    KEY_F3 = 140,
    KEY_F4 = 141,
    KEY_F5 = 142,
    KEY_F6 = 143,
    KEY_F7 = 144,
    KEY_F8 = 145,
    KEY_F9 = 146,
    KEY_F10 = 147,
    KEY_F11 = 148,
    KEY_F12 = 149,
} keycode_t;

bool caps_lock_active = false;
bool num_lock_active = true;
bool shift_active = false;
bool extended = false;

const char scancode_to_ascii_lower[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0};

const char scancode_to_ascii_upper[128] = {
    0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0};

// Table pour le pavé numérique (Num Lock ON)
const char numpad_map[] = {
    '7', '8', '9', 0, // 0x47-0x4A
    '4', '5', '6', 0, // 0x4B-0x4E
    '1', '2', '3',    // 0x4F-0x51
    '0', '.'          // 0x52-0x53
};

void process_special_key(keycode_t key)
{
    switch (key)
    {
    case KEY_ARROW_UP:
        terminal_move_cursor_up();
        break;
    case KEY_ARROW_DOWN:
        terminal_move_cursor_down();
        break;
    case KEY_ARROW_LEFT:
        terminal_move_cursor_left();
        break;
    case KEY_ARROW_RIGHT:
        terminal_move_cursor_right();
        break;
    case KEY_HOME:
        printk("[HOME]");
        break;
    case KEY_END:
        printk("[END]");
        break;
    case KEY_PAGE_UP:
        printk("[PGUP]");
        break;
    case KEY_PAGE_DOWN:
        printk("[PGDN]");
        break;
    case KEY_INSERT:
        printk("[INS]");
        break;
    case KEY_DELETE:
        printk("\b");
        break;
    case KEY_F1:
        printk("[F1]");
        break;
    case KEY_F2:
        printk("[F2]");
        break;
    case KEY_F3:
        printk("[F3]");
        break;
    case KEY_F4:
        printk("[F4]");
        break;
    case KEY_F5:
        printk("[F5]");
        break;
    case KEY_F6:
        printk("[F6]");
        break;
    case KEY_F7:
        printk("[F7]");
        break;
    case KEY_F8:
        printk("[F8]");
        break;
    case KEY_F9:
        printk("[F9]");
        break;
    case KEY_F10:
        printk("[F10]");
        break;
    case KEY_F11:
        printk("[F11]");
        break;
    case KEY_F12:
        printk("[F12]");
        break;
    case KEY_UNKNOWN:
    default:
        break;
    }
}

char get_ascii_char(uint8_t scancode)
{
    if (scancode >= 128)
        return 0;
    if (caps_lock_active || shift_active)
        return scancode_to_ascii_upper[scancode];
    return scancode_to_ascii_lower[scancode];
}

char get_numpad_char(uint8_t scancode)
{
    if (scancode < 0x47 || scancode > 0x53)
        return 0;

    return numpad_map[scancode - 0x47];
}

void handle_numpad_no_numlock(uint8_t scancode)
{
    keycode_t key = KEY_UNKNOWN;

    switch (scancode)
    {
    case 0x47:
        key = KEY_HOME;
        break;
    case 0x48:
        key = KEY_ARROW_UP;
        break;
    case 0x49:
        key = KEY_PAGE_UP;
        break;
    case 0x4B:
        key = KEY_ARROW_LEFT;
        break;
    case 0x4C:
        break;
    case 0x4D:
        key = KEY_ARROW_RIGHT;
        break;
    case 0x4F:
        key = KEY_END;
        break;
    case 0x50:
        key = KEY_ARROW_DOWN;
        break;
    case 0x51:
        key = KEY_PAGE_DOWN;
        break;
    case 0x52:
        key = KEY_INSERT;
        break;
    case 0x53:
        key = KEY_DELETE;
        break;
    }

    if (key != KEY_UNKNOWN)
    {
        process_special_key(key);
    }
}

void handle_normal_scancode(uint8_t scancode)
{
    if (scancode == 0x45)
    {
        num_lock_active = !num_lock_active;
        PIC_sendEOI(1);
        return;
    }
    if (scancode == 0x3a)
    {
        caps_lock_active = !caps_lock_active;
        PIC_sendEOI(1);
        return;
    }
    if (scancode == 0x2a || scancode == 0x36)
    {
        shift_active = true;
        PIC_sendEOI(1);
        return;
    }
    if (scancode == 0xaa || scancode == 0xb6)
    {
        shift_active = false;
        PIC_sendEOI(1);
        return;
    }

    if (scancode & 0x80)
    {
        PIC_sendEOI(1);
        return;
    }
    if (scancode >= 0x47 && scancode <= 0x53)
    {
        if (num_lock_active)
        {
            char c = get_numpad_char(scancode);
            if (c != 0)
                printk("%c", c);
        }
        else
        {
            handle_numpad_no_numlock(scancode);
        }
        PIC_sendEOI(1);
        return;
    }
    if (scancode >= 0x3B && scancode <= 0x44)
    {
        keycode_t fkey = KEY_F1 + (scancode - 0x3B);
        process_special_key(fkey);
        PIC_sendEOI(1);
        return;
    }
    if (scancode == 0x57)
    {
        process_special_key(KEY_F11);
        PIC_sendEOI(1);
        return;
    }
    if (scancode == 0x58)
    {
        process_special_key(KEY_F12);
        PIC_sendEOI(1);
        return;
    }
    if (scancode == 0x35)
    {
        printk("/");
        PIC_sendEOI(1);
        return;
    }
    if (scancode == 0x4A)
    {
        printk("-");
        PIC_sendEOI(1);
        return;
    }
    if (scancode == 0x4E)
    {
        printk("+");
        PIC_sendEOI(1);
        return;
    }
    char c = get_ascii_char(scancode);
    if (c != 0)
        printk("%c", c);

    PIC_sendEOI(1);
}

void handle_extended_scancode(uint8_t scancode)
{
    if (scancode & 0x80)
        return;

    keycode_t key = KEY_UNKNOWN;

    switch (scancode)
    {
    case 0x48:
        key = KEY_ARROW_UP;
        break;
    case 0x50:
        key = KEY_ARROW_DOWN;
        break;
    case 0x4B:
        key = KEY_ARROW_LEFT;
        break;
    case 0x4D:
        key = KEY_ARROW_RIGHT;
        break;
    case 0x47:
        key = KEY_HOME;
        break;
    case 0x4F:
        key = KEY_END;
        break;
    case 0x49:
        key = KEY_PAGE_UP;
        break;
    case 0x51:
        key = KEY_PAGE_DOWN;
        break;
    case 0x52:
        key = KEY_INSERT;
        break;
    case 0x53:
        key = KEY_DELETE;
        break;
    case 0x35:
        printk("/");
        return;
    case 0x1C:
        printk("\n");
        return;
    }

    if (key != KEY_UNKNOWN)
    {
        process_special_key(key);
    }
}

void interrupt_handler(void)
{
    uint8_t scancode = inb(0x60);

    switch (kb_state)
    {
    case KB_NORMAL_MODE:
        if (scancode == 0xe0)
        {
            kb_state = KB_EXTENDED_MODE;
            PIC_sendEOI(1);
            return;
        }
        handle_normal_scancode(scancode);
        break;
    case KB_EXTENDED_MODE:
        handle_extended_scancode(scancode);
        kb_state = KB_NORMAL_MODE;
        PIC_sendEOI(1);
        return;
    }
}