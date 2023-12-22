#ifndef _MIPI_REG_H
#define _MIPI_REG_H

#define ST_CMD_DELAY 0x80 // special signifier for command lists

#define FRMCTR1 0xB1
#define FRMCTR2 0xB2
#define FRMCTR3 0xB3
#define INVCTR 0xB4
#define DISSET5 0xB6

#define PWCTR1 0xC0
#define PWCTR2 0xC1
#define PWCTR3 0xC2
#define PWCTR4 0xC3
#define PWCTR5 0xC4
#define VMCTR1 0xC5
#define VMCTR2 0xC7

#define PWCTR6 0xFC

#define GMCTRP1 0xE0
#define GMCTRN1 0xE1

#define NOP 0x00
#define SWRESET 0x01
#define RDDID 0x04
#define RDDST 0x09

#define SLPIN 0x10
#define SLPOUT 0x11
#define PTLON 0x12
#define NORON 0x13

#define INVOFF 0x20
#define INVON 0x21
#define GAMMASET 0x26 
#define DISPOFF 0x28
#define DISPON 0x29
#define CASET 0x2A
#define RASET 0x2B
#define RAMWR 0x2C
#define RAMRD 0x2E

#define PTLAR 0x30
#define TEOFF 0x34
#define TEON 0x35
#define MADCTL 0x36
#define VSCRSADD 0x37
#define COLMOD 0x3A

#define MADCTL_MY 0x80
#define MADCTL_MX 0x40
#define MADCTL_MV 0x20
#define MADCTL_ML 0x10
#define MADCTL_RGB 0x00
#define MADCTL_BGR 0x08
#define MADCTL_MH 0x04

#endif