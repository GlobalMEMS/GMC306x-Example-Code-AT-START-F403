/*
 *
 ****************************************************************************
 * Copyright (C) 2018 GlobalMEMS, Inc. <www.globalmems.com>
 * All rights reserved.
 *
 * File : main.c
 *
 * Usage: main function
 *
 ****************************************************************************
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 **************************************************************************/

/*! @file main.c
 *  @brief main program
 *  @author Joseph FC Tseng
 */

/* Includes ------------------------------------------------------------------*/
#include "i2c_gmems.h"
#include "gmc306x.h"
#include "Lcd_Driver.h"
#include "GUI.h"
#include "usart.h"
#include "delay.h"
#include "key.h"
#include "string.h"
#include "math.h"

/* Private macro -------------------------------------------------------------*/
/* global variables ---------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

#ifdef  USE_FULL_ASSERT

/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
    {}
}

#endif

const u16 RESOLUTION_X = 128;
const u16 RESOLUTION_Y = 120;
const u16 FONT_HEIGHT = 16;
const u16 LINE_HEIGHT = FONT_HEIGHT + 2;
const u16 MAX_DISPLAY_ITEM = 9;
void showMsg(u16 x, u16 line, u8* str, u16 color, u8 reDraw){

  int i;
  char* subStr;

  if(reDraw) Lcd_Clear(GRAY0);

  subStr = strtok((char*)str, "\n");

  for(i = line; subStr; ++i){
    Gui_DrawFont_GBK16(x, LINE_HEIGHT * i, color, GRAY0, (u8*)subStr);
    subStr = strtok(NULL, "\n");
  }
}

void floatCatToStr(float fIn, u8 precision, u8* outStr){

  s32 i = 0;
  float fTmp;
  s32 s32Dec, s32Dig;

  if(fIn < 0){
    fIn = -fIn;
    strcat((char*)outStr, "-");
  }

  s32Dec = (s32)fIn;
  fTmp = fIn - s32Dec;
  for(i = 0; i < precision; ++i)
    fTmp *= 10;
  s32Dig = (s32)(fTmp + 0.5f);

  itoa(s32Dec, &outStr[strlen((const char*)outStr)]);
  strcat((char*)outStr, ".");

  fTmp = 1;
  for(i = 0; i < precision; ++i)
    fTmp *= 10;
  for(i = 0; i < precision; ++i){
    fTmp /= 10;
    if(s32Dig < fTmp){
      strcat((char*)outStr, "0");
    }
    else{
      itoa(s32Dig, &outStr[strlen((const char*)outStr)]);
      break;
    }
  }
}

/**
 * @brief   Main program
 * @param  None
 * @retval None
 */
int main(void)
{
  bus_support_t gmc306_bus;
  raw_data_xyzt_t rawData;
  float_xyzt_t calibData;  //m-sensor data in code
  float_xyzt_t mData;      //m-sensor data in uT
  float_xyzt_t adjustVal = { 1.0, 1.0, 1.0, 0.0 };
  s32 i;
  u8 str[64];

  /* System Initialization */
  SystemInit();

  /* I2C1 initialization */
  I2C1_Init();

  /* Init Key */
  KEY_Init();

  /* Initialize the LCD */
  uart_init(19200);
  delay_init();
  Lcd_Init();

  /* GMC306 I2C bus setup */
  bus_init_I2C1(&gmc306_bus, GMC306_8BIT_I2C_ADDR);  //Initialize bus support to I2C1
  gmc306_bus_init(&gmc306_bus);  //Initailze GMC306 bus to I2C1

  /* GMC306 soft reset */
  gmc306_soft_reset();

  /* Wait 10ms for reset complete */
  delay_ms(10);

  /* GMC306 get the sensitivity adjust values */
  gmc306_get_sensitivity_adjust_val(&adjustVal);

  /* User message: show sensitivity adjustment value */
  strcpy((char*)str, "X_Sadj= ");
  floatCatToStr(adjustVal.u.x, 4, str);
  strcat((char*)str, "\nY_Sadj= ");
  floatCatToStr(adjustVal.u.y, 4, str);
  strcat((char*)str, "\nZ_Sadj= ");
  floatCatToStr(adjustVal.u.z, 4, str);
  showMsg(0, 0, str, BLACK, 1);
  strcpy((char*)str, "Press Key1 to\ncontinue");
  showMsg(0, 4, str, RED, 0);

  do{
    delay_ms(10);
  }while(KEY_Scan() != KEY1_PRES);

  //Set to CM 10Hz
  gmc306_set_operation_mode(GMC306_OP_MODE_CM_10HZ);

  strcpy((char*)str, "XYZ (code):");
  showMsg(0, 0, str, BLACK, 1);
  strcpy((char*)str, "X=");
  showMsg(0, 1, str, GRAY1, 0);
  strcpy((char*)str, "Y=");
  showMsg(0, 2, str, GRAY1, 0);
  strcpy((char*)str, "Z=");
  showMsg(0, 3, str, GRAY1, 0);
  strcpy((char*)str, "XYZ (uT):");
  showMsg(0, 5, str, BLACK, 0);
  strcpy((char*)str, "X=");
  showMsg(0, 6, str, GRAY1, 0);
  strcpy((char*)str, "Y=");
  showMsg(0, 7, str, GRAY1, 0);
  strcpy((char*)str, "Z=");
  showMsg(0, 8, str, GRAY1, 0);

  while (1){

    //Read XYZ raw
    gmc306_read_data_xyz(&rawData);

    //Sensitivity adjustment
    for(i = 0; i < 3; ++i)
      calibData.v[i] = rawData.v[i] * adjustVal.v[i];

    //code to uT
    for(i = 0; i < 3; ++i)
      mData.v[i] = calibData.v[i] / GMC306_RAW_DATA_SENSITIVITY;

    /* User message: XYZ in code*/
    strcpy((char*)str, "");
    floatCatToStr(calibData.u.x, 1, str);
    strcat((char*)str, "       ");
    showMsg(30, 1, str, BLUE, 0);
    strcpy((char*)str, "");
    floatCatToStr(calibData.u.y, 1, str);
    strcat((char*)str, "       ");
    showMsg(30, 2, str, BLUE, 0);
    strcpy((char*)str, "");
    floatCatToStr(calibData.u.z, 1, str);
    strcat((char*)str, "       ");
    showMsg(30, 3, str, BLUE, 0);

    /* User message: XYZ in uT*/
    strcpy((char*)str, "");
    floatCatToStr(mData.u.x, 1, str);
    strcat((char*)str, "       ");
    showMsg(30, 6, str, BLUE, 0);
    strcpy((char*)str, "");
    floatCatToStr(mData.u.y, 1, str);
    strcat((char*)str, "       ");
    showMsg(30, 7, str, BLUE, 0);
    strcpy((char*)str, "");
    floatCatToStr(mData.u.z, 1, str);
    strcat((char*)str, "       ");
    showMsg(30, 8, str, BLUE, 0);

    /* Delay 1 sec */
    delay_ms(1000);
  }
}

