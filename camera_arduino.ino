/*
 *  camera.ino - One second interval time-lapse Camera
 *  Copyright 2018 Sony Semiconductor Solutions Corporation
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *  This is a test app for the camera library.
 *  This library can only be used on the Spresense with the FCBGA chip package.
 */
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

#define TFT_CS -1 
#define TFT_RST 8 
#define TFT_DC  9 

Adafruit_ILI9341 tft = Adafruit_ILI9341(&SPI, TFT_DC, TFT_CS, TFT_RST);


#include <SDHCI.h>
#include <stdio.h>  /* for sprintf */
#include <Camera.h>
#include "gpio.h"

#define BAUDRATE                (115200)

SDClass  theSD;
int take_picture_count = 0;

extern bool capture_start;
extern bool exit_app;

/**
 * Print error message
 */

void printError(enum CamErr err)
{
  Serial.print("Error: ");
  switch (err)
    {
      case CAM_ERR_NO_DEVICE:
        Serial.println("No Device");
        break;
      case CAM_ERR_ILLEGAL_DEVERR:
        Serial.println("Illegal device error");
        break;
      case CAM_ERR_ALREADY_INITIALIZED:
        Serial.println("Already initialized");
        break;
      case CAM_ERR_NOT_INITIALIZED:
        Serial.println("Not initialized");
        break;
      case CAM_ERR_NOT_STILL_INITIALIZED:
        Serial.println("Still picture not initialized");
        break;
      case CAM_ERR_CANT_CREATE_THREAD:
        Serial.println("Failed to create thread");
        break;
      case CAM_ERR_INVALID_PARAM:
        Serial.println("Invalid parameter");
        break;
      case CAM_ERR_NO_MEMORY:
        Serial.println("No memory");
        break;
      case CAM_ERR_USR_INUSED:
        Serial.println("Buffer already in use");
        break;
      case CAM_ERR_NOT_PERMITTED:
        Serial.println("Operation not permitted");
        break;
      default:
        break;
    }
}

/**
 * Callback from Camera library when video frame is captured.
 */

void CamCB(CamImage img)
{

  /* Check the img instance is available or not. */

  if (img.isAvailable())
    {

      /* If you want RGB565 data, convert image data format to RGB565 */

      img.convertPixFormat(CAM_IMAGE_PIX_FMT_RGB565);

      tft.drawRGBBitmap(0, 0, (uint16_t *)img.getImgBuff(), 320, 240);

#if 0
      /* You can use image data directly by using getImgSize() and getImgBuff().
       * for displaying image to a display, etc. */

      Serial.print("Image data size = ");
      Serial.print(img.getImgSize(), DEC);
      Serial.print(" , ");

      Serial.print("buff addr = ");
      Serial.print((unsigned long)img.getImgBuff(), HEX);
      Serial.println("");
#endif
    }
  else
    {
      Serial.print("Failed to get video stream image\n");
    }
}

/**
 * @brief Initialize camera
 */
void setup()
{
  CamErr err;

  gpio_init();
  
  /* Open serial communications and wait for port to open */
  Serial.begin(BAUDRATE);
//  while (!Serial)
//    {
//      ; /* wait for serial port to connect. Needed for native USB port only */
//    }

  /* Initialize SD */
  while (!theSD.begin()) 
    {
      /* wait until SD card is mounted. */
      Serial.println("Insert SD card.");
    }

  /* begin() without parameters means that
   * number of buffers = 1, 30FPS, QVGA, YUV 4:2:2 format */

  Serial.println("Prepare camera");
  err = theCamera.begin();
  if (err != CAM_ERR_SUCCESS)
    {
      printError(err);
    }

  /* Start video stream.
   * If received video stream data from camera device,
   *  camera library call CamCB.
   */

  Serial.println("Start streaming");
  err = theCamera.startStreaming(true, CamCB);
  if (err != CAM_ERR_SUCCESS)
    {
      printError(err);
    }

  /* Auto white balance configuration */

  Serial.println("Set Auto white balance parameter");
  err = theCamera.setAutoWhiteBalanceMode(CAM_WHITE_BALANCE_DAYLIGHT);
  if (err != CAM_ERR_SUCCESS)
    {
      printError(err);
    }
 
  /* Set parameters about still picture.
   * In the following case, QUADVGA and JPEG.
   */

  Serial.println("Set still picture format");
  err = theCamera.setStillPictureImageFormat(
     CAM_IMGSIZE_QUADVGA_H,
     CAM_IMGSIZE_QUADVGA_V,
     CAM_IMAGE_PIX_FMT_JPG);
  if (err != CAM_ERR_SUCCESS)
    {
      printError(err);
    }

  tft.begin(40000000); 
//  tft.setRotation(3); // 270 degree rotation
  tft.setRotation(1);

}

/**
 * @brief Take picture with format JPEG per second
 */

void loop()
{
//  sleep(1); /* wait for one second to take still picture. */

  /* You can change the format of still picture at here also, if you want. */

  /* theCamera.setStillPictureImageFormat(
   *   CAM_IMGSIZE_HD_H,
   *   CAM_IMGSIZE_HD_V,
   *   CAM_IMAGE_PIX_FMT_JPG);
   */

  /* This sample code can take 100 pictures in every one second from starting. */

//  if (take_picture_count < 100)
    if (capture_start)
    {
      capture_start = false;
      
      /* Take still picture.
      * Unlike video stream(startStreaming) , this API wait to receive image data
      *  from camera device.
      */
  
      Serial.println("call takePicture()");
      CamImage img = theCamera.takePicture();

      /* Check availability of the img instance. */
      /* If any error was occured, the img is not available. */

      if (img.isAvailable())
        {
          /* Create file name */
    
          char filename[16] = {0};
          sprintf(filename, "PICT%03d.JPG", take_picture_count);
    
          Serial.print("Save taken picture as ");
          Serial.print(filename);
          Serial.println("");

          /* Remove the old file with the same file name as new created file,
           * and create new file.
           */

          theSD.remove(filename);
          File myFile = theSD.open(filename, FILE_WRITE);
          myFile.write(img.getImgBuff(), img.getImgSize());
          myFile.close();
        }

      take_picture_count++;
    }
//  else
//    {
//      theCamera.end();
//    }

    if (exit_app)
    {
      exit_app = false;
//      theCamera.end();
    }

}
