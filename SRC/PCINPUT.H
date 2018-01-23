/*
 * Uses BIOS functions to get keys held and/or released
 *
 * (copied from https://stackoverflow.com/questions/40961527/checking-if-a-key-is-down-in-ms-dos-c-c)
*/

#define RELEASED(x) ((x) + 128)

/*
 * Returns key code if pressed,
 * and key code + 128 if released
*/
static unsigned char read_scancode() {
    unsigned char res;
    _asm {
        in al, 60h
        mov res, al
        in al, 61h
        or al, 128
        out 61h, al
        xor al, 128
        out 61h, al
    }
    return res;
}

/*
 * Frees BIOS keyboard buffer
 * Make sure to call if doing read_scancode() in a loop
*/
static void free_keyb_buf() {
    *(char*)(0x0040001A) = 0x20;
    *(char*)(0x0040001C) = 0x20;
}
