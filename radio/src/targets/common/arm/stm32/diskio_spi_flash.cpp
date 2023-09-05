/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "diskio_spi_flash.h"
#include "spi_flash.h"

#include "drivers/frftl.h"

static FrFTL* frftl = nullptr;

static bool flashRead(uint32_t addr, uint8_t* buf, uint32_t len)
{
  flashSpiRead(addr, buf, len);
  return true;
}

static bool flashWrite(uint32_t addr, const uint8_t *buf, uint32_t len)
{
  size_t pageSize = flashSpiGetPageSize();
  if(len%pageSize != 0)
    return false;
  while(len > 0)
  {
    flashSpiWrite(addr, buf, pageSize);
    len -= pageSize;
    buf += pageSize;
    addr += pageSize;
  }
  if(len != 0)
    return false;
  return true;
}

static bool flashErase(uint32_t addr)
{
  flashSpiErase(addr);
  return true;
}

static bool isFlashErased(uint32_t addr)
{
  return flashSpiIsErased(addr);
}

void flushFTL()
{
  ftlSync(frftl);
}

static DSTATUS spi_flash_initialize(BYTE lun)
{
  DSTATUS stat = 0;

  if(frftl != nullptr)
    return stat;
  
  // if(!tjftl_detect(flashRead, nullptr))
  //   flashSpiEraseAll();

  int flashSize = flashSpiGetSize();
  int flashSizeMB = flashSize  / 1024 / 1024;
  // int blockCount = flashSpiGetBlockCount();
    
  // tjftl requires 1/64 overhead and some blocks for GC (at least 10)
  // However, if give it more GC blocks, it can help to reduce wearing level and improve performance
  // Simulation is done to give a balanace between wearing and overhead

  // int overheadBlockCount = blockCount / 64 + (flashSizeMB >= 32 ? flashSizeMB * 2 : 32);

  // tjftl = tjftl_init(flashRead, flashErase, flashWrite, nullptr, flashSize,
  //                    (flashSize - overheadBlockCount * 32768)/512, 0);
  frftl = ftlInit(flashRead, flashWrite, flashErase, isFlashErased, flashSizeMB);

  if (frftl == nullptr) stat |= STA_NOINIT;
  return stat;
}

static DSTATUS spi_flash_status (BYTE lun)
{
  DSTATUS stat = 0;

  if(frftl == nullptr)
    stat |= STA_NODISK;

  return stat;
}

static DRESULT spi_flash_read(BYTE lun, BYTE * buff, DWORD sector, UINT count)
{
  if(frftl == nullptr) {
    return RES_ERROR;
  }

  while(count) {

    if(!ftlRead(frftl, sector, (uint8_t*)buff))
      return RES_ERROR;
 
    buff += 512;
    sector++;
    count --;
  }

  return RES_OK;
}

static DRESULT spi_flash_write(BYTE lun, const BYTE *buff, DWORD sector, UINT count)
{
  if(frftl == nullptr) {
    return RES_ERROR;
  }

  if (!ftlWrite(frftl, sector, count, (uint8_t*)buff)) {
    return RES_ERROR;
  }

  return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */

static DRESULT spi_flash_ioctl(BYTE lun, BYTE ctrl, void *buff)
{
  if(frftl == nullptr) {
    return RES_ERROR;
  }

  DRESULT res = RES_OK;

  switch (ctrl) {

  case GET_SECTOR_COUNT: /* Get number of sectors on the disk (DWORD) */
    *(DWORD*)buff = frftl->usableSectorCount;
    break;

  case GET_SECTOR_SIZE:  /* Get R/W sector size (WORD) */
    *(WORD*)buff = 512;
    break;

  case GET_BLOCK_SIZE :   /* Get erase block size in unit of sector (DWORD) */
    // TODO verify that this is the correct value
    *(DWORD*)buff = 4096;
    break;

  case CTRL_SYNC:
    if (!ftlSync(frftl)) {
      res = RES_ERROR;
    }
    break;

  case CTRL_TRIM:
    if (!ftlTrim(frftl, *(DWORD*)buff, 1 + *((DWORD*)buff + 1) - *(DWORD*)buff)) {
      res = RES_ERROR;
    }
    break;
  }

  return res;
}

const diskio_driver_t spi_flash_diskio_driver = {
  .initialize = spi_flash_initialize,
  .status = spi_flash_status,
  .read = spi_flash_read,
  .write = spi_flash_write,
  .ioctl = spi_flash_ioctl,
};
