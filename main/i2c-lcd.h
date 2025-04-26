#pragma once

#define SLAVE_ADDRESS_LCD 0x27
#define I2C_NUM I2C_NUM_0

void lcd_init (void);  
void lcd_send_cmd (char cmd); 
void lcd_send_data (char data);  
void lcd_send_string (char *str); 
void lcd_set_cursor(int row, int col);
void lcd_clear (void);
