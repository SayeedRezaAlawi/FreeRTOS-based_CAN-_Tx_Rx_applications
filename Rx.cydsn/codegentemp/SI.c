/*******************************************************************************
* File Name: SI.c  
* Version 2.10
*
* Description:
*  This file contains API to enable firmware control of a Pins component.
*
* Note:
*
********************************************************************************
* Copyright 2008-2014, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#include "cytypes.h"
#include "SI.h"

/* APIs are not generated for P15[7:6] on PSoC 5 */
#if !(CY_PSOC5A &&\
	 SI__PORT == 15 && ((SI__MASK & 0xC0) != 0))


/*******************************************************************************
* Function Name: SI_Write
********************************************************************************
*
* Summary:
*  Assign a new value to the digital port's data output register.  
*
* Parameters:  
*  prtValue:  The value to be assigned to the Digital Port. 
*
* Return: 
*  None
*  
*******************************************************************************/
void SI_Write(uint8 value) 
{
    uint8 staticBits = (SI_DR & (uint8)(~SI_MASK));
    SI_DR = staticBits | ((uint8)(value << SI_SHIFT) & SI_MASK);
}


/*******************************************************************************
* Function Name: SI_SetDriveMode
********************************************************************************
*
* Summary:
*  Change the drive mode on the pins of the port.
* 
* Parameters:  
*  mode:  Change the pins to one of the following drive modes.
*
*  SI_DM_STRONG     Strong Drive 
*  SI_DM_OD_HI      Open Drain, Drives High 
*  SI_DM_OD_LO      Open Drain, Drives Low 
*  SI_DM_RES_UP     Resistive Pull Up 
*  SI_DM_RES_DWN    Resistive Pull Down 
*  SI_DM_RES_UPDWN  Resistive Pull Up/Down 
*  SI_DM_DIG_HIZ    High Impedance Digital 
*  SI_DM_ALG_HIZ    High Impedance Analog 
*
* Return: 
*  None
*
*******************************************************************************/
void SI_SetDriveMode(uint8 mode) 
{
	CyPins_SetPinDriveMode(SI_0, mode);
}


/*******************************************************************************
* Function Name: SI_Read
********************************************************************************
*
* Summary:
*  Read the current value on the pins of the Digital Port in right justified 
*  form.
*
* Parameters:  
*  None
*
* Return: 
*  Returns the current value of the Digital Port as a right justified number
*  
* Note:
*  Macro SI_ReadPS calls this function. 
*  
*******************************************************************************/
uint8 SI_Read(void) 
{
    return (SI_PS & SI_MASK) >> SI_SHIFT;
}


/*******************************************************************************
* Function Name: SI_ReadDataReg
********************************************************************************
*
* Summary:
*  Read the current value assigned to a Digital Port's data output register
*
* Parameters:  
*  None 
*
* Return: 
*  Returns the current value assigned to the Digital Port's data output register
*  
*******************************************************************************/
uint8 SI_ReadDataReg(void) 
{
    return (SI_DR & SI_MASK) >> SI_SHIFT;
}


/* If Interrupts Are Enabled for this Pins component */ 
#if defined(SI_INTSTAT) 

    /*******************************************************************************
    * Function Name: SI_ClearInterrupt
    ********************************************************************************
    * Summary:
    *  Clears any active interrupts attached to port and returns the value of the 
    *  interrupt status register.
    *
    * Parameters:  
    *  None 
    *
    * Return: 
    *  Returns the value of the interrupt status register
    *  
    *******************************************************************************/
    uint8 SI_ClearInterrupt(void) 
    {
        return (SI_INTSTAT & SI_MASK) >> SI_SHIFT;
    }

#endif /* If Interrupts Are Enabled for this Pins component */ 

#endif /* CY_PSOC5A... */

    
/* [] END OF FILE */
