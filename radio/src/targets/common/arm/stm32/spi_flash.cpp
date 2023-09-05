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

#include "stm32_spi.h"
#include "delays_driver.h"
#include "hal.h"

#define SPI_MODE        	           0x00
#define SPI_STATUS      	           0x01
#define SPI_BASS        	           0x02
#define SPI_CLOCKF      	           0x03
#define SPI_DECODE_TIME 	           0x04
#define SPI_AUDATA      	           0x05
#define SPI_WRAM        	           0x06
#define SPI_WRAMADDR    	           0x07
#define SPI_HDAT0       	           0x08
#define SPI_HDAT1       	           0x09
#define SPI_AIADDR      	           0x0a
#define SPI_VOL         	           0x0b
#define SPI_AICTRL0     	           0x0c
#define SPI_AICTRL1     	           0x0d
#define SPI_AICTRL2     	           0x0e
#define SPI_AICTRL3     	           0x0f

#define CS_HIGH() stm32_spi_unselect(&_flash_spi)
#define CS_LOW()  stm32_spi_select(&_flash_spi)

#define MIN(a,b) (a < b ? a : b)

struct SpiFlashDescriptor
{
  uint16_t id;
  uint32_t pageSize;
  uint32_t sectorSize;
  uint32_t blockSize;
  uint32_t blockCount;

  uint8_t readStatusCmd;
  uint8_t readCmd;
  uint8_t writeCmd;
  uint8_t writeEnableCmd;
  uint8_t eraseSectorCmd;
  uint8_t eraseBlockCmd;
  uint8_t eraseChipCmd;
  bool use4BytesAddress;
};

// * RadioMaster/Eachine TX16S, RadioKing TX18S and Jumper T18 use GD25Q127C (16 MByte)
// * FlySky PL18, Paladin EV and NV14 use WinBond W25Q64JV (8 MByte)

static const SpiFlashDescriptor spiFlashDescriptors[] = {
  { // MX25L25645G
    .id = 0xC218,
    .pageSize = 256,
    .sectorSize = 4096,
    .blockSize = 32768,
    .blockCount = 1024,

    .readStatusCmd = 0x05,
    .readCmd = 0x13,  // 4 bytes address command
    .writeCmd = 0x12,  // 4 bytes address command
    .writeEnableCmd = 0x06,
    .eraseSectorCmd = 0x21,  // 4 bytes address 4k block erase command
    .eraseBlockCmd = 0x5C,  // 4 bytes address 32k block erase command
    .eraseChipCmd = 0x60,
    .use4BytesAddress = true
  },
  { // GD25Q127C
    .id = 0xC817,
    .pageSize = 256,
    .sectorSize = 4096,
    .blockSize = 32768,
    .blockCount = 512,

    .readStatusCmd = 0x05,
    .readCmd = 0x03,
    .writeCmd = 0x02,
    .writeEnableCmd = 0x06,
    .eraseSectorCmd = 0x20,  // 4k block erase command
    .eraseBlockCmd = 0x52,  // 32k block erase command
    .eraseChipCmd = 0x60,
    .use4BytesAddress = false
  },
  { // W25Q64JV
    .id = 0xEF16,
    .pageSize = 256,
    .sectorSize = 4096,
    .blockSize = 32768,
    .blockCount = 256,

    .readStatusCmd = 0x05,
    .readCmd = 0x03,
    .writeCmd = 0x02,
    .writeEnableCmd = 0x06,
    .eraseSectorCmd = 0x20,  // 4k block erase command
    .eraseBlockCmd = 0x52,  // 32k block erase command
    .eraseChipCmd = 0xC7,
    .use4BytesAddress = false
  }
};

const stm32_spi_t _flash_spi = {
  .SPIx = FLASH_SPI,
  .SPI_GPIOx = FLASH_SPI_GPIO,
  .SPI_Pins = FLASH_SPI_SCK_GPIO_PIN | FLASH_SPI_MISO_GPIO_PIN | FLASH_SPI_MOSI_GPIO_PIN,
  .CS_GPIOx = FLASH_SPI_CS_GPIO,
  .CS_Pin = FLASH_SPI_CS_GPIO_PIN,
  .DMA = nullptr,
  .DMA_Channel = 0,
  .txDMA_Stream = 0,
  .rxDMA_Stream = 0,
};

static const SpiFlashDescriptor* flashDescriptor = nullptr;

static inline uint8_t flashSpiReadWriteByte(uint8_t value)
{
  return stm32_spi_transfer_byte(&_flash_spi, value);
}

static uint16_t flashSpiReadID()
{
  CS_LOW();
  flashSpiReadWriteByte(0x90);
  flashSpiReadWriteByte(0x00);
  flashSpiReadWriteByte(0x00);
  flashSpiReadWriteByte(0x00);
  uint16_t result = flashSpiReadWriteByte(0xff) << 8;
  result += flashSpiReadWriteByte(0xff);
  delay_01us(100); // 10us
  CS_HIGH();

  return result;
}

void flashSpiInit(void)
{
  stm32_spi_init(&_flash_spi);
  delay_ms(1);

  uint16_t id = flashSpiReadID();
  for(uint32_t i = 0; i < sizeof(spiFlashDescriptors)/sizeof(SpiFlashDescriptor); i++) {
    if(spiFlashDescriptors[i].id == id) {
      flashDescriptor = &spiFlashDescriptors[i];
      break;
    }
  }
}

void flashSpiSync()
{
  delay_01us(100);
  CS_LOW();
  while(true)
  {
    uint8_t status = flashSpiReadWriteByte(flashDescriptor->readStatusCmd);
    if((status & 0x01) == 0)
      break;
  }
  CS_HIGH();
  delay_01us(100);
}

uint32_t flashSpiGetSize()
{
  return flashDescriptor->blockSize * flashDescriptor->blockCount;
}

// uint32_t flashSpiGetBlockCount()
// {
//   return flashDescriptor->blockCount;
// }

bool flashSpiIsErased(uint32_t address)
{
  if(address%flashDescriptor->sectorSize != 0)
    return false;

  flashSpiSync();

  CS_LOW();

  flashSpiReadWriteByte(flashDescriptor->readCmd);
  if (flashDescriptor->use4BytesAddress)
  {
    flashSpiReadWriteByte((address>>24)&0xFF);
  }
  flashSpiReadWriteByte((address>>16)&0xFF);
  flashSpiReadWriteByte((address>>8)&0xFF);
  flashSpiReadWriteByte(address&0xFF);

  bool ret = true;
  for(uint32_t i = 0; i < flashDescriptor->sectorSize; i++)
  {
    uint8_t byte = flashSpiReadWriteByte(0xFF);
    if (byte != 0xff)
    {
      ret = false;
      break;
    }
  }

  delay_01us(100); // 10us
  CS_HIGH();

  return ret;
}

uint32_t flashSpiRead(uint32_t address, uint8_t* data, uint32_t size)
{
  flashSpiSync();

  size = MIN(size, (uint32_t)(flashSpiGetSize() - address));

  CS_LOW();

  flashSpiReadWriteByte(flashDescriptor->readCmd);
  if (flashDescriptor->use4BytesAddress) {
    flashSpiReadWriteByte((address>>24)&0xFF);
  }
  flashSpiReadWriteByte((address>>16)&0xFF);
  flashSpiReadWriteByte((address>>8)&0xFF);
  flashSpiReadWriteByte(address&0xFF);

  for(uint32_t i=0; i < size; i++)
    *data++ = flashSpiReadWriteByte(0xFF);

  delay_01us(100); // 10us
  CS_HIGH();

  return size;
}

uint32_t flashSpiWrite(uint32_t address, const uint8_t* data, uint32_t size)
{
  size = MIN(size, (uint32_t)(flashSpiGetSize() - address));
  if(size != flashDescriptor->pageSize || address%flashDescriptor->pageSize != 0)
	  return -1;

  flashSpiSync();

  CS_LOW();
  flashSpiReadWriteByte(flashDescriptor->writeEnableCmd);
  delay_01us(100); // 10us
  CS_HIGH();
  delay_01us(100); // 10us
  CS_LOW();

  flashSpiReadWriteByte(flashDescriptor->writeCmd);
  if (flashDescriptor->use4BytesAddress) {
    flashSpiReadWriteByte((address >> 24) & 0xFF);
  }
  flashSpiReadWriteByte((address >> 16) & 0xFF);
  flashSpiReadWriteByte((address >> 8) & 0xFF);
  flashSpiReadWriteByte(address & 0xFF);

  for(uint32_t i=0; i < size; i++)
    flashSpiReadWriteByte(*data++);

  delay_01us(100); // 10us
  CS_HIGH();

  flashSpiSync();

  return size;
}

int flashSpiErase(uint32_t address)
{
  if(address%flashDescriptor->sectorSize != 0)
    return -1;

  flashSpiSync();

  CS_LOW();
  flashSpiReadWriteByte(flashDescriptor->writeEnableCmd);
  delay_01us(100); // 10us
  CS_HIGH();
  delay_01us(100); // 10us
  CS_LOW();
  flashSpiReadWriteByte(flashDescriptor->eraseSectorCmd);
  if (flashDescriptor->use4BytesAddress) {
    flashSpiReadWriteByte((address >> 24) & 0xFF);
  }
  flashSpiReadWriteByte((address >> 16) & 0xFF);
  flashSpiReadWriteByte((address >> 8) & 0xFF);
  flashSpiReadWriteByte(address & 0xFF);
  delay_01us(100);  // 10us
  CS_HIGH();

  flashSpiSync();

  return 0;
}

int flashSpiBlockErase(uint32_t address)
{
  if(address%flashDescriptor->blockSize != 0)
    return -1;

  flashSpiSync();

  CS_LOW();
  flashSpiReadWriteByte(flashDescriptor->writeEnableCmd);
  delay_01us(100); // 10us
  CS_HIGH();
  delay_01us(100); // 10us
  CS_LOW();
  flashSpiReadWriteByte(flashDescriptor->eraseBlockCmd);
  if (flashDescriptor->use4BytesAddress) {
    flashSpiReadWriteByte((address >> 24) & 0xFF);
  }
  flashSpiReadWriteByte((address >> 16) & 0xFF);
  flashSpiReadWriteByte((address >> 8) & 0xFF);
  flashSpiReadWriteByte(address & 0xFF);
  delay_01us(100);  // 10us
  CS_HIGH();

  flashSpiSync();

  return 0;
}

void flashSpiEraseAll()
{
  flashSpiSync();

  delay_01us(100); // 10us//

  CS_LOW();
  flashSpiReadWriteByte(flashDescriptor->writeEnableCmd);
  delay_01us(100); // 10us
  CS_HIGH();
  delay_01us(100); // 10us

  CS_LOW();
  flashSpiReadWriteByte(flashDescriptor->eraseChipCmd);
  delay_01us(100); // 10us
  CS_HIGH();

  flashSpiSync();
}

uint16_t flashSpiGetPageSize()
{
  return flashDescriptor->pageSize;
}

uint16_t flashSpiGetSectorSize()
{
  return flashDescriptor->sectorSize;
}

uint16_t flashSpiGetSectorCount()
{
  return flashDescriptor->blockCount * (flashDescriptor->blockSize / flashDescriptor->sectorSize);
}

void flashInit()
{
  flashSpiInit();
  delay_ms(1);

  uint16_t id = flashSpiReadID();
  for(uint32_t i = 0; i < sizeof(spiFlashDescriptors)/sizeof(SpiFlashDescriptor); i++) {
    if(spiFlashDescriptors[i].id == id) {
      flashDescriptor = &spiFlashDescriptors[i];
      break;
    }
  }
}
