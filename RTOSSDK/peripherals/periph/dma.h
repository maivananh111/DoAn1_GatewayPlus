/*
 * dma.h
 *
 *  Created on: Jan 13, 2023
 *      Author: anh
 */

#ifndef PERIPH_DMA_H_
#define PERIPH_DMA_H_


#include "peripheral_config.h"

#if ENABLE_DMA

#ifdef __cplusplus
extern "C"{
#endif

#include "sdkconfig.h"
#include st_header
#include "system/ret_err.h"



typedef enum{
	DMA_Channel0 = (0U),
	DMA_Channel1,
	DMA_Channel2,
	DMA_Channel3,
	DMA_Channel4,
	DMA_Channel5,
	DMA_Channel6,
	DMA_Channel7,
}dma_channel_t;

typedef enum{
	DMA_PERIH_TO_MEM = (0U),
	DMA_MEM_TO_PERIPH,
	DMA_MEM_TO_MEM,
}dma_direction_t;

typedef enum{
	DMA_MODE_NORMAL = (0U),
	DMA_MODE_CIRCULAR,
} dma_mode_t;

typedef enum{
	DMA_DATASIZE_8BIT,
	DMA_DATASIZE_16BIT,
	DMA_DATASIZE_32BIT,
} dma_datasize_t;

typedef enum{
	DMA_NOFIFO = (0U),
	DMA_FIF0 = DMA_SxFCR_DMDIS,
}dma_fifo_t;

typedef enum{
	DMA_SINGLE_TRANFER = (0U),
	DMA_BURST_4INCREMENTAL,
	DMA_BURST_8INCREMENTAL,
	DMA_BURST_16INCREMENTAL,
} dma_burst_t;

typedef enum{
	DMA_CHANNEL_PRIORITY_LOW = (0U),
	DMA_CHANNEL_PRIORITY_MEDIUM,
	DMA_CHANNEL_PRIORITY_HIGH,
	DMA_CHANNEL_PRIORITY_VERYHIGH,
} dma_channelpriority_t;

typedef enum{
	DMA_TRANSFER_COMPLETE_INTERRUPT = DMA_SxCR_TCIE,
	DMA_HALF_TRANSFER_INTERRUPT = DMA_SxCR_HTIE,
	DMA_TRANSFER_ERROR_INTERRUPT = DMA_SxCR_TEIE,
	DMA_DIRECTMODE_ERROR_INTERRUPT = DMA_SxCR_DMEIE,
} dma_interruptselect_t;

typedef enum{
	DMA_EVENT_NOEVENT,
	DMA_EVENT_TRANFER_COMPLETE,
	DMA_EVENT_HALF_TRANFER,
	DMA_EVENT_TRANFER_ERROR,
} dma_event_t;

typedef struct {
	DMA_Stream_TypeDef 	  *stream;
	dma_channel_t 		  channel;
	dma_direction_t 	  direction = DMA_MEM_TO_PERIPH;
	dma_mode_t 	 		  mode = DMA_MODE_NORMAL;
	dma_datasize_t 		  datasize = DMA_DATASIZE_8BIT;
	dma_fifo_t 		      fifo = DMA_NOFIFO;
	dma_burst_t 		  burst = DMA_BURST_4INCREMENTAL;
	uint32_t 			  interruptselect = DMA_TRANSFER_COMPLETE_INTERRUPT | DMA_HALF_TRANSFER_INTERRUPT | DMA_TRANSFER_ERROR_INTERRUPT;
	dma_channelpriority_t channelpriority = DMA_CHANNEL_PRIORITY_HIGH;
	uint32_t 			  interruptpriority = 0;
} dma_config_t;


class dma {
	public:
	dma(DMA_TypeDef *dma);
		stm_ret_t init(dma_config_t *conf);

		void register_event_handler(void (*Event_Callback)(void *Parameter, dma_event_t event), void *Parameter);

		stm_ret_t start(uint32_t Src_Address, uint32_t Dest_Address, uint32_t Number_Data);
		stm_ret_t stop(void);

		uint16_t get_counter(void);

		stm_ret_t poll_for_tranfer(dma_interruptselect_t PollLevel, uint32_t TimeOut);

		dma_config_t *get_config(void);

		void (*Event_Callback)(void *Parameter, dma_event_t event);
		void *Parameter;

	private:
		void ClearIFCR(__IO uint32_t Value);
		void ClearAllIntrFlag(void);
		__IO uint32_t GetISR(void);

		DMA_TypeDef *_dma;
		dma_config_t *_conf;
		stm_err_t _state = STM_READY;
		IRQn_Type _IRQn = DMA1_Stream0_IRQn;
		__IO uint32_t _Intr_Index = 0U;
		uint32_t Stream = 0U;
		uint8_t DMA_Reg_Level = 0U; // 0 is LOW register.
		volatile uint32_t *ICFR = 0x00000000U;
		volatile uint32_t *ISR  = 0x00000000U;
};

typedef dma* dma_t;



void DMA_IRQ_Handler(DMA_TypeDef *pdma, DMA_Stream_TypeDef *stream, dma *dmaptr);

/**
 *  DMA1 Stream class.
 */
/* DMA1 IRQ HANDLER */
#if defined(DMA1_Stream0)
extern dma_t dma1_stream0;
#endif /* defined(DMA1_Stream0) */
#if defined(DMA1_Stream1)
extern dma_t dma1_stream1;
#endif /* defined(DMA1_Stream1) */
#if defined(DMA1_Stream2)
extern dma_t dma1_stream2;
#endif /* defined(DMA1_Stream2) */
#if defined(DMA1_Stream3)
extern dma_t dma1_stream3;
#endif /* defined(DMA1_Stream3) */
#if defined(DMA1_Stream4)
extern dma_t dma1_stream4;
#endif /* defined(DMA1_Stream4) */
#if defined(DMA1_Stream5)
extern dma_t dma1_stream5;
#endif /* defined(DMA1_Stream5) */
#if defined(DMA1_Stream6)
extern dma_t dma1_stream6;
#endif /* defined(DMA1_Stream6) */
#if defined(DMA1_Stream7)
extern dma_t dma1_stream7;
#endif /* defined(DMA1_Stream7) */

/**
 *  DMA2 Stream class.
 */
#if defined(DMA2_Stream0)
extern dma_t dma2_stream0;
#endif /* defined(DMA2_Stream0) */
#if defined(DMA2_Stream1)
extern dma_t dma2_stream1;
#endif /* defined(DMA2_Stream1) */
#if defined(DMA2_Stream2)
extern dma_t dma2_stream2;
#endif /* defined(DMA2_Stream2) */
#if defined(DMA2_Stream3)
extern dma_t dma2_stream3;
#endif /* defined(DMA2_Stream3) */
#if defined(DMA2_Stream4)
extern dma_t dma2_stream4;
#endif /* defined(DMA2_Stream4) */
#if defined(DMA2_Stream5)
extern dma_t dma2_stream5;
#endif /* defined(DMA2_Stream5) */
#if defined(DMA2_Stream6)
extern dma_t dma2_stream6;
#endif /* defined(DMA2_Stream6) */
#if defined(DMA2_Stream7)
extern dma_t dma2_stream7;
#endif /* defined(DMA2_Stream7) */

#ifdef __cplusplus
}
#endif

#endif /* ENABLE_DMA */

#endif /* PERIPH_DMA_H_ */
