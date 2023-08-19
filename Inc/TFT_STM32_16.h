

#include "main.h"  
#include "stdint.h"
#include "stdbool.h"
#include "string.h"
#include "stdio.h"
#include "stm32f4xx_hal.h"
#include "libjpeg.h"
#include "fatfs.h"

#pragma pack (1)

typedef struct{
	//const char path[10];
	uint16_t xcor;
	uint16_t ycor;
	uint32_t height;
	uint32_t high;
//	float scale;
} tftIcon_t;


/*PINES */ //Parametros a cambiar
#define TFT_RST GPIO_PIN_11
#define TFT_CS  GPIO_PIN_12
#define TFT_RD  GPIO_PIN_13
#define TFT_WR  GPIO_PIN_14
#define TFT_RS  GPIO_PIN_15

#define TFT_PORT_DT    GPIOE   //Puerto de datos que se desea utilizar 
#define TFT_PORT_DT_HI GPIOE   //Puerto de datos MSB que se desea utilizar 
#define TFT_PORT       GPIOB   //Puerto de control de datos

#define TFT_SET_PORT   1       //0: Si el puerto de datos menos significativo no es igual al mas signiticativo caso contrario poner 1
#define TFT_FPOS GPIO_BSRR_BS0_Pos //Puntero al bit de datos (Si el bit es 11 sera GPIO_BSRR_BS11_Pos)

//Solo cambiar hasta aqui
#define TFT_HIGH 16

#define TFT_WritePort(TFT_Val)  TFT_PORT_DT->BSRR    = (((~(uint32_t)TFT_Val) & 0x00FF) << TFT_FPOS) << TFT_HIGH  \
                                                     |  (((uint32_t)TFT_Val & 0x00FF) << TFT_FPOS) \

#if TFT_SET_PORT == 0
#define TFT_Write16Port(TFT_Val1, TFT_Val2) TFT_PORT_DT->BSRR     = (((~(uint32_t)TFT_Val1) & 0x00FF) << TFT_FPOS) << TFT_HIGH  \  //revisar
                                                                 |  (((uint32_t)TFT_Val1 & 0x00FF) << TFT_FPOS); \
                                            TFT_PORT_DT_HI->BSRR  = (((~(uint32_t)TFT_Val2) & 0x00FF) << TFT_FPOS) << TFT_HIGH  \
                                                                 |  (((uint32_t)TFT_Val2 & 0x00FF) << TFT_FPOS)                 
#else 
#define TFT_Write16Port(TFT_Val) TFT_PORT_DT->BSRR = (((~(uint32_t)TFT_Val) & 0xFFFF) << TFT_FPOS) << TFT_HIGH  \
                                                   |  ((uint32_t)TFT_Val << TFT_FPOS)
#endif

#define TFT_RSTSet()      TFT_PORT->BSRR |= TFT_RST 
#define TFT_RSTReset()    TFT_PORT->BSRR |= TFT_RST << TFT_HIGH   
#define TFT_CSSet()       TFT_PORT->BSRR |= TFT_CS
#define TFT_CSReset()     TFT_PORT->BSRR |= TFT_CS  << TFT_HIGH   
#define TFT_RDSet()       TFT_PORT->BSRR |= TFT_RD
#define TFT_RDReset()     TFT_PORT->BSRR |= TFT_RD  << TFT_HIGH   
#define TFT_WRSet()       TFT_PORT->BSRR |= TFT_WR
#define TFT_WRReset()     TFT_PORT->BSRR |= TFT_WR  << TFT_HIGH  
#define TFT_RSSet()       TFT_PORT->BSRR |= TFT_RS
#define TFT_RSReset()     TFT_PORT->BSRR |= TFT_RS  << TFT_HIGH  

#define DISKTYPE 1 //poner 1: si es con USB, 0: si es con SDCARD

#if DISKTYPE == 0
#define PFILE SDFile
#else
#define PFILE USBHFile
#endif

#define firstbmp 54   //puntero a los datos en un archivo bmp

#define swap8bit(fatbuf, n)  (fatbuf[n]) | (((uint16_t)fatbuf[n + 1] << 8) & 0xFF00)
#define rot32bit(fatbuf, n)   fatbuf[n]  |  ((uint16_t)fatbuf[n + 1] << 8) | \
               ((uint32_t)fatbuf[n + 2] << 16) | ((uint32_t)fatbuf[n + 3] << 24)

void TFT_WriteCmd(uint8_t tftcmd);
void TFT_WritePmt(uint8_t tftpmr);
void TFT_Write16Pmt(TCHAR * tftptr);
void TFT_Init(void);
void TFT_Bmp(const char * tftfilename );
void TFT_Jpeg(const char * tftfilename );
void TFT_SetScroll(tftIcon_t * icon);
void TFT_IconBmp(const char * tftfilename , tftIcon_t * icon);
void TFT_IconJpeg(const char * tftfilename , tftIcon_t * icon, float scale);
void TFT_putchar(char tftChar, tftIcon_t * icon, float scale);
void TFT_ReadChar(char tftChar);

void my_error_exit (j_common_ptr cinfo);
