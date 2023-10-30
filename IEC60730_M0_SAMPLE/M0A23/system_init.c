/* ____  _____             ____    ____    _                                 ______    _________   ____  _    *
* |_   \|_   _|           |_   \  /   _|  (_)                              .' ____ \  |  _   _  | |_   _|     *
*   |   \ | |    __   _     |   \/   |    __    .---.   _ .--.    .--.     | (___ \_| |_/ | | \_|   | |       *
*   | |\ \| |   [  | | |    | |\  /| |   [  |  / /'`\] [ `/'`\] / .'`\ \    _.____`.      | |       | |   _   *
*  _| |_\   |_   | \_/ |,  _| |_\/_| |_   | |  | \__.   | |     | \__. |   | \____) |    _| |_     _| |__/ |  *
* |_____|\____|  '.__.'_/ |_____||_____| [___] '.___.' [___]     '.__.'     \______.'   |_____|   |________|  *
*                                                                                                             *
* @file     system_init.c                                                                                     *
* @version  V1.00                                                                                             *
* $Date: 21/03/09 6:43p $                                                                                     *
* @brief    system initilize                                                                                  *
* @note                                                                                                       *
* SPDX-License-Identifier: Apache-2.0                                                                         *
* @copyright (C) 2016-2020 Nuvoton Technology Corp. All rights reserved.                                      *
***************************************************************************************************************/
#include "stdio.h"
#include "NuMicro.h"
#include "IEC60730_CONTROL_PARAM.h"

void SYS_Init(void)
{	
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init System Clock                                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Set PF multi-function pins for XT1_OUT(PA.4) and XT1_IN(PA.5) */
    SYS->GPA_MFP1 &= ~(SYS_GPA_MFP1_PA4MFP_Msk | SYS_GPA_MFP1_PA5MFP_Msk);
    SYS->GPA_MFP1 |= SYS_GPA_MFP1_PA4MFP_XT1_OUT | SYS_GPA_MFP1_PA5MFP_XT1_IN;
    GPIO_SetMode(PA, BIT4, GPIO_MODE_OUTPUT);
    GPIO_SetMode(PA, BIT5, GPIO_MODE_INPUT);
	
    /* Enable HIRC, HXT and LXT clock */
    CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);
    CLK_EnableXtalRC(CLK_PWRCTL_HXTEN_Msk);
    CLK_EnableXtalRC(CLK_PWRCTL_LXTEN_Msk);
    CLK_EnableXtalRC(CLK_PWRCTL_LIRCEN_Msk);

    /* Wait for HIRC, HXT and LXT clock ready */
    CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk);
    CLK_WaitClockReady(CLK_STATUS_HXTSTB_Msk);
    CLK_WaitClockReady(CLK_STATUS_LXTSTB_Msk);
    CLK_WaitClockReady(CLK_STATUS_LIRCSTB_Msk);

    /* Select HCLK clock source as HIRC and and HCLK clock divider as 1 */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKDIV0_HCLK(1));

    /* Enable perpherial module clock */
    CLK_EnableModuleClock(UART0_MODULE);
    CLK_EnableModuleClock(CRC_MODULE);
    CLK_EnableModuleClock(TMR0_MODULE);
    CLK_EnableModuleClock(WDT_MODULE);
    CLK_EnableModuleClock(WWDT_MODULE);
    CLK_EnableModuleClock(ADC_MODULE);

    /* Select WDT clock source selection from LIRC*/
    CLK_SetModuleClock(WDT_MODULE, CLK_CLKSEL1_WDTSEL_LIRC, 0);

    /* Select WWDT clock source selection from LIRC*/
    CLK_SetModuleClock(WWDT_MODULE, CLK_CLKSEL1_WWDTSEL_LIRC, 0);

    /* Select UART module clock source as HIRC and UART module clock divider as 1 */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_HIRC, CLK_CLKDIV0_UART0(1));
    
    /* Select Timer0 module clock source from HIRC*/	
    CLK_SetModuleClock(TMR0_MODULE, CLK_CLKSEL1_TMR0SEL_HIRC, 0);

    /* Select ADC module clock source from HIRC and set ADC module clock divider as 10 */
    CLK_SetModuleClock(ADC_MODULE, CLK_CLKSEL2_ADCSEL_HIRC, CLK_CLKDIV0_ADC(10));

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Set GPB multi-function pins for UART0 RXD and TXD */
    SYS->GPB_MFP1 &= ~(SYS_GPB_MFP1_PB4MFP_Msk | SYS_GPB_MFP1_PB6MFP_Msk);
    SYS->GPB_MFP1 |= (SYS_GPB_MFP1_PB4MFP_UART0_TXD | SYS_GPB_MFP1_PB6MFP_UART0_RXD);
}

void UART0_Init(void)
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init UART                                                                                               */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Reset UART module */
    SYS_ResetModule(UART0_RST);

    /* Configure UART0 and set UART0 baud rate */
    UART_Open(UART0, 115200);
}
/*-------------------------------------------*/
/*                wdt isr                    */
/*-------------------------------------------*/
void WDT_IRQHandler()
{
    /* for wdt startup test */
    SYS_UnlockReg();
    WDT_CLEAR_TIMEOUT_INT_FLAG();
    SYS_LockReg();
    CLASSB_LSCLOCK_INT();
}

/*-------------------------------------------*/
/*                timer0 isr                 */
/*-------------------------------------------*/
void TMR0_IRQHandler(void)
{
    TIMER_ClearIntFlag(TIMER0);
    CLASSB_HSCLOCK_INT();
}

/*-------------------------------------------*/
/*                init timer0                */
/*-------------------------------------------*/
static void InitTimer0(void)
{
    TIMER_Open(TIMER0, TIMER_PERIODIC_MODE, HSCLOCK_FREQ);

    /* enable TIMER0 Intettupt */
    TIMER_EnableInt(TIMER0);
    NVIC_EnableIRQ(TMR0_IRQn);
    TIMER_Start(TIMER0);
}
/*-------------------------------------------*/
/*  init wdt                                 */
/*-------------------------------------------*/
void InitTimer1_WDT(void)
{
    /* enable wdt interrupt */
    NVIC_EnableIRQ(WDT_IRQn);

    /* unlock register */
    SYS_UnlockReg();

    /* clear WDT */
    WDT_RESET_COUNTER();
    WDT_CLEAR_TIMEOUT_INT_FLAG();
    
    /* set WDT time out period (1024 ticks), enable WDT, and enable WDT interrupt */
    WDT->CTL = WDT_TIMEOUT_2POW10 | WDT_CTL_WDTEN_Msk | WDT_CTL_INTEN_Msk | (1 << WDT_CTL_RSTEN_Pos);
    WDT->ALTCTL = WDT_RESET_DELAY_1026CLK;

    /* lock register */
    SYS_LockReg();
}

/*---------------------------------------------------------------------------------------------------------*/
/*  Systen Initilization Functions                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int32_t system_init(void)
{
    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Init System, peripheral clock and multi-function I/O */
    SYS_Init();

    /* Lock protected registers */
    SYS_LockReg();

    /* Init UART0 for printf */
    UART0_Init();
    
    /* enable Brown Out Detector and LVR reset */
    SYS_UnlockReg();
    SYS -> BODCTL |= (SYS_BODCTL_BODRSTEN_Msk | SYS_BODCTL_LVREN_Msk);
    SYS_LockReg();

    InitTimer0();
    InitTimer1_WDT();

    return 0;
}
