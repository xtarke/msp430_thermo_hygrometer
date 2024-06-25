/*
 * SSD1306.h
 *
 *  Created on: Jun 19, 2024
 *      Author: xtarke
 */

#ifndef SSD1306_H_
#define SSD1306_H_

#include <stdint.h>

/* Following definitions are from:
   http://robotcantalk.blogspot.com/2015/03/interfacing-arduino-with-ssd1306-driven.html
*/
// #define OLED_I2C_ADDRESS   0x3C

// Configuration data
#define OLED_DISPLAY_WIDTH_PX           0x80
#define OLED_PAGE_HEIGHT_PX             0x08

#define OLED_HEIGHT 64
#define OLED_WIDTH 128

// Control byte
#define OLED_CONTROL_BYTE_CMD_SINGLE    0x80
#define OLED_CONTROL_BYTE_CMD_STREAM    0x00
#define OLED_CONTROL_BYTE_DATA_STREAM   0x40

// Fundamental commands (pg.28)
#define OLED_CMD_SET_CONTRAST           0x81    // follow with 0x7F
#define OLED_CMD_DISPLAY_RAM            0xA4
#define OLED_CMD_DISPLAY_ALLON          0xA5
#define OLED_CMD_DISPLAY_NORMAL         0xA6
#define OLED_CMD_DISPLAY_INVERTED       0xA7
#define OLED_CMD_DISPLAY_OFF            0xAE
#define OLED_CMD_DISPLAY_ON             0xAF
#define OLED_DEACTIVATE_SCROLL          0x2E

// Addressing Command Table (pg.30)
#define OLED_CMD_SET_MEMORY_ADDR_MODE   0x20    // follow with 0x00 = HORZ mode = Behave like a KS108 graphic LCD
#define OLED_CMD_SET_COLUMN_RANGE       0x21    // can be used only in HORZ/VERT mode - follow with 0x00 and 0x7F = COL127
#define OLED_CMD_SET_PAGE_RANGE         0x22    // can be used only in HORZ/VERT mode - follow with 0x00 and 0x07 = PAGE7

// Hardware Config (pg.31)
#define OLED_CMD_SET_DISPLAY_START_LINE 0x40
//#define OLED_CMD_SET_SEGMENT_REMAP      0xA0
#define OLED_CMD_SET_SEGMENT_REMAP      0xA1
#define OLED_CMD_SET_MUX_RATIO          0xA8    // follow with 0x3F = 64 MUX
#define OLED_CMD_SET_COM_SCAN_MODE      0xC8
#define OLED_CMD_SET_DISPLAY_OFFSET     0xD3    // follow with 0x00
#define OLED_CMD_SET_COM_PIN_MAP        0xDA    // follow with 0x12
#define OLED_CMD_NOP                    0xE3    // NOP
#define OLED_SETSTARTLINE               0x40

// Timing and Driving Scheme (pg.32)
#define OLED_CMD_SET_DISPLAY_CLK_DIV    0xD5    // follow with 0x80
#define OLED_CMD_SET_PRECHARGE          0xD9    // follow with 0xF1
#define OLED_CMD_SET_VCOMH_DESELCT      0xDB    // follow with 0x30

// Charge Pump (pg.62)
#define OLED_CMD_SET_CHARGE_PUMP        0x8D    // follow with 0x14

class SSD1306
{
public:
    typedef enum {
         WHITE_PIXEL, BLACK_PIXEL
    } pixel_color_t;

    /* Not enough RAM for 1k OLED frame Buffer *
     * Using 4 partitions                      */
    typedef enum {
        LINE_1 = 0,     /* First line PAGE_RANGE */
        LINE_2 = 0x32,  /* Second line PAGE_RANGE */
        LINE_3 = 0x64,  /* Third line PAGE_RANGE */
        LINE_4 = 0x96   /* Fourth line PAGE_RANGE */
    } oled_partition_t;

    SSD1306(uint8_t i2c_addr);

    void Init();
    void ClearFrameBuffer(void);
    void DrawPixel(int16_t x, int16_t y, pixel_color_t color);
    void FillRect(int16_t x, int16_t y, int16_t w, int16_t h, pixel_color_t color);
    void WriteFastVLine(int16_t x, int16_t y, int16_t h, pixel_color_t color);
    void WriteLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, SSD1306::pixel_color_t color);
    void WriteScaledChar(int16_t x, int16_t y, char data, uint8_t scale);
    void Refresh();
    void Refresh(oled_partition_t line);

private:
    uint8_t my_i2c_addr;

#if defined(__MSP430G2553__)
    /* Not enough RAM for 1k OLED frame Buffer *
     * Using 4 partitions                      */
    uint8_t frame_buffer[(OLED_WIDTH * ((OLED_HEIGHT + 7) / 8))/4];
#else
    /*1k OLED frame Buffer */
    uint8_t frame_buffer[(OLED_WIDTH * ((OLED_HEIGHT + 7) / 8))];
#endif

    void send_single_command(uint8_t data);
    void send_command_list(uint8_t *data, uint8_t size);

    const uint8_t init[3][5] = {
        {
            0x00,
            OLED_CMD_DISPLAY_OFF,           // 0xAE
            OLED_CMD_SET_DISPLAY_CLK_DIV,   // 0xD5
            0x80,                           // the suggested ratio 0x80
            OLED_CMD_SET_MUX_RATIO          // 0xA8
        },
        {
            0x00,
            OLED_CMD_SET_DISPLAY_OFFSET,    // 0xD3
            0x0,                            // no offset
            OLED_SETSTARTLINE | 0x0,        // 0x40 line #0
            OLED_CMD_SET_CHARGE_PUMP        // 0x8D
        },
        {
            0x00,
            OLED_CMD_SET_MEMORY_ADDR_MODE,      // 0x20
            0x00,                               // 0x0 act like ks0108
            OLED_CMD_SET_SEGMENT_REMAP | 0x1,   //0xa1
            OLED_CMD_SET_COM_SCAN_MODE,         //0xc8
        },
    };

    const uint8_t init_disp_on[7] = {
        0x00,
        OLED_CMD_SET_VCOMH_DESELCT, // 0xDB
        0x40,
        OLED_CMD_DISPLAY_RAM, // 0xA4
        OLED_CMD_DISPLAY_NORMAL,       // 0xA6
        OLED_DEACTIVATE_SCROLL,
        OLED_CMD_DISPLAY_ON         // Main screen turn on
    };

};

#endif /* SSD1306_H_ */
