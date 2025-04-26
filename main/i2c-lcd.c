#include "i2c-lcd.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include "unistd.h"

#define SLAVE_ADDRESS_LCD 0x27

esp_err_t err;

#define I2C_NUM I2C_NUM_0

static const char *TAG = "LCD";

// Функция отправки команд в LCD
void lcd_send_cmd(char cmd)
{
    char data_u, data_l;
    uint8_t data_t[4];

    // Делаем 4-битный режим передачи
    data_u = (cmd & 0xf0); // старшие 4 бита
    data_l = ((cmd << 4) & 0xf0); // младшие 4 бита

    // Генерация последовательности байтов для передачи
    data_t[0] = data_u | 0x0C;  // Включение Enable (en=1, rs=0)
    data_t[1] = data_u | 0x08;  // Выключение Enable (en=0, rs=0)
    data_t[2] = data_l | 0x0C;  // Включение Enable (en=1, rs=0)
    data_t[3] = data_l | 0x08;  // Выключение Enable (en=0, rs=0)
}

// Функция отправки данных в LCD (символы)
void lcd_send_data(char data)
{
    char data_u, data_l;
    uint8_t data_t[4];

    // Делаем 4-битный режим передачи
    data_u = (data & 0xf0);
    data_l = ((data << 4) & 0xf0);

    // Генерация последовательности байтов для передачи
    data_t[0] = data_u | 0x0D;  // Включение Enable (en=1, rs=1)
    data_t[1] = data_u | 0x09;  // Выключение Enable (en=0, rs=1)
    data_t[2] = data_l | 0x0D;  // Включение Enable (en=1, rs=1)
    data_t[3] = data_l | 0x09;  // Выключение Enable (en=0, rs=1)
}

// Функция очистки экрана
void lcd_clear(void)
{
    lcd_send_cmd(0x01);  // Команда очистки экрана
    usleep(5000);        // Задержка на выполнение команды
}


void lcd_set_cursor(int row, int col)
{
    if (col < 0 || col >= 20) return;

	switch (row)
	{
		case 0:
			col |= 0x80;
			break;
		case 1:
			col |= 0xC0;
			break;
		case 2:
			col |= 0x94;
			break;
		case 3:
			col |= 0xD4;
			break;
	}
	lcd_send_cmd(col);
}

void lcd_init (void)
{
	// 4 bit initialisation
	usleep(50000);  // wait for >40ms
	lcd_send_cmd (0x30); // 8bit mode
	usleep(5000);  // wait for >4.1ms
	lcd_send_cmd (0x30);
	usleep(200);  // wait for >100us
	lcd_send_cmd (0x30); // ready to 4bit
	usleep(10000);
	lcd_send_cmd (0x20);  // 4bit mode
	usleep(10000);

  // dislay initialisation
	lcd_send_cmd (0x28); // Function set --> DL=0 (4 bit mode), N = 1 (2 line display) F = 0 (5x8 characters)
	usleep(1000);
	lcd_send_cmd (0x08); //Display on/off control --> D=0,C=0, B=0  ---> display off
	usleep(1000);
	lcd_send_cmd (0x01);  // clear display
	usleep(1000);
	usleep(1000);
	lcd_send_cmd (0x06); //Entry mode set --> I/D = 1 (increment cursor) & S = 0 (no shift)
	usleep(1000);
	lcd_send_cmd (0x0C); //Display on/off control --> D = 1, C and B = 0. (Cursor and blink, last two bits)
	usleep(1000);
}

void lcd_send_string (char *str)
{
	while (*str) lcd_send_data (*str++);
}