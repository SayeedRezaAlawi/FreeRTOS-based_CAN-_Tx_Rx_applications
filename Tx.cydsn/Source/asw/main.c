/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "project.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"
#include "seven.h"
#include <stdio.h>
#include "ugui.h"
#include "ugui_st7735.h"
#include "CAN_packet.h"
#include "CAN_Tx.h"


uint8 CAN_Data = 0;

TaskHandle_t CANRxTASKHandler=NULL;
TaskHandle_t CANTxTASKHandler=NULL;
TaskHandle_t CANRemoteTxTASKHanler=NULL;
xSemaphoreHandle TxGiver=NULL;
CAN_packet packet;
uint16_t Lidar_data1[256];
uint16_t Lidar_data2[256];
uint16_t Lidar_data3[256];
uint8_t startEnd;
uint16_t indx=0;
uint8 bRTRreceive0;
CAN_Tx_TX_MSG Msg_Ptr;
CAN_Tx_DATA_BYTES_MSG Data;

#define DATALENGTH 0x08

void can_packet(uint32 id, uint8 dlc, uint8 ide, uint8 rtr, uint8 irq, 
    CAN_Tx_TX_MSG *Msg_Ptr, CAN_Tx_DATA_BYTES_MSG *Data);

void packet_fill(CAN_Tx_DATA_BYTES_MSG *Data, uint8 *Message,uint8 datalength);

void CAN_Rx_tsk(void* p)
{
    while(1)
    {
    if(xSemaphoreTake(TxGiver,portMAX_DELAY))
        {
        }
        
    }
}

void CAN_Tx_tsk(void* p)
{
    CAN_Tx_TX_MSG data_packet;
    CAN_Tx_DATA_BYTES_MSG sentdata;
    char data[20];
    uint8 j=1;
    uint8 MESSAGE[1];
    
    data_packet.id = 0x123; /*write the value of ID to the structure*/
    data_packet.rtr = 0; /*write the value of RTR to the structure*/
    data_packet.ide = 0; /*write the value of IDE to the structure*/
    data_packet.dlc = 2; /*write the value of DLC to the structure*/
    data_packet.irq = 1;
    data_packet.msg = &sentdata;
    
    while(1)
    {   
        packet_fill(&sentdata,&MESSAGE,0);
        CAN_Tx_SendMsg(&data_packet);
        vTaskDelay(1000);  
    }
}

int main(void)
{
    prvHardwareSetup();
    CyGlobalIntEnable; /* Enable global interrupts. */

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    CAN_Tx_Init(); 
    CAN_Tx_Start();
    UART_LOG_Start();
    
    TxGiver = xSemaphoreCreateBinary(); 
    
    //xTaskCreate(CAN_Rx_tsk,(signed portCHAR *) "CAN_Rx_tsk",configMINIMAL_STACK_SIZE,NULL,tskIDLE_PRIORITY+1,&CANRxTASKHandler);
    xTaskCreate(CAN_Tx_tsk,(signed portCHAR *) "CAN_Tx_tsk",configMINIMAL_STACK_SIZE,NULL,tskIDLE_PRIORITY+2,&CANTxTASKHandler);
    //xTaskCreate(CAN_Remote_Tx_tsk,(signed portCHAR *) "CAN_Remote_Tx_tsk",configMINIMAL_STACK_SIZE,NULL,tskIDLE_PRIORITY+2,&CANRemoteTxTASKHanler);
    vTaskStartScheduler();
    
}

/* ---------------------------------------------------------------------------
 * FreeRTOS support and configuration functions
 * --------------------------------------------------------------------------- */

void prvHardwareSetup( void )
{
	/* Port layer functions that need to be copied into the vector table. */
	extern void xPortPendSVHandler( void );
	extern void xPortSysTickHandler( void );
	extern void vPortSVCHandler( void );
	extern cyisraddress CyRamVectors[];

	/* Install the OS Interrupt Handlers. */
	CyRamVectors[ 11 ] = ( cyisraddress ) vPortSVCHandler;
	CyRamVectors[ 14 ] = ( cyisraddress ) xPortPendSVHandler;
	CyRamVectors[ 15 ] = ( cyisraddress ) xPortSysTickHandler;
}

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
	/* The stack space has been execeeded for a task, considering allocating more. */
	taskDISABLE_INTERRUPTS();
	for( ;; );
    
}
/*---------------------------------------------------------------------------*/

void vApplicationMallocFailedHook( void )
{
	/* The heap space has been execeeded. */
	taskDISABLE_INTERRUPTS();
	for( ;; );
}
/*---------------------------------------------------------------------------*/
CY_ISR(isr_Button_RisingEdge_Interrupt)
{
//   long task_woken=0;
//   UART_LOG_PutChar(5);
//   UART_LOG_PutString("\n");
//   xSemaphoreGiveFromISR(binary_sem,&task_woken);
//   if(task_woken){
//    portEND_SWITCHING_ISR( sem_task );
    //BaseType_t checkIfYieldRequired=NULL;
    //checkIfYieldRequired=xTaskResumeFromISR(myIntTaskHandler);
    //portYIELD_FROM_ISR(checkIfYieldRequired);
}
/* [] END OF FILE */

void can_packet(uint32 id, uint8 dlc, uint8 ide, uint8 rtr, uint8 irq, 
    CAN_Tx_TX_MSG *Msg_Ptr, CAN_Tx_DATA_BYTES_MSG *Data)
{
    Msg_Ptr->id=id;
    Msg_Ptr->dlc=dlc;
    Msg_Ptr->ide=ide;
    Msg_Ptr->rtr=irq;
    Msg_Ptr->msg=Data;
}

void packet_fill(CAN_Tx_DATA_BYTES_MSG *Data, uint8 *Message, uint8 datalength)
{
    char data[30];
    for(uint8 bIndex=0;bIndex<datalength;bIndex++)
    {
    Data->byte[bIndex] = Message[bIndex]; /* move data to the structure*/
    }
}