/* This file is responsible for the timer that the game uses to run at
 * a consistent speed on any machine.
 *
 * BIOS timer and other assembly instructions referenced from
 * this DOS game tutorial:
 * http://www3.telus.net/alexander_russell/course/chapter_5.htm
 *
 * From what I understood, there is a 18.2Hz timer in the BIOS universally,
 * but it's obiously far too slow to use as a game timer so there is a way of
 * speeding up the timer with interrupts.
 *
 * (how it actually works is a complete mystery as I don't know assembly,
 *  feel free to read the original article for more thorough explanation)
 *
 * Probably is a very very bad idea to include and/or use this header in
 * multiple files at once.
 */
#ifndef PCTIMER_H
#define PCTIMER_H

/* This points to the 18.2Hz timer function in the bios */
#define TICKS	(volatile unsigned long far *)(0x0040006CL)

static volatile unsigned long fast_tick, slow_tick; /* Using fast_tick inside game code for 120Hz tick */
static void interrupt (far *oldtimer)(void);        /* The old timer callback, used by DOS and BIOS */

/*
 * This function gets called by the BIOS interrupt
 * whenever the timer is triggered
 *
 * In this we call the original timer callback at roughly 18.2Hz
 * so that things relying on it (time/date, etc...) still works
 * correctly
 */
static void interrupt new_timer(void)
{
	asm cli
	fast_tick++;

	if(!(fast_tick & 3))         /* Calls the old timer every 4th new tick */
	{
		oldtimer();
		slow_tick++;
	}
	else
	{
		/* reset PIC (whatever that means) */
		asm {
			mov al, 20h
			out 20h, al
		}
	}

	asm sti
}

/*
 * This function replaces the interrupt 8 timer with a faster one
 *
 * There are two changes, first we must save the function pointer
 * that the 18.2Hz timer currently calls. DOS and the BIOS apparently
 * use this to time stuff and make the date/time increment.
 *
 * The second change is to actually increase the timer's clock speed
 * to 120Hz. We do this by dividing 1193180 by the desired frequency.
 * (I have no idea what that magic number means)
 */
static void init_timer(void)
{
	slow_tick = 0l;
	fast_tick = 0l;
	oldtimer = getvect(8);       /* save the old timer interrupt function */

	asm cli

	/* speed up the clock */
	asm {
		mov bx,  9943           /* Sets the clock to 120Hz (1193180/120) */
		mov al,  00110110b       /* not quite sure what the rest of this does */
		out 43h, al
		mov al,  bl
		out 40h, al
		mov al,  bh
		out 40h, al
	}

	setvect(8, new_timer);       /* install the new timer interrupt function */

	asm sti
}

/*
 * This puts back the old timer interrupt callback and slows the clock back down
 * to 18.2Hz. This should absolutely be called before exiting to DOS otherwise
 * bad things could happen.
*/
static void restore_timer(void)
{
	asm cli

	/* The original clock is 18.2Hz (1193180 / 65536)
	 * but for some reason the tutorial sets it to zero
	 * (not sure why) */
	asm {
		xor bx,  bx             /* apparently this sets it to zero? */
		mov al,  00110110b 		/* magic asm incantation follows */
		out 43h, al
		mov al,  bl
		out 40h, al
		mov al,  bh
		out 40h, al
	}

	setvect(8, oldtimer);       /* Put in the old timer function */

	asm sti
}

/*
 * This function is obsolete, it would only be used if we weren't
 * replacing the 18.2Hz clock with a faster one and switching interrupts
*/
static unsigned long get_tick(void)
{
	return (*(TICKS));
}

#endif /* PCTIMER_H */
