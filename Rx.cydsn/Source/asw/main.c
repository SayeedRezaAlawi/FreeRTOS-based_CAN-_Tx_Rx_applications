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
#include "portmacro.h"
#include "seven.h"
#include <stdio.h>
#include "ugui.h"
#include "ugui_st7735.h"
#include "queue.h"
#include "CAN_Rx.h"

uint8 CAN_Data = 0;
TaskHandle_t CANRxTASKHandler=NULL;
TaskHandle_t DataPrintTASKHandler=NULL;
TaskHandle_t DISPTASKHandler=NULL;
TaskHandle_t CANRemoteTxTASKHanler=NULL;
xSemaphoreHandle RxGiver=NULL; 
SemaphoreHandle_t xMutex;
uint16_t Lidar_data1[256];
uint16_t Lidar_data2[256];
boolean_t startpacket=FALSE;
boolean_t endpacket=FALSE;
uint16_t indx=0;
QueueHandle_t xQueue = NULL;
uint8_t buffernumber=0;
uint16_t Receivedata1[1];
uint16_t Receivedata2[1];
uint8_t LEDNumber;

CAN_Rx_TX_MSG Msg_Ptr;
CAN_Rx_DATA_BYTES_MSG Data;

#define DATALENGTH 0x08

void can_packet(uint32 id, uint8 dlc, uint8 ide, uint8 rtr, uint8 irq, 
    CAN_Rx_TX_MSG *Msg_Ptr, CAN_Rx_DATA_BYTES_MSG *Data);

void packet_fill(CAN_Rx_DATA_BYTES_MSG *Data, uint8 *Message,uint8 datalength);


void CAN_Rx_tsk(void* p)
{
    char data[30];
    uint16_t *pc1 = Lidar_data1;
    uint16_t *pc2 = Lidar_data2;

    while(1)
    {
        if(xSemaphoreTake(RxGiver,portMAX_DELAY))
        {
            switch(LEDNumber){
              case 1: RED_LED_Write(1);GREEN_LED_Write(0);YELLOW_LED_Write(0); break; 
              case 2: RED_LED_Write(0);GREEN_LED_Write(1);YELLOW_LED_Write(0); break;  
              case 3: RED_LED_Write(0);GREEN_LED_Write(0);YELLOW_LED_Write(1);  break; 
            }
        }
    }
}

void data_print(void* p)
{
    char data[20];
    uint16_t* dataset;
    uint8_t data_ll=0;
        uint8_t data_hh=0;
    
    while(1)
    {
        if(xSemaphoreTake(xMutex, portMAX_DELAY)){
                if(xQueueReceive(xQueue,&dataset,100) == pdTRUE){
                    UG_FillScreen(C_KHAKI);
                    UG_FontSelect(&FONT_10X16);
                    UG_SetBackcolor(C_KHAKI);
                    UG_SetForecolor(C_RED);
                    UG_PutString(1, 5 , "Recvd Data: ");
                    for(uint16_t i=0; i < 256; i++){
                        itoa(*dataset , data, 10);
                        UG_FontSelect(&FONT_24X40);
                        UG_SetBackcolor(C_KHAKI);
                        UG_SetForecolor(C_MEDIUM_SLATE_BLUE);
                        UG_PutString(15, 40,data);  
                        dataset++;
                        //vTaskDelay(50);
                        }
                }
        xSemaphoreGive(xMutex);
        taskYIELD();
        }
    }
}

void DISP_tsk(void * p)
{
    while(1)
    {
    if(xSemaphoreTake(xMutex, portMAX_DELAY)){
        UG_FontSelect(&FONT_10X16);
        UG_SetBackcolor(C_KHAKI);
        UG_SetForecolor(C_RED);
        UG_PutString(1, 5 , "Recvd Data: ");
        //UG_PutString(1, 80 , "Sent Data: ");
        xSemaphoreGive(xMutex);
        taskYIELD();
        }
    }
}

void CAN_Remote_Tx_tsk(void* p)
{    
    while(1)
    {                        
        CAN_Rx_SendMsg0(); /*send RTR message*/ 
    }
}



int main(void)
{
    prvHardwareSetup();
    CyGlobalIntEnable; /* Enable global interrupts. */
    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    CAN_Rx_Start();
    UART_LOG_Start();
    xMutex = xSemaphoreCreateMutex();
    RxGiver = xSemaphoreCreateBinary();
    xQueue = xQueueCreate( 1, sizeof(uint16_t* ) );    
    
    Display_Init();
    
    /* Clear screen. */
    UG_FillScreen(C_KHAKI);

    xTaskCreate(CAN_Rx_tsk,(signed portCHAR *) "CAN_Rx_tsk",configMINIMAL_STACK_SIZE,NULL,tskIDLE_PRIORITY+1,&CANRxTASKHandler);
    //xTaskCreate(data_print,(signed portCHAR *) "data_print",configMINIMAL_STACK_SIZE,NULL,tskIDLE_PRIORITY+1,&DataPrintTASKHandler);
    //xTaskCreate(DISP_tsk,(signed portCHAR *) "DISP_tsk",configMINIMAL_STACK_SIZE,NULL,tskIDLE_PRIORITY+1,&DISPTASKHandler);
    xTaskCreate(CAN_Remote_Tx_tsk,(signed portCHAR *) "CAN_Remote_Tx_tsk",configMINIMAL_STACK_SIZE,NULL,tskIDLE_PRIORITY+2,&CANRemoteTxTASKHanler);
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
    CAN_Rx_TX_MSG *Msg_Ptr, CAN_Rx_DATA_BYTES_MSG *Data)
{
    Msg_Ptr->id=id;
    Msg_Ptr->dlc=dlc;
    Msg_Ptr->ide=ide;
    Msg_Ptr->rtr=irq;
    Msg_Ptr->msg=Data;
}

void packet_fill(CAN_Rx_DATA_BYTES_MSG *Data, uint8 *Message, uint8 datalength)
{
    char data[30];
    for(uint8 bIndex=0;bIndex<datalength;bIndex++)
    {
    Data->byte[bIndex] = Message[bIndex]; /* move data to the structure*/
    }
}