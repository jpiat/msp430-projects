#include "cc1101.h"

// Sync word qualifier mode = 30/32 sync word bits detected 
// CRC autoflush = false 
// Channel spacing = 199.951172 
// Data format = Normal mode 
// Data rate = 38.3835 
// RX filter BW = 101.562500 
// PA ramping = false 
// Preamble count = 4 
// Whitening = false 
// Address config = No address check 
// Carrier frequency = 432.999817 
// Device address = 0 
// TX power = 0 
// Manchester enable = false 
// CRC enable = true 
// Deviation = 20.629883 
// Packet length mode = Variable packet length mode. Packet length configured by the first byte after sync word 
// Packet length = 255 
// Modulation format = GFSK 
// Base frequency = 432.999817 
// Modulated = true 
// Channel number = 0 

static const unsigned char cc1101_cfg[][2]= 
{
  {CC1101_IOCFG0,      0x06},
  {CC1101_FIFOTHR,     0x47},
  {CC1101_PKTCTRL0,    0x05},
  {CC1101_FSCTRL1,     0x06},
  {CC1101_FREQ2,       0x10},
  {CC1101_FREQ1,       0xA7},
  {CC1101_FREQ0,       0x62},
  {CC1101_MDMCFG4,     0xCA},
  {CC1101_MDMCFG3,     0x83},
  {CC1101_MDMCFG2,     0x13},
  {CC1101_DEVIATN,     0x35},
  {CC1101_MCSM0,       0x18},
  {CC1101_FOCCFG,      0x16},
  {CC1101_AGCCTRL2,    0x43},
  {CC1101_WORCTRL,     0xFB},
  {CC1101_FSCAL3,      0xE9},
  {CC1101_FSCAL2,      0x2A},
  {CC1101_FSCAL1,      0x00},
  {CC1101_FSCAL0,      0x1F},
  {CC1101_TEST2,       0x81},
  {CC1101_TEST1,       0x35},
  {CC1101_TEST0,       0x09},
};

#define ADD_STATUS 1
