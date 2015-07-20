/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_it.c
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include <board.h>
#include <rtthread.h>

/** @addtogroup Template_Project
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
    /* Go to infinite loop when Memory Manage exception occurs */
    while (1)
    {
    }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
    /* Go to infinite loop when Bus Fault exception occurs */
    while (1)
    {
    }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
    /* Go to infinite loop when Usage Fault exception occurs */
    while (1)
    {
    }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}


//------------------------------  UART1 -----------------------//
// RX
void USART1_IRQHandler(void)
{
	extern void UART1_RX_Isr(void);
    /* enter interrupt */
    rt_interrupt_enter();

   	UART1_RX_Isr();

    /* leave interrupt */
    rt_interrupt_leave();
}
// TX
void DMA1_Channel4_IRQHandler(void)
{
	extern void UART1_TX_DMA_Isr(void);
    /* enter interrupt */
    rt_interrupt_enter();

 	UART1_TX_DMA_Isr();

    /* leave interrupt */
    rt_interrupt_leave();
}


//------------------------------  UART3 -----------------------//
// RX
void USART3_IRQHandler(void)
{
	extern void UART3_RX_Isr(void);
    /* enter interrupt */
    rt_interrupt_enter();

   	UART3_RX_Isr();

    /* leave interrupt */
    rt_interrupt_leave();
}
// TX
void DMA1_Channel2_IRQHandler(void)
{
	extern void UART3_TX_DMA_Isr(void);
    /* enter interrupt */
    rt_interrupt_enter();

 	UART3_TX_DMA_Isr();

    /* leave interrupt */
    rt_interrupt_leave();
}

//------------------------------  UART4 -----------------------//
// RX
void UART4_IRQHandler(void)
{
	extern void UART4_RX_Isr(void);
    /* enter interrupt */
    rt_interrupt_enter();

   	UART4_RX_Isr();

    /* leave interrupt */
    rt_interrupt_leave();
}
// TX
void DMA2_Channel4_5_IRQHandler(void)
{
	extern void UART4_TX_DMA_Isr(void);
    /* enter interrupt */
    rt_interrupt_enter();

 	UART4_TX_DMA_Isr();

    /* leave interrupt */
    rt_interrupt_leave();
}


//------------------------------  UART5 -----------------------//
// RX
void UART5_IRQHandler(void)
{
	extern void UART5_RX_Isr(void);
    /* enter interrupt */
    rt_interrupt_enter();

   	UART5_RX_Isr();

    /* leave interrupt */
    rt_interrupt_leave();
}



//void SysTick_Handler(void)
//{
//    // definition in boarc.c
//}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

#ifdef  RT_USING_LWIP
/*******************************************************************************
* Function Name  : EXTI4_IRQHandler
* Description    : This function handles External lines 9 to 5 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTI4_IRQHandler(void)
{
    extern void rt_dm9000_isr(void);

    /* enter interrupt */
    rt_interrupt_enter();

    /* Clear the DM9000A EXTI line pending bit */
    EXTI_ClearITPendingBit(EXTI_Line4);

    rt_dm9000_isr();

    /* leave interrupt */
    rt_interrupt_leave();
}
#endif /* RT_USING_LWIP */

/**
  * @}
  */


/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
