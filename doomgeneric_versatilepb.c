/* doomgeneric_versatilepb.c
 * Minimal DoomGeneric glue for VersatilePB (bare-metal)
 *
 * This file implements the platform hooks expected by DoomGeneric.
 * Replace the extern driver stubs below with your real platform drivers,
 * or adapt the wrapper functions to call into your existing drivers.
 *
 * Notes:
 *  - DoomGeneric will render into an 8-bit paletted framebuffer (320x200)
 *    so this code converts paletted pixels to a 16-bit RGB565 framebuffer
 *    suitable for the VersatilePB PL110 CLCD (often 16bpp).
 *  - If your framebuffer is different (e.g. 24bpp/32bpp) change the
 *    conversion routine or implement direct blitting in vb_fb_blit().
 *  - WAD: this example expects the WAD to be embedded in the firmware
 *    as symbols created by the linker (see README in the project).
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "doomgeneric.h"
/* --- User platform driver API (replace with your drivers) --- */



/* --- DoomGeneric expected hooks ---
 * The exact names/signatures may vary slightly between DoomGeneric forks.
 * If your fork uses different names, adapt accordingly. Typical hooks:
 *  - void DG_Init(void);
 *  - void DG_Quit(void);
 *  - void DG_DrawFrame(void *framebuffer, int width, int height);
 *  - int  DG_GetEvent(void); // return keycode or 0
 *  - uint32_t DG_TicksMs(void);
 *  - void DG_SleepMs(int ms);
 */

/* If your DoomGeneric expects different types, change the definitions below */

//void DG_Init(void){}
//void DG_Quit(void){}
//void DG_DrawFrame(const uint8_t *paletted_frame, int fb_width, int fb_height);
void DG_Init(){}
extern int * fb;
void DG_DrawFrame(){

for (int line = 0; line < DOOMGENERIC_RESY; line++)
    {
        for (int x = 0; x < DOOMGENERIC_RESX; x++) //+120 for 640
            fb[line * 640 + x ] = DG_ScreenBuffer[DOOMGENERIC_RESX * line+ x];
		/*memcpy(
			(void *)((uintptr_t)(fb) + (fbStride * line) + fbOffsetY + fbOffsetX),
				(void *)(((uintptr_t)DG_ScreenBuffer) + (DOOMGENERIC_RESX * line * fbBytesPerPixel)),
			 (fbBytesPerPixel * DOOMGENERIC_RESX)
		);*/
	}
	}




/* --- Configuration --- */
#define DOOM_WIDTH  320
#define DOOM_HEIGHT 200



/* --- Hook implementations --- */
extern int * fb;

int counter1;
/* The main draw hook. DoomGeneric will pass an 8-bit paletted framebuffer that is
 * DOOM_WIDTH x DOOM_HEIGHT. Convert it to the platform frame format and blit.
 */




int mkdir(){}

