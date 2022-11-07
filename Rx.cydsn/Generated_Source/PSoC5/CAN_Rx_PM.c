/*******************************************************************************
* File Name: CAN_Rx_PM.c
* Version 3.0
*
* Description:
*  This file contains the setup, control and status commands to support
*  component operations in the low power mode.
*
* Note:
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "CAN_Rx.h"

static CAN_Rx_BACKUP_STRUCT CAN_Rx_backup =
{
    0u,
    #if (CY_PSOC3 || CY_PSOC5)
        0u,
        CAN_Rx_INIT_INTERRUPT_MASK,
        CAN_Rx_MODE_MASK,
        (((uint32) ((uint32) CAN_Rx_SYNC_EDGE    << CAN_Rx_EDGE_MODE_SHIFT))     |
        ((uint32) ((uint32) CAN_Rx_SAMPLING_MODE << CAN_Rx_SAMPLE_MODE_SHIFT))   |
        ((uint32) ((uint32) CAN_Rx_CFG_REG_SJW   << CAN_Rx_CFG_REG_SJW_SHIFT))   |
        ((uint32) ((uint32) CAN_Rx_RESET_TYPE    << CAN_Rx_RESET_SHIFT))         |
        ((uint32) ((uint32) CAN_Rx_CFG_REG_TSEG2 << CAN_Rx_CFG_REG_TSEG2_SHIFT)) |
        ((uint32) ((uint32) CAN_Rx_CFG_REG_TSEG1 << CAN_Rx_CFG_REG_TSEG1_SHIFT)) |
        ((uint32) ((uint32) CAN_Rx_ARBITER       << CAN_Rx_ARBITER_SHIFT))       |
        ((uint32) ((uint32) CAN_Rx_BITRATE       << CAN_Rx_BITRATE_SHIFT)))
    #endif /* CY_PSOC3 || CY_PSOC5 */
};


/*******************************************************************************
* Function Name: CAN_Rx_SaveConfig
********************************************************************************
*
* Summary:
*  Save the CAN configuration.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global Variables:
*  CAN_Rx_backup - Modified when non-retention registers are saved.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void CAN_Rx_SaveConfig(void) 
{
    #if (CY_PSOC3 || CY_PSOC5)
        CAN_Rx_backup.intSr = (CY_GET_REG32(CAN_Rx_INT_SR_PTR));
        CAN_Rx_backup.intEn = (CY_GET_REG32(CAN_Rx_INT_EN_PTR));
        CAN_Rx_backup.cmd = (CY_GET_REG32(CAN_Rx_CMD_PTR));
        CAN_Rx_backup.cfg = (CY_GET_REG32(CAN_Rx_CFG_PTR));
    #endif /* CY_PSOC3 || CY_PSOC5 */
}


/*******************************************************************************
* Function Name: CAN_Rx_RestoreConfig
********************************************************************************
*
* Summary:
*  Restore the CAN configuration.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global Variables:
*  CAN_Rx_backup - Used when non-retention registers are restored.
*
* Side Effects:
*  If this API is called without first calling SaveConfig then default values
*  from Customizer will be in the following registers: CAN_Rx_INT_SR,
*  CAN_Rx_INT_EN, CAN_Rx_CMD, CAN_Rx_CFG.
*
*******************************************************************************/
void CAN_Rx_RestoreConfig(void) 
{
    #if (CY_PSOC3 || CY_PSOC5)
        CY_SET_REG32(CAN_Rx_INT_SR_PTR, CAN_Rx_backup.intSr);
        CY_SET_REG32(CAN_Rx_INT_EN_PTR, CAN_Rx_backup.intEn);
        CY_SET_REG32(CAN_Rx_CMD_PTR, CAN_Rx_backup.cmd);
        CY_SET_REG32(CAN_Rx_CFG_PTR, CAN_Rx_backup.cfg);
    #endif /* CY_PSOC3 || CY_PSOC5 */
}


/*******************************************************************************
* Function Name: CAN_Rx_Sleep
********************************************************************************
*
* Summary:
*  Prepares CAN Component to go to sleep.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global Variables:
*  CAN_Rx_backup - Modified when non-retention registers are saved.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void CAN_Rx_Sleep(void) 
{
    #if (!(CY_PSOC3 || CY_PSOC5))
        uint8 i;
    #endif /* (!(CY_PSOC3 || CY_PSOC5)) */

    if (0u != (CY_GET_REG32(CAN_Rx_CMD_PTR) & CAN_Rx_MODE_MASK))
    {
        CAN_Rx_backup.enableState = 1u;
    }
    else /* CAN block is disabled */
    {
        CAN_Rx_backup.enableState = 0u;
    }

    #if (CY_PSOC3 || CY_PSOC5)
        CAN_Rx_SaveConfig();
        (void) CAN_Rx_Stop();
    #else /* CY_PSOC4 */
        /* Abort respective pending TX message requests */
        for (i = 0u; i < CAN_Rx_NUMBER_OF_TX_MAILBOXES; i++)
        {
            CAN_Rx_TX_ABORT_MESSAGE(i);
        }

        /* Sets CAN controller to Stop mode */
        (void) CAN_Rx_Stop();

        /* Clear Global Interrupt enable Flag */
        (void) CAN_Rx_GlobalIntDisable();
    #endif /* CY_PSOC3 || CY_PSOC5 */
}


/*******************************************************************************
* Function Name: CAN_Rx_Wakeup
********************************************************************************
*
* Summary:
*  Prepares CAN Component to wake up.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global Variables:
*  CAN_Rx_backup - Used when non-retention registers are restored.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void CAN_Rx_Wakeup(void) 
{
    #if (CY_PSOC3 || CY_PSOC5)
        /* Enable CAN block in Active mode */
        CAN_Rx_PM_ACT_CFG_REG |= CAN_Rx_ACT_PWR_EN;
        /* Enable CAN block in Alternate Active (Standby) mode */
        CAN_Rx_PM_STBY_CFG_REG |= CAN_Rx_STBY_PWR_EN;

        CAN_Rx_RestoreConfig();

        /* Reconfigure Rx and Tx Buffers control registers */
        (void) CAN_Rx_RxTxBuffersConfig();

        if (CAN_Rx_backup.enableState != 0u)
        {
            /* Enable component's operation */
            (void) CAN_Rx_Enable();
        } /* Do nothing if component's block was disabled before */
    #else /* CY_PSOC4 */
        /* Clear all INT_STATUS bits */
        CAN_Rx_INT_SR_REG = CAN_Rx_INT_STATUS_MASK;

        /* Set Global Interrupt enable Flag */
        (void) CAN_Rx_GlobalIntEnable();

        if (CAN_Rx_backup.enableState != 0u)
        {
            (void) CAN_Rx_Enable();
        } /* Do nothing if component's block was disabled before */
    #endif /* CY_PSOC3 || CY_PSOC5 */
}


/* [] END OF FILE */
