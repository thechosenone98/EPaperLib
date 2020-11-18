/* 
 * File:   EPD.h
 * Author: zacfi
 *
 * Created on November 14, 2020, 3:40 PM
 */

#ifndef EPD_H
#define	EPD_H

#define WIDTH 128
#define HEIGHT 296

extern bool using_partial_mode;
extern bool use_partial_update_window;
extern bool initial_write;
extern bool powered;
extern bool hibernating;

typedef struct
{
  int param_length;
  uint8_t command;
  uint8_t parameter[];
} commandArray;

void SendCommand(commandArray *cmd);
void SendData(uint8_t data);
void InitDisplay(void);
void InitFullMode(void);
void InitPartMode(void);
void PowerOn(void);
void PowerOff(void);
void Reset(void);
void SetPartialRamArea(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void Update(void);
void Refresh(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void WriteScreenBuffer(uint8_t black_value);
void ClearScreen(uint8_t color);
void WriteImage(const uint8_t *bitmap, uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool invert, bool mirror_y);
void WriteImagePart(const uint8_t* bitmap, uint16_t w_bitmap, uint16_t h_bitmap, uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end,
                   uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool invert, bool mirror_y);
void DrawImage(const uint8_t *bitmap, uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool invert, bool mirror_y);
void DrawImagePart(const uint8_t* bitmap, uint16_t w_bitmap, uint16_t h_bitmap, uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end,
                   uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool invert, bool mirror_y);


void ClearScreenTest(void);
void SendDataTest(uint8_t data);
void SendCommandTest(uint8_t cmd);


#endif	/* EPD_H */

