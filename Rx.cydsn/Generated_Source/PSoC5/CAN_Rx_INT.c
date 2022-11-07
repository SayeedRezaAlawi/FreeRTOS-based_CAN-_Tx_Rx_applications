/*******************************************************************************
* File Name: CAN_Rx_INT.c
* Version 3.0
*
* Description:
*  This file contains Interrupt Service Routine (ISR) for CAN Component.
*  The Interrupt handlers functions are generated accordingly to the PSoC
*  Creator Customizer inputs.
*
* Note:
*  None
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "CAN_Rx.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "portmacro.h"
#include "UART_LOG.h"
#include "global.h"


/* `#START CAN_INT_C_CODE_DEFINITION` */
extern xSemaphoreHandle RxGiver;
extern boolean_t startpacket;
extern boolean_t endpacket;
/* `#END` */

#if (CAN_Rx_ARB_LOST)
    /*******************************************************************************
    * FUNCTION NAME:   CAN_Rx_ArbLostIsr
    ********************************************************************************
    *
    * Summary:
    *  This function is entry point to Arbitration Lost Interrupt. Clears
    *  Arbitration Lost interrupt flag. Only generated if Arbitration Lost
    *  Interrupt enable in Customizer.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    * Reentrant:
    *  Depends on Customer code.
    *
    *******************************************************************************/
    void CAN_Rx_ArbLostIsr(void) 
    {
        /* Clear Arbitration Lost flag */
        CAN_Rx_INT_SR_REG.byte[0u] = CAN_Rx_ARBITRATION_LOST_MASK;

        /* `#START ARBITRATION_LOST_ISR` */

        /* `#END` */

        #ifdef CAN_Rx_ARB_LOST_ISR_CALLBACK
            CAN_Rx_ArbLostIsr_Callback();
        #endif /* CAN_Rx_ARB_LOST_ISR_CALLBACK */
    }
#endif /* CAN_Rx_ARB_LOST */


#if (CAN_Rx_OVERLOAD)
    /*******************************************************************************
    * FUNCTION NAME:   CAN_Rx_OvrLdErrorIsr
    ********************************************************************************
    *
    * Summary:
    *  This function is entry point to Overload Error Interrupt. Clears Overload
    *  Error interrupt flag. Only generated if Overload Error Interrupt enable
    *  in Customizer.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    * Reentrant:
    *  Depends on Customer code.
    *
    *******************************************************************************/
    void CAN_Rx_OvrLdErrorIsr(void) 
    {
        /* Clear Overload Error flag */
        CAN_Rx_INT_SR_REG.byte[0u] = CAN_Rx_OVERLOAD_ERROR_MASK;

        /* `#START OVER_LOAD_ERROR_ISR` */

        /* `#END` */

        #ifdef CAN_Rx_OVR_LD_ERROR_ISR_CALLBACK
            CAN_Rx_OvrLdErrorIsr_Callback();
        #endif /* CAN_Rx_OVR_LD_ERROR_ISR_CALLBACK */
    }
#endif /* CAN_Rx_OVERLOAD */


#if (CAN_Rx_BIT_ERR)
    /*******************************************************************************
    * FUNCTION NAME:   CAN_Rx_BitErrorIsr
    ********************************************************************************
    *
    * Summary:
    *  This function is entry point to Bit Error Interrupt. Clears Bit Error
    *  interrupt flag. Only generated if Bit Error Interrupt enable in Customizer.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    * Reentrant:
    *  Depends on Customer code.
    *
    *******************************************************************************/
    void CAN_Rx_BitErrorIsr(void) 
    {
        /* Clear Bit Error flag */
        CAN_Rx_INT_SR_REG.byte[0u] = CAN_Rx_BIT_ERROR_MASK;

        /* `#START BIT_ERROR_ISR` */

        /* `#END` */

        #ifdef CAN_Rx_BIT_ERROR_ISR_CALLBACK
            CAN_Rx_BitErrorIsr_Callback();
        #endif /* CAN_Rx_BIT_ERROR_ISR_CALLBACK */
    }
#endif /* CAN_Rx_BIT_ERR */


#if (CAN_Rx_STUFF_ERR)
    /*******************************************************************************
    * FUNCTION NAME:   CAN_Rx_BitStuffErrorIsr
    ********************************************************************************
    *
    * Summary:
    *  This function is entry point to Bit Stuff Error Interrupt. Clears Bit Stuff
    *  Error interrupt flag. Only generated if Bit Stuff Error Interrupt enable
    *  in Customizer.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    * Reentrant:
    *  Depends on Customer code.
    *
    *******************************************************************************/
    void CAN_Rx_BitStuffErrorIsr(void) 
    {
        /* Clear Stuff Error flag */
        CAN_Rx_INT_SR_REG.byte[0u] = CAN_Rx_STUFF_ERROR_MASK;

        /* `#START BIT_STUFF_ERROR_ISR` */

        /* `#END` */

        #ifdef CAN_Rx_BIT_STUFF_ERROR_ISR_CALLBACK
            CAN_Rx_BitStuffErrorIsr_Callback();
        #endif /* CAN_Rx_BIT_STUFF_ERROR_ISR_CALLBACK */
    }
#endif /* CAN_Rx_STUFF_ERR */


#if (CAN_Rx_ACK_ERR)
    /*******************************************************************************
    * FUNCTION NAME:   CAN_Rx_AckErrorIsr
    ********************************************************************************
    *
    * Summary:
    *  This function is entry point to Acknowledge Error Interrupt. Clears
    *  Acknowledge Error interrupt flag. Only generated if Acknowledge Error
    *  Interrupt enable in Customizer.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    * Reentrant:
    *  Depends on Customer code.
    *
    *******************************************************************************/
    void CAN_Rx_AckErrorIsr(void) 
    {
        /* Clear Acknoledge Error flag */
        CAN_Rx_INT_SR_REG.byte[0u] = CAN_Rx_ACK_ERROR_MASK;

        /* `#START ACKNOWLEDGE_ERROR_ISR` */

        /* `#END` */

        #ifdef CAN_Rx_ACK_ERROR_ISR_CALLBACK
            CAN_Rx_AckErrorIsr_Callback();
        #endif /* CAN_Rx_ACK_ERROR_ISR_CALLBACK */
    }
#endif /* CAN_Rx_ACK_ERR */


#if (CAN_Rx_FORM_ERR)
    /*******************************************************************************
    * FUNCTION NAME:   CAN_Rx_MsgErrorIsr
    ********************************************************************************
    *
    * Summary:
    *  This function is entry point to Form Error Interrupt. Clears Form Error
    *  interrupt flag. Only generated if Form Error Interrupt enable in Customizer.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    * Reentrant:
    *  Depends on Customer code.
    *
    *******************************************************************************/
    void CAN_Rx_MsgErrorIsr(void) 
    {
        /* Clear Form Error flag */
        CAN_Rx_INT_SR_REG.byte[0u] = CAN_Rx_FORM_ERROR_MASK;

        /* `#START MESSAGE_ERROR_ISR` */

        /* `#END` */

        #ifdef CAN_Rx_MSG_ERROR_ISR_CALLBACK
            CAN_Rx_MsgErrorIsr_Callback();
        #endif /* CAN_Rx_MSG_ERROR_ISR_CALLBACK */
    }
#endif /* CAN_Rx_FORM_ERR */


#if (CAN_Rx_CRC_ERR)
    /*******************************************************************************
    * FUNCTION NAME:   CAN_Rx_CrcErrorIsr
    ********************************************************************************
    *
    * Summary:
    *  This function is entry point to CRC Error Interrupt. Clears CRC Error
    *  interrupt flag. Only generated if CRC Error Interrupt enable in Customizer.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    * Reentrant:
    *  Depends on Customer code.
    *
    *******************************************************************************/
    void CAN_Rx_CrcErrorIsr(void) 
    {
        /* Clear CRC Error flag */
        CAN_Rx_INT_SR_REG.byte[1u] = CAN_Rx_CRC_ERROR_MASK;

        /* `#START CRC_ERROR_ISR` */

        /* `#END` */

        #ifdef CAN_Rx_CRC_ERROR_ISR_CALLBACK
            CAN_Rx_CrcErrorIsr_Callback();
        #endif /* CAN_Rx_CRC_ERROR_ISR_CALLBACK */
    }
#endif /* CAN_Rx_CRC_ERR */


#if (CAN_Rx_BUS_OFF)
    /*******************************************************************************
    * FUNCTION NAME:   CAN_Rx_BusOffIsr
    ********************************************************************************
    *
    * Summary:
    *  This function is entry point to Bus Off Interrupt. Places CAN Component
    *  to Stop mode. Only generated if Bus Off Interrupt enable in Customizer.
    *  Recommended setting to enable this interrupt.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    * Side Effects:
    *  Stops CAN component operation.
    *
    *******************************************************************************/
    void CAN_Rx_BusOffIsr(void) 
    {
        /* Clear Bus Off flag */
        CAN_Rx_INT_SR_REG.byte[1u] = CAN_Rx_BUS_OFF_MASK;
        (void) CAN_Rx_GlobalIntDisable();

        /* `#START BUS_OFF_ISR` */

        /* `#END` */

        #ifdef CAN_Rx_BUS_OFF_ISR_CALLBACK
            CAN_Rx_BusOffIsr_Callback();
        #endif /* CAN_Rx_BUS_OFF_ISR_CALLBACK */

        (void) CAN_Rx_Stop();
    }
#endif /* CAN_Rx_BUS_OFF */


#if (CAN_Rx_RX_MSG_LOST)
    /*******************************************************************************
    * FUNCTION NAME:   CAN_Rx_MsgLostIsr
    ********************************************************************************
    *
    * Summary:
    *  This function is entry point to Message Lost Interrupt. Clears Message Lost
    *  interrupt flag. Only generated if Message Lost Interrupt enable in Customizer
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    * Reentrant:
    *  Depends on Customer code.
    *
    *******************************************************************************/
    void CAN_Rx_MsgLostIsr(void) 
    {
        /* Clear Receive Message Lost flag */
        CAN_Rx_INT_SR_REG.byte[1u] = CAN_Rx_RX_MSG_LOST_MASK;

        /* `#START MESSAGE_LOST_ISR` */

        /* `#END` */

        #ifdef CAN_Rx_MSG_LOST_ISR_CALLBACK
            CAN_Rx_MsgLostIsr_Callback();
        #endif /* CAN_Rx_MSG_LOST_ISR_CALLBACK */
    }
#endif /* CAN_Rx_RX_MSG_LOST */


#if (CAN_Rx_TX_MESSAGE)
    /*******************************************************************************
    * FUNCTION NAME:   CAN_Rx_MsgTXIsr
    ********************************************************************************
    *
    * Summary:
    *  This function is entry point to Transmit Message Interrupt. Clears Transmit
    *  Message interrupt flag. Only generated if Transmit Message Interrupt enable
    *  in Customizer.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    * Reentrant:
    *  Depends on Customer code.
    *
    *******************************************************************************/
    void CAN_Rx_MsgTXIsr(void) 
    {
        /* Clear Transmit Message flag */
        CAN_Rx_INT_SR_REG.byte[1u] = CAN_Rx_TX_MESSAGE_MASK;

        /* `#START MESSAGE_TRANSMITTED_ISR` */

        /* `#END` */

        #ifdef CAN_Rx_MSG_TX_ISR_CALLBACK
            CAN_Rx_MsgTXIsr_Callback();
        #endif /* CAN_Rx_MSG_TX_ISR_CALLBACK */
    }
#endif /* CAN_Rx_TX_MESSAGE */


#if (CAN_Rx_RX_MESSAGE)
    /*******************************************************************************
    * FUNCTION NAME:   CAN_Rx_MsgRXIsr
    ********************************************************************************
    *
    * Summary:
    *  This function is entry point to Receive Message Interrupt. Clears Receive
    *  Message interrupt flag and call appropriate handlers for Basic and Full
    *  interrupt based mailboxes. Only generated if Receive Message Interrupt
    *  enable in Customizer. Recommended setting to enable this interrupt.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    *******************************************************************************/
    void CAN_Rx_MsgRXIsr(void) 
    {
        uint8 mailboxNumber;
        uint16 shift = 0x01u;

        /* Clear Receive Message flag */
        CAN_Rx_INT_SR_REG.byte[1u] = CAN_Rx_RX_MESSAGE_MASK;

        /* `#START MESSAGE_RECEIVE_ISR` */

        /* `#END` */

        #ifdef CAN_Rx_MSG_RX_ISR_CALLBACK
            CAN_Rx_MsgRXIsr_Callback();
        #endif /* CAN_Rx_MSG_RX_ISR_CALLBACK */

        for (mailboxNumber = 0u; mailboxNumber < CAN_Rx_NUMBER_OF_RX_MAILBOXES; mailboxNumber++)
        {
            if ((CY_GET_REG16((reg16 *) &CAN_Rx_BUF_SR_REG.byte[0u]) & shift) != 0u)
            {
                if ((CAN_Rx_RX[mailboxNumber].rxcmd.byte[0u] & CAN_Rx_RX_INT_ENABLE_MASK) != 0u)
                {
                    if ((CAN_Rx_RX_MAILBOX_TYPE & shift) != 0u)
                    {
                        /* RX Full mailboxes handler */
                    }
                    else
                    {
                        /* RX Basic mailbox handler */
                        CAN_Rx_ReceiveMsg(mailboxNumber);
                    }
                }
            }
            shift <<= 1u;
        }
    }
#endif /* CAN_Rx_RX_MESSAGE */


/*******************************************************************************
* Function Name: CAN_Rx_ISR
********************************************************************************
*
* Summary:
*  This ISR is executed when CAN Core generates and interrupt on one of events:
*  Arb_lost, Overload, Bit_err, Stuff_err, Ack_err, Form_err, Crc_err,
*  Buss_off, Rx_msg_lost, Tx_msg or Rx_msg. The interrupt sources depends
*  on the Customizer inputs.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
*******************************************************************************/
CY_ISR(CAN_Rx_ISR)
{
    #ifdef CAN_Rx_ISR_INTERRUPT_CALLBACK
        CAN_Rx_ISR_InterruptCallback();
    #endif /* CAN_Rx_ISR_INTERRUPT_CALLBACK */
    
    /* Place your Interrupt code here. */
    /* `#START CAN_ISR` */

    /* `#END` */
    
    /* Arbitration */
    #if (CAN_Rx_ARB_LOST && (CAN_Rx_ARB_LOST_USE_HELPER || \
        (!CAN_Rx_ADVANCED_INTERRUPT_CFG)))
        if ((CAN_Rx_INT_SR_REG.byte[0u] & CAN_Rx_ARBITRATION_LOST_MASK) != 0u)
        {
            CAN_Rx_ArbLostIsr();
        }
    #endif /* CAN_Rx_ARB_LOST && CAN_Rx_ARB_LOST_USE_HELPER */

    /* Overload Error */
    #if (CAN_Rx_OVERLOAD && (CAN_Rx_OVERLOAD_USE_HELPER || \
        (!CAN_Rx_ADVANCED_INTERRUPT_CFG)))
        if ((CAN_Rx_INT_SR_REG.byte[0u] & CAN_Rx_OVERLOAD_ERROR_MASK) != 0u)
        {
            CAN_Rx_OvrLdErrorIsr();
        }
    #endif /* CAN_Rx_OVERLOAD && CAN_Rx_OVERLOAD_USE_HELPER */

    /* Bit Error */
    #if (CAN_Rx_BIT_ERR && (CAN_Rx_BIT_ERR_USE_HELPER || \
        (!CAN_Rx_ADVANCED_INTERRUPT_CFG)))
        if ((CAN_Rx_INT_SR_REG.byte[0u] & CAN_Rx_BIT_ERROR_MASK) != 0u)
        {
            CAN_Rx_BitErrorIsr();
        }
    #endif /* CAN_Rx_BIT_ERR && CAN_Rx_BIT_ERR_USE_HELPER */

    /* Bit Staff Error */
    #if (CAN_Rx_STUFF_ERR && (CAN_Rx_STUFF_ERR_USE_HELPER || \
        (!CAN_Rx_ADVANCED_INTERRUPT_CFG)))
        if ((CAN_Rx_INT_SR_REG.byte[0u] & CAN_Rx_STUFF_ERROR_MASK) != 0u)
        {
            CAN_Rx_BitStuffErrorIsr();
        }
    #endif /* CAN_Rx_STUFF_ERR && CAN_Rx_STUFF_ERR_USE_HELPER */

    /* ACK Error */
    #if (CAN_Rx_ACK_ERR && (CAN_Rx_ACK_ERR_USE_HELPER || \
        (!CAN_Rx_ADVANCED_INTERRUPT_CFG)))
        if ((CAN_Rx_INT_SR_REG.byte[0u] & CAN_Rx_ACK_ERROR_MASK) != 0u)
        {
            CAN_Rx_AckErrorIsr();
        }
    #endif /* CAN_Rx_ACK_ERR && CAN_Rx_ACK_ERR_USE_HELPER */

    /* Form(msg) Error */
    #if (CAN_Rx_FORM_ERR && (CAN_Rx_FORM_ERR_USE_HELPER || \
        (!CAN_Rx_ADVANCED_INTERRUPT_CFG)))
        if ((CAN_Rx_INT_SR_REG.byte[0u] & CAN_Rx_FORM_ERROR_MASK) != 0u)
        {
            CAN_Rx_MsgErrorIsr();
        }
    #endif /* CAN_Rx_FORM_ERR && CAN_Rx_FORM_ERR_USE_HELPER */

    /* CRC Error */
    #if (CAN_Rx_CRC_ERR && (CAN_Rx_CRC_ERR_USE_HELPER || \
        (!CAN_Rx_ADVANCED_INTERRUPT_CFG)))
        if ((CAN_Rx_INT_SR_REG.byte[1u] & CAN_Rx_CRC_ERROR_MASK) != 0u)
        {
            CAN_Rx_CrcErrorIsr();
        }
    #endif /* CAN_Rx_CRC_ERR && CAN_Rx_CRC_ERR_USE_HELPER */

    /* Bus Off state */
    #if (CAN_Rx_BUS_OFF && (CAN_Rx_BUS_OFF_USE_HELPER || \
        (!CAN_Rx_ADVANCED_INTERRUPT_CFG)))
        if ((CAN_Rx_INT_SR_REG.byte[1u] & CAN_Rx_BUS_OFF_MASK) != 0u)
        {
            CAN_Rx_BusOffIsr();
        }
    #endif /* CAN_Rx_BUS_OFF && CAN_Rx_BUS_OFF_USE_HELPER */

    /* Message Lost */
    #if (CAN_Rx_RX_MSG_LOST && (CAN_Rx_RX_MSG_LOST_USE_HELPER || \
        (!CAN_Rx_ADVANCED_INTERRUPT_CFG)))
        if ((CAN_Rx_INT_SR_REG.byte[1u] & CAN_Rx_RX_MSG_LOST_MASK) != 0u)
        {
            CAN_Rx_MsgLostIsr();
        }
    #endif /* CAN_Rx_RX_MSG_LOST && CAN_Rx_RX_MSG_LOST_USE_HELPER */

    /* TX Message Send */
    #if (CAN_Rx_TX_MESSAGE && (CAN_Rx_TX_MESSAGE_USE_HELPER || \
        (!CAN_Rx_ADVANCED_INTERRUPT_CFG)))
        if ((CAN_Rx_INT_SR_REG.byte[1u] & CAN_Rx_TX_MESSAGE_MASK) != 0u)
        {
            CAN_Rx_MsgTXIsr();
        }
    #endif /* CAN_Rx_TX_MESSAGE && CAN_Rx_TX_MESSAGE_USE_HELPER */

    /* RX Message Available */
    #if (CAN_Rx_RX_MESSAGE && (CAN_Rx_RX_MESSAGE_USE_HELPER || \
        (!CAN_Rx_ADVANCED_INTERRUPT_CFG)))
        static signed long xHigherPriorityTaskWoken;
        xHigherPriorityTaskWoken = pdFALSE;
        if ((CAN_Rx_INT_SR_REG.byte[1u] & CAN_Rx_RX_MESSAGE_MASK) != 0u)
        {
                CAN_Rx_MsgRXIsr();
                xSemaphoreGiveFromISR( RxGiver, &xHigherPriorityTaskWoken );
        }
        portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
    #endif /* CAN_Rx_RX_MESSAGE && CAN_Rx_RX_MESSAGE_USE_HELPER */
}


/* [] END OF FILE */
