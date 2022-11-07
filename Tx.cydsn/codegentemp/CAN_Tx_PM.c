/*******************************************************************************
* File Name: CAN_Tx_PM.c
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

#include "CAN_Tx.h"

static CAN_Tx_BACKUP_STRUCT CAN_Tx_backup =
{
    0u,
    #if (CY_PSOC3 || CY_PSOC5)
        0u,
        CAN_Tx_INIT_INTERRUPT_MASK,
        CAN_Tx_MODE_MASK,
        (((uint32) ((uint32) CAN_Tx_SYNC_EDGE    << CAN_Tx_EDGE_MODE_SHIFT))     |
        ((uint32) ((uint32) CAN_Tx_SAMPLING_MODE << CAN_Tx_SAMPLE_MODE_SHIFT))   |
        ((uint32) ((uint32) CAN_Tx_CFG_REG_SJW   << CAN_Tx_CFG_REG_SJW_SHIFT))   |
        ((uint32) ((uint32) CAN_Tx_RESET_TYPE    << CAN_Tx_RESET_SHIFT))         |
        ((uint32) ((uint32) CAN_Tx_CFG_REG_TSEG2 << CAN_Tx_CFG_REG_TSEG2_SHIFT)) |
        ((uint32) ((uint32) CAN_Tx_CFG_REG_TSEG1 << CAN_Tx_CFG_REG_TSEG1_SHIFT)) |
        ((uint32) ((uint32) CAN_Tx_ARBITER       << CAN_Tx_ARBITER_SHIFT))       |
        ((uint32) ((uint32) CAN_Tx_BITRATE       << CAN_Tx_BITRATE_SHIFT)))
    #endif /* CY_PSOC3 || CY_PSOC5 */
};


/*******************************************************************************
* Function Name: CAN_Tx_SaveConfig
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
*  CAN_Tx_backup - Modified when non-retention registers are saved.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void CAN_Tx_SaveConfig(void) 
{
    #if (CY_PSOC3 || CY_PSOC5)
        CAN_Tx_backup.intSr = (CY_GET_REG32(CAN_Tx_INT_SR_PTR));
        CAN_Tx_backup.intEn = (CY_GET_REG32(CAN_Tx_INT_EN_PTR));
        CAN_Tx_backup.cmd = (CY_GET_REG32(CAN_Tx_CMD_PTR));
        CAN_Tx_backup.cfg = (CY_GET_REG32(CAN_Tx_CFG_PTR));
    #endif /* CY_PSOC3 || CY_PSOC5 */
}


/*******************************************************************************
* Function Name: CAN_Tx_RestoreConfig
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
*  CAN_Tx_backup - Used when non-retention registers are restored.
*
* Side Effects:
*  If this API is called without first calling SaveConfig then default values
*  from Customizer will be in the following registers: CAN_Tx_INT_SR,
*  CAN_Tx_INT_EN, CAN_Tx_CMD, CAN_Tx_CFG.
*
*******************************************************************************/
void CAN_Tx_RestoreConfig(void) 
{
    #if (CY_PSOC3 || CY_PSOC5)
        CY_SET_REG32(CAN_Tx_INT_SR_PTR, CAN_Tx_backup.intSr);
        CY_SET_REG32(CAN_Tx_INT_EN_PTR, CAN_Tx_backup.intEn);
        CY_SET_REG32(CAN_Tx_CMD_PTR, CAN_Tx_backup.cmd);
        CY_SET_REG32(CAN_Tx_CFG_PTR, CAN_Tx_backup.cfg);
    #endif /* CY_PSOC3 || CY_PSOC5 */
}


/*******************************************************************************
* Function Name: CAN_Tx_Sleep
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
*  CAN_Tx_backup - Modified when non-retention registers are saved.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void CAN_Tx_Sleep(void) 
{
    #if (!(CY_PSOC3 || CY_PSOC5))
        uint8 i;
    #endif /* (!(CY_PSOC3 || CY_PSOC5)) */

    if (0u != (CY_GET_REG32(CAN_Tx_CMD_PTR) & CAN_Tx_MODE_MASK))
    {
        CAN_Tx_backup.enableState = 1u;
    }
    else /* CAN block is disabled */
    {
        CAN_Tx_backup.enableState = 0u;
    }

    #if (CY_PSOC3 || CY_PSOC5)
        CAN_Tx_SaveConfig();
        (void) CAN_Tx_Stop();
    #else /* CY_PSOC4 */
        /* Abort respective pending TX message requests */
        for (i = 0u; i < CAN_Tx_NUMBER_OF_TX_MAILBOXES; i++)
        {
            CAN_Tx_TX_ABORT_MESSAGE(i);
        }

        /* Sets CAN controller to Stop mode */
        (void) CAN_Tx_Stop();

        /* Clear Global Interrupt enable Flag */
        (void) CAN_Tx_GlobalIntDisable();
    #endif /* CY_PSOC3 || CY_PSOC5 */
}


/*******************************************************************************
* Function Name: CAN_Tx_Wakeup
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
*  CAN_Tx_backup - Used when non-retention registers are restored.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void CAN_Tx_Wakeup(void) 
{
    #if (CY_PSOC3 || CY_PSOC5)
        /* Enable CAN block in Active mode */
        CAN_Tx_PM_ACT_CFG_REG |= CAN_Tx_ACT_PWR_EN;
        /* Enable CAN block in Alternate Active (Standby) mode */
        CAN_Tx_PM_STBY_CFG_REG |= CAN_Tx_STBY_PWR_EN;

        CAN_Tx_RestoreConfig();

        /* Reconfigure Rx and Tx Buffers control registers */
        (void) CAN_Tx_RxTxBuffersConfig();

        if (CAN_Tx_backup.enableState != 0u)
        {
            /* Enable component's operation */
            (void) CAN_Tx_Enable();
        } /* Do nothing if component's block was disabled before */
    #else /* CY_PSOC4 */
        /* Clear all INT_STATUS bits */
        CAN_Tx_INT_SR_REG = CAN_Tx_INT_STATUS_MASK;

        /* Set Global Interrupt enable Flag */
        (void) CAN_Tx_GlobalIntEnable();

        if (CAN_Tx_backup.enableState != 0u)
        {
            (void) CAN_Tx_Enable();
        } /* Do nothing if component's block was disabled before */
    #endif /* CY_PSOC3 || CY_PSOC5 */
}


/* [] END OF FILE */
