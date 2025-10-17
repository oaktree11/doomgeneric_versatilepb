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
void DG_DrawFrame(){}
void DG_SleepMs(uint32_t ms){}
uint32_t DG_GetTicksMs(){}
int DG_GetKey(int* pressed, unsigned char* key){return 0;}
void DG_SetWindowTitle(const char * title) {}
/* --- Configuration --- */
#define DOOM_WIDTH  320
#define DOOM_HEIGHT 200



/* --- Hook implementations --- */
extern int * fb;

int counter1;
/* The main draw hook. DoomGeneric will pass an 8-bit paletted framebuffer that is
 * DOOM_WIDTH x DOOM_HEIGHT. Convert it to the platform frame format and blit.
 */


/* Input: return a keycode understood by DoomGeneric. If none, return 0.
 * You can map your platform keycodes to DoomGeneric's expected codes here.
 */
//int DG_GetKey(void)
//{
    // Poll your input driver and map to Doom's keycodes. For example, simple ASCII.
  //  int code = DG_GetKey();//vb_input_poll();
   // if (code == 0)
    //    return 0;

    // Example mapping: if your vb_input_poll returns ASCII, just return it.
    //return code;/
//}

int mkdir(){}

