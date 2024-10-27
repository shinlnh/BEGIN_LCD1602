#include <LCD_4Bit.h>
#include<stm32f4xx.h>
#define RS_PIN GPIO_PIN_1
#define RW_PIN GPIO_PIN_10
#define EN_PIN GPIO_PIN_3
#define D4_PIN GPIO_PIN_4
#define D5_PIN GPIO_PIN_5
#define D6_PIN GPIO_PIN_6
#define D7_PIN GPIO_PIN_7

#define RS_PORT GPIOB
#define RW_PORT GPIOB
#define EN_PORT GPIOB
#define D4_PORT GPIOB
#define D5_PORT GPIOB
#define D6_PORT GPIOB
#define D7_PORT GPIOB

#define timer htim1
//extern TIM_HandleTypeDef timer;
//void delay (uint16_t us)
//{
//	__HAL_TIM_SET_COUNTER(&timer, 0);
//	while (__HAL_TIM_GET_COUNTER(&timer) < us);
//}
static uint32_t DWT_Delay_Init(void){
	// disable TRC
	 CoreDebug->DEMCR &= ~CoreDebug_DEMCR_TRCENA_Msk;
	// enable TRC
	 CoreDebug->DEMCR |=  CoreDebug_DEMCR_TRCENA_Msk;
	// disable clock cycle counter
	 DWT->CTRL &= ~DWT_CTRL_CYCCNTENA_Msk;
	// enable clock cycle counter
	 DWT->CTRL |=  DWT_CTRL_CYCCNTENA_Msk;
	 /* Reset the clock cycle counter value */
	 DWT->CYCCNT = 0;
	 /* 3 NO OPERATION instructions */
	 __ASM volatile ("NOP");
	 __ASM volatile ("NOP");
	 __ASM volatile ("NOP");
     /* Check if clock cycle counter has started */
	 if(DWT->CYCCNT){
	    return 0; /*clock cycle counter started*/
	 }
	 else{
	    return 1; /*clock cycle counter not started*/
	 }
}
//inline function
__STATIC_INLINE void delay(volatile uint32_t microseconds)
{
  uint32_t clk_cycle_start = DWT->CYCCNT;

  /* Go to number of cycles for system */
  microseconds *= (HAL_RCC_GetHCLKFreq() / 1000000);

  /* Delay till end */
  while ((DWT->CYCCNT - clk_cycle_start) < microseconds);
}

void send_to_lcd(char data,int rs){
	DWT_Delay_Init();
	//xác định chế độ ghi lệnh hay ghi dữ liệu
	HAL_GPIO_WritePin(RS_PORT, RS_PIN, rs);
	//ghi 4 bit cao
	HAL_GPIO_WritePin(D7_PORT, D7_PIN, (data>>3)&0x01);
	HAL_GPIO_WritePin(D6_PORT, D6_PIN, (data>>2)&0x01);
	HAL_GPIO_WritePin(D5_PORT, D5_PIN, (data>>1)&0x01);
	HAL_GPIO_WritePin(D4_PORT, D4_PIN, (data>>0)&0x01);
	//xung báo cho lcd thực hiện lệnh
	HAL_GPIO_WritePin(EN_PORT, EN_PIN, 1);
	delay(20);
	HAL_GPIO_WritePin(EN_PORT, EN_PIN, 0);
	delay(20);
}
void lcd_send_cmd(char cmd){
	char data;
	data = (cmd>>4)&0x0f;
	send_to_lcd(data,0);
	data = cmd&0x0f;
	send_to_lcd(data,0);
}
void lcd_send_data(char data){
	char data_send;
	data_send = (data>>4)&0x0f;
	send_to_lcd(data_send,1);
	data_send = data&0x0f;
	send_to_lcd(data_send,1);
}
void lcd_clear(void){
	lcd_send_cmd(0x01);
	HAL_Delay(2);
}
void lcd_put_cur(int row,int col){
	switch(row){
	case 0:
		col |= 0x80;
		break;
	case 1:
		col |= 0xC0;
		break;
	}
	lcd_send_cmd(col);
}
void lcd_init(void){
	HAL_Delay(100);
	lcd_send_cmd(0x30);
	HAL_Delay(5);
	lcd_send_cmd(0x30);
	HAL_Delay(1);
	lcd_send_cmd(0x30);
	HAL_Delay(1);
	lcd_send_cmd(0x20);
	HAL_Delay(1);
	lcd_send_cmd(0x28);// Function set --> DL=0 (4 bit mode), N = 1 (2 line display) F = 0 (5x8 characters)
	HAL_Delay(1);
	lcd_send_cmd(0x08);//Display on/off control --> D=0,C=0, B=0  ---> display off
	HAL_Delay(1);
	lcd_send_cmd(0x01); // clear display
	HAL_Delay(3);
	lcd_send_cmd(0x06);//Entry mode set --> I/D = 1 (increment cursor) & S = 0 (no shift)
	HAL_Delay(1);
	lcd_send_cmd (0x0C);//Display on/off control --> D = 1, C and B = 0. (Cursor and blink, last two bits)
}
void lcd_send_string (char *str)
{
	while (*str) lcd_send_data (*str++);
}
