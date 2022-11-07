/*******************************************************************************
* File Name: CAN_Rx_TX_RX_func.c
* Version 3.0
*
* Description:
*  There are functions process "Full" Receive and Transmit mailboxes:
*     - CAN_Rx_SendMsg0-7();
*     - CAN_Rx_ReceiveMsg0-15();
*  Transmission of message, and receive routine for "Basic" mailboxes:
*     - CAN_Rx_SendMsg();
*     - CAN_Rx_TxCancel();
*     - CAN_Rx_ReceiveMsg();
*
*  Note:
*   None
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "CAN_Rx.h"
#include "seven.h"
#include "UART_LOG.h"
#include "FreeRTOS.h"
#include "semphr.h"

/* `#START TX_RX_FUNCTION` */

extern uint16_t Lidar_data1[256];
extern uint16_t Lidar_data2[256];
extern uint16_t indx;
extern uint8_t buffernumber;
extern boolean_t startpacket;
extern boolean_t endpacket;
extern uint16_t Receivedata1[1];
extern uint16_t Receivedata2[1];
extern uint8_t LEDNumber;

/* `#END` */



/*******************************************************************************
* FUNCTION NAME:   CAN_Rx_SendMsg
********************************************************************************
*
* Summary:
*  This function is Send Message from one of Basic mailboxes. The function loops
*  through the transmit message buffer designed as Basic CAN mailboxes for the
*  first free available and sends from it. The number of retries is limited.
*
* Parameters:
*  message: The pointer to a structure that contains all required data to send
*           messages.
*
* Return:
*  Indication if message has been sent.
*   Define                             Description
*    CYRET_SUCCESS                      The function passed successfully
*    CAN_Rx_FAIL              The function failed
*
*******************************************************************************/
uint8 CAN_Rx_SendMsg(const CAN_Rx_TX_MSG *message) 
{
    uint8 i, j, shift;
    uint8 retry = 0u;
    uint8 result = CAN_Rx_FAIL;
    uint32 regTemp;

    while (retry < CAN_Rx_RETRY_NUMBER)
    {
        shift = 1u;    /* Start from first mailbox */
        for (i = 0u; i < CAN_Rx_NUMBER_OF_TX_MAILBOXES; i++)
        {
            /* Find Basic TX mailboxes */
            if ((CAN_Rx_TX_MAILBOX_TYPE & shift) == 0u)
            {
                /* Find free mailbox */
                #if (CY_PSOC3 || CY_PSOC5)
                    if ((CAN_Rx_BUF_SR_REG.byte[2] & shift) == 0u)
                #else  /* CY_PSOC4 */
                    if ((CAN_Rx_BUF_SR_REG &
                        (uint32) ((uint32) shift << CAN_Rx_TWO_BYTE_OFFSET)) == 0u)
                #endif /* CY_PSOC3 || CY_PSOC5 */
                    {
                        regTemp = 0u;

                        /* Set message parameters */
                        if (message->rtr != CAN_Rx_STANDARD_MESSAGE)
                        {
                            regTemp = CAN_Rx_TX_RTR_MASK;    /* RTR message Enable */
                        }

                        if (message->ide == CAN_Rx_STANDARD_MESSAGE)
                        {
                            CAN_Rx_SET_TX_ID_STANDARD_MSG(i, message->id);
                        }
                        else
                        {
                            regTemp |= CAN_Rx_TX_IDE_MASK;
                            CAN_Rx_SET_TX_ID_EXTENDED_MSG(i, message->id);
                        }

                        if (message->dlc < CAN_Rx_TX_DLC_MAX_VALUE)
                        {
                            regTemp |= ((uint32) message->dlc) << CAN_Rx_TWO_BYTE_OFFSET;
                        }
                        else
                        {
                            regTemp |= CAN_Rx_TX_DLC_UPPER_VALUE;
                        }

                        if (message->irq != CAN_Rx_TRANSMIT_INT_DISABLE)
                        {
                            regTemp |= CAN_Rx_TX_INT_ENABLE_MASK;    /* Transmit Interrupt Enable */
                        }

                        for (j = 0u; (j < message->dlc) && (j < CAN_Rx_TX_DLC_MAX_VALUE); j++)
                        {
                            #if (CY_PSOC3 || CY_PSOC5)
                                CAN_Rx_TX_DATA_BYTE(i, j) = message->msg->byte[j];
                            #else /* CY_PSOC4 */
                                CAN_Rx_TX_DATA_BYTE(i, j, message->msg->byte[j]);
                            #endif /* CY_PSOC3 || CY_PSOC5 */
                        }

                        /* Disable isr */
    CyIntDisable(CAN_Rx_ISR_NUMBER);

                        /* WPN[23] and WPN[3] set to 1 for write to CAN Control reg */
                        CY_SET_REG32(CAN_Rx_TX_CMD_PTR(i), (regTemp | CAN_Rx_TX_WPN_SET));

                        #if (CY_PSOC3 || CY_PSOC5)
                            CY_SET_REG32(CAN_Rx_TX_CMD_PTR(i), CAN_Rx_SEND_MESSAGE);
                        #else /* CY_PSOC4 */
                            if (message->sst != CAN_Rx_STANDARD_MESSAGE)
                            {
                                /* Single Shot Transmission */
                                CAN_Rx_TX_CMD_REG(i) |= CAN_Rx_SEND_MESSAGE |
                                CAN_Rx_TX_ABORT_MASK;
                            }
                            else
                            {
                                CAN_Rx_TX_CMD_REG(i) |= CAN_Rx_SEND_MESSAGE;
                            }
                        #endif /* CY_PSOC3 || CY_PSOC5 */

                        /* Enable isr */
    CyIntEnable(CAN_Rx_ISR_NUMBER);

                        result = CYRET_SUCCESS;
                    }
            }
            shift <<= 1u;
            if (result == CYRET_SUCCESS)
            {
                break;
            }
        }
        if (result == CYRET_SUCCESS)
        {
            break;
        }
        else
        {
            retry++;
        }
    }

    return (result);
}


/*******************************************************************************
* FUNCTION NAME:   CAN_Rx_TxCancel
********************************************************************************
*
* Summary:
*  This function cancels transmission of a message that has been queued to be
*  transmitted. Values between 0 and 7 are valid.
*
* Parameters:
*  bufferId: The mailbox number.
*
* Return:
*  None.
*
*******************************************************************************/
void CAN_Rx_TxCancel(uint8 bufferId) 
{
    if (bufferId < CAN_Rx_NUMBER_OF_TX_MAILBOXES)
    {
        CAN_Rx_TX_ABORT_MESSAGE(bufferId);
    }
}


#if (CAN_Rx_TX0_FUNC_ENABLE)
    /*******************************************************************************
    * FUNCTION NAME:   CAN_Rx_SendMsg0
    ********************************************************************************
    *
    * Summary:
    *  This function is the entry point to Transmit Message 0. The function checks
    *  if mailbox 0 doesn't already have un-transmitted messages waiting for
    *  arbitration. If not initiate transmission of the message.
    *  Generated only for the Transmit mailbox designed as Full.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  Indication if Message has been sent.
    *   Define                             Description
    *    CYRET_SUCCESS                      The function passed successfully
    *    CAN_Rx_FAIL              The function failed
    *
    *******************************************************************************/
    uint8 CAN_Rx_SendMsg0(void) 
    {
        uint8 result = CYRET_SUCCESS;

        #if (CY_PSOC3 || CY_PSOC5)
            if ((CAN_Rx_TX[0u].txcmd.byte[0u] & CAN_Rx_TX_REQUEST_PENDING) != 0u)
        #else  /* CY_PSOC4 */
            if ((CAN_Rx_TX_CMD_REG(0u) & CAN_Rx_TX_REQUEST_PENDING) != 0u)
        #endif /* CY_PSOC3 || CY_PSOC5 */
            {
                result = CAN_Rx_FAIL;
            }
            else
            {
                /* `#START MESSAGE_0_TRASMITTED` */

                /* `#END` */

                #ifdef CAN_Rx_SEND_MSG_0_CALLBACK
                    CAN_Rx_SendMsg_0_Callback();
                #endif /* CAN_Rx_SEND_MSG_0_CALLBACK */

                CY_SET_REG32(CAN_Rx_TX_CMD_PTR(0u),
                CY_GET_REG32(CAN_Rx_TX_CMD_PTR(0u)) | CAN_Rx_SEND_MESSAGE);
            }

        return (result);
    }
#endif /* CAN_Rx_TX0_FUNC_ENABLE */


#if (CAN_Rx_TX1_FUNC_ENABLE)
    /*******************************************************************************
    * FUNCTION NAME:   CAN_Rx_SendMsgTx_message
    ********************************************************************************
    *
    * Summary:
    *  This function is the entry point to Transmit Message 1. The function checks
    *  if mailbox 1 doesn't already have un-transmitted messages waiting for
    *  arbitration. If not initiate transmission of the message.
    *  Generated only for the Transmit mailbox designed as Full.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  Indication if Message has been sent.
    *   Define                             Description
    *    CYRET_SUCCESS                      The function passed successfully
    *    CAN_Rx_FAIL              The function failed
    *
    *******************************************************************************/
    uint8 CAN_Rx_SendMsgTx_message(void) 
    {
        uint8 result = CYRET_SUCCESS;

        #if (CY_PSOC3 || CY_PSOC5)
            if ((CAN_Rx_TX[1u].txcmd.byte[0u] & CAN_Rx_TX_REQUEST_PENDING) != 0u)
        #else  /* CY_PSOC4 */
            if ((CAN_Rx_TX_CMD_REG(1u) & CAN_Rx_TX_REQUEST_PENDING) != 0u)
        #endif /* CY_PSOC3 || CY_PSOC5 */
            {
                result = CAN_Rx_FAIL;
            }
            else
            {
                /* `#START MESSAGE_Tx_message_TRASMITTED` */
                UART_LOG_PutString("CAN_Rx_SendMsgTx_message\n");
                //CAN_Rx_TX_DATA_BYTE1(1) = CAN_Data;

                /* `#END` */

                #ifdef CAN_Rx_SEND_MSG_Tx_message_CALLBACK
                    CAN_Rx_SendMsg_Tx_message_Callback();
                #endif /* CAN_Rx_SEND_MSG_Tx_message_CALLBACK */

                CY_SET_REG32(CAN_Rx_TX_CMD_PTR(1u),
                CY_GET_REG32(CAN_Rx_TX_CMD_PTR(1u)) | CAN_Rx_SEND_MESSAGE);
            }

        return (result);
    }
#endif /* CAN_Rx_TX1_FUNC_ENABLE */


#if (CAN_Rx_TX2_FUNC_ENABLE)
    /*******************************************************************************
    * FUNCTION NAME:   CAN_Rx_SendMsg2
    ********************************************************************************
    *
    * Summary:
    *  This function is the entry point to Transmit Message 2. The function checks
    *  if mailbox 2 doesn't already have un-transmitted messages waiting for
    *  arbitration. If not initiate transmission of the message.
    *  Generated only for the Transmit mailbox designed as Full.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  Indication if Message has been sent.
    *   Define                             Description
    *    CYRET_SUCCESS                      The function passed successfully
    *    CAN_Rx_FAIL              The function failed
    *
    *******************************************************************************/
    uint8 CAN_Rx_SendMsg2(void) 
    {
        uint8 result = CYRET_SUCCESS;

        #if (CY_PSOC3 || CY_PSOC5)
            if ((CAN_Rx_TX[2u].txcmd.byte[0u] & CAN_Rx_TX_REQUEST_PENDING) != 0u)
        #else  /* CY_PSOC4 */
            if ((CAN_Rx_TX_CMD_REG(2u) & CAN_Rx_TX_REQUEST_PENDING) != 0u)
        #endif /* CY_PSOC3 || CY_PSOC5 */
            {
                result = CAN_Rx_FAIL;
            }
            else
            {
                /* `#START MESSAGE_2_TRASMITTED` */

                /* `#END` */

                #ifdef CAN_Rx_SEND_MSG_2_CALLBACK
                    CAN_Rx_SendMsg_2_Callback();
                #endif /* CAN_Rx_SEND_MSG_2_CALLBACK */

                CY_SET_REG32(CAN_Rx_TX_CMD_PTR(2u),
                CY_GET_REG32(CAN_Rx_TX_CMD_PTR(2u)) | CAN_Rx_SEND_MESSAGE);
            }

        return (result);
    }
#endif /* CAN_Rx_TX2_FUNC_ENABLE */


#if (CAN_Rx_TX3_FUNC_ENABLE)
    /*******************************************************************************
    * FUNCTION NAME:   CAN_Rx_SendMsg3
    ********************************************************************************
    *
    * Summary:
    *  This function is the entry point to Transmit Message 3. The function checks
    *  if mailbox 3 doesn't already have un-transmitted messages waiting for
    *  arbitration. If not initiate transmission of the message.
    *  Generated only for the Transmit mailbox designed as Full.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  Indication if Message has been sent.
    *   Define                             Description
    *    CYRET_SUCCESS                      The function passed successfully
    *    CAN_Rx_FAIL              The function failed
    *
    *******************************************************************************/
    uint8 CAN_Rx_SendMsg3(void) 
    {
        uint8 result = CYRET_SUCCESS;

        #if (CY_PSOC3 || CY_PSOC5)
            if ((CAN_Rx_TX[3u].txcmd.byte[0u] & CAN_Rx_TX_REQUEST_PENDING) != 0u)
        #else  /* CY_PSOC4 */
            if ((CAN_Rx_TX_CMD_REG(3u) & CAN_Rx_TX_REQUEST_PENDING) != 0u)
        #endif /* CY_PSOC3 || CY_PSOC5 */
            {
                result = CAN_Rx_FAIL;
            }
            else
            {
                /* `#START MESSAGE_3_TRASMITTED` */

                /* `#END` */

                #ifdef CAN_Rx_SEND_MSG_3_CALLBACK
                    CAN_Rx_SendMsg_3_Callback();
                #endif /* CAN_Rx_SEND_MSG_3_CALLBACK */

                CY_SET_REG32(CAN_Rx_TX_CMD_PTR(3u),
                CY_GET_REG32(CAN_Rx_TX_CMD_PTR(3u)) | CAN_Rx_SEND_MESSAGE);
            }

        return (result);
    }
#endif /* CAN_Rx_TX3_FUNC_ENABLE */


#if (CAN_Rx_TX4_FUNC_ENABLE)
    /*******************************************************************************
    * FUNCTION NAME:   CAN_Rx_SendMsg4
    ********************************************************************************
    *
    * Summary:
    *  This function is the entry point to Transmit Message 4. The function checks
    *  if mailbox 4 doesn't already have un-transmitted messages waiting for
    *  arbitration. If not initiate transmission of the message.
    *  Generated only for the Transmit mailbox designed as Full.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  Indication if Message has been sent.
    *   Define                             Description
    *    CYRET_SUCCESS                      The function passed successfully
    *    CAN_Rx_FAIL              The function failed
    *
    *******************************************************************************/
    uint8 CAN_Rx_SendMsg4(void) 
    {
        uint8 result = CYRET_SUCCESS;

        #if (CY_PSOC3 || CY_PSOC5)
            if ((CAN_Rx_TX[4u].txcmd.byte[0u] & CAN_Rx_TX_REQUEST_PENDING) != 0u)
        #else  /* CY_PSOC4 */
            if ((CAN_Rx_TX_CMD_REG(4u) & CAN_Rx_TX_REQUEST_PENDING) != 0u)
        #endif /* CY_PSOC3 || CY_PSOC5 */
            {
                result = CAN_Rx_FAIL;
            }
            else
            {
                /* `#START MESSAGE_4_TRASMITTED` */

                /* `#END` */

                #ifdef CAN_Rx_SEND_MSG_4_CALLBACK
                    CAN_Rx_SendMsg_4_Callback();
                #endif /* CAN_Rx_SEND_MSG_4_CALLBACK */

                CY_SET_REG32(CAN_Rx_TX_CMD_PTR(4u),
                CY_GET_REG32(CAN_Rx_TX_CMD_PTR(4u)) | CAN_Rx_SEND_MESSAGE);
            }

        return (result);
    }
#endif /* CAN_Rx_TX4_FUNC_ENABLE */


#if (CAN_Rx_TX5_FUNC_ENABLE)
    /*******************************************************************************
    * FUNCTION NAME:   CAN_Rx_SendMsg5
    ********************************************************************************
    *
    * Summary:
    *  This function is the entry point to Transmit Message 5. The function checks
    *  if mailbox 5 doesn't already have un-transmitted messages waiting for
    *  arbitration. If not initiate transmission of the message.
    *  Generated only for the Transmit mailbox designed as Full.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  Indication if Message has been sent.
    *   Define                             Description
    *    CYRET_SUCCESS                      The function passed successfully
    *    CAN_Rx_FAIL              The function failed
    *
    *******************************************************************************/
    uint8 CAN_Rx_SendMsg5(void) 
    {
        uint8 result = CYRET_SUCCESS;

        #if (CY_PSOC3 || CY_PSOC5)
            if ((CAN_Rx_TX[5u].txcmd.byte[0u] & CAN_Rx_TX_REQUEST_PENDING) != 0u)
        #else  /* CY_PSOC4 */
            if ((CAN_Rx_TX_CMD_REG(5u) & CAN_Rx_TX_REQUEST_PENDING) != 0u)
        #endif /* CY_PSOC3 || CY_PSOC5 */
            {
                result = CAN_Rx_FAIL;
            }
            else
            {
                /* `#START MESSAGE_5_TRASMITTED` */

                /* `#END` */

                #ifdef CAN_Rx_SEND_MSG_5_CALLBACK
                    CAN_Rx_SendMsg_5_Callback();
                #endif /* CAN_Rx_SEND_MSG_5_CALLBACK */

                CY_SET_REG32(CAN_Rx_TX_CMD_PTR(5u),
                CY_GET_REG32(CAN_Rx_TX_CMD_PTR(5u)) | CAN_Rx_SEND_MESSAGE);
            }

        return (result);
    }
#endif /* CAN_Rx_TX5_FUNC_ENABLE */


#if (CAN_Rx_TX6_FUNC_ENABLE)
    /*******************************************************************************
    * FUNCTION NAME:   CAN_Rx_SendMsg6
    ********************************************************************************
    *
    * Summary:
    *  This function is the entry point to Transmit Message 6. The function checks
    *  if mailbox 6 doesn't already have un-transmitted messages waiting for
    *  arbitration. If not initiate transmission of the message.
    *  Generated only for the Transmit mailbox designed as Full.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  Indication if Message has been sent.
    *   Define                             Description
    *    CYRET_SUCCESS                      The function passed successfully
    *    CAN_Rx_FAIL              The function failed
    *
    *******************************************************************************/
    uint8 CAN_Rx_SendMsg6(void) 
    {
        uint8 result = CYRET_SUCCESS;

        #if (CY_PSOC3 || CY_PSOC5)
            if ((CAN_Rx_TX[6u].txcmd.byte[0u] & CAN_Rx_TX_REQUEST_PENDING) != 0u)
        #else  /* CY_PSOC4 */
            if ((CAN_Rx_TX_CMD_REG(6u) & CAN_Rx_TX_REQUEST_PENDING) != 0u)
        #endif /* CY_PSOC3 || CY_PSOC5 */
            {
                result = CAN_Rx_FAIL;
            }
            else
            {
                /* `#START MESSAGE_6_TRASMITTED` */

                /* `#END` */

                #ifdef CAN_Rx_SEND_MSG_6_CALLBACK
                    CAN_Rx_SendMsg_6_Callback();
                #endif /* CAN_Rx_SEND_MSG_6_CALLBACK */

                CY_SET_REG32(CAN_Rx_TX_CMD_PTR(6u),
                CY_GET_REG32(CAN_Rx_TX_CMD_PTR(6u)) | CAN_Rx_SEND_MESSAGE);
            }

        return (result);
    }
#endif /* CAN_Rx_TX6_FUNC_ENABLE */


#if (CAN_Rx_TX7_FUNC_ENABLE)
    /*******************************************************************************
    * FUNCTION NAME:   CAN_Rx_SendMsg7)
    ********************************************************************************
    *
    * Summary:
    *  This function is the entry point to Transmit Message 7. The function checks
    *  if mailbox 7 doesn't already have un-transmitted messages waiting for
    *  arbitration. If not initiate transmission of the message.
    *  Generated only for the Transmit mailbox designed as Full.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  Indication if Message has been sent.
    *   Define                             Description
    *    CYRET_SUCCESS                      The function passed successfully
    *    CAN_Rx_FAIL              The function failed
    *
    *******************************************************************************/
    uint8 CAN_Rx_SendMsg7(void) 
    {
        uint8 result = CYRET_SUCCESS;

        #if (CY_PSOC3 || CY_PSOC5)
            if ((CAN_Rx_TX[7u].txcmd.byte[0u] & CAN_Rx_TX_REQUEST_PENDING) != 0u)
        #else  /* CY_PSOC4 */
            if ((CAN_Rx_TX_CMD_REG(7u) & CAN_Rx_TX_REQUEST_PENDING) != 0u)
        #endif /* CY_PSOC3 || CY_PSOC5 */
            {
                result = CAN_Rx_FAIL;
            }
            else
            {
                /* `#START MESSAGE_7_TRASMITTED` */

                /* `#END` */

                #ifdef CAN_Rx_SEND_MSG_7_CALLBACK
                    CAN_Rx_SendMsg_7_Callback();
                #endif /* CAN_Rx_SEND_MSG_7_CALLBACK */

                CY_SET_REG32(CAN_Rx_TX_CMD_PTR(7u),
                CY_GET_REG32(CAN_Rx_TX_CMD_PTR(7u)) | CAN_Rx_SEND_MESSAGE);
            }

        return (result);
    }
#endif /* CAN_Rx_TX7_FUNC_ENABLE */


/*******************************************************************************
* FUNCTION NAME:   CAN_Rx_ReceiveMsg
********************************************************************************
*
* Summary:
*  This function is the entry point to Receive Message Interrupt for Basic
*  mailboxes. Clears the Receive particular Message interrupt flag. Generated
*  only if one of the Receive mailboxes is designed as Basic.
*
* Parameters:
*  rxMailbox: The mailbox number that trig Receive Message Interrupt.
*
* Return:
*  None.
*
* Reentrant:
*  Depends on the Customer code.
*
*******************************************************************************/
void CAN_Rx_ReceiveMsg(uint8 rxMailbox) 
{
    #if (CY_PSOC3 || CY_PSOC5)
        if ((CAN_Rx_RX[rxMailbox].rxcmd.byte[0u] & CAN_Rx_RX_ACK_MSG) != 0u)
    #else  /* CY_PSOC4 */
        if ((CAN_Rx_RX_CMD_REG(rxMailbox) & CAN_Rx_RX_ACK_MSG) != 0u)
    #endif /* CY_PSOC3 || CY_PSOC5 */
        {
            /* `#START MESSAGE_BASIC_RECEIVED` */
            uint8 bIndex=0;
            uint8 bLength=0;
            char data[20];
            bLength = CAN_Rx_GET_DLC(rxMailbox);

                for (bIndex=0;bIndex<bLength;bIndex++)
                {
                LEDNumber = CAN_Rx_RX_DATA_BYTE(rxMailbox,bIndex);
                }

            /* `#END` */

            #ifdef CAN_Rx_RECEIVE_MSG_CALLBACK
                CAN_Rx_ReceiveMsg_Callback();
            #endif /* CAN_Rx_RECEIVE_MSG_CALLBACK */

            #if (CY_PSOC3 || CY_PSOC5)
                CAN_Rx_RX[rxMailbox].rxcmd.byte[0u] |= CAN_Rx_RX_ACK_MSG;
            #else  /* CY_PSOC4 */
                CAN_Rx_RX_CMD_REG(rxMailbox) |= CAN_Rx_RX_ACK_MSG;
            #endif /* CY_PSOC3 || CY_PSOC5 */
        }
}


#if (CAN_Rx_RX0_FUNC_ENABLE)
    /*******************************************************************************
    * FUNCTION NAME:   CAN_Rx_ReceiveMsg0
    ********************************************************************************
    *
    * Summary:
    *  This function is the entry point to Receive Message 0 Interrupt. Clears the
    *  Receive Message 0 interrupt flag. Generated only for the Receive mailbox
    *  designed as Full.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    * Reentrant:
    *  Depends on the Customer code.
    *
    *******************************************************************************/
    void CAN_Rx_ReceiveMsg0(void) 
    {
        /* `#START MESSAGE_0_RECEIVED` */

        /* `#END` */

        #ifdef CAN_Rx_RECEIVE_MSG_0_CALLBACK
            CAN_Rx_ReceiveMsg_0_Callback();
        #endif /* CAN_Rx_RECEIVE_MSG_0_CALLBACK */

        CAN_Rx_RX[0u].rxcmd.byte[0u] |= CAN_Rx_RX_ACK_MSG;
    }
#endif /* CAN_Rx_RX0_FUNC_ENABLE */


#if (CAN_Rx_RX1_FUNC_ENABLE)
    /*******************************************************************************
    * FUNCTION NAME:    CAN_Rx_ReceiveMsg1
    ********************************************************************************
    *
    * Summary:
    *  This function is the entry point to Receive Message 1 Interrupt. Clears the
    *  Receive Message 1 interrupt flag. Generated only for the Receive mailbox
    *  designed as Full.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    * Reentrant:
    *  Depends on the Customer code.
    *
    *******************************************************************************/
    void CAN_Rx_ReceiveMsg1(void) 
    {
        /* `#START MESSAGE_1_RECEIVED` */

        /* `#END` */

        #ifdef CAN_Rx_RECEIVE_MSG_1_CALLBACK
            CAN_Rx_ReceiveMsg_1_Callback();
        #endif /* CAN_Rx_RECEIVE_MSG_1_CALLBACK */

        CAN_Rx_RX[1u].rxcmd.byte[0u] |= CAN_Rx_RX_ACK_MSG;
    }
#endif /* CAN_Rx_RX1_FUNC_ENABLE */


#if (CAN_Rx_RX2_FUNC_ENABLE)
    /*******************************************************************************
    * FUNCTION NAME:   CAN_Rx_ReceiveMsg2
    ********************************************************************************
    *
    * Summary:
    *  This function is the entry point to Receive Message 2 Interrupt. Clears the
    *  Receive Message 2 interrupt flag. Generated only for the Receive mailbox
    *  designed as Full.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    * Reentrant:
    *  Depends on the Customer code.
    *
    *******************************************************************************/
    void CAN_Rx_ReceiveMsg2(void) 
    {
        /* `#START MESSAGE_2_RECEIVED` */

        /* `#END` */

        #ifdef CAN_Rx_RECEIVE_MSG_2_CALLBACK
            CAN_Rx_ReceiveMsg_2_Callback();
        #endif /* CAN_Rx_RECEIVE_MSG_2_CALLBACK */

        CAN_Rx_RX[2u].rxcmd.byte[0u] |= CAN_Rx_RX_ACK_MSG;
    }
#endif /* CAN_Rx_RX2_FUNC_ENABLE */


#if (CAN_Rx_RX3_FUNC_ENABLE)
    /*******************************************************************************
    * FUNCTION NAME:   CAN_Rx_ReceiveMsg3
    ********************************************************************************
    *
    * Summary:
    *  This function is the entry point to Receive Message 3 Interrupt. Clears the
    *  Receive Message 3 interrupt flag. Generated only for the Receive mailbox
    *  designed as Full.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    * Reentrant:
    *  Depends on the Customer code.
    *
    *******************************************************************************/
    void CAN_Rx_ReceiveMsg3(void) 
    {
        /* `#START MESSAGE_3_RECEIVED` */

        /* `#END` */

        #ifdef CAN_Rx_RECEIVE_MSG_3_CALLBACK
            CAN_Rx_ReceiveMsg_3_Callback();
        #endif /* CAN_Rx_RECEIVE_MSG_3_CALLBACK */

        CAN_Rx_RX[3u].rxcmd.byte[0u] |= CAN_Rx_RX_ACK_MSG;
    }
#endif /* CAN_Rx_RX3_FUNC_ENABLE */


#if (CAN_Rx_RX4_FUNC_ENABLE)
    /*******************************************************************************
    * FUNCTION NAME:   CAN_Rx_ReceiveMsg4
    ********************************************************************************
    *
    * Summary:
    *  This function is the entry point to Receive Message 4 Interrupt. Clears the
    *  Receive Message 4 interrupt flag. Generated only for the Receive mailbox
    *  designed as Full.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    * Reentrant:
    *  Depends on the Customer code.
    *
    *******************************************************************************/
    void CAN_Rx_ReceiveMsg4(void) 
    {
        /* `#START MESSAGE_4_RECEIVED` */

        /* `#END` */

        #ifdef CAN_Rx_RECEIVE_MSG_4_CALLBACK
            CAN_Rx_ReceiveMsg_4_Callback();
        #endif /* CAN_Rx_RECEIVE_MSG_4_CALLBACK */

        CAN_Rx_RX[4u].rxcmd.byte[0u] |= CAN_Rx_RX_ACK_MSG;
    }
#endif /* CAN_Rx_RX4_FUNC_ENABLE */


#if (CAN_Rx_RX5_FUNC_ENABLE)
    /*******************************************************************************
    * FUNCTION NAME:   CAN_Rx_ReceiveMsg5
    ********************************************************************************
    *
    * Summary:
    *  This function is the entry point to Receive Message 5 Interrupt. Clears the
    *  Receive Message 5 interrupt flag. Generated only for the Receive mailbox
    *  designed as Full.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    * Reentrant:
    *  Depends on the Customer code.
    *
    *******************************************************************************/
    void CAN_Rx_ReceiveMsg5(void) 
    {
        /* `#START MESSAGE_5_RECEIVED` */

        /* `#END` */

        #ifdef CAN_Rx_RECEIVE_MSG_5_CALLBACK
            CAN_Rx_ReceiveMsg_5_Callback();
        #endif /* CAN_Rx_RECEIVE_MSG_5_CALLBACK */

        CAN_Rx_RX[5u].rxcmd.byte[0u] |= CAN_Rx_RX_ACK_MSG;
    }
#endif /* CAN_Rx_RX5_FUNC_ENABLE */


#if (CAN_Rx_RX6_FUNC_ENABLE)
    /*******************************************************************************
    * FUNCTION NAME:   CAN_Rx_ReceiveMsg6
    ********************************************************************************
    *
    * Summary:
    *  This function is the entry point to Receive Message 6 Interrupt. Clears the
    *  Receive Message 6 interrupt flag. Generated only for the Receive mailbox
    *  designed as Full.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    * Reentrant:
    *  Depends on the Customer code.
    *
    *******************************************************************************/
    void CAN_Rx_ReceiveMsg6(void) 
    {
        /* `#START MESSAGE_6_RECEIVED` */

        /* `#END` */

        #ifdef CAN_Rx_RECEIVE_MSG_6_CALLBACK
            CAN_Rx_ReceiveMsg_6_Callback();
        #endif /* CAN_Rx_RECEIVE_MSG_6_CALLBACK */

        CAN_Rx_RX[6u].rxcmd.byte[0u] |= CAN_Rx_RX_ACK_MSG;
    }
#endif /* CAN_Rx_RX6_FUNC_ENABLE */


#if (CAN_Rx_RX7_FUNC_ENABLE)
    /*******************************************************************************
    * FUNCTION NAME:   CAN_Rx_ReceiveMsg7
    ********************************************************************************
    *
    * Summary:
    *  This function is the entry point to Receive Message 7 Interrupt. Clears the
    *  Receive Message 7 interrupt flag. Generated only for the Receive mailbox
    *  designed as Full.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    * Reentrant:
    *  Depends on the Customer code.
    *
    *******************************************************************************/
    void CAN_Rx_ReceiveMsg7(void) 
    {
        /* `#START MESSAGE_7_RECEIVED` */

        /* `#END` */

        #ifdef CAN_Rx_RECEIVE_MSG_7_CALLBACK
            CAN_Rx_ReceiveMsg_7_Callback();
        #endif /* CAN_Rx_RECEIVE_MSG_7_CALLBACK */

        CAN_Rx_RX[7u].rxcmd.byte[0u] |= CAN_Rx_RX_ACK_MSG;
    }
#endif /* CAN_Rx_RX7_FUNC_ENABLE */


#if (CAN_Rx_RX8_FUNC_ENABLE)
    /*******************************************************************************
    * FUNCTION NAME:   CAN_Rx_ReceiveMsg8
    ********************************************************************************
    *
    * Summary:
    *  This function is the entry point to Receive Message 8 Interrupt. Clears the
    *  Receive Message 8 interrupt flag. Generated only for the Receive mailbox
    *  designed as Full.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    * Reentrant:
    *  Depends on the Customer code.
    *
    *******************************************************************************/
    void CAN_Rx_ReceiveMsg8(void) 
    {
        /* `#START MESSAGE_8_RECEIVED` */

        /* `#END` */

        #ifdef CAN_Rx_RECEIVE_MSG_8_CALLBACK
            CAN_Rx_ReceiveMsg_8_Callback();
        #endif /* CAN_Rx_RECEIVE_MSG_8_CALLBACK */

        CAN_Rx_RX[8u].rxcmd.byte[0u] |= CAN_Rx_RX_ACK_MSG;
    }
#endif /* CAN_Rx_RX8_FUNC_ENABLE */


#if (CAN_Rx_RX9_FUNC_ENABLE)
    /*******************************************************************************
    * FUNCTION NAME:   CAN_Rx_ReceiveMsg9
    ********************************************************************************
    *
    * Summary:
    *  This function is the entry point to Receive Message 9 Interrupt. Clears the
    *  Receive Message 9 interrupt flag. Generated only for the Receive mailbox
    *  designed as Full.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    * Reentrant:
    *  Depends on the Customer code.
    *
    *******************************************************************************/
    void CAN_Rx_ReceiveMsg9(void) 
    {
        /* `#START MESSAGE_9_RECEIVED` */

        /* `#END` */

        #ifdef CAN_Rx_RECEIVE_MSG_9_CALLBACK
            CAN_Rx_ReceiveMsg_9_Callback();
        #endif /* CAN_Rx_RECEIVE_MSG_9_CALLBACK */

        CAN_Rx_RX[9u].rxcmd.byte[0u] |= CAN_Rx_RX_ACK_MSG;
    }
#endif /* CAN_Rx_RX9_FUNC_ENABLE */


#if (CAN_Rx_RX10_FUNC_ENABLE)
    /*******************************************************************************
    * FUNCTION NAME:   CAN_Rx_ReceiveMsg10
    ********************************************************************************
    *
    * Summary:
    *  This function is the entry point to Receive Message 10 Interrupt. Clears the
    *  Receive Message 10 interrupt flag. Generated only for the Receive mailbox
    *  designed as Full.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    * Reentrant:
    *  Depends on the Customer code.
    *
    *******************************************************************************/
    void CAN_Rx_ReceiveMsg10(void) 
    {
        /* `#START MESSAGE_10_RECEIVED` */

        /* `#END` */

        #ifdef CAN_Rx_RECEIVE_MSG_10_CALLBACK
            CAN_Rx_ReceiveMsg_10_Callback();
        #endif /* CAN_Rx_RECEIVE_MSG_10_CALLBACK */

        CAN_Rx_RX[10u].rxcmd.byte[0u] |= CAN_Rx_RX_ACK_MSG;
    }
#endif /* CAN_Rx_RX10_FUNC_ENABLE */


#if (CAN_Rx_RX11_FUNC_ENABLE)
    /*******************************************************************************
    * FUNCTION NAME:   CAN_Rx_ReceiveMsg11
    ********************************************************************************
    *
    * Summary:
    *  This function is the entry point to Receive Message 11 Interrupt. Clears the
    *  Receive Message 11 interrupt flag. Generated only for the Receive mailbox
    *  designed as Full.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    * Reentrant:
    *  Depends on the Customer code.
    *
    *******************************************************************************/
    void CAN_Rx_ReceiveMsg11(void) 
    {
        /* `#START MESSAGE_11_RECEIVED` */

        /* `#END` */

        #ifdef CAN_Rx_RECEIVE_MSG_11_CALLBACK
            CAN_Rx_ReceiveMsg_11_Callback();
        #endif /* CAN_Rx_RECEIVE_MSG_11_CALLBACK */

        CAN_Rx_RX[11u].rxcmd.byte[0u] |= CAN_Rx_RX_ACK_MSG;
    }
#endif /* CAN_Rx_RX11_FUNC_ENABLE */


#if (CAN_Rx_RX12_FUNC_ENABLE)
    /*******************************************************************************
    * FUNCTION NAME:   CAN_Rx_ReceiveMsg12
    ********************************************************************************
    *
    * Summary:
    *  This function is the entry point to Receive Message 12 Interrupt. Clears the
    *  Receive Message 12 interrupt flag. Generated only for the Receive mailbox
    *  designed as Full.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    * Reentrant:
    *  Depends on the Customer code.
    *
    *******************************************************************************/
    void CAN_Rx_ReceiveMsg12(void) 
    {
        /* `#START MESSAGE_12_RECEIVED` */

        /* `#END` */

        #ifdef CAN_Rx_RECEIVE_MSG_12_CALLBACK
            CAN_Rx_ReceiveMsg_12_Callback();
        #endif /* CAN_Rx_RECEIVE_MSG_12_CALLBACK */

        CAN_Rx_RX[12u].rxcmd.byte[0u] |= CAN_Rx_RX_ACK_MSG;
    }
#endif /* CAN_Rx_RX12_FUNC_ENABLE */


#if (CAN_Rx_RX13_FUNC_ENABLE)
    /*******************************************************************************
    * FUNCTION NAME:   CAN_Rx_ReceiveMsg13
    ********************************************************************************
    *
    * Summary:
    *  This function is the entry point to Receive Message 13 Interrupt. Clears the
    *  Receive Message 13 interrupt flag. Generated only for the Receive mailbox
    *  designed as Full.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    * Reentrant:
    *  Depends on the Customer code.
    *
    *******************************************************************************/
    void CAN_Rx_ReceiveMsg13(void) 
    {
        /* `#START MESSAGE_13_RECEIVED` */

        /* `#END` */

        #ifdef CAN_Rx_RECEIVE_MSG_13_CALLBACK
            CAN_Rx_ReceiveMsg_13_Callback();
        #endif /* CAN_Rx_RECEIVE_MSG_13_CALLBACK */

        CAN_Rx_RX[13u].rxcmd.byte[0u] |= CAN_Rx_RX_ACK_MSG;
    }
#endif /* CAN_Rx_RX13_FUNC_ENABLE */


#if (CAN_Rx_RX14_FUNC_ENABLE)
    /*******************************************************************************
    * FUNCTION NAME:   CAN_Rx_ReceiveMsg14
    ********************************************************************************
    *
    * Summary:
    *  This function is the entry point to Receive Message 14 Interrupt. Clears the
    *  Receive Message 14 interrupt flag. Generated only for the Receive mailbox
    *  designed as Full.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    * Reentrant:
    *  Depends on the Customer code.
    *
    *******************************************************************************/
    void CAN_Rx_ReceiveMsg14(void) 
    {
        /* `#START MESSAGE_14_RECEIVED` */

        /* `#END` */

        #ifdef CAN_Rx_RECEIVE_MSG_14_CALLBACK
            CAN_Rx_ReceiveMsg_14_Callback();
        #endif /* CAN_Rx_RECEIVE_MSG_14_CALLBACK */

        CAN_Rx_RX[14u].rxcmd.byte[0u] |= CAN_Rx_RX_ACK_MSG;
    }
#endif /* CAN_Rx_RX14_FUNC_ENABLE */


#if (CAN_Rx_RX15_FUNC_ENABLE)
    /*******************************************************************************
    * FUNCTION NAME:   CAN_Rx_ReceiveMsg15
    ********************************************************************************
    *
    * Summary:
    *  This function is the entry point to Receive Message 15 Interrupt. Clears the
    *  Receive Message 15 interrupt flag. Generated only for the Receive mailbox
    *  designed as Full.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    * Reentrant:
    *  Depends on the Customer code.
    *
    *******************************************************************************/
    void CAN_Rx_ReceiveMsg15(void) 
    {
        /* `#START MESSAGE_15_RECEIVED` */

        /* `#END` */

        #ifdef CAN_Rx_RECEIVE_MSG_15_CALLBACK
            CAN_Rx_ReceiveMsg_15_Callback();
        #endif /* CAN_Rx_RECEIVE_MSG_15_CALLBACK */

        CAN_Rx_RX[15u].rxcmd.byte[0u] |= CAN_Rx_RX_ACK_MSG;
    }
#endif /* CAN_Rx_RX15_FUNC_ENABLE */


/* [] END OF FILE */
#if 0 /* begin disabled code */
`#start MESSAGE_Rx_message_RECEIVED` -- section removed from template
        uint16_t block=0;
        uint8_t data_l=0;
        uint8_t data_h=0;
        uint16_t i=0;
        char data[20];     
        if(buffernumber == 0){
                    //buffernumber=1;                    
                        while( i < 8)
                        {
                            data_l=CAN_Rx_RX[0].rxdata.byte[i];
                            i++;
                            data_h=CAN_Rx_RX[0].rxdata.byte[i];
                            block = (data_h << 8) | data_l;
                            if(block == 1)
                            {
//                                itoa(block , data, 10);
//                                UART_LOG_PutString("buffernumber0_Lidar_data1_Start block: ");
//                                UART_LOG_PutString(data);
//                                UART_LOG_PutString("\n");
                                startpacket=TRUE;
                                endpacket=FALSE;
                                i=8;
                                //break;
                            }
                            else if(block == 2)
                            {
//                                itoa(block , data, 10);
//                                UART_LOG_PutString("buffernumber0_Lidar_data1_End block: ");
//                                UART_LOG_PutString(data);
//                                UART_LOG_PutString("\n");
                                endpacket=TRUE;
                                //startpacket=FALSE;
                                i=8;
                                //break;
                            }
                            else
                            {
                                Lidar_data1[indx] = block;
//                                itoa(&Lidar_data1[indx] , data, 10);
//                                UART_LOG_PutString("buffernumber0_Lidar_data1_data block: ");
//                                UART_LOG_PutString(data);
//                                itoa(block , data, 10);
//                                UART_LOG_PutString("_: ");
//                                UART_LOG_PutString(data);
//                                UART_LOG_PutString("\n");
                                indx++;
                                if(indx > 255){indx=0;}
                                i++;
                            }
                        }
                }
                else{
                    while( i < 8)
                        {
                            data_l=CAN_Rx_RX[0].rxdata.byte[i];
                            i++;
                            data_h=CAN_Rx_RX[0].rxdata.byte[i];
                            block = (data_h << 8) | data_l;
                            if(block == 1)
                            {
//                                itoa(block , data, 10);
//                                UART_LOG_PutString("buffernumber1_Lidar_data2_Start block: ");
//                                UART_LOG_PutString(data);
//                                UART_LOG_PutString("\n");
                                startpacket=TRUE;
                                endpacket=FALSE;
                                i=8;
                                //break;
                            }
                            else if(block == 2)
                            {
//                                itoa(block , data, 10);
//                                UART_LOG_PutString("buffernumber1_Lidar_data2_End block: ");
//                                UART_LOG_PutString(data);
//                                UART_LOG_PutString("\n");
                                endpacket=TRUE;
                                //startpacket=FALSE;
                                i=8;
                                //break;
                            }
                            else
                            {
                                Lidar_data2[indx] = block;
//                                itoa(&Lidar_data2[indx] , data, 10);
//                                UART_LOG_PutString("buffernumber1_Lidar_data2_data block: ");
//                                UART_LOG_PutString(data);
//                                itoa(block , data, 10);
//                                UART_LOG_PutString("_: ");
//                                UART_LOG_PutString(data);
//                                UART_LOG_PutString("\n");
                                indx++;
                                if(indx > 255){indx=0;}
                                i++;
                            }
                        }
                }
`#end`

#endif /* end disabled code */
#if 0 /* begin disabled code */
`#start MESSAGE_Tx2_message_TRASMITTED` -- section removed from template
                UART_LOG_PutString("CAN_Rx_SendMsgTx2_message\n");
                CAN_Rx_TX_DATA_BYTE1(1) = CAN_Data;

`#end`

#endif /* end disabled code */
#if 0 /* begin disabled code */
`#start MESSAGE_Rx1_message_RECEIVED` -- section removed from template
        
        CAN_Data = CAN_Rx_RX_DATA_BYTE1(0);
        UART_LOG_PutString("CAN_Rx_ReceiveMsgRx1_message\n");
        RED_LED_Write(~RED_LED_Read());
        SEVEN_id_t seven = SEVEN_0;
            seven = SEVEN_0;
            SEVEN_Set(seven,CAN_Data%10);
            seven = SEVEN_1;
            SEVEN_Set(seven,CAN_Data/10);

`#end`

#endif /* end disabled code */
#if 0 /* begin disabled code */
`#start MESSAGE_Tx1_message_TRASMITTED` -- section removed from template
                CAN_Data++;
                CAN_Rx_TX_DATA_BYTE1(0) = CAN_Data;
`#end`

#endif /* end disabled code */
#if 0 /* begin disabled code */
`#start MESSAGE_Rx2_message_RECEIVED` -- section removed from template
        CAN_Data = CAN_Rx_RX_DATA_BYTE1(1);
        UART_LOG_PutString("CAN_Rx_ReceiveMsgRx2_message\n");
        RED_LED_Write(~RED_LED_Read());
        SEVEN_id_t seven = SEVEN_0;
            seven = SEVEN_0;
            SEVEN_Set(seven,CAN_Data%10);
            seven = SEVEN_1;
            SEVEN_Set(seven,CAN_Data/10);

`#end`

#endif /* end disabled code */
