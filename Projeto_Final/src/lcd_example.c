#include <stdint.h>
#include "TM4C1294NCPDT.h"

typedef enum Estados
{
	Begin,
	Girando,
	Fim
}Estados;

void GPIO_Init_Motor (void);
void GPIO_Init (void);
void PLL_Init(void);
void SysTick_Init(void);
void SysTick_Wait1ms(uint32_t delay);
uint32_t PortJ_Input(void);
void GPIOPortJ_Handler(void);
void init_LCD(void);
void escreve_texto_LCD(char* texto);
void escreve_caracter_LCD(uint32_t c);
void clear_LCD(void);
void seleciona_primeira_linha_LCD(void);
void seleciona_segunda_linha_LCD(void);		
char le_teclado(void);
void set_estado(Estados estado);
void gira_motor_passo_completo(char voltas, char sentido);
void gira_motor_meio_passo(char voltas, char sentido);
void apaga_LEDS(void);

int main (void)
{
		PLL_Init();
		SysTick_Init();
		GPIO_Init();
		GPIO_Init_Motor();
		init_LCD();
		seleciona_primeira_linha_LCD();
		escreve_texto_LCD("Aperte *");
		seleciona_segunda_linha_LCD();
		escreve_texto_LCD("para comecar!");
		set_estado(Begin);
}

void set_estado(Estados estado)
{
	char velocidade;
	char n_voltas;
	char sentido;
	
	switch(estado)
	{
		case Begin:
		{
			char user_input = '0';
			
			clear_LCD();
			apaga_LEDS();
			seleciona_primeira_linha_LCD();
			escreve_texto_LCD("Aperte *");
			seleciona_segunda_linha_LCD();
			escreve_texto_LCD("para comecar!");
			
			while(user_input != '5')
			{
				user_input = le_teclado();
			}
			
			clear_LCD();		
			seleciona_primeira_linha_LCD();
			escreve_texto_LCD("Digite n de");
			seleciona_segunda_linha_LCD();
			escreve_texto_LCD("voltas:");
			n_voltas = '0';
			
			while (n_voltas == '0')
			{					
				n_voltas = le_teclado();			
			}
			
			escreve_caracter_LCD(n_voltas);
			
			clear_LCD();
			seleciona_primeira_linha_LCD();
			escreve_texto_LCD("H (1) ou AH (2)?");
			
			sentido = '0';
			
			while (sentido == '0')
			{
				sentido = le_teclado();
			}
			
			seleciona_primeira_linha_LCD();
			escreve_texto_LCD("PC (1) ou MP (2)?");
			
			velocidade = '0';
			
			while (velocidade == '0')
			{
				velocidade = le_teclado();
			}
			
			set_estado(Girando);
			break;
		}
		case Girando:
		{
			if (velocidade == '1')
			{
				gira_motor_passo_completo(n_voltas,sentido);
				set_estado(Fim);
			}
			else
			{
				gira_motor_meio_passo(n_voltas,sentido);
				set_estado(Fim);
			}
			break;
		}
		case Fim:
		{
			clear_LCD();
			seleciona_primeira_linha_LCD();
			escreve_texto_LCD("       FIM      ");
			SysTick_Wait1ms(2000);
			set_estado(Begin);
			break;
		}	
	}
	
}

void GPIOPortJ_Handler(void)
{
		int temp;	
		temp = 0x01;
		temp = temp | GPIO_PORTJ_AHB_ICR_R;
		GPIO_PORTJ_AHB_ICR_R = temp;	
		set_estado(Fim);
}


