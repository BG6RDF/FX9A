#ifndef __BSP_USART_H__
#define __BSP_USART_H__

#include "bsp_board.h"

#define UART_TX_BUF_MAX_SIZE	64
typedef struct{
	unsigned char head;
	unsigned char rear;
	unsigned char size;
	unsigned char dat[UART_TX_BUF_MAX_SIZE];
}UartTxBufDef;

void usart_init();
bool DMA_IsTxComplete();
void CopyTBufToDMA();
void UART_DMAStart(uint8_t length);
void UartTx(uint8_t *wdat, uint8_t len);
unsigned char UART_TxBufSize();
#endif