#ifndef SSD1306_H_INCLUDED
#define SSD1306_H_INCLUDED

#include <stdint.h>

// Force a framebuffer type
#define SSD1306_TWI_INTERFACE 1

#ifdef __cplusplus
extern "C" {
#endif

void ssd1306_init(void);
void ssd1306_set_display_start_line_address(uint8_t address);
void ssd1306_set_page_address(uint8_t address);
void ssd1306_set_column_address(uint8_t address);
void ssd1306_write_data(uint8_t data);
void ssd1306_write_data_buffer(const uint8_t *data, uint8_t size);
uint8_t ssd1306_read_data();

#ifdef __cplusplus
}
#endif

#endif /* SSD1306_H_INCLUDED */
