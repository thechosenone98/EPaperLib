/*
 * File:   EPD.c
 * Author: zacfi
 *
 * Created on November 14, 2020, 3:42 PM
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "EPD.h"
#include "xc.h"
#include "mcc_generated_files/system.h"
#include "mcc_generated_files/delay.h"
#include "mcc_generated_files/drivers/spi_master.h"
#include "mcc_generated_files/pin_manager.h"

// <editor-fold defaultstate="collapsed" desc="EPD Global Variables">
bool using_partial_mode = false;
bool use_partial_update_window = true;
bool initial_write = true;
bool powered = false;
bool hibernating = true;
// </editor-fold>
// <editor-fold defaultstate="collapsed" desc="EPD Commands">
//Booster soft Start
commandArray booster_soft_start_cmd = {
    .command = 0x06,
    .parameter = {0x17,0x17,0x17},
    .param_length = 3,
};
//Power Settings
commandArray power_settings_cmd = {
    .command = 0x01,
    .parameter = {0x03,0x00,0x2b,0x2b,0x09},
    .param_length = 5,
};
//Power on
commandArray power_on_cmd = {
    .command = 0x04,
    .parameter = {NULL},
    .param_length = 0,
};
//Power off
commandArray power_off_cmd = {
    .command = 0x02,
    .parameter = {NULL},
    .param_length = 0,
};
//Panel Settings
commandArray panel_settings_cmd = {
    .command = 0x00,
    .parameter = {0x9F},
    .param_length = 1,
};
//Resolution Settings
commandArray resolution_settings_cmd = {
    .command = 0x61,
    .parameter = {0x80,0x01,0x28},
    .param_length = 3,
};
//VCOM and Data Interval (Full screen Mode)
commandArray vcom_data_interval_full_cmd = {
    .command = 0x50,
    .parameter = {0x57},
    .param_length = 1,
};
//VCOM and Data Interval (Partial Mode)
commandArray vcom_data_interval_part_cmd = {
    .command = 0x50,
    .parameter = {0xD7},
    .param_length = 1,
};
//Display Refresh
commandArray display_refresh_cmd = {
    .command = 0x12,
    .parameter = {NULL},
    .param_length = 0,
};
//Data Start Transmission
commandArray data_start_old_cmd = {
    .command = 0x10,
    .parameter = {NULL},
    .param_length = 0,
};
//Data Start Transmission
commandArray data_start_new_cmd = {
    .command = 0x13,
    .parameter = {NULL},
    .param_length = 0,
};
//Data Stop Transmission
commandArray data_stop_cmd = {
    .command = 0x11,
    .parameter = {0x00},
    .param_length = 1,
};
//Partial Window
commandArray partial_window_cmd = {
    .command = 0x90,
    .parameter = {0x00,0x00,0x00,0x00,0x00,0x00,0x01},
    .param_length = 7,
};
//Partial In
commandArray partial_in_cmd = {
    .command = 0x91,
    .parameter = {NULL},
    .param_length = 0,
};
//Partial Out
commandArray partial_out_cmd = {
    .command = 0x92,
    .parameter = {NULL},
    .param_length = 0,
};
// </editor-fold>
// <editor-fold defaultstate="collapsed" desc="EPD Functions">
void SendCommand(commandArray *cmd){
    SS_SetLow();
    if(!(cmd->parameter == NULL)){
        DC_SetLow();
        spi1_exchangeByte(cmd->command);
    }
    if(cmd->param_length != 0)
    {
        DC_SetHigh();
        spi1_exchangeBlock(cmd->parameter, cmd->param_length);
        DC_SetLow();
    }
    SS_SetHigh();
}

void SendData(uint8_t data){
    SS_SetLow();
    DC_SetHigh();
    spi1_exchangeByte(data);
    DC_SetLow();
    SS_SetHigh();
}

void InitDisplay(void){
    printf("INIT DISPLAY\n");
    if(hibernating) Reset();
    //Booster Soft Start SPI
    SendCommand(&booster_soft_start_cmd);
    if(!powered){
        //Power Settings
        SendCommand(&power_settings_cmd);
    }
    //Panel Settings
    SendCommand(&panel_settings_cmd);
    //VCOM and Data Interval
    SendCommand(&vcom_data_interval_full_cmd);
    //Resolution Settings
    SendCommand(&resolution_settings_cmd);
}

void InitFullMode(void){
    InitDisplay();
    PowerOn();
    using_partial_mode = false;
}

void InitPartMode(void){
    printf("INIT PART MODE\n");
    InitDisplay();
    SendCommand(&vcom_data_interval_part_cmd);
    PowerOn();
    using_partial_mode = true;
}

void PowerOn(void){
    printf("POWER ON\n");
    if(!powered){
        SendCommand(&power_on_cmd);
        while(BUSY_GetValue() == 0);
    }
    powered = true;
}

void PowerOff(void){
    if(powered){
        SendCommand(&power_off_cmd);
        while(BUSY_GetValue() == 0);
    }
    powered = false;
}

void Reset(void){
    RESET_SetLow();
    DELAY_milliseconds(1000);
    RESET_SetHigh();
    while(BUSY_GetValue() == 0);
    hibernating = false;
}

void SetPartialRamArea(uint16_t x, uint16_t y, uint16_t w, uint16_t h){
    printf("SET PARTIAL RAM AREA\n");
    uint16_t xe = (x + w - 1) | 0x0007; // byte boundary inclusive (last byte)
    uint16_t ye = y + h - 1;
    x &= (uint16_t)0xFFF8; // byte boundary
    partial_window_cmd.parameter[0] = x % 256;
    partial_window_cmd.parameter[1] = xe % 256;
    partial_window_cmd.parameter[2] = y / 256;
    partial_window_cmd.parameter[3] = y % 256;
    partial_window_cmd.parameter[4] = ye / 256;
    partial_window_cmd.parameter[5] = ye % 256;
    SendCommand(&partial_window_cmd); // partial window
}

void Update(void){
    printf("UPDATE\n");
    SendCommand(&display_refresh_cmd);
    while(BUSY_GetValue() == 0);
}

void Refresh(uint16_t x, uint16_t y, uint16_t w, uint16_t h){
    printf("REFRESH\n");
    x -= x % 8; // byte boundary
    w -= x % 8; // byte boundary
    uint16_t x1 = x < 0 ? 0 : x; // limit
    uint16_t y1 = y < 0 ? 0 : y; // limit
    uint16_t w1 = x + w < (uint16_t)WIDTH ? w : (uint16_t)WIDTH - x; // limit
    uint16_t h1 = y + h < (uint16_t)HEIGHT ? h : (uint16_t)HEIGHT - y; // limit
    w1 -= x1 - x;
    h1 -= y1 - y;
    if(use_partial_update_window) SendCommand(&partial_in_cmd); // partial in
    SetPartialRamArea(x1, y1, w1, h1);
    Update();
    if(use_partial_update_window) SendCommand(&partial_out_cmd); // partial out
}

void WriteScreenBuffer(uint8_t black_value)
{
    printf("WRITE SCREEN BUFFER\n");
    InitPartMode();
    SendCommand(&partial_in_cmd); // partial in
    SetPartialRamArea(0, 0, WIDTH, HEIGHT);
    SendCommand(&data_start_new_cmd);
    uint32_t i = 0;
    for (i = 0; i < (uint32_t)WIDTH * (uint32_t)HEIGHT / 8; i++)
    {
      SendData(black_value);
    }
    SendCommand(&partial_out_cmd); // partial out
    initial_write = false; // initial full screen buffer clean done
}

void ClearScreen(uint8_t color){
    printf("CLEAR SCREEN\n");
    //Initialize the screen in partial mode
    InitPartMode();
    //Partial In
    SendCommand(&partial_in_cmd);
    SetPartialRamArea(0, 0, WIDTH, HEIGHT);
    //Start NEW data transmission 2
    SendCommand(&data_start_new_cmd);
    //Set all of these to color
    uint32_t i = 0;
    for(i = 0; i < (uint32_t)WIDTH * HEIGHT / 8; ++i)
        SendData(color);
    Update();
    SendCommand(&partial_out_cmd);
    initial_write = false;
}

void WriteImage(const uint8_t *bitmap, uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool invert, bool mirror_y){
    printf("WRITE IMAGE\n");
    if(initial_write) WriteScreenBuffer(0x00); // Initial full screen buffer clean
    uint16_t wb = (w + 7) / 8; // width bytes, bitmaps are padded
    x -= x % 8; // byte boundary
    w = wb * 8; // byte boundary
    uint16_t x1 = x < 0 ? 0 : x; // limit
    uint16_t y1 = y < 0 ? 0 : y; // limit
    uint16_t w1 = x + w < (uint16_t)WIDTH ? w : (uint16_t)WIDTH - x; // limit
    uint16_t h1 = y + h < (uint16_t)HEIGHT ? h : (uint16_t)HEIGHT - y; // limit
    uint16_t dx = x1 - x;
    uint16_t dy = y1 - y;
    w1 -= dx;
    h1 -= dy;
    if ((w1 <= 0) || (h1 <= 0)) return;
    //Start Partial Mode
    SendCommand(&partial_in_cmd);
    SetPartialRamArea(x1, y1, w1, h1);
    uint16_t i = 0;
    uint16_t j = 0;
    SendCommand(&data_start_new_cmd);
    for (i = 0; i < h1; i++){
        for (j = 0; j < w1 / 8; j++){
            uint8_t data;
            int16_t idx = mirror_y ? j + dx / 8 + ((h - 1 - (i + dy))) * wb : j + dx / 8 + (i + dy) * wb;
            data = bitmap[idx];
            if (invert) data = ~data;
            SendData(data);
        }
    }
    SendCommand(&partial_out_cmd);
}

void WriteImagePart(const uint8_t* bitmap, uint16_t x_part, uint16_t y_part, uint16_t w_bitmap, uint16_t h_bitmap,
                   uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool invert, bool mirror_y){
    if (initial_write) WriteScreenBuffer(0x00); // initial full screen buffer clean
    printf("FIRST TEST");
    if ((w_bitmap < 0) || (h_bitmap < 0) || (w < 0) || (h < 0)) return;
    printf("SECOND TEST");
    if ((x_part < 0) || (x_part >= w_bitmap)) return;
    printf("THIRD TEST");
    if ((y_part < 0) || (y_part >= h_bitmap)) return;
    printf("BUNCH OF MATH");
//    int16_t wb_bitmap = (w_bitmap + 7) / 8; // width bytes, bitmaps are padded
//    x_part -= x_part % 8; // byte boundary
//    w = w_bitmap - x_part < w ? w_bitmap - x_part : w; // limit
//    h = h_bitmap - y_part < h ? h_bitmap - y_part : h; // limit
//    x -= x % 8; // byte boundary
//    w = 8 * ((w + 7) / 8); // byte boundary, bitmaps are padded
//    int16_t x1 = x < 0 ? 0 : x; // limit
//    int16_t y1 = y < 0 ? 0 : y; // limit
//    int16_t w1 = x + w < (int16_t)WIDTH ? w : (int16_t)WIDTH - x; // limit
//    int16_t h1 = y + h < (int16_t)HEIGHT ? h : (int16_t)HEIGHT - y; // limit
//    int16_t dx = x1 - x;
//    int16_t dy = y1 - y;
//    w1 -= dx;
//    h1 -= dy;
    printf("LAST TEST");
//    if ((x + w <= 0) || (y + h <= 0)) return;
    if (!using_partial_mode) InitPartMode();
    SendCommand(&partial_in_cmd);
    SetPartialRamArea(x, y, x + w, y + h);
    SendCommand(&data_start_new_cmd);
    int16_t i = 0;
    int16_t j = 0;
    for (i = y_part; i < h_bitmap; i++){
        for (j = x_part / 8; j < w_bitmap / 8; j++){
            uint8_t data;
            int16_t idx = j + i * (w_bitmap / 8);
            data = bitmap[idx];
            if (invert) data = ~data;
            SendData(data);
        }
    }
    SendCommand(&partial_out_cmd);
}

void DrawImage(const uint8_t* bitmap, uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool invert, bool mirror_y){
    printf("DRAW IMAGE\n");
    WriteImage(bitmap, x, y, w, h, invert, mirror_y);
    Refresh(x, y, w, h);
}

void DrawImagePart(const uint8_t* bitmap, uint16_t x_part, uint16_t y_part, uint16_t w_bitmap, uint16_t h_bitmap,
                   uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool invert, bool mirror_y){
    WriteImagePart(bitmap, x_part, y_part, w_bitmap, h_bitmap, x, y, w, h, invert, mirror_y);
    Refresh(x, y, w, h);
}
// </editor-fold>
// <editor-fold defaultstate="collapsed" desc="TEST Functions">
void SendCommandTest(uint8_t cmd){
    SS_SetLow();
    DC_SetLow();
    spi1_exchangeByte(cmd);
    DC_SetHigh();
    SS_SetHigh();
}

void SendDataTest(uint8_t data){
    SS_SetLow();
    spi1_exchangeByte(data);
    SS_SetHigh();
}

void ClearScreenTest(void){
    RESET_SetLow();
    DELAY_milliseconds(1000);
    RESET_SetHigh();
    while(BUSY_GetValue() == 0);
    SendCommandTest(0x06);
    SendDataTest(0x17);
    SendDataTest(0x17);
    SendDataTest(0x17);
    SendCommandTest(0x01);
    SendDataTest(0x03);
    SendDataTest(0x00);
    SendDataTest(0x2B);
    SendDataTest(0x2B);
    SendDataTest(0x09);
    SendCommandTest(0x04);
    while(BUSY_GetValue() == 0);
    SendCommandTest(0x00);
    SendDataTest(0x1f);
    SendCommandTest(0x50);
    SendDataTest(0x87);
    SendCommandTest(0x61);
    SendDataTest(0x80);
    SendDataTest(0x01);
    SendDataTest(0x28);
    SendCommandTest(0x91);
    SetPartialRamArea(0,0,WIDTH, HEIGHT);
    SendCommandTest(0x10);
    uint32_t i = 0;
    for (i = 0; i < (uint32_t)WIDTH * (uint32_t)HEIGHT / 8; i++)
        SendDataTest(0xFF);
    SendCommandTest(0x13);
    for (i = 0; i < (uint32_t)WIDTH * (uint32_t)HEIGHT / 8; i++)
        SendDataTest(0xFF);
    SendCommandTest(0x12);
    while(BUSY_GetValue() == 0);
    SendCommandTest(0x92);
}
// </editor-fold>