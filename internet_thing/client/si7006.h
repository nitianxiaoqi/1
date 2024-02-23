#ifndef __SI7006_H__
#define __SI7006_H__

#define SERIAL_ADDR 0xfcc9
#define FIRMWARE_ADDR 0x84b8

#define RH_ADDR 0xe5
#define TEM_ADDR 0xe3

#define GET_SERIAL _IOR('k', 0, int)
#define GET_FIRMWARE _IOR('k', 1, int)
#define GET_RH _IOR('k', 2, int)
#define GET_TEM _IOR('k', 3, int)

#endif