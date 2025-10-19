#include "ff.h"
#include "diskio.h"   /* FatFs lower layer API */
#include <stdint.h>

/* We assume drive number 0 = SD card */
#define DEV_SD   0

int sd_init() { return 0; }
/* Prototypes of your SD driver */
extern int sd_init(void);
extern int sd_read_sector(uint32_t lba, uint8_t *buf);
extern int sd_write_sector(uint32_t lba, const uint8_t *buf);

/*-----------------------------------------------------------------------*/
/* Initialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (BYTE pdrv)
{
    if (pdrv != DEV_SD)
        return STA_NOINIT;

    if (sd_init() == 0) {
        return 0;   /* success */
    } else {
        return STA_NOINIT;
    }
}

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (BYTE pdrv)
{
    if (pdrv != DEV_SD)
        return STA_NOINIT;

    return 0;   /* always OK for now */
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (BYTE pdrv, BYTE *buff, LBA_t sector, UINT count)
{
    if (pdrv != DEV_SD)
        return RES_PARERR;

    for (UINT i = 0; i < count; i++) {
        if (getSector(sector + i, buff + i*512) != 0)
            return RES_ERROR;
    }
    return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write (BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count)
{
#if _FS_READONLY
    return RES_WRPRT;
#else
    if (pdrv != DEV_SD)
        return RES_PARERR;

    for (UINT i = 0; i < count; i++) {
        if (putSector(sector + i, buff + i*512) != 0)
            return RES_ERROR;
    }
    return RES_OK;
#endif
}

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (BYTE pdrv, BYTE cmd, void *buff)
{
    if (pdrv != DEV_SD)
        return RES_PARERR;

    switch (cmd) {
    case CTRL_SYNC:
        return RES_OK;  /* nothing to do */
    case GET_SECTOR_SIZE:
        *(WORD*)buff = 512;
        return RES_OK;
    case GET_BLOCK_SIZE:
        *(DWORD*)buff = 1;  /* erase block = 1 sector */
        return RES_OK;
    case GET_SECTOR_COUNT:
        *(LBA_t*)buff = 128*1024; /* e.g. 64 MB card / 512 = 131072 */
        return RES_OK;
    }
    return RES_PARERR;
}

