/*==================================================================================================
*   Project              : RTD AUTOSAR 4.7
*   Platform             : CORTEXM
*   Peripheral           : SIUL2
*   Dependencies         : none
*
*   Autosar Version      : 4.7.0
*   Autosar Revision     : ASR_REL_4_7_REV_0000
*   Autosar Conf.Variant :
*   SW Version           : 6.0.0
*   Build Version        : S32K3_RTD_6_0_0_D2506_ASR_REL_4_7_REV_0000_20250610
*
*   Copyright 2020 - 2025 NXP
*
*   NXP Confidential and Proprietary. This software is owned or controlled by NXP and may only be
*   used strictly in accordance with the applicable license terms. By expressly
*   accepting such terms or by downloading, installing, activating and/or otherwise
*   using the software, you are agreeing that you have read, and that you agree to
*   comply with and are bound by, such license terms. If you do not agree to be
*   bound by the applicable license terms, then you may not retain, install,
*   activate or otherwise use the software.
==================================================================================================*/

/**
*   @file    Siul2_Port_Ip_PBcfg.c
*
*   @addtogroup Port_CFG
*   @{
*/

#ifdef __cplusplus
extern "C"{
#endif

/*==================================================================================================
*                                          INCLUDE FILES
* 1) system and project includes
* 2) needed interfaces from external units
* 3) internal and external interfaces from this unit
==================================================================================================*/
#include "Siul2_Port_Ip.h"

/*==================================================================================================
*                                 SOURCE FILE VERSION INFORMATION
==================================================================================================*/
#define SIUL2_PORT_IP_VENDOR_ID_PBCFG_C                       43
#define SIUL2_PORT_IP_AR_RELEASE_MAJOR_VERSION_PBCFG_C        4
#define SIUL2_PORT_IP_AR_RELEASE_MINOR_VERSION_PBCFG_C        7
#define SIUL2_PORT_IP_AR_RELEASE_REVISION_VERSION_PBCFG_C     0
#define SIUL2_PORT_IP_SW_MAJOR_VERSION_PBCFG_C                6
#define SIUL2_PORT_IP_SW_MINOR_VERSION_PBCFG_C                0
#define SIUL2_PORT_IP_SW_PATCH_VERSION_PBCFG_C                0

/*==================================================================================================
*                                       FILE VERSION CHECKS
==================================================================================================*/
/* Check if Siul2_Port_Ip_PBcfg.c and Siul2_Port_Ip.h are of the same vendor */
#if (SIUL2_PORT_IP_VENDOR_ID_PBCFG_C != SIUL2_PORT_IP_VENDOR_ID_H)
    #error "Siul2_Port_Ip_PBcfg.c and Siul2_Port_Ip.h have different vendor ids"
#endif
/* Check if Siul2_Port_Ip_PBcfg.c and Siul2_Port_Ip.h are of the same Autosar version */
#if ((SIUL2_PORT_IP_AR_RELEASE_MAJOR_VERSION_PBCFG_C    != SIUL2_PORT_IP_AR_RELEASE_MAJOR_VERSION_H) || \
     (SIUL2_PORT_IP_AR_RELEASE_MINOR_VERSION_PBCFG_C    != SIUL2_PORT_IP_AR_RELEASE_MINOR_VERSION_H) || \
     (SIUL2_PORT_IP_AR_RELEASE_REVISION_VERSION_PBCFG_C != SIUL2_PORT_IP_AR_RELEASE_REVISION_VERSION_H) \
    )
    #error "AutoSar Version Numbers of Siul2_Port_Ip_PBcfg.c and Siul2_Port_Ip.h are different"
#endif

/* Check if Siul2_Port_Ip_PBcfg.c and Siul2_Port_Ip.h are of the same software version */
#if ((SIUL2_PORT_IP_SW_MAJOR_VERSION_PBCFG_C != SIUL2_PORT_IP_SW_MAJOR_VERSION_H) || \
     (SIUL2_PORT_IP_SW_MINOR_VERSION_PBCFG_C != SIUL2_PORT_IP_SW_MINOR_VERSION_H) || \
     (SIUL2_PORT_IP_SW_PATCH_VERSION_PBCFG_C != SIUL2_PORT_IP_SW_PATCH_VERSION_H)    \
    )
    #error "Software Version Numbers of Siul2_Port_Ip_PBcfg.c and Siul2_Port_Ip.h are different"
#endif

/*==================================================================================================
*                           LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/


/*==================================================================================================
*                                          LOCAL MACROS
==================================================================================================*/


/*==================================================================================================
*                                         LOCAL CONSTANTS
==================================================================================================*/

/*==================================================================================================
*                                         LOCAL VARIABLES
==================================================================================================*/

/*==================================================================================================
*                                        GLOBAL CONSTANTS
==================================================================================================*/

/*==================================================================================================
*                                        GLOBAL VARIABLES
==================================================================================================*/
#define PORT_START_SEC_CONFIG_DATA_UNSPECIFIED
#include "Port_MemMap.h"


const Siul2_Port_Ip_PinSettingsConfig g_pin_mux_InitConfigArr[NUM_OF_CONFIGURED_PINS] = 
{
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 224,
        .mux                   = PORT_MUX_ALT1,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_DISABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_ENABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   0
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 225,
        .mux                   = PORT_MUX_AS_GPIO,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_ENABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_DISABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   417
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_ALT4,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 226,
        .mux                   = PORT_MUX_ALT1,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_DISABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_ENABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   0
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 227,
        .mux                   = PORT_MUX_AS_GPIO,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_ENABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_DISABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   418
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_ALT4,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 231,
        .mux                   = PORT_MUX_AS_GPIO,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_ENABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_DISABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   189
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_ALT7,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 232,
        .mux                   = PORT_MUX_ALT2,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_DISABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_ENABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   0
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 4,
        .mux                   = PORT_MUX_ALT7,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_UP_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_ENABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_ENABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   186
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_ALT0,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 5,
        .mux                   = PORT_MUX_AS_GPIO,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_DISABLED,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_ENABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_DISABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   0
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 7,
        .mux                   = PORT_MUX_ALT4,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_DISABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_ENABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   0
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 6,
        .mux                   = PORT_MUX_AS_GPIO,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_NOT_AVAILABLE,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_ENABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_DISABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   0
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_ALT2,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 8,
        .mux                   = PORT_MUX_AS_GPIO,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_NOT_AVAILABLE,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_ENABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_DISABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   0
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 10,
        .mux                   = PORT_MUX_ALT7,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_ENABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_FASTEST,
        .inputBuffer           = PORT_INPUT_BUFFER_DISABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_ENABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   0
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 18,
        .mux                   = PORT_MUX_ALT3,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_NOT_AVAILABLE,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_DISABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_ENABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   0
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 19,
        .mux                   = PORT_MUX_AS_GPIO,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_NOT_AVAILABLE,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_ENABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_DISABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   188
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_ALT5,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 24,
        .mux                   = PORT_MUX_AS_GPIO,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_DISABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_NOT_AVAILABLE,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 2,
        .inputMuxReg           = {
                                   0
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 25,
        .mux                   = PORT_MUX_AS_GPIO,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_ENABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_NOT_AVAILABLE,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 2,
        .inputMuxReg           = {
                                   0
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 28,
        .mux                   = PORT_MUX_ALT8,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_DISABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_ENABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   0
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 29,
        .mux                   = PORT_MUX_ALT8,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_ENABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_ENABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   454
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_ALT1,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 32,
        .mux                   = PORT_MUX_AS_GPIO,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_ENABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_DISABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   187
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_ALT2,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 33,
        .mux                   = PORT_MUX_ALT2,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_DISABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_ENABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   0
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 34,
        .mux                   = PORT_MUX_AS_GPIO,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_DISABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_ENABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   0
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 35,
        .mux                   = PORT_MUX_ALT8,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_FASTEST,
        .inputBuffer           = PORT_INPUT_BUFFER_DISABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_ENABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   0
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 36,
        .mux                   = PORT_MUX_AS_GPIO,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_ENABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_DISABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   345
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_ALT1,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 41,
        .mux                   = PORT_MUX_AS_GPIO,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_NOT_AVAILABLE,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_ENABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_DISABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   196
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_ALT1,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 42,
        .mux                   = PORT_MUX_ALT5,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_NOT_AVAILABLE,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_DISABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_ENABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   0
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 46,
        .mux                   = PORT_MUX_AS_GPIO,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_NOT_AVAILABLE,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_ENABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_DISABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   194
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_ALT1,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 47,
        .mux                   = PORT_MUX_ALT6,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_NOT_AVAILABLE,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_DISABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_ENABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   0
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 48,
        .mux                   = PORT_MUX_ALT4,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_NOT_AVAILABLE,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_DISABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_ENABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   0
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 49,
        .mux                   = PORT_MUX_AS_GPIO,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_NOT_AVAILABLE,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_ENABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_DISABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   191
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_ALT3,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 54,
        .mux                   = PORT_MUX_AS_GPIO,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_ENABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_DISABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   300
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_ALT7,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 55,
        .mux                   = PORT_MUX_AS_GPIO,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_ENABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_DISABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   301
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_ALT5,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 56,
        .mux                   = PORT_MUX_AS_GPIO,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_ENABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_DISABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   302
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_ALT3,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 57,
        .mux                   = PORT_MUX_ALT6,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_FASTEST,
        .inputBuffer           = PORT_INPUT_BUFFER_DISABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_ENABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   0
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 60,
        .mux                   = PORT_MUX_ALT2,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_FASTEST,
        .inputBuffer           = PORT_INPUT_BUFFER_DISABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_ENABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   0
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 61,
        .mux                   = PORT_MUX_ALT2,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_FASTEST,
        .inputBuffer           = PORT_INPUT_BUFFER_DISABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_ENABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   0
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 64,
        .mux                   = PORT_MUX_ALT8,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_FASTEST,
        .inputBuffer           = PORT_INPUT_BUFFER_DISABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_ENABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   0
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 65,
        .mux                   = PORT_MUX_ALT8,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_FASTEST,
        .inputBuffer           = PORT_INPUT_BUFFER_DISABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_ENABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   0
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 66,
        .mux                   = PORT_MUX_AS_GPIO,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_SLOWEST,
        .inputBuffer           = PORT_INPUT_BUFFER_ENABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_DISABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   451
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_ALT1,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 68,
        .mux                   = PORT_MUX_AS_GPIO,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_DOWN_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_ENABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_DISABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   184
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_ALT0,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 69,
        .mux                   = PORT_MUX_AS_GPIO,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_UP_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_ENABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_DISABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   185
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_ALT0,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 72,
        .mux                   = PORT_MUX_ALT3,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_DISABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_ENABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   0
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 73,
        .mux                   = PORT_MUX_AS_GPIO,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_ENABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_DISABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   1
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_ALT1,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 74,
        .mux                   = PORT_MUX_ALT3,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_DISABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_ENABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   0
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 75,
        .mux                   = PORT_MUX_AS_GPIO,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_NOT_AVAILABLE,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_ENABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_DISABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   5
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_ALT2,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 78,
        .mux                   = PORT_MUX_AS_GPIO,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_ENABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_DISABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   294
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_ALT4,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 79,
        .mux                   = PORT_MUX_AS_GPIO,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_ENABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_DISABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   295
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_ALT6,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 80,
        .mux                   = PORT_MUX_AS_GPIO,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_SLOWEST,
        .inputBuffer           = PORT_INPUT_BUFFER_ENABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_DISABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   292
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_ALT6,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 81,
        .mux                   = PORT_MUX_ALT8,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_FASTEST,
        .inputBuffer           = PORT_INPUT_BUFFER_DISABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_ENABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   0
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 82,
        .mux                   = PORT_MUX_ALT5,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_FASTEST,
        .inputBuffer           = PORT_INPUT_BUFFER_DISABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_ENABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   0
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 83,
        .mux                   = PORT_MUX_ALT8,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_FASTEST,
        .inputBuffer           = PORT_INPUT_BUFFER_DISABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_ENABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   0
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 92,
        .mux                   = PORT_MUX_ALT1,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_DISABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_ENABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   0
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 93,
        .mux                   = PORT_MUX_AS_GPIO,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_ENABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_DISABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   3
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_ALT3,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 96,
        .mux                   = PORT_MUX_ALT3,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_NOT_AVAILABLE,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_DISABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_ENABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   0
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 97,
        .mux                   = PORT_MUX_ALT3,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_NOT_AVAILABLE,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_DISABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_ENABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   0
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 98,
        .mux                   = PORT_MUX_ALT6,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_NOT_AVAILABLE,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_DISABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_ENABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   0
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 99,
        .mux                   = PORT_MUX_AS_GPIO,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_NOT_AVAILABLE,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_ENABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_DISABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   190
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_ALT3,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 101,
        .mux                   = PORT_MUX_AS_GPIO,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_SLOWEST,
        .inputBuffer           = PORT_INPUT_BUFFER_ENABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_DISABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   448
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_ALT1,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 102,
        .mux                   = PORT_MUX_AS_GPIO,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_SLOWEST,
        .inputBuffer           = PORT_INPUT_BUFFER_ENABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_DISABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   449
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_ALT1,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 103,
        .mux                   = PORT_MUX_AS_GPIO,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_SLOWEST,
        .inputBuffer           = PORT_INPUT_BUFFER_ENABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_DISABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   453
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_ALT1,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 104,
        .mux                   = PORT_MUX_ALT8,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_FASTEST,
        .inputBuffer           = PORT_INPUT_BUFFER_DISABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_ENABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   0
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 105,
        .mux                   = PORT_MUX_ALT8,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_FASTEST,
        .inputBuffer           = PORT_INPUT_BUFFER_DISABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_ENABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   0
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 106,
        .mux                   = PORT_MUX_AS_GPIO,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_SLOWEST,
        .inputBuffer           = PORT_INPUT_BUFFER_ENABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_DISABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   450
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_ALT1,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 107,
        .mux                   = PORT_MUX_ALT8,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_SLOWEST,
        .inputBuffer           = PORT_INPUT_BUFFER_DISABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_ENABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   0
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 108,
        .mux                   = PORT_MUX_AS_GPIO,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_SLOWEST,
        .inputBuffer           = PORT_INPUT_BUFFER_ENABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_DISABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   452
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_ALT1,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 109,
        .mux                   = PORT_MUX_ALT4,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_ENABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_ENABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   214
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_ALT2,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 110,
        .mux                   = PORT_MUX_ALT4,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_DISABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_ENABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   0
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 119,
        .mux                   = PORT_MUX_AS_GPIO,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_NOT_AVAILABLE,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_ENABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_DISABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   0
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 120,
        .mux                   = PORT_MUX_AS_GPIO,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_NOT_AVAILABLE,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_ENABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_DISABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   0
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 121,
        .mux                   = PORT_MUX_AS_GPIO,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_NOT_AVAILABLE,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_ENABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_DISABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   0
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 128,
        .mux                   = PORT_MUX_AS_GPIO,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_NOT_AVAILABLE,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_ENABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_DISABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   230
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_ALT1,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 129,
        .mux                   = PORT_MUX_ALT2,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_NOT_AVAILABLE,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_DISABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_ENABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   0
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 130,
        .mux                   = PORT_MUX_ALT2,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_NOT_AVAILABLE,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_DISABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_ENABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   0
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 131,
        .mux                   = PORT_MUX_AS_GPIO,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_ENABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_DISABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   192
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_ALT1,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 138,
        .mux                   = PORT_MUX_AS_GPIO,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_NOT_AVAILABLE,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_ENABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_DISABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   253
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_ALT1,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 142,
        .mux                   = PORT_MUX_ALT4,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_NOT_AVAILABLE,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_DISABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_ENABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   0
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 143,
        .mux                   = PORT_MUX_AS_GPIO,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_NOT_AVAILABLE,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_DISABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_ENABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   0
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 144,
        .mux                   = PORT_MUX_AS_GPIO,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_NOT_AVAILABLE,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_DISABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_ENABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   0
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 152,
        .mux                   = PORT_MUX_ALT3,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_DISABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_ENABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   0
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 153,
        .mux                   = PORT_MUX_AS_GPIO,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_NOT_AVAILABLE,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_ENABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_DISABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   2
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_ALT3,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 158,
        .mux                   = PORT_MUX_ALT1,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_DISABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_ENABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   0
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 159,
        .mux                   = PORT_MUX_AS_GPIO,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_NOT_AVAILABLE,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_ENABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_DISABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   4
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_ALT4,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 162,
        .mux                   = PORT_MUX_ALT1,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_NOT_AVAILABLE,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_DISABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_ENABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   0
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 163,
        .mux                   = PORT_MUX_AS_GPIO,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_NOT_AVAILABLE,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_ENABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_DISABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   193
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_ALT3,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 180,
        .mux                   = PORT_MUX_ALT1,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_DISABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_ENABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   0
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 181,
        .mux                   = PORT_MUX_AS_GPIO,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_ENABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_DISABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   195
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_ALT3,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 186,
        .mux                   = PORT_MUX_AS_GPIO,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_NOT_AVAILABLE,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_DISABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_ENABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   0
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 187,
        .mux                   = PORT_MUX_AS_GPIO,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_NOT_AVAILABLE,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_DISABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_ENABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   0
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 188,
        .mux                   = PORT_MUX_ALT5,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_DISABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_ENABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   0
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 189,
        .mux                   = PORT_MUX_ALT5,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_DISABLED,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_DISABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_ENABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   0
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 112,
        .mux                   = PORT_MUX_AS_GPIO,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_NOT_AVAILABLE,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_ENABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_ENABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   0
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    },
    {
        .base                  = IP_SIUL2,
        .pinPortIdx            = 113,
        .mux                   = PORT_MUX_AS_GPIO,
        .safeMode              = PORT_SAFE_MODE_DISABLED,
        .pullConfig            = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveStrength         = PORT_DRIVE_STRENTGTH_NOT_AVAILABLE,
        .inputFilter           = PORT_INPUT_FILTER_NOT_AVAILABLE,
        .pullKeep              = PORT_PULL_KEEP_DISABLED,
        .invert                = PORT_INVERT_DISABLED,
        .slewRateCtrlSel       = PORT_SLEW_RATE_NOT_AVAILABLE,
        .inputBuffer           = PORT_INPUT_BUFFER_ENABLED,
        .outputBuffer          = PORT_OUTPUT_BUFFER_ENABLED,
        .adcInterleaves        = {
                                   MUX_MODE_NOT_AVAILABLE,
                                   MUX_MODE_NOT_AVAILABLE
                                 },
        .initValue             = 1,
        .inputMuxReg           = {
                                   0
                                 },
        .inputMux              = {
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT,
                                   PORT_INPUT_MUX_NO_INIT
                                 }
    }
};
#define PORT_STOP_SEC_CONFIG_DATA_UNSPECIFIED
#include "Port_MemMap.h"

/*==================================================================================================
*                                        GLOBAL CONSTANTS
==================================================================================================*/

/*==================================================================================================
*                                        GLOBAL VARIABLES
==================================================================================================*/

/*==================================================================================================
*                                    LOCAL FUNCTION PROTOTYPES
==================================================================================================*/

/*==================================================================================================
*                                         LOCAL FUNCTIONS
==================================================================================================*/

/*==================================================================================================
*                                        GLOBAL FUNCTIONS
==================================================================================================*/

#ifdef __cplusplus
}
#endif

/** @} */

