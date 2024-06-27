/*
 * SSD1306.cpp
 *
 *  Created on: Jun 19, 2024
 *      Author: Renan Augusto Starke
 */

#include <lib/i2c_master_f247_g2xxx.h>
#include <string.h>

#include "SSD1306.h"
#include "font8x8_basic.h"

#ifndef _swap_int16_t
#define _swap_int16_t(a, b)                                                    \
        {                                                                            \
    int16_t t = a;                                                             \
    a = b;                                                                     \
    b = t;                                                                     \
        }
#endif


SSD1306::SSD1306(uint8_t i2c_addr)
{
    my_i2c_addr = i2c_addr;
    /* Clear frame buffer */
    memset(frame_buffer, 0, sizeof(frame_buffer));
}

void SSD1306::Init(){
    /* Send all initialization commands */
    send_command_list((uint8_t *)init[0], sizeof(init[0]));
    send_single_command(OLED_HEIGHT - 1);
    send_command_list((uint8_t *)init[1], sizeof(init[1]));
    send_single_command(0x14);
    send_command_list((uint8_t *)init[2], sizeof(init[2]));

    send_single_command(OLED_CMD_SET_COM_PIN_MAP);
    /* Pin MAP must be followed by 0x12*/
    send_single_command(0x12);
    send_single_command(OLED_CMD_SET_CONTRAST);
    /* Contrast  must be followed by 0xCF*/
    send_single_command(0xCF);
    send_single_command(OLED_CMD_SET_PRECHARGE);
    /* Precharge must be followed by 0xF1 */
    send_single_command(0xF1);
    send_command_list((uint8_t *)init_disp_on, sizeof(init_disp_on));
}

void SSD1306::send_single_command(uint8_t data){
    i2c_master_write_reg(my_i2c_addr, 0x00, &data, 1);
}

void SSD1306::send_command_list(uint8_t *data, uint8_t size){
    i2c_master_write_reg(my_i2c_addr, data[0], data + 1, size - 1);
}

void SSD1306::ClearFrameBuffer(void) {
    memset(frame_buffer, 0, sizeof(frame_buffer));
}

void SSD1306::Refresh(){
    uint8_t *data = frame_buffer;
    int i;
    const uint8_t cmd[] = {
                                  0x00,
                                  OLED_CMD_SET_PAGE_RANGE,   // 0x22
                                  0x00, //
                                  0xFF,
                                  OLED_CMD_SET_COLUMN_RANGE, // 0x21
                                  0};

    send_command_list((uint8_t *)cmd, sizeof(cmd));
    send_single_command(OLED_WIDTH - 1);

#if defined(__MSP430G2553__)
    for (i=0; i < 256; i+=128){
#else
    for (i=0; i < 1024; i+=128){
#endif
        i2c_master_write_reg(my_i2c_addr, 0x40, data + i, 128);
    }
}

void SSD1306::Refresh(oled_partition_t line){
    uint8_t *data = frame_buffer;
    int i;
    const uint8_t cmd[] = {
                                  0x00,
                                  OLED_CMD_SET_PAGE_RANGE,   // 0x22
                                  (uint8_t) line, //
                                  0xFF,
                                  OLED_CMD_SET_COLUMN_RANGE, // 0x21
                                  0};

    send_command_list((uint8_t *)cmd, sizeof(cmd));
    send_single_command(OLED_WIDTH - 1);

#if defined(__MSP430G2553__)
    for (i=0; i < 256; i+=128){
#else
    for (i=0; i < 1024; i+=128) {
#endif
        i2c_master_write_reg(my_i2c_addr, 0x40, data + i, 128);
    }

}

void SSD1306::DrawPixel(int16_t x, int16_t y, pixel_color_t color){
    if ((x >= 0) && (x < OLED_WIDTH && (y >= 0) && (y < OLED_HEIGHT))) {
        uint16_t i = x + (y >> 3) * OLED_WIDTH;

        if (i > sizeof(frame_buffer) * 4)
            return;

        if (color)
            // oled_buffer[x + (y / 8) * OLED_WIDTH] &= ~(1 << (y & 7));
            //frame_buffer[x + (y >> 3) * OLED_WIDTH] &= ~(1 << (y & 7));
            frame_buffer[i] &= ~(1 << (y & 7));
        else
            //oled_buffer[x + (y / 8) * OLED_WIDTH] |= (1 << (y & 7));
            //frame_buffer[x + (y >> 3) * OLED_WIDTH] |= (1 << (y & 7));
            frame_buffer[i] |= (1 << (y & 7));
    }
}

void SSD1306::WriteScaledChar(int16_t x, int16_t y, char data, uint8_t scale){

    int8_t i;
    int8_t j;
    const uint8_t *font_ptr =  font8x8_basic_tr[(uint8_t)data];

    for (i = 0; i < 8; i++) {
        uint8_t line = *(font_ptr + i);

        for (j = 0; j < 8; j++, line >>= 1) {
            if (line & 1)
                FillRect(x + i * scale, y + j * scale, scale, scale, WHITE_PIXEL);
            else
                FillRect(x + i * scale, y + j * scale, scale, scale, BLACK_PIXEL);
        }
    }
}


void SSD1306::WriteLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, pixel_color_t color){
    int16_t steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep) {
        _swap_int16_t(x0, y0);
        _swap_int16_t(x1, y1);
    }

    if (x0 > x1) {
        _swap_int16_t(x0, x1);
        _swap_int16_t(y0, y1);
    }

    int16_t dx, dy;
    dx = x1 - x0;
    dy = abs(y1 - y0);

    int16_t err = dx >> 1;
    int16_t ystep;

    if (y0 < y1) {
        ystep = 1;
    } else {
        ystep = -1;
    }

    for (; x0 <= x1; x0++) {
        if (steep) {
            DrawPixel(y0, x0, color);
        } else {
            DrawPixel(x0, y0, color);
        }
        err -= dy;
        if (err < 0) {
            y0 += ystep;
            err += dx;
        }
    }
}

void SSD1306::WriteFastVLine(int16_t x, int16_t y, int16_t h, pixel_color_t color){
    WriteLine(x, y, x, y + h - 1, color);
}


void SSD1306::FillRect(int16_t x, int16_t y, int16_t w, int16_t h, pixel_color_t color){
    int16_t i;
    for (i = x; i < x + w; i++) {
        WriteFastVLine(i, y, h, color);
    }
}

