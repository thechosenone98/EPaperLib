/**
  Generated main.c file from MPLAB Code Configurator

  @Company
    Microchip Technology Inc.

  @File Name
    main.c

  @Summary
    This is the generated main.c using PIC24 / dsPIC33 / PIC32MM MCUs.

  @Description
    This source file provides main entry point for system initialization and application code development.
    Generation Information :
        Product Revision  :  PIC24 / dsPIC33 / PIC32MM MCUs - 1.169.0
        Device            :  PIC24FJ64GA002
    The generated drivers are tested against the following:
        Compiler          :  XC16 v1.50
        MPLAB 	          :  MPLAB X v5.40
*/

/*
    (c) 2020 Microchip Technology Inc. and its subsidiaries. You may use this
    software and any derivatives exclusively with Microchip products.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
    WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
    TERMS.
*/

/**
  Section: Included Files
*/
#include <string.h>

#include "mcc_generated_files/system.h"
#include "mcc_generated_files/delay.h"
#include "mcc_generated_files/drivers/spi_master.h"
#include "mcc_generated_files/pin_manager.h"
#include "stdlib.h"
#include "EPD.h"
#include "epaper_image.h"

/*
                         Main application
 */
int main(void)
{
    // initialize the device
    SYSTEM_Initialize();
    //Add delay so the system settles before we start bombarding SPI ports
    printf("Waiting for Startup\n");
    DELAY_milliseconds(2000);
    printf("Open SPI port\n");
    //Open SPI port
    if(!spi_master_open(MASTER0))
        _RB3 = 0;
    else
        _RB3 = 1;
    //Initialize EPD
    DrawImagePart(gImage_epaper_image, 0, 0, WIDTH-50, HEIGHT-50, 25, 25, WIDTH-50, HEIGHT-50, true, false);
    while (1)
    {
    }

    return 1;
}
/**
 End of File
*/
