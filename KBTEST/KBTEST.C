unsigned char read_scancode() {
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

void free_keyb_buf() {
    *(char*)(0x0040001A) = 0x20;
    *(char*)(0x0040001C) = 0x20;
}

void main(void)
{
	unsigned char a;
	while(1) {
		a = read_scancode();

		printf("(%u) ", a);

		if(a == 72)
			printf("Pressed Up\n");
		if(a == (72 + 128))
			printf("Released Up\n");
		if(a == 80)
			printf("Pressed Down\n");
		if(a == 80 + 128) {
			printf("Released DOwn\n");
			return;
		}


		free_keyb_buf();
	}
}
