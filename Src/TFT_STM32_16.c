/*
 * TFT_STM32_16.c
 * 
 *  Created on: Jun 18, 2019
 *      Author: Steve
 *  El conocimiento es poder
 */

#include "TFT_STM32_16.h"
#include "libjpeg.h"
#include "usb_host.h"
#include "font.h"


//extern  const uint8_t font [][FSPACE];
struct jpeg_decompress_struct cinfo;
struct jpeg_error_mgr jerr;
JSAMPARRAY buffer2;
TCHAR t_buffer[512];
TCHAR t_buffer2[15000];
unsigned char t_buffer3[480*4];

//#pragma GCC optimize ("O1")

void TFT_WriteCmd(uint8_t tftcmd){
    uint8_t tftcnt = 50;
    while(tftcnt) tftcnt--;
    TFT_CSReset();
    //TFT_RDSet();     //DESACTIVA LA LECTURA Y ACTIVA LA ESCRITUDA
    TFT_RSReset();
    TFT_WritePort(tftcmd);
    TFT_WRReset();
    TFT_WRSet();
    TFT_CSSet();
}

void TFT_WritePmt(uint8_t tftpmr){
    TFT_CSReset();
    //TFT_RDSet();     //DESACTIVA LA LECTURA Y ACTIVA LA ESCRITUDA
    TFT_RSSet();
    TFT_WritePort(tftpmr);
    TFT_WRReset();
    TFT_WRSet();
    TFT_CSSet();
//    uint8_t tftcnt = 0;
//    while(tftcnt) tftcnt--;
}

void TFT_Write16Pmt(TCHAR * tftptr){
    TFT_CSReset();
    //TFT_RDSet();     //DESACTIVA LA LECTURA Y ACTIVA LA ESCRITUDA
    TFT_RSSet();
#if TFT_SET_PORT == 0
    TFT_Write16Port(*(tftptr + 1), *tftptr);
#else
    uint16_t tftpmt = ((uint16_t)*tftptr << 8) | *(tftptr + 1);
    TFT_Write16Port(tftpmt);
#endif
    TFT_WRReset();
    TFT_WRSet();
    TFT_CSSet();
//    uint8_t tftcnt = 0;
//    while(tftcnt) tftcnt--;
}

void TFT_Init(){
    TFT_RSTReset();
    TFT_RSSet();
    TFT_CSSet();
    TFT_RDSet();
    TFT_WRSet();
    HAL_Delay(100);
    TFT_RSTSet();
    HAL_Delay(200);
    TFT_WriteCmd(0x01);     //Software Reset
    TFT_WriteCmd(0x01);
    TFT_WriteCmd(0x01);

    TFT_WriteCmd(0xE0); TFT_WritePmt(0x01); //START PLL
    HAL_Delay(1);
    TFT_WriteCmd(0xE0); TFT_WritePmt(0x03); //LOCK PLL

    TFT_WriteCmd(0xF0); TFT_WritePmt(0x02); //Formato 16 bits
    //TFT_WriteCmd(0x36); TFT_WritePmt(0x88); //
    TFT_WriteCmd(0x36); TFT_WritePmt(0x8A);

    TFT_WriteCmd(0xE2); TFT_WritePmt(78); TFT_WritePmt(0x0); TFT_WritePmt(0x04); //790 MHz
    TFT_WriteCmd(0xB0); TFT_WritePmt(0x20); TFT_WritePmt(0x00); TFT_WritePmt(0x01); //SET LCD MODE
    TFT_WritePmt(0xDF); TFT_WritePmt(0x01); TFT_WritePmt(0x0F); TFT_WritePmt(0x1B);  //SET even/odd line RGB seq.=RGB
    TFT_WriteCmd(0xB4); TFT_WritePmt(0x20); TFT_WritePmt(0xAF); TFT_WritePmt(0x00); //
    TFT_WritePmt(0xA3); TFT_WritePmt(0x07); TFT_WritePmt(0x00); TFT_WritePmt(0x00);  TFT_WritePmt(0x00);
    TFT_WriteCmd(0xB6); TFT_WritePmt(0x01); TFT_WritePmt(0xEF); TFT_WritePmt(0x00); //
    TFT_WritePmt(0x04); TFT_WritePmt(0x01); TFT_WritePmt(0x00); TFT_WritePmt(0x00);
    TFT_WriteCmd(0x3A); TFT_WritePmt(0x60);
    TFT_WriteCmd(0xE6); TFT_WritePmt(0x02); TFT_WritePmt(0xEF); TFT_WritePmt(0xFF); //SET PCLK freq=33.26MHz

    TFT_WriteCmd(0x2A); TFT_WritePmt(0x00); TFT_WritePmt(0x00); TFT_WritePmt(0x01); TFT_WritePmt(0xDF); //hor
    TFT_WriteCmd(0x2B); TFT_WritePmt(0x00); TFT_WritePmt(0x00); TFT_WritePmt(0x01); TFT_WritePmt(0x0F); //ver

    TFT_WriteCmd(0x29);
    HAL_Delay(10);
    TFT_WriteCmd(0x2C);
    uint32_t cnt ;
    for(cnt = 0; cnt < 391680;cnt++){
        TFT_Write16Pmt((TCHAR *)0xFF);
    }
    TFT_WriteCmd(0x2C);
}

void TFT_Bmp(const char * tftfilename ){
    uint32_t tftcnt ;
    UINT  br;
    TFT_WriteCmd(0x2A); TFT_WritePmt(0x00); TFT_WritePmt(0x00); TFT_WritePmt(0x01); TFT_WritePmt(0xDF); //hor
    TFT_WriteCmd(0x2B); TFT_WritePmt(0x00); TFT_WritePmt(0x00); TFT_WritePmt(0x01); TFT_WritePmt(0x0F); //ver
    if (f_open (& PFILE, tftfilename, FA_READ) != FR_OK) return;
    TFT_WriteCmd(0x2C);
    f_lseek(&PFILE, firstbmp);
    while(1) {
        f_read( &PFILE, t_buffer, sizeof t_buffer, &br ) ;
        if(!br) break;
        for(tftcnt = 0; tftcnt < 512;tftcnt+=2)  TFT_Write16Pmt(&t_buffer[tftcnt]);
    }
    f_close(& PFILE);
}

void TFT_Jpeg(const char * tftfilename ){
    uint32_t tftcnt ;
    UINT  br;
    JSAMPARRAY buffer;
    buffer[0] = t_buffer3;
    //TFT_WriteCmd(0x36); TFT_WritePmt(0x89);
    TFT_WriteCmd(0x2A); TFT_WritePmt(0x00); TFT_WritePmt(0x00); TFT_WritePmt(0x00); TFT_WritePmt(0xEF); //hor
    TFT_WriteCmd(0x2B); TFT_WritePmt(0x00); TFT_WritePmt(0x00); TFT_WritePmt(0x00); TFT_WritePmt(0x87); //ver
    if (f_open (& PFILE, tftfilename, FA_READ) != FR_OK) return;
    f_read( &USBHFile, t_buffer2, sizeof t_buffer2, &br ) ;
    //f_close(& USBHFile);
	//jpeg_mem_src(&cinfo,t_buffer2,sizeof t_buffer);
//    while(1){
//    	HAL_GPIO_TogglePin(GPIOD,GPIO_PIN_13);
//    	HAL_Delay(1000);
//    }
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_mem_src(&cinfo,t_buffer2,sizeof t_buffer2);
    //jpeg_stdio_src(&cinfo,& PFILE);
	(void) jpeg_read_header(&cinfo, TRUE);
	cinfo.scale_num = 1;
	cinfo.scale_denom = 1;
	 //cinfo.dct_method = JDCT_FLOAT;
	jpeg_start_decompress(&cinfo);
//	if(!cinfo.output_scanline){
//	    while(1){
//	    	HAL_GPIO_TogglePin(GPIOD,GPIO_PIN_13);
//	    	HAL_Delay(1000);
//	    }
//	}
    TFT_WriteCmd(0x2C);
	while (cinfo.output_scanline < cinfo.output_height){
		jpeg_read_scanlines(&cinfo, buffer, 1);
		for(tftcnt = 0; tftcnt < 480*3;tftcnt+=2)  TFT_Write16Pmt(&t_buffer3[tftcnt]);
	}
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
    f_close(& PFILE);
}

void TFT_SetScroll(tftIcon_t * icon){
	//(icon->height)--;
	//(icon->high)--;
    TFT_WriteCmd(0x2A); TFT_WritePmt((icon->ycor & 0xFF00) >> 8); TFT_WritePmt(icon->ycor);
    TFT_WritePmt(((icon->ycor + icon->high - 1) & 0xFF00) >> 8); TFT_WritePmt(icon->ycor + icon->high - 1); //hor
    TFT_WriteCmd(0x2B); TFT_WritePmt(((icon->xcor - icon->height + 1) & 0xFF00) >> 8); TFT_WritePmt(icon->xcor - icon->height + 1);
    TFT_WritePmt((icon->xcor & 0xFF00) >> 8); TFT_WritePmt(icon->xcor); //ver
}

void TFT_IconBmp(const char * tftfilename , tftIcon_t * icon){ //CONSIDER TAMAÑOS IMPARES BMP
    uint32_t tftcnt, size , rdcnt = 0; // scnt: muestra la cantidad de datos leidos
    UINT  br;
    if (f_open (& PFILE, tftfilename, FA_READ) != FR_OK) return;
    f_lseek(&PFILE, 0);
    f_read( &PFILE, t_buffer, sizeof t_buffer, &br ) ;
    icon->high     = rot32bit(t_buffer, 0x12); //CALCULA EL TAMAÑO DE LA IMAGEN
    icon->height   = rot32bit(t_buffer, 0x16);
    size = icon->high * icon->height * 3;
    TFT_SetScroll(icon); //DIBUJA LA IMAGEN
    TFT_WriteCmd(0x2C);
    f_lseek(&PFILE, firstbmp);
    while(1) {
        f_read( &PFILE, t_buffer, sizeof t_buffer, &br ) ;
        if(!br) break;
        for(tftcnt = 0; tftcnt < 512;tftcnt+=2, rdcnt++){
        	if(t_buffer[tftcnt] == 0xFF) t_buffer[tftcnt] = 0xFC;
        	if(t_buffer[tftcnt + 1] == 0xFF) t_buffer[tftcnt + 1] = 0xFC;
        	//if(t_buffer[tftcnt] == 0x0) t_buffer[tftcnt] = 0x0;
        	//if(t_buffer[tftcnt + 1] == 0) t_buffer[tftcnt + 1] = 0;
        	TFT_Write16Pmt(&t_buffer[tftcnt]);
        }
        if(rdcnt >= size) break;
    }
    f_close(& PFILE);
}

void TFT_IconJpeg(const char * tftfilename , tftIcon_t * icon, float scale){
    uint32_t tftcnt;
    UINT  br;
    JSAMPARRAY buffer;
    buffer[0] = t_buffer3;
    if (f_open (& PFILE, tftfilename, FA_READ) != FR_OK) return;
    f_read( &USBHFile, t_buffer2, sizeof t_buffer2, &br ) ;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_mem_src(&cinfo,t_buffer2,sizeof t_buffer2);
	(void) jpeg_read_header(&cinfo, TRUE);
	cinfo.scale_num = scale*10;
	cinfo.scale_denom = 10;
    icon->high     = cinfo.image_width*scale; //CALCULA EL TAMAÑO DE LA IMAGEN
    icon->height   = cinfo.image_height*scale;
    TFT_SetScroll(icon); //DIBUJA LA IMAGEN
	jpeg_start_decompress(&cinfo);
    TFT_WriteCmd(0x2C);
	while (cinfo.output_scanline < cinfo.output_height){
		jpeg_read_scanlines(&cinfo, buffer, 1);
		for(tftcnt = 0; tftcnt < icon->high*3;tftcnt+=2)  TFT_Write16Pmt(&t_buffer3[tftcnt]);
	}
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
    f_close(& PFILE);
}

//#pragma GCC push_options
//#pragma GCC optimize ("O1")
void TFT_putchar(char tftChar, tftIcon_t * icon, float scale){
    uint32_t tftcnt;
//    UINT  br;
    JSAMPARRAY buffer;
    unsigned char *aux;
    aux = t_buffer3;
//    buffer2[0] = aux;
    buffer[0] = aux;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
//    jpeg_mem_src(&cinfo,&font[tftChar - 97][0],FSPACE);
    TFT_ReadChar(tftChar);
    jpeg_mem_src(&cinfo,t_buffer2,758);
	(void) jpeg_read_header(&cinfo, TRUE);
	cinfo.scale_num = scale*10;
	cinfo.scale_denom = 10;
    icon->high     = cinfo.image_width*scale; //CALCULA EL TAMAÑO DE LA IMAGEN
    icon->height   = cinfo.image_height*scale;
    TFT_SetScroll(icon); //DIBUJA LA IMAGEN
	jpeg_start_decompress(&cinfo);
//	while(1){
//		HAL_GPIO_TogglePin(GPIOD,GPIO_PIN_13);
//		HAL_Delay(1000);
//	}
    TFT_WriteCmd(0x2C);
	while (cinfo.output_scanline < cinfo.output_height){
		jpeg_read_scanlines(&cinfo, buffer, 1);
		for(tftcnt = 0; tftcnt < icon->high*3;tftcnt+=2)  TFT_Write16Pmt(&t_buffer3[tftcnt]);
	}
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
//	buffer[0] = aux;
//	for(uint16_t tftCnt = 0;tftCnt < 480*3;tftCnt++){
////		t_buffer3[tftCnt] = 0;
//	}
	//t_buffer3 = aux;
}

//#pragma GCC pop_options

void TFT_ReadChar(char tftChar){
    for(uint16_t tftCnt = 0;tftCnt < FSPACE;tftCnt++){
        t_buffer2[tftCnt] = font[tftChar - 97][tftCnt];
    }
}
