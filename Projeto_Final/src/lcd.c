#include <stdint.h>

void PortK_Output(uint32_t v);
void PortM_Output_LCD(uint32_t v);
void SysTick_Init(void);
void SysTick_Wait1ms(uint32_t delay);
void SysTick_Wait1us(uint32_t delay);

void init_LCD()
{
	PortK_Output(0x38);
	PortM_Output_LCD(0x04);
	SysTick_Wait1us(10);
	PortM_Output_LCD(0x00);
	SysTick_Wait1us(40);
	
	PortK_Output(0x38);
	PortM_Output_LCD(0x04);
	SysTick_Wait1us(10);
	PortM_Output_LCD(0x00);
	SysTick_Wait1us(40);
	
	PortK_Output(0x06);
	PortM_Output_LCD(0x04);
	SysTick_Wait1us(10);
	PortM_Output_LCD(0x00);
	SysTick_Wait1us(40);

	PortK_Output(0x0E);
	PortM_Output_LCD(0x04);
	SysTick_Wait1us(10);
	PortM_Output_LCD(0x00);
	SysTick_Wait1us(40);
	
	PortK_Output(0x01);
	PortM_Output_LCD(0x04);
	SysTick_Wait1us(10);
	PortM_Output_LCD(0x00);
	SysTick_Wait1us(40);	
}

void escreve_caracter_LCD(uint32_t c)
{
	PortK_Output(c);
	PortM_Output_LCD(0x05);
	SysTick_Wait1ms(10);
	PortM_Output_LCD(0x00);
	SysTick_Wait1ms(10);
}


void escreve_texto_LCD (char* texto)
{
	while (*texto != '\0')
	{
		escreve_caracter_LCD(*texto);
		texto++;
	}
}

void seleciona_primeira_linha_LCD(void)
{
	PortK_Output(0x80);
	PortM_Output_LCD(0x04);
	SysTick_Wait1us(10);
	PortM_Output_LCD(0x00);
	SysTick_Wait1us(40);
}

void clear_LCD(void)
{
	PortK_Output(0x01);
	PortM_Output_LCD(0x04);
	SysTick_Wait1ms(10);
	PortM_Output_LCD(0x00);
	SysTick_Wait1us(40);
}

void seleciona_segunda_linha_LCD(void)
{
	PortK_Output(0xC0);
	PortM_Output_LCD(0x04);
	SysTick_Wait1us(10);
	PortM_Output_LCD(0x00);
	SysTick_Wait1us(40);
}

