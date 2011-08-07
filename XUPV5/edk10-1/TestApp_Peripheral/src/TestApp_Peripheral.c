#include "xparameters.h"
#include "xbasic_types.h"
#include "xgpio.h"
#include "xstatus.h"
#include "xtmrctr.h"

// Masks to the pins on the GPIO port

#define LCD_DB4    0x01
#define LCD_DB5    0x02
#define LCD_DB6    0x04
#define LCD_DB7    0x08
#define LCD_RW     0x10
#define LCD_RS     0x20
#define LCD_E      0x40
#define LCD_TEST   0x80

// Global variables

XGpio GpioOutput;
XTmrCtr DelayTimer;

// Function prototypes

void delay_us(Xuint32 time);
void delay_ms(Xuint32 time);
void gpio_write(Xuint32 c);
Xuint32 gpio_read(void);
void lcd_clk(void);
void lcd_set_test(void);
void lcd_reset_test(void);
void lcd_set_rs(void);
void lcd_reset_rs(void);
void lcd_set_rw(void);
void lcd_reset_rw(void);
void lcd_write(Xuint32 c);
void lcd_clear(void);
void lcd_puts(const char * s);
void lcd_putch(Xuint32 c);
void lcd_goto(Xuint32 line,Xuint32 pos);
void lcd_init(void);

// Main function

int main (void)
{
  Xuint32 status;

  // Clear the screen
  xil_printf("%c[2J",27);
  xil_printf("16x2 LCD Driver by Virtex-5 Resource\r\n");
  xil_printf("http://virtex5.blogspot.com\r\n");

  // Initialize the Timer
  status = XTmrCtr_Initialize(&DelayTimer,
                                XPAR_XPS_TIMER_0_DEVICE_ID);
  if (status != XST_SUCCESS){
    xil_printf("Timer failed to initialize\r\n");
    return XST_FAILURE;
  }
  XTmrCtr_SetOptions(&DelayTimer, 1, XTC_DOWN_COUNT_OPTION);
 
  // Initialize the GPIO driver for the LCD
  status = XGpio_Initialize(&GpioOutput,
                                 XPAR_XPS_GPIO_0_DEVICE_ID);
  if (status != XST_SUCCESS){
    xil_printf("GPIO failed to initialize\r\n");
    return XST_FAILURE;
  }
  // Set the direction for all signals to be outputs
  XGpio_SetDataDirection(&GpioOutput, 1, 0x00);

  // Initialize the LCD
  lcd_init();

  // Example write to the LCD
  lcd_puts("http://virtex5");
  lcd_goto(1,2);
  lcd_puts(".blogspot.com");

  while(1){
  }
}

// Delay function (microseconds)
void delay_us(Xuint32 time)
{
  XTmrCtr_SetResetValue(&DelayTimer, 1, time * 125);
  XTmrCtr_Start(&DelayTimer, 1);
  while(!(XTmrCtr_IsExpired(&DelayTimer, 1))){}
  XTmrCtr_Stop(&DelayTimer, 1);
}

// Delay function (milliseconds)
void delay_ms(Xuint32 time)
{
  XTmrCtr_SetResetValue(&DelayTimer, 1, time * 125000);
  XTmrCtr_Start(&DelayTimer, 1);
  while(!(XTmrCtr_IsExpired(&DelayTimer, 1))){}
  XTmrCtr_Stop(&DelayTimer, 1);
}

// Write to GPIO outputs
void gpio_write(Xuint32 c)
{
  // Write to the GP IOs
  XGpio_DiscreteWrite(&GpioOutput, 1, c & 0x0FF);
}

// Read the GPIO outputs
Xuint32 gpio_read()
{
  // Read from the GP IOs
  return(XGpio_DiscreteRead(&GpioOutput, 1));
}

// Clock the LCD (toggles E)
void lcd_clk()
{
  Xuint32 c;
  // Get existing outputs
  c = gpio_read();
  delay_us(1);
  // Assert clock signal
  gpio_write(c | LCD_E);
  delay_us(1);
  // Deassert the clock signal
  gpio_write(c & (~LCD_E));
  delay_us(1);
}

// Assert the RS signal
void lcd_set_rs()
{
  Xuint32 c;
  // Get existing outputs
  c = gpio_read();
  // Assert RS
  gpio_write(c | LCD_RS);
  delay_us(1);
}

// Deassert the RS signal
void lcd_reset_rs()
{
  Xuint32 c;
  // Get existing outputs
  c = gpio_read();
  // Assert RS
  gpio_write(c & (~LCD_RS));
  delay_us(1);
}

// Assert the RW signal
void lcd_set_rw()
{
  Xuint32 c;
  // Get existing outputs
  c = gpio_read();
  // Assert RS
  gpio_write(c | LCD_RW);
  delay_us(1);
}

// Deassert the RW signal
void lcd_reset_rw()
{
  Xuint32 c;
  // Get existing outputs
  c = gpio_read();
  // Assert RS
  gpio_write(c & (~LCD_RW));
  delay_us(1);
}

// Write a byte to LCD (4 bit mode)
void lcd_write(Xuint32 c)
{
  Xuint32 temp;
  // Get existing outputs
  temp = gpio_read();
  temp = temp & 0xF0;
  // Set the high nibble
  temp = temp | ((c >> 4) & 0x0F);
  gpio_write(temp);
  // Clock
  lcd_clk();
  // Delay for "Write data into internal RAM 43us"
  delay_us(100);
  // Set the low nibble
  temp = temp & 0xF0;
  temp = temp | (c & 0x0F);
  gpio_write(temp);
  // Clock
  lcd_clk();
  // Delay for "Write data into internal RAM 43us"
  delay_us(100);
}

// Clear LCD
void lcd_clear(void)
{
  lcd_reset_rs();
  // Clear LCD
  lcd_write(0x01);
  // Delay for "Clear display 1.53ms"
  delay_ms(2);
}

// Write a string to the LCD
void lcd_puts(const char * s)
{
  lcd_set_rs();
  while(*s)
    lcd_write(*s++);
}

// Write character to the LCD
void lcd_putch(Xuint32 c)
{
  lcd_set_rs();
  lcd_write(c);
}


// Change cursor position
// (line = 0 or 1, pos = 0 to 15)
void lcd_goto(Xuint32 line, Xuint32 pos)
{
  lcd_reset_rs();
  pos = pos & 0x3F;
  if(line == 0)
    lcd_write(0x80 | pos);
  else
    lcd_write(0xC0 | pos);
}
 
// Initialize the LCD
void lcd_init(void)
{
  Xuint32 temp;

  // Write mode (always)
  lcd_reset_rw();
  // Write control bytes
  lcd_reset_rs();

  // Delay 15ms
  delay_ms(15);

  // Initialize
  temp = gpio_read();
  temp = temp | LCD_DB5;
  gpio_write(temp);
  lcd_clk();
  lcd_clk();
  lcd_clk();

  // Delay 15ms
  delay_ms(15);

  // Function Set: 4 bit mode, 1/16 duty, 5x8 font, 2 lines
  lcd_write(0x28);
  // Display ON/OFF Control: ON
  lcd_write(0x0C);
  // Entry Mode Set: Increment (cursor moves forward)
  lcd_write(0x06);
 
  // Clear the display
  lcd_clear();
}
