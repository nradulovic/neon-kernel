/*
 * This file is part of nKernel.
 *
 * Copyright (C) 2010 - 2014 Nenad Radulovic
 *
 * nKernel is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * nKernel is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with nKernel.  If not, see <http://www.gnu.org/licenses/>.
 *
 * web site:    http://github.com/nradulovic
 * e-mail  :    nenad.b.radulovic@gmail.com
 */

#ifndef ES_CORTEX_M3_H_
#define ES_CORTEX_M3_H_

/*=========================================================  INCLUDE FILES  ==*/

#include <stdint.h>

/*===============================================================  MACRO's  ==*/

#define PORT_COREDEBUG_BASE             (0xe000edf0ul)

#define PORT_COREDEBUG                  ((struct portCoreDebug *)PORT_COREDEBUG_BASE)

/*--  Debug Halting Control and Status Register  -----------------------------*/
#define PORT_COREDEBUG_DHCSR_DBGKEY_Pos         16
#define PORT_COREDEBUG_DHCSR_DBGKEY_Msk         (0xfffful << PORT_COREDEBUG_DHCSR_DBGKEY_Pos)

#define PORT_COREDEBUG_DHCSR_S_RESET_ST_Pos     25
#define PORT_COREDEBUG_DHCSR_S_RESET_ST_Msk     (1ul << PORT_COREDEBUG_DHCSR_S_RESET_ST_Pos)

#define PORT_COREDEBUG_DHCSR_S_RETIRE_ST_Pos    24
#define PORT_COREDEBUG_DHCSR_S_RETIRE_ST_Msk    (1ul << PORT_COREDEBUG_DHCSR_S_RETIRE_ST_Pos)

#define PORT_COREDEBUG_DHCSR_S_LOCKUP_Pos       19
#define PORT_COREDEBUG_DHCSR_S_LOCKUP_Msk       (1ul << PORT_COREDEBUG_DHCSR_S_LOCKUP_Pos)

#define PORT_COREDEBUG_DHCSR_S_SLEEP_Pos        18
#define PORT_COREDEBUG_DHCSR_S_SLEEP_Msk        (1ul << PORT_COREDEBUG_DHCSR_S_SLEEP_Pos)

#define PORT_COREDEBUG_DHCSR_S_HALT_Pos         17
#define PORT_COREDEBUG_DHCSR_S_HALT_Msk         (1ul << PORT_COREDEBUG_DHCSR_S_HALT_Pos)

#define PORT_COREDEBUG_DHCSR_S_REGRDY_Pos       16
#define PORT_COREDEBUG_DHCSR_S_REGRDY_Msk       (1ul << PORT_COREDEBUG_DHCSR_S_REGRDY_Pos)

#define PORT_COREDEBUG_DHCSR_C_SNAPSTALL_Pos    5
#define PORT_COREDEBUG_DHCSR_C_SNAPSTALL_Msk    (1ul << PORT_COREDEBUG_DHCSR_C_SNAPSTALL_Pos)

#define PORT_COREDEBUG_DHCSR_C_MASKINTS_Pos     3
#define PORT_COREDEBUG_DHCSR_C_MASKINTS_Msk     (1ul << PORT_COREDEBUG_DHCSR_C_MASKINTS_Pos)

#define PORT_COREDEBUG_DHCSR_C_STEP_Pos         2
#define PORT_COREDEBUG_DHCSR_C_STEP_Msk         (1ul << PORT_COREDEBUG_DHCSR_C_STEP_Pos)

#define PORT_COREDEBUG_DHCSR_C_HALT_Pos         1
#define PORT_COREDEBUG_DHCSR_C_HALT_Msk         (1ul << PORT_COREDEBUG_DHCSR_C_HALT_Pos)

#define PORT_COREDEBUG_DHCSR_C_DEBUGEN_Pos      0
#define PORT_COREDEBUG_DHCSR_C_DEBUGEN_Msk      (1ul << PORT_COREDEBUG_DHCSR_C_DEBUGEN_Pos)

/*--  Debug Core Register Selector Register  ---------------------------------*/
#define PORT_COREDEBUG_DCRSR_REGWNR_Pos         16
#define PORT_COREDEBUG_DCRSR_REGWNR_Msk         (1ul << PORT_COREDEBUG_DCRSR_REGWNR_Pos)

#define PORT_COREDEBUG_DCRSR_REGSEL_Pos          0
#define PORT_COREDEBUG_DCRSR_REGSEL_Msk         (0x1ful << PORT_COREDEBUG_DCRSR_REGSEL_Pos)

/*--  Debug Exception and Monitor Control Register  --------------------------*/
#define PORT_COREDEBUG_DEMCR_TRCENA_Pos         24
#define PORT_COREDEBUG_DEMCR_TRCENA_Msk         (1ul << PORT_COREDEBUG_DEMCR_TRCENA_Pos)

#define PORT_COREDEBUG_DEMCR_MON_REQ_Pos        19
#define PORT_COREDEBUG_DEMCR_MON_REQ_Msk        (1ul << PORT_COREDEBUG_DEMCR_MON_REQ_Pos)

#define PORT_COREDEBUG_DEMCR_MON_STEP_Pos       18
#define PORT_COREDEBUG_DEMCR_MON_STEP_Msk       (1ul << PORT_COREDEBUG_DEMCR_MON_STEP_Pos)

#define PORT_COREDEBUG_DEMCR_MON_PEND_Pos       17
#define PORT_COREDEBUG_DEMCR_MON_PEND_Msk       (1ul << PORT_COREDEBUG_DEMCR_MON_PEND_Pos)

#define PORT_COREDEBUG_DEMCR_MON_EN_Pos         16
#define PORT_COREDEBUG_DEMCR_MON_EN_Msk         (1ul << PORT_COREDEBUG_DEMCR_MON_EN_Pos)

#define PORT_COREDEBUG_DEMCR_VC_HARDERR_Pos     10
#define PORT_COREDEBUG_DEMCR_VC_HARDERR_Msk     (1ul << PORT_COREDEBUG_DEMCR_VC_HARDERR_Pos)

#define PORT_COREDEBUG_DEMCR_VC_INTERR_Pos      9
#define PORT_COREDEBUG_DEMCR_VC_INTERR_Msk      (1ul << PORT_COREDEBUG_DEMCR_VC_INTERR_Pos)

#define PORT_COREDEBUG_DEMCR_VC_BUSERR_Pos      8
#define PORT_COREDEBUG_DEMCR_VC_BUSERR_Msk      (1ul << PORT_COREDEBUG_DEMCR_VC_BUSERR_Pos)

#define PORT_COREDEBUG_DEMCR_VC_STATERR_Pos     7
#define PORT_COREDEBUG_DEMCR_VC_STATERR_Msk     (1ul << PORT_COREDEBUG_DEMCR_VC_STATERR_Pos)

#define PORT_COREDEBUG_DEMCR_VC_CHKERR_Pos      6
#define PORT_COREDEBUG_DEMCR_VC_CHKERR_Msk      (1ul << PORT_COREDEBUG_DEMCR_VC_CHKERR_Pos)

#define PORT_COREDEBUG_DEMCR_VC_NOCPERR_Pos     5
#define PORT_COREDEBUG_DEMCR_VC_NOCPERR_Msk     (1ul << PORT_COREDEBUG_DEMCR_VC_NOCPERR_Pos)

#define PORT_COREDEBUG_DEMCR_VC_MMERR_Pos       4
#define PORT_COREDEBUG_DEMCR_VC_MMERR_Msk       (1ul << PORT_COREDEBUG_DEMCR_VC_MMERR_Pos)

#define PORT_COREDEBUG_DEMCR_VC_CORERESET_Pos   0
#define PORT_COREDEBUG_DEMCR_VC_CORERESET_Msk   (1ul << PORT_COREDEBUG_DEMCR_VC_CORERESET_Pos)

#define PORT_DWT_BASE                   (0xe0001000ul)

#define PORT_DWT                        ((struct portDWT *)PORT_DWT_BASE)

/*--  DWT Control Register Definitions  --------------------------------------*/
#define PORT_DWT_CTRL_NUMCOMP_Pos               28
#define PORT_DWT_CTRL_NUMCOMP_Msk               (0xful << PORT_DWT_CTRL_NUMCOMP_Pos)

#define PORT_DWT_CTRL_NOTRCPKT_Pos              27
#define PORT_DWT_CTRL_NOTRCPKT_Msk              (0x1ul << PORT_DWT_CTRL_NOTRCPKT_Pos)

#define PORT_DWT_CTRL_NOEXTTRIG_Pos             26
#define PORT_DWT_CTRL_NOEXTTRIG_Msk             (0x1ul << PORT_DWT_CTRL_NOEXTTRIG_Pos)

#define PORT_DWT_CTRL_NOCYCCNT_Pos              25
#define PORT_DWT_CTRL_NOCYCCNT_Msk              (0x1ul << PORT_DWT_CTRL_NOCYCCNT_Pos)

#define PORT_DWT_CTRL_NOPRFCNT_Pos              24
#define PORT_DWT_CTRL_NOPRFCNT_Msk              (0x1ul << PORT_DWT_CTRL_NOPRFCNT_Pos)

#define PORT_DWT_CTRL_CYCEVTENA_Pos             22
#define PORT_DWT_CTRL_CYCEVTENA_Msk             (0x1ul << PORT_DWT_CTRL_CYCEVTENA_Pos)

#define PORT_DWT_CTRL_FOLDEVTENA_Pos            21
#define PORT_DWT_CTRL_FOLDEVTENA_Msk            (0x1ul << PORT_DWT_CTRL_FOLDEVTENA_Pos)

#define PORT_DWT_CTRL_LSUEVTENA_Pos             20
#define PORT_DWT_CTRL_LSUEVTENA_Msk             (0x1ul << PORT_DWT_CTRL_LSUEVTENA_Pos)

#define PORT_DWT_CTRL_SLEEPEVTENA_Pos           19
#define PORT_DWT_CTRL_SLEEPEVTENA_Msk           (0x1ul << PORT_DWT_CTRL_SLEEPEVTENA_Pos)

#define PORT_DWT_CTRL_EXCEVTENA_Pos             18
#define PORT_DWT_CTRL_EXCEVTENA_Msk             (0x1ul << PORT_DWT_CTRL_EXCEVTENA_Pos)

#define PORT_DWT_CTRL_CPIEVTENA_Pos             17
#define PORT_DWT_CTRL_CPIEVTENA_Msk             (0x1ul << PORT_DWT_CTRL_CPIEVTENA_Pos)

#define PORT_DWT_CTRL_EXCTRCENA_Pos             16
#define PORT_DWT_CTRL_EXCTRCENA_Msk             (0x1ul << PORT_DWT_CTRL_EXCTRCENA_Pos)

#define PORT_DWT_CTRL_PCSAMPLENA_Pos            12
#define PORT_DWT_CTRL_PCSAMPLENA_Msk            (0x1ul << PORT_DWT_CTRL_PCSAMPLENA_Pos)

#define PORT_DWT_CTRL_SYNCTAP_Pos               10
#define PORT_DWT_CTRL_SYNCTAP_Msk               (0x3ul << PORT_DWT_CTRL_SYNCTAP_Pos)

#define PORT_DWT_CTRL_CYCTAP_Pos                9
#define PORT_DWT_CTRL_CYCTAP_Msk                (0x1ul << PORT_DWT_CTRL_CYCTAP_Pos)

#define PORT_DWT_CTRL_POSTINIT_Pos              5
#define PORT_DWT_CTRL_POSTINIT_Msk              (0xful << PORT_DWT_CTRL_POSTINIT_Pos)

#define PORT_DWT_CTRL_POSTPRESET_Pos            1
#define PORT_DWT_CTRL_POSTPRESET_Msk            (0xful << PORT_DWT_CTRL_POSTPRESET_Pos)

#define PORT_DWT_CTRL_CYCCNTENA_Pos             0
#define PORT_DWT_CTRL_CYCCNTENA_Msk             (0x1ul << PORT_DWT_CTRL_CYCCNTENA_Pos)

/*--  DWT CPI Count Register Definitions  ------------------------------------*/
#define PORT_DWT_CPICNT_CPICNT_Pos              0
#define PORT_DWT_CPICNT_CPICNT_Msk              (0xfful << PORT_DWT_CPICNT_CPICNT_Pos)

/*--  DWT Exception Overhead Count Register Definitions  ---------------------*/
#define PORT_DWT_EXCCNT_EXCCNT_Pos              0
#define PORT_DWT_EXCCNT_EXCCNT_Msk              (0xfful << PORT_DWT_EXCCNT_EXCCNT_Pos)

/*--  DWT Sleep Count Register Definitions  ----------------------------------*/
#define PORT_DWT_SLEEPCNT_SLEEPCNT_Pos          0
#define PORT_DWT_SLEEPCNT_SLEEPCNT_Msk          (0xfful << PORT_DWT_SLEEPCNT_SLEEPCNT_Pos)

/*--  DWT LSU Count Register Definitions  ------------------------------------*/
#define PORT_DWT_LSUCNT_LSUCNT_Pos              0
#define PORT_DWT_LSUCNT_LSUCNT_Msk              (0xfful << PORT_DWT_LSUCNT_LSUCNT_Pos)

/*--  DWT Folded-instruction Count Register Definitions  ---------------------*/
#define PORT_DWT_FOLDCNT_FOLDCNT_Pos            0
#define PORT_DWT_FOLDCNT_FOLDCNT_Msk            (0xfful << PORT_DWT_FOLDCNT_FOLDCNT_Pos)

/*--  DWT Comparator Mask Register Definitions  ------------------------------*/
#define PORT_DWT_MASK_MASK_Pos                   0
#define PORT_DWT_MASK_MASK_Msk                  (0x1ful << PORT_DWT_MASK_MASK_Pos)

/*--  DWT Comparator Function Register Definitions  --------------------------*/
#define PORT_DWT_FUNCTION_MATCHED_Pos           24
#define PORT_DWT_FUNCTION_MATCHED_Msk           (0x1ul << PORT_DWT_FUNCTION_MATCHED_Pos)

#define PORT_DWT_FUNCTION_DATAVADDR1_Pos        16
#define PORT_DWT_FUNCTION_DATAVADDR1_Msk        (0xful << PORT_DWT_FUNCTION_DATAVADDR1_Pos)

#define PORT_DWT_FUNCTION_DATAVADDR0_Pos        12
#define PORT_DWT_FUNCTION_DATAVADDR0_Msk        (0xful << PORT_DWT_FUNCTION_DATAVADDR0_Pos)

#define PORT_DWT_FUNCTION_DATAVSIZE_Pos         10
#define PORT_DWT_FUNCTION_DATAVSIZE_Msk         (0x3ul << PORT_DWT_FUNCTION_DATAVSIZE_Pos)

#define PORT_DWT_FUNCTION_LNK1ENA_Pos           9
#define PORT_DWT_FUNCTION_LNK1ENA_Msk           (0x1ul << PORT_DWT_FUNCTION_LNK1ENA_Pos)

#define PORT_DWT_FUNCTION_DATAVMATCH_Pos        8
#define PORT_DWT_FUNCTION_DATAVMATCH_Msk        (0x1ul << PORT_DWT_FUNCTION_DATAVMATCH_Pos)

#define PORT_DWT_FUNCTION_CYCMATCH_Pos          7
#define PORT_DWT_FUNCTION_CYCMATCH_Msk          (0x1ul << PORT_DWT_FUNCTION_CYCMATCH_Pos)

#define PORT_DWT_FUNCTION_EMITRANGE_Pos         5
#define PORT_DWT_FUNCTION_EMITRANGE_Msk         (0x1ul << PORT_DWT_FUNCTION_EMITRANGE_Pos)

#define PORT_DWT_FUNCTION_FUNCTION_Pos          0
#define PORT_DWT_FUNCTION_FUNCTION_Msk          (0xful << PORT_DWT_FUNCTION_FUNCTION_Pos)

#define PORT_ITM_BASE                   (0xE0000000ul)

#define PORT_ITM                        ((struct portITM *)PORT_ITM_BASE)

/*--  ITM Trace Privilege Register Definitions  ------------------------------*/
#define PORT_ITM_TPR_PRIVMASK_Pos                0
#define PORT_ITM_TPR_PRIVMASK_Msk               (0xful << PORT_ITM_TPR_PRIVMASK_Pos)

/*--  ITM Trace Control Register Definitions  --------------------------------*/
#define PORT_ITM_TCR_BUSY_Pos                   23
#define PORT_ITM_TCR_BUSY_Msk                   (1ul << PORT_ITM_TCR_BUSY_Pos)

#define PORT_ITM_TCR_TRACEBUSID_Pos             16
#define PORT_ITM_TCR_TRACEBUSID_Msk             (0x7ful << PORT_ITM_TCR_TRACEBUSID_Pos)

#define PORT_ITM_TCR_GTSFREQ_Pos                10
#define PORT_ITM_TCR_GTSFREQ_Msk                (3ul << PORT_ITM_TCR_GTSFREQ_Pos)

#define PORT_ITM_TCR_TSPRESCALE_Pos             8
#define PORT_ITM_TCR_TSPRESCALE_Msk             (3ul << PORT_ITM_TCR_TSPRESCALE_Pos)

#define PORT_ITM_TCR_SWOENA_Pos                 4
#define PORT_ITM_TCR_SWOENA_Msk                 (1ul << PORT_ITM_TCR_SWOENA_Pos)

#define PORT_ITM_TCR_DWTENA_Pos                 3
#define PORT_ITM_TCR_DWTENA_Msk                 (1ul << PORT_ITM_TCR_DWTENA_Pos)

#define PORT_ITM_TCR_SYNCENA_Pos                2
#define PORT_ITM_TCR_SYNCENA_Msk                (1ul << PORT_ITM_TCR_SYNCENA_Pos)

#define PORT_ITM_TCR_TSENA_Pos                  1
#define PORT_ITM_TCR_TSENA_Msk                  (1ul << PORT_ITM_TCR_TSENA_Pos)

#define PORT_ITM_TCR_ITMENA_Pos                 0
#define PORT_ITM_TCR_ITMENA_Msk                 (1ul << PORT_ITM_TCR_ITMENA_Pos)

/*--  ITM Integration Write Register Definitions  ----------------------------*/
#define PORT_ITM_IWR_ATVALIDM_Pos               0
#define PORT_ITM_IWR_ATVALIDM_Msk               (1ul << PORT_ITM_IWR_ATVALIDM_Pos)

/*--  ITM Integration Read Register Definitions  -----------------------------*/
#define PORT_ITM_IRR_ATREADYM_Pos               0
#define PORT_ITM_IRR_ATREADYM_Msk               (1ul << PORT_ITM_IRR_ATREADYM_Pos)

/*--  ITM Integration Mode Control Register Definitions  ---------------------*/
#define PORT_ITM_IMCR_INTEGRATION_Pos           0
#define PORT_ITM_IMCR_INTEGRATION_Msk           (1ul << PORT_ITM_IMCR_INTEGRATION_Pos)

/*--  ITM Lock Status Register Definitions  ----------------------------------*/
#define PORT_ITM_LSR_BYTEACC_Pos                2
#define PORT_ITM_LSR_BYTEACC_Msk                (1ul << PORT_ITM_LSR_BYTEACC_Pos)

#define PORT_ITM_LSR_ACCESS_Pos                 1
#define PORT_ITM_LSR_ACCESS_Msk                 (1ul << PORT_ITM_LSR_ACCESS_Pos)

#define PORT_ITM_LSR_PRESENT_Pos                0
#define PORT_ITM_LSR_PRESENT_Msk                (1ul << PORT_ITM_LSR_PRESENT_Pos)

/*--  System Control Space Base Address  -------------------------------------*/
#define PORT_SCS_BASE                   (0xe000e000ul)

#if defined(__MPU_PRESENT) && (__MPU_PRESENT == 1) || defined(__DOXYGEN__)
#define PORT_MPU_BASE                   (PORT_SCS_BASE + 0x0d90ul)

#define PORT_MPU                        ((struct portMPU *)PORT_MPU_BASE)

/*--  MPU Type Register  -----------------------------------------------------*/
#define PORT_MPU_TYPE_IREGION_Pos               16
#define PORT_MPU_TYPE_IREGION_Msk               (0xfful << PORT_MPU_TYPE_IREGION_Pos)

#define PORT_MPU_TYPE_DREGION_Pos               8
#define PORT_MPU_TYPE_DREGION_Msk               (0xfful << PORT_MPU_TYPE_DREGION_Pos)

#define PORT_MPU_TYPE_SEPARATE_Pos              0
#define PORT_MPU_TYPE_SEPARATE_Msk              (1ul << PORT_MPU_TYPE_SEPARATE_Pos)

/*--  MPU Control Register  --------------------------------------------------*/
#define PORT_MPU_CTRL_PRIVDEFENA_Pos            2
#define PORT_MPU_CTRL_PRIVDEFENA_Msk            (1ul << PORT_MPU_CTRL_PRIVDEFENA_Pos)

#define PORT_MPU_CTRL_HFNMIENA_Pos              1
#define PORT_MPU_CTRL_HFNMIENA_Msk              (1ul << PORT_MPU_CTRL_HFNMIENA_Pos)

#define PORT_MPU_CTRL_ENABLE_Pos                0
#define PORT_MPU_CTRL_ENABLE_Msk                (1ul << PORT_MPU_CTRL_ENABLE_Pos)

/*--  MPU Region Number Register  --------------------------------------------*/
#define PORT_MPU_RNR_REGION_Pos                 0
#define PORT_MPU_RNR_REGION_Msk                 (0xfful << PORT_MPU_RNR_REGION_Pos)

/*--  MPU Region Base Address Register  --------------------------------------*/
#define PORT_MPU_RBAR_ADDR_Pos                  5
#define PORT_MPU_RBAR_ADDR_Msk                  (0x7fffffful << PORT_MPU_RBAR_ADDR_Pos)

#define PORT_MPU_RBAR_VALID_Pos                 4
#define PORT_MPU_RBAR_VALID_Msk                 (1ul << PORT_MPU_RBAR_VALID_Pos)

#define PORT_MPU_RBAR_REGION_Pos                0
#define PORT_MPU_RBAR_REGION_Msk                (0xful << PORT_MPU_RBAR_REGION_Pos)

/*--  MPU Region Attribute and Size Register  --------------------------------*/
#define PORT_MPU_RASR_ATTRS_Pos                 16
#define PORT_MPU_RASR_ATTRS_Msk                 (0xfffful << PORT_MPU_RASR_ATTRS_Pos)

#define PORT_MPU_RASR_XN_Pos                    28
#define PORT_MPU_RASR_XN_Msk                    (1ul << PORT_MPU_RASR_XN_Pos)

#define PORT_MPU_RASR_AP_Pos                    24
#define PORT_MPU_RASR_AP_Msk                    (0x7UL << PORT_MPU_RASR_AP_Pos)

#define PORT_MPU_RASR_TEX_Pos                   19
#define PORT_MPU_RASR_TEX_Msk                   (0x7UL << PORT_MPU_RASR_TEX_Pos)

#define PORT_MPU_RASR_S_Pos                     18
#define PORT_MPU_RASR_S_Msk                     (1ul << PORT_MPU_RASR_S_Pos)

#define PORT_MPU_RASR_C_Pos                     17
#define PORT_MPU_RASR_C_Msk                     (1ul << PORT_MPU_RASR_C_Pos)

#define PORT_MPU_RASR_B_Pos                     16
#define PORT_MPU_RASR_B_Msk                     (1ul << PORT_MPU_RASR_B_Pos)

#define PORT_MPU_RASR_SRD_Pos                   8
#define PORT_MPU_RASR_SRD_Msk                   (0xfful << PORT_MPU_RASR_SRD_Pos)

#define PORT_MPU_RASR_SIZE_Pos                  1
#define PORT_MPU_RASR_SIZE_Msk                  (0x1ful << PORT_MPU_RASR_SIZE_Pos)

#define PORT_MPU_RASR_ENABLE_Pos                0
#define PORT_MPU_RASR_ENABLE_Msk                (1ul << PORT_MPU_RASR_ENABLE_Pos)

#endif /* (__MPU_PRESENT == 1) */

#define PORT_NVIC_BASE                  (PORT_SCS_BASE + 0x0100ul)

#define PORT_NVIC                       ((struct portNVIC *)PORT_NVIC_BASE)

/*--  Software Triggered Interrupt Register Definitions  ---------------------*/
#define PORT_NVIC_STIR_INTID_Pos                0
#define PORT_NVIC_STIR_INTID_Msk                (0x1fful << NVIC_STIR_INTID_Pos)

#define PORT_SCB_BASE                   (PORT_SCS_BASE + 0x0d00ul)

#define PORT_SCB                        ((struct port_scb *)PORT_SCB_BASE)

/*--  SCB CPUID Register Definitions  ----------------------------------------*/
#define PORT_SCB_CPUID_IMPLEMENTER_Pos          24
#define PORT_SCB_CPUID_IMPLEMENTER_Msk          (0xfful << PORT_SCB_CPUID_IMPLEMENTER_Pos)

#define PORT_SCB_CPUID_VARIANT_Pos              20
#define PORT_SCB_CPUID_VARIANT_Msk              (0xful << PORT_SCB_CPUID_VARIANT_Pos)

#define PORT_SCB_CPUID_ARCHITECTURE_Pos         16
#define PORT_SCB_CPUID_ARCHITECTURE_Msk         (0xful << PORT_SCB_CPUID_ARCHITECTURE_Pos)

#define PORT_SCB_CPUID_PARTNO_Pos               4
#define PORT_SCB_CPUID_PARTNO_Msk               (0xFFFUL << PORT_SCB_CPUID_PARTNO_Pos)

#define PORT_SCB_CPUID_REVISION_Pos             0
#define PORT_SCB_CPUID_REVISION_Msk             (0xful << PORT_SCB_CPUID_REVISION_Pos)

/*--  SCB Interrupt Control State Register Definitions  ----------------------*/
#define PORT_SCB_ICSR_NMIPENDSET_Pos            31
#define PORT_SCB_ICSR_NMIPENDSET_Msk            (1ul << PORT_SCB_ICSR_NMIPENDSET_Pos)

#define PORT_SCB_ICSR_PENDSVSET_Pos             28
#define PORT_SCB_ICSR_PENDSVSET_Msk             (1ul << PORT_SCB_ICSR_PENDSVSET_Pos)

#define PORT_SCB_ICSR_PENDSVCLR_Pos             27
#define PORT_SCB_ICSR_PENDSVCLR_Msk             (1ul << PORT_SCB_ICSR_PENDSVCLR_Pos)

#define PORT_SCB_ICSR_PENDSTSET_Pos             26
#define PORT_SCB_ICSR_PENDSTSET_Msk             (1ul << PORT_SCB_ICSR_PENDSTSET_Pos)

#define PORT_SCB_ICSR_PENDSTCLR_Pos             25
#define PORT_SCB_ICSR_PENDSTCLR_Msk             (1ul << PORT_SCB_ICSR_PENDSTCLR_Pos)

#define PORT_SCB_ICSR_ISRPREEMPT_Pos            23
#define PORT_SCB_ICSR_ISRPREEMPT_Msk            (1ul << PORT_SCB_ICSR_ISRPREEMPT_Pos)

#define PORT_SCB_ICSR_ISRPENDING_Pos            22
#define PORT_SCB_ICSR_ISRPENDING_Msk            (1ul << PORT_SCB_ICSR_ISRPENDING_Pos)

#define PORT_SCB_ICSR_VECTPENDING_Pos           12
#define PORT_SCB_ICSR_VECTPENDING_Msk           (0x1fful << PORT_SCB_ICSR_VECTPENDING_Pos)

#define PORT_SCB_ICSR_RETTOBASE_Pos             11
#define PORT_SCB_ICSR_RETTOBASE_Msk             (1ul << PORT_SCB_ICSR_RETTOBASE_Pos)

#define PORT_SCB_ICSR_VECTACTIVE_Pos            0
#define PORT_SCB_ICSR_VECTACTIVE_Msk            (0x1fful << PORT_SCB_ICSR_VECTACTIVE_Pos)

/*--  SCB Vector Table Offset Register Definitions  --------------------------*/
#if defined(__CM3_REV) && (__CM3_REV < 0x0201)                   /* core r2p1 */
#define PORT_SCB_VTOR_TBLBASE_Pos               29
#define PORT_SCB_VTOR_TBLBASE_Msk               (1ul << PORT_SCB_VTOR_TBLBASE_Pos)

#define PORT_SCB_VTOR_TBLOFF_Pos                7
#define PORT_SCB_VTOR_TBLOFF_Msk                (0x3ffffful << PORT_SCB_VTOR_TBLOFF_Pos)
#else
#define PORT_SCB_VTOR_TBLOFF_Pos                7
#define PORT_SCB_VTOR_TBLOFF_Msk                (0x1fffffful << PORT_SCB_VTOR_TBLOFF_Pos)
#endif

/*--  SCB Application Interrupt and Reset Control Register Definitions  ------*/
#define PORT_SCB_AIRCR_VECTKEY_Pos              16
#define PORT_SCB_AIRCR_VECTKEY_Msk              (0xfffful << PORT_SCB_AIRCR_VECTKEY_Pos)

#define PORT_SCB_AIRCR_VECTKEYSTAT_Pos          16
#define PORT_SCB_AIRCR_VECTKEYSTAT_Msk          (0xfffful << PORT_SCB_AIRCR_VECTKEYSTAT_Pos)

#define PORT_SCB_AIRCR_ENDIANESS_Pos            15
#define PORT_SCB_AIRCR_ENDIANESS_Msk            (1ul << PORT_SCB_AIRCR_ENDIANESS_Pos)

#define PORT_SCB_AIRCR_PRIGROUP_Pos             8
#define PORT_SCB_AIRCR_PRIGROUP_Msk             (7ul << PORT_SCB_AIRCR_PRIGROUP_Pos)

#define PORT_SCB_AIRCR_SYSRESETREQ_Pos          2
#define PORT_SCB_AIRCR_SYSRESETREQ_Msk          (1ul << PORT_SCB_AIRCR_SYSRESETREQ_Pos)

#define PORT_SCB_AIRCR_VECTCLRACTIVE_Pos        1
#define PORT_SCB_AIRCR_VECTCLRACTIVE_Msk        (1ul << PORT_SCB_AIRCR_VECTCLRACTIVE_Pos)

#define PORT_SCB_AIRCR_VECTRESET_Pos            0
#define PORT_SCB_AIRCR_VECTRESET_Msk            (1ul << PORT_SCB_AIRCR_VECTRESET_Pos)

/*--  SCB System Control Register Definitions  -------------------------------*/
#define PORT_SCB_SCR_SEVONPEND_Pos              4
#define PORT_SCB_SCR_SEVONPEND_Msk              (1ul << PORT_SCB_SCR_SEVONPEND_Pos)

#define PORT_SCB_SCR_SLEEPDEEP_Pos              2
#define PORT_SCB_SCR_SLEEPDEEP_Msk              (1ul << PORT_SCB_SCR_SLEEPDEEP_Pos)

#define PORT_SCB_SCR_SLEEPONEXIT_Pos            1
#define PORT_SCB_SCR_SLEEPONEXIT_Msk            (1ul << PORT_SCB_SCR_SLEEPONEXIT_Pos)

/*--  SCB Configuration Control Register Definitions  ------------------------*/
#define PORT_SCB_CCR_STKALIGN_Pos               9
#define PORT_SCB_CCR_STKALIGN_Msk               (1ul << PORT_SCB_CCR_STKALIGN_Pos)

#define PORT_SCB_CCR_BFHFNMIGN_Pos              8
#define PORT_SCB_CCR_BFHFNMIGN_Msk              (1ul << PORT_SCB_CCR_BFHFNMIGN_Pos)

#define PORT_SCB_CCR_DIV_0_TRP_Pos              4
#define PORT_SCB_CCR_DIV_0_TRP_Msk              (1ul << PORT_SCB_CCR_DIV_0_TRP_Pos)

#define PORT_SCB_CCR_UNALIGN_TRP_Pos            3
#define PORT_SCB_CCR_UNALIGN_TRP_Msk            (1ul << PORT_SCB_CCR_UNALIGN_TRP_Pos)

#define PORT_SCB_CCR_USERSETMPEND_Pos           1
#define PORT_SCB_CCR_USERSETMPEND_Msk           (1ul << PORT_SCB_CCR_USERSETMPEND_Pos)

#define PORT_SCB_CCR_NONBASETHRDENA_Pos         0
#define PORT_SCB_CCR_NONBASETHRDENA_Msk         (1ul << PORT_SCB_CCR_NONBASETHRDENA_Pos)

/*--  SCB System Handler Control and State Register Definitions  -------------*/
#define PORT_SCB_SHCSR_USGFAULTENA_Pos          18
#define PORT_SCB_SHCSR_USGFAULTENA_Msk          (1ul << PORT_SCB_SHCSR_USGFAULTENA_Pos)

#define PORT_SCB_SHCSR_BUSFAULTENA_Pos          17
#define PORT_SCB_SHCSR_BUSFAULTENA_Msk          (1ul << PORT_SCB_SHCSR_BUSFAULTENA_Pos)

#define PORT_SCB_SHCSR_MEMFAULTENA_Pos          16
#define PORT_SCB_SHCSR_MEMFAULTENA_Msk          (1ul << PORT_SCB_SHCSR_MEMFAULTENA_Pos)

#define PORT_SCB_SHCSR_SVCALLPENDED_Pos         15
#define PORT_SCB_SHCSR_SVCALLPENDED_Msk         (1ul << PORT_SCB_SHCSR_SVCALLPENDED_Pos)

#define PORT_SCB_SHCSR_BUSFAULTPENDED_Pos       14
#define PORT_SCB_SHCSR_BUSFAULTPENDED_Msk       (1ul << PORT_SCB_SHCSR_BUSFAULTPENDED_Pos)

#define PORT_SCB_SHCSR_MEMFAULTPENDED_Pos       13
#define PORT_SCB_SHCSR_MEMFAULTPENDED_Msk       (1ul << PORT_SCB_SHCSR_MEMFAULTPENDED_Pos)

#define PORT_SCB_SHCSR_USGFAULTPENDED_Pos       12
#define PORT_SCB_SHCSR_USGFAULTPENDED_Msk       (1ul << PORT_SCB_SHCSR_USGFAULTPENDED_Pos)

#define PORT_SCB_SHCSR_SYSTICKACT_Pos           11
#define PORT_SCB_SHCSR_SYSTICKACT_Msk           (1ul << PORT_SCB_SHCSR_SYSTICKACT_Pos)

#define PORT_SCB_SHCSR_PENDSVACT_Pos            10
#define PORT_SCB_SHCSR_PENDSVACT_Msk            (1ul << PORT_SCB_SHCSR_PENDSVACT_Pos)

#define PORT_SCB_SHCSR_MONITORACT_Pos           8
#define PORT_SCB_SHCSR_MONITORACT_Msk           (1ul << PORT_SCB_SHCSR_MONITORACT_Pos)

#define PORT_SCB_SHCSR_SVCALLACT_Pos            7
#define PORT_SCB_SHCSR_SVCALLACT_Msk            (1ul << PORT_SCB_SHCSR_SVCALLACT_Pos)

#define PORT_SCB_SHCSR_USGFAULTACT_Pos          3
#define PORT_SCB_SHCSR_USGFAULTACT_Msk          (1ul << PORT_SCB_SHCSR_USGFAULTACT_Pos)

#define PORT_SCB_SHCSR_BUSFAULTACT_Pos          1
#define PORT_SCB_SHCSR_BUSFAULTACT_Msk          (1ul << PORT_SCB_SHCSR_BUSFAULTACT_Pos)

#define PORT_SCB_SHCSR_MEMFAULTACT_Pos          0
#define PORT_SCB_SHCSR_MEMFAULTACT_Msk          (1ul << PORT_SCB_SHCSR_MEMFAULTACT_Pos)

/*--  SCB Configurable Fault Status Registers Definitions  -------------------*/
#define PORT_SCB_CFSR_USGFAULTSR_Pos            16
#define PORT_SCB_CFSR_USGFAULTSR_Msk            (0xfffful << PORT_SCB_CFSR_USGFAULTSR_Pos)

#define PORT_SCB_CFSR_BUSFAULTSR_Pos            8
#define PORT_SCB_CFSR_BUSFAULTSR_Msk            (0xfful << PORT_SCB_CFSR_BUSFAULTSR_Pos)

#define PORT_SCB_CFSR_MEMFAULTSR_Pos            0
#define PORT_SCB_CFSR_MEMFAULTSR_Msk            (0xfful << PORT_SCB_CFSR_MEMFAULTSR_Pos)

/*--  SCB Hard Fault Status Registers Definitions  ---------------------------*/
#define PORT_SCB_HFSR_DEBUGEVT_Pos              31
#define PORT_SCB_HFSR_DEBUGEVT_Msk              (1ul << PORT_SCB_HFSR_DEBUGEVT_Pos)

#define PORT_SCB_HFSR_FORCED_Pos                30
#define PORT_SCB_HFSR_FORCED_Msk                (1ul << PORT_SCB_HFSR_FORCED_Pos)

#define PORT_SCB_HFSR_VECTTBL_Pos               1
#define PORT_SCB_HFSR_VECTTBL_Msk               (1ul << PORT_SCB_HFSR_VECTTBL_Pos)

/*--  SCB Debug Fault Status Register Definitions  ---------------------------*/
#define PORT_SCB_DFSR_EXTERNAL_Pos              4
#define PORT_SCB_DFSR_EXTERNAL_Msk              (1ul << PORT_SCB_DFSR_EXTERNAL_Pos)

#define PORT_SCB_DFSR_VCATCH_Pos                3
#define PORT_SCB_DFSR_VCATCH_Msk                (1ul << PORT_SCB_DFSR_VCATCH_Pos)

#define PORT_SCB_DFSR_DWTTRAP_Pos               2
#define PORT_SCB_DFSR_DWTTRAP_Msk               (1ul << PORT_SCB_DFSR_DWTTRAP_Pos)

#define PORT_SCB_DFSR_BKPT_Pos                  1
#define PORT_SCB_DFSR_BKPT_Msk                  (1ul << PORT_SCB_DFSR_BKPT_Pos)

#define PORT_SCB_DFSR_HALTED_Pos                0
#define PORT_SCB_DFSR_HALTED_Msk                (1ul << PORT_SCB_DFSR_HALTED_Pos)

/**@brief       System control Register not in SCB
 */
#define PORT_SCNSCB                     ((struct portSCnSCB *)PORT_SCS_BASE)

/*--  Interrupt Controller Type Register Definitions  ------------------------*/
#define PORT_SCNSCB_ICTR_INTLINESNUM_Pos        0
#define PORT_SCNSCB_ICTR_INTLINESNUM_Msk        (0xful << PORT_SCNSCB_ICTR_INTLINESNUM_Pos)

/*--  Auxiliary Control Register Definitions  --------------------------------*/
#define PORT_SCNSCB_ACTLR_DISFOLD_Pos           2
#define PORT_SCNSCB_ACTLR_DISFOLD_Msk           (1ul << PORT_SCNSCB_ACTLR_DISFOLD_Pos)

#define PORT_SCNSCB_ACTLR_DISDEFWBUF_Pos        1
#define PORT_SCNSCB_ACTLR_DISDEFWBUF_Msk        (1ul << PORT_SCNSCB_ACTLR_DISDEFWBUF_Pos)

#define PORT_SCNSCB_ACTLR_DISMCYCINT_Pos        0
#define PORT_SCNSCB_ACTLR_DISMCYCINT_Msk        (1ul << PORT_SCNSCB_ACTLR_DISMCYCINT_Pos)

#define PORT_SYSTICK_BASE               (PORT_SCS_BASE + 0x0010ul)

#define PORT_SYSTICK                    ((struct port_sys_tick *)PORT_SYSTICK_BASE)

/*--  SysTick Control / Status Register Definitions  -------------------------*/
#define PORT_SYSTICK_CTRL_COUNTFLAG_Pos         16
#define PORT_SYSTICK_CTRL_COUNTFLAG_Msk         (1ul << PORT_SYSTICK_CTRL_COUNTFLAG_Pos)

#define PORT_SYSTICK_CTRL_CLKSOURCE_Pos         2
#define PORT_SYSTICK_CTRL_CLKSOURCE_Msk         (1ul << PORT_SYSTICK_CTRL_CLKSOURCE_Pos)

#define PORT_SYSTICK_CTRL_TICKINT_Pos           1
#define PORT_SYSTICK_CTRL_TICKINT_Msk           (1ul << PORT_SYSTICK_CTRL_TICKINT_Pos)

#define PORT_SYSTICK_CTRL_ENABLE_Pos            0
#define PORT_SYSTICK_CTRL_ENABLE_Msk            (1ul << PORT_SYSTICK_CTRL_ENABLE_Pos)

/*--  SysTick Reload Register Definitions  -----------------------------------*/
#define PORT_SYSTICK_LOAD_RELOAD_Pos            0
#define PORT_SYSTICK_LOAD_RELOAD_Msk            (0xfffffful << PORT_SYSTICK_LOAD_RELOAD_Pos)

/*--  SysTick Current Register Definitions  ----------------------------------*/
#define PORT_SYSTICK_VAL_CURRENT_Pos            0
#define PORT_SYSTICK_VAL_CURRENT_Msk            (0xfffffful << PORT_SYSTICK_VAL_CURRENT_Pos)

/*--  SysTick Calibration Register Definitions  ------------------------------*/
#define PORT_SYSTICK_CALIB_NOREF_Pos            31
#define PORT_SYSTICK_CALIB_NOREF_Msk            (1ul << PORT_SYSTICK_CALIB_NOREF_Pos)

#define PORT_SYSTICK_CALIB_SKEW_Pos             30
#define PORT_SYSTICK_CALIB_SKEW_Msk             (1ul << PORT_SYSTICK_CALIB_SKEW_Pos)

#define PORT_SYSTICK_CALIB_TENMS_Pos            0
#define PORT_SYSTICK_CALIB_TENMS_Msk            (0xfffffful << PORT_SYSTICK_CALIB_TENMS_Pos)

#define PORT_TPI_BASE                   (0xe0040000ul)

#define PORT_TPI                        ((struct port_tpi *)PORT_TPI_BASE)

/*--  TPI Asynchronous Clock Prescaler Register Definitions  -----------------*/
#define PORT_TPI_ACPR_PRESCALER_Pos             0
#define PORT_TPI_ACPR_PRESCALER_Msk             (0x1ffful << PORT_TPI_ACPR_PRESCALER_Pos)

/*--  TPI Selected Pin Protocol Register Definitions  ------------------------*/
#define PORT_TPI_SPPR_TXMODE_Pos                0
#define PORT_TPI_SPPR_TXMODE_Msk                (0x3ul << PORT_TPI_SPPR_TXMODE_Pos)

/*--  TPI Formatter and Flush Status Register Definitions  -------------------*/
#define PORT_TPI_FFSR_FTNONSTOP_Pos             3
#define PORT_TPI_FFSR_FTNONSTOP_Msk             (0x1ul << PORT_TPI_FFSR_FTNONSTOP_Pos)

#define PORT_TPI_FFSR_TCPRESENT_Pos             2
#define PORT_TPI_FFSR_TCPRESENT_Msk             (0x1ul << PORT_TPI_FFSR_TCPRESENT_Pos)

#define PORT_TPI_FFSR_FTSTOPPED_Pos             1
#define PORT_TPI_FFSR_FTSTOPPED_Msk             (0x1ul << PORT_TPI_FFSR_FTSTOPPED_Pos)

#define PORT_TPI_FFSR_FlINPROG_Pos              0
#define PORT_TPI_FFSR_FlINPROG_Msk              (0x1ul << PORT_TPI_FFSR_FlINPROG_Pos)

/*--  TPI Formatter and Flush Control Register Definitions  ------------------*/
#define PORT_TPI_FFCR_TRIGIN_Pos                8
#define PORT_TPI_FFCR_TRIGIN_Msk                (0x1ul << PORT_TPI_FFCR_TRIGIN_Pos)

#define PORT_TPI_FFCR_ENFCONT_Pos               1
#define PORT_TPI_FFCR_ENFCONT_Msk               (0x1ul << PORT_TPI_FFCR_ENFCONT_Pos)

/* TPI TRIGGER Register Definitions */
#define PORT_TPI_TRIGGER_TRIGGER_Pos            0
#define PORT_TPI_TRIGGER_TRIGGER_Msk            (0x1ul << PORT_TPI_TRIGGER_TRIGGER_Pos)

/* TPI Integration ETM Data Register Definitions (FIFO0) */
#define PORT_TPI_FIFO0_ITM_ATVALID_Pos          29
#define PORT_TPI_FIFO0_ITM_ATVALID_Msk          (0x3ul << PORT_TPI_FIFO0_ITM_ATVALID_Pos)

#define PORT_TPI_FIFO0_ITM_BYTECOUNT_Pos        27
#define PORT_TPI_FIFO0_ITM_BYTECOUNT_Msk        (0x3ul << PORT_TPI_FIFO0_ITM_BYTECOUNT_Pos)

#define PORT_TPI_FIFO0_ETM_ATVALID_Pos          26
#define PORT_TPI_FIFO0_ETM_ATVALID_Msk          (0x3ul << PORT_TPI_FIFO0_ETM_ATVALID_Pos)

#define PORT_TPI_FIFO0_ETM_BYTECOUNT_Pos        24
#define PORT_TPI_FIFO0_ETM_BYTECOUNT_Msk        (0x3ul << PORT_TPI_FIFO0_ETM_BYTECOUNT_Pos)

#define PORT_TPI_FIFO0_ETM2_Pos                 16
#define PORT_TPI_FIFO0_ETM2_Msk                 (0xfful << PORT_TPI_FIFO0_ETM2_Pos)

#define PORT_TPI_FIFO0_ETM1_Pos                 8
#define PORT_TPI_FIFO0_ETM1_Msk                 (0xfful << PORT_TPI_FIFO0_ETM1_Pos)

#define PORT_TPI_FIFO0_ETM0_Pos                 0
#define PORT_TPI_FIFO0_ETM0_Msk                 (0xfful << PORT_TPI_FIFO0_ETM0_Pos)

/*--  TPI ITATBCTR2 Register Definitions  ------------------------------------*/
#define PORT_TPI_ITATBCTR2_ATREADY_Pos          0
#define PORT_TPI_ITATBCTR2_ATREADY_Msk          (0x1ul << PORT_TPI_ITATBCTR2_ATREADY_Pos)

/*--  TPI Integration ITM Data Register Definitions (FIFO1)  -----------------*/
#define PORT_TPI_FIFO1_ITM_ATVALID_Pos          29
#define PORT_TPI_FIFO1_ITM_ATVALID_Msk          (0x3ul << PORT_TPI_FIFO1_ITM_ATVALID_Pos)

#define PORT_TPI_FIFO1_ITM_BYTECOUNT_Pos        27
#define PORT_TPI_FIFO1_ITM_BYTECOUNT_Msk        (0x3ul << PORT_TPI_FIFO1_ITM_BYTECOUNT_Pos)

#define PORT_TPI_FIFO1_ETM_ATVALID_Pos          26
#define PORT_TPI_FIFO1_ETM_ATVALID_Msk          (0x3ul << PORT_TPI_FIFO1_ETM_ATVALID_Pos)

#define PORT_TPI_FIFO1_ETM_BYTECOUNT_Pos        24
#define PORT_TPI_FIFO1_ETM_BYTECOUNT_Msk        (0x3ul << PORT_TPI_FIFO1_ETM_BYTECOUNT_Pos)

#define PORT_TPI_FIFO1_ITM2_Pos                 16
#define PORT_TPI_FIFO1_ITM2_Msk                 (0xfful << PORT_TPI_FIFO1_ITM2_Pos)

#define PORT_TPI_FIFO1_ITM1_Pos                 8
#define PORT_TPI_FIFO1_ITM1_Msk                 (0xfful << PORT_TPI_FIFO1_ITM1_Pos)

#define PORT_TPI_FIFO1_ITM0_Pos                 0
#define PORT_TPI_FIFO1_ITM0_Msk                 (0xfful << PORT_TPI_FIFO1_ITM0_Pos)

/*--  TPI ITATBCTR0 Register Definitions  ------------------------------------*/
#define PORT_TPI_ITATBCTR0_ATREADY_Pos          0
#define PORT_TPI_ITATBCTR0_ATREADY_Msk          (0x1ul << PORT_TPI_ITATBCTR0_ATREADY_Pos)

/*--  TPI Integration Mode Control Register Definitions  ---------------------*/
#define PORT_TPI_ITCTRL_MODE_Pos                0
#define PORT_TPI_ITCTRL_MODE_Msk                (0x1ul << PORT_TPI_ITCTRL_`Pos)

/*--  TPI DEVID Register Definitions  ----------------------------------------*/
#define PORT_TPI_DEVID_NRZVALID_Pos             11
#define PORT_TPI_DEVID_NRZVALID_Msk             (0x1ul << PORT_TPI_DEVID_NRZVALID_Pos)

#define PORT_TPI_DEVID_MANCVALID_Pos            10
#define PORT_TPI_DEVID_MANCVALID_Msk            (0x1ul << PORT_TPI_DEVID_MANCVALID_Pos)

#define PORT_TPI_DEVID_PTINVALID_Pos            9
#define PORT_TPI_DEVID_PTINVALID_Msk            (0x1ul << PORT_TPI_DEVID_PTINVALID_Pos)

#define PORT_TPI_DEVID_MINBUFSZ_Pos             6
#define PORT_TPI_DEVID_MINBUFSZ_Msk             (0x7ul << PORT_TPI_DEVID_MINBUFSZ_Pos)

#define PORT_TPI_DEVID_ASYNCLKIN_Pos            5
#define PORT_TPI_DEVID_ASYNCLKIN_Msk            (0x1ul << PORT_TPI_DEVID_ASYNCLKIN_Pos)

#define PORT_TPI_DEVID_NRTRACEINPUT_Pos         0
#define PORT_TPI_DEVID_NRTRACEINPUT_Msk         (0x1ful << PORT_TPI_DEVID_NRTRACEINPUT_Pos)

/*--  TPI DEVTYPE Register Definitions  --------------------------------------*/
#define PORT_TPI_DEVTYPE_SUBTYPE_Pos            0
#define PORT_TPI_DEVTYPE_SUBTYPE_Msk            (0xful << PORT_TPI_DEVTYPE_SUBTYPE_Pos)

#define PORT_TPI_DEVTYPE_MAJORTYPE_Pos          4
#define PORT_TPI_DEVTYPE_MAJORTYPE_Msk          (0xful << PORT_TPI_DEVTYPE_MAJORTYPE_Pos)

/*--  On AIRCR register writes, write 0x5FA, otherwise the write is ignored  -*/
#define PORT_SCB_AIRCR_VECTKEY_VALUE    0x5faul

#define PORT_RO                         volatile const
#define PORT_WO                         volatile
#define PORT_RW                         volatile

/*-------------------------------------------------------  C++ extern base  --*/
#ifdef __cplusplus
extern "C" {
#endif

/*============================================================  DATA TYPES  ==*/

/*--  Structure to access the Nested Vectored Interrupt Controller (NVIC)  ---*/
struct port_nvic {
    PORT_RW uint32_t    ISER[8];
            uint32_t    reserved0[24];
    PORT_RW uint32_t    ICER[8];
            uint32_t    reserved1[24];
    PORT_RW uint32_t    ISPR[8];
            uint32_t    reserved2[24];
    PORT_RW uint32_t    ICPR[8];
            uint32_t    reserved3[24];
    PORT_RW uint32_t    IABR[8];
            uint32_t    reserved4[56];
    PORT_RW uint8_t     IP[240];
            uint32_t    reserved5[644];
    PORT_WO uint32_t    STIR;
};

/*--  Structure to access the System Control Block (SCB)  --------------------*/
struct port_scb {
    PORT_RO uint32_t    CPUID;
    PORT_RW uint32_t    ICSR;
    PORT_RW uint32_t    VTOR;
    PORT_RW uint32_t    AIRCR;
    PORT_RW uint32_t    SCR;
    PORT_RW uint32_t    CCR;
    PORT_RW uint8_t     SHP[12];
    PORT_RW uint32_t    SHCSR;
    PORT_RW uint32_t    CFSR;
    PORT_RW uint32_t    HFSR;
    PORT_RW uint32_t    DFSR;
    PORT_RW uint32_t    MMFAR;
    PORT_RW uint32_t    BFAR;
    PORT_RW uint32_t    AFSR;
    PORT_RO uint32_t    PFR[2];
    PORT_RO uint32_t    DFR;
    PORT_RO uint32_t    ADR;
    PORT_RO uint32_t    MMFR[4];
    PORT_RO uint32_t    ISAR[5];
            uint32_t    reserved0[5];
    PORT_RW uint32_t    CPACR;
};

/*--  Structure to access the System Control and ID Register not in the SCB  -*/
struct port_sc_nscb {
            uint32_t    reserved0[1];
    PORT_RO uint32_t    ICTR;
#if ((defined __CM3_REV) && (__CM3_REV >= 0x200))
    PORT_RW uint32_t    ACTLR;
#else
            uint32_t    reserved1[1];
#endif
};

/*--  Structure to access the System Timer (SysTick)  ------------------------*/
struct port_sys_tick
{
    PORT_RW uint32_t    CTRL;
    PORT_RW uint32_t    LOAD;
    PORT_RW uint32_t    VAL;
    PORT_RO uint32_t    CALIB;
};

/*--  Structure to access the Instrumentation Trace Macrocell (ITM)  ---------*/
struct port_itm
{
    PORT_WO union port
    {
        PORT_WO uint8_t     u8;
        PORT_WO uint16_t    u16;
        PORT_WO uint32_t    u32;
    }                   PORT [32];
            uint32_t    reserved0[864];
    PORT_RW uint32_t    TER;
            uint32_t    reserved1[15];
    PORT_RW uint32_t    TPR;
            uint32_t    reserved2[15];
    PORT_RW uint32_t    TCR;
            uint32_t    reserved3[29];
    PORT_WO uint32_t    IWR;
    PORT_RO uint32_t    IRR;
    PORT_RW uint32_t    IMCR;
            uint32_t    reserved4[43];
    PORT_WO uint32_t    LAR;
    PORT_RO uint32_t    LSR;
            uint32_t    reserved5[6];
    PORT_RO uint32_t    PID4;
    PORT_RO uint32_t    PID5;
    PORT_RO uint32_t    PID6;
    PORT_RO uint32_t    PID7;
    PORT_RO uint32_t    PID0;
    PORT_RO uint32_t    PID1;
    PORT_RO uint32_t    PID2;
    PORT_RO uint32_t    PID3;
    PORT_RO uint32_t    CID0;
    PORT_RO uint32_t    CID1;
    PORT_RO uint32_t    CID2;
    PORT_RO uint32_t    CID3;
};

/*--  Structure to access the Data Watchpoint and Trace Register (DWT)  ------*/
struct port_dwt {
    PORT_RW uint32_t    CTRL;
    PORT_RW uint32_t    CYCCNT;
    PORT_RW uint32_t    CPICNT;
    PORT_RW uint32_t    EXCCNT;
    PORT_RW uint32_t    SLEEPCNT;
    PORT_RW uint32_t    LSUCNT;
    PORT_RW uint32_t    FOLDCNT;
    PORT_RO uint32_t    PCSR;
    PORT_RW uint32_t    COMP0;
    PORT_RW uint32_t    MASK0;
    PORT_RW uint32_t    FUNCTION0;
            uint32_t    reserved0[1];
    PORT_RW uint32_t    COMP1;
    PORT_RW uint32_t    MASK1;
    PORT_RW uint32_t    FUNCTION1;
            uint32_t    reserved1[1];
    PORT_RW uint32_t    COMP2;
    PORT_RW uint32_t    MASK2;
    PORT_RW uint32_t    FUNCTION2;
            uint32_t    reserved2[1];
    PORT_RW uint32_t    COMP3;
    PORT_RW uint32_t    MASK3;
    PORT_RW uint32_t    FUNCTION3;
};

/*--  Structure to access the Trace Port Interface Register (TPI)  -----------*/
struct port_tpi {
    PORT_RW uint32_t    SSPSR;
    PORT_RW uint32_t    CSPSR;
            uint32_t    reserved0[2];
    PORT_RW uint32_t    ACPR;
            uint32_t    reserved1[55];
    PORT_RW uint32_t    SPPR;
            uint32_t    reserved2[131];
    PORT_RO uint32_t    FFSR;
    PORT_RW uint32_t    FFCR;
    PORT_RO uint32_t    FSCR;
            uint32_t    reserved3[759];
    PORT_RO uint32_t    TRIGGER;
    PORT_RO uint32_t    FIFO0;
    PORT_RO uint32_t    ITATBCTR2;
            uint32_t    reserved4[1];
    PORT_RO uint32_t    ITATBCTR0;
    PORT_RO uint32_t    FIFO1;
    PORT_RW uint32_t    ITCTRL;
            uint32_t    reserved5[39];
    PORT_RW uint32_t    CLAIMSET;
    PORT_RW uint32_t    CLAIMCLR;
            uint32_t    reserved7[8];
    PORT_RO uint32_t    DEVID;
    PORT_RO uint32_t    DEVTYPE;
};

#if defined(__MPU_PRESENT) && (__MPU_PRESENT == 1) || defined(__DOXYGEN__)

/*--  Structure to access the Memory Protection Unit (MPU)  ------------------*/
struct port_mpu {
    PORT_RO uint32_t    TYPE;
    PORT_RW uint32_t    CTRL;
    PORT_RW uint32_t    RNR;
    PORT_RW uint32_t    RBAR;
    PORT_RW uint32_t    RASR;
    PORT_RW uint32_t    RBAR_A1;
    PORT_RW uint32_t    RASR_A1;
    PORT_RW uint32_t    RBAR_A2;
    PORT_RW uint32_t    RASR_A2;
    PORT_RW uint32_t    RBAR_A3;
    PORT_RW uint32_t    RASR_A3;
};
#endif

/*--  Structure to access the Core Debug Register (CoreDebug)  ---------------*/
struct port_core_debug {
    PORT_RW uint32_t    DHCSR;
    PORT_WO uint32_t    DCRSR;
    PORT_RW uint32_t    DCRDR;
    PORT_RW uint32_t    DEMCR;
};

/*======================================================  GLOBAL VARIABLES  ==*/
/*===================================================  FUNCTION PROTOTYPES  ==*/

#if 0
/**@brief  Set Priority Grouping

  The function sets the priority grouping field using the required unlock sequence.
  The parameter PriorityGroup is assigned to the field SCB->AIRCR [10:8] PRIGROUP field.
  Only values from 0..7 are used.
  In case of a conflict between priority grouping and available
  priority bits (PORT_ISR_PRIO_BITS), the smallest possible priority group is set.

    \param [in]      PriorityGroup  Priority grouping field.
 */
static PORT_C_INLINE void NVIC_SetPriorityGrouping(uint32_t PriorityGroup)
{
  uint32_t reg_value;
  uint32_t PriorityGroupTmp = (PriorityGroup & (uint32_t)0x07);               /* only values 0..7 are used          */

  reg_value  =  SCB->AIRCR;                                                   /* read old register configuration    */
  reg_value &= ~(SCB_AIRCR_VECTKEY_Msk | SCB_AIRCR_PRIGROUP_Msk);             /* clear bits to change               */
  reg_value  =  (reg_value                                 |
                ((uint32_t)0x5FA << SCB_AIRCR_VECTKEY_Pos) |
                (PriorityGroupTmp << 8));                                     /* Insert write key and priorty group */
  SCB->AIRCR =  reg_value;
}


/**@brief  Get Priority Grouping

  The function reads the priority grouping field from the NVIC Interrupt Controller.

    \return                Priority grouping field (SCB->AIRCR [10:8] PRIGROUP field).
 */
static PORT_C_INLINE uint32_t NVIC_GetPriorityGrouping(void)
{
  return ((SCB->AIRCR & SCB_AIRCR_PRIGROUP_Msk) >> SCB_AIRCR_PRIGROUP_Pos);   /* read priority grouping field */
}


/**@brief  Enable External Interrupt

    The function enables a device-specific interrupt in the NVIC interrupt controller.

    \param [in]      IRQn  External interrupt number. Value cannot be negative.
 */
static PORT_C_INLINE void NVIC_EnableIRQ(IRQn_Type IRQn)
{
  NVIC->ISER[((uint32_t)(IRQn) >> 5)] = (1 << ((uint32_t)(IRQn) & 0x1F)); /* enable interrupt */
}


/**@brief  Disable External Interrupt

    The function disables a device-specific interrupt in the NVIC interrupt controller.

    \param [in]      IRQn  External interrupt number. Value cannot be negative.
 */
static PORT_C_INLINE void NVIC_DisableIRQ(IRQn_Type IRQn)
{
  NVIC->ICER[((uint32_t)(IRQn) >> 5)] = (1 << ((uint32_t)(IRQn) & 0x1F)); /* disable interrupt */
}


/**@brief  Get Pending Interrupt

    The function reads the pending register in the NVIC and returns the pending bit
    for the specified interrupt.

    \param [in]      IRQn  Interrupt number.

    \return             0  Interrupt status is not pending.
    \return             1  Interrupt status is pending.
 */
static PORT_C_INLINE uint32_t NVIC_GetPendingIRQ(IRQn_Type IRQn)
{
  return((uint32_t) ((NVIC->ISPR[(uint32_t)(IRQn) >> 5] & (1 << ((uint32_t)(IRQn) & 0x1F)))?1:0)); /* Return 1 if pending else 0 */
}


/**@brief  Set Pending Interrupt

    The function sets the pending bit of an external interrupt.

    \param [in]      IRQn  Interrupt number. Value cannot be negative.
 */
static PORT_C_INLINE void NVIC_SetPendingIRQ(IRQn_Type IRQn)
{
  NVIC->ISPR[((uint32_t)(IRQn) >> 5)] = (1 << ((uint32_t)(IRQn) & 0x1F)); /* set interrupt pending */
}


/**@brief  Clear Pending Interrupt

    The function clears the pending bit of an external interrupt.

    \param [in]      IRQn  External interrupt number. Value cannot be negative.
 */
static PORT_C_INLINE void NVIC_ClearPendingIRQ(IRQn_Type IRQn)
{
  NVIC->ICPR[((uint32_t)(IRQn) >> 5)] = (1 << ((uint32_t)(IRQn) & 0x1F)); /* Clear pending interrupt */
}


/**@brief  Get Active Interrupt

    The function reads the active register in NVIC and returns the active bit.

    \param [in]      IRQn  Interrupt number.

    \return             0  Interrupt status is not active.
    \return             1  Interrupt status is active.
 */
static PORT_C_INLINE uint32_t NVIC_GetActive(IRQn_Type IRQn)
{
  return((uint32_t)((NVIC->IABR[(uint32_t)(IRQn) >> 5] & (1 << ((uint32_t)(IRQn) & 0x1F)))?1:0)); /* Return 1 if active else 0 */
}


/**@brief  Set Interrupt Priority

    The function sets the priority of an interrupt.

    \note The priority cannot be set for every core interrupt.

    \param [in]      IRQn  Interrupt number.
    \param [in]  priority  Priority to set.
 */
static PORT_C_INLINE void NVIC_SetPriority(IRQn_Type IRQn, uint32_t priority)
{
  if(IRQn < 0) {
    SCB->SHP[((uint32_t)(IRQn) & 0xF)-4] = ((priority << (8 - PORT_ISR_PRIO_BITS)) & 0xff); } /* set Priority for Cortex-M  System Interrupts */
  else {
    NVIC->IP[(uint32_t)(IRQn)] = ((priority << (8 - PORT_ISR_PRIO_BITS)) & 0xff);    }        /* set Priority for device specific Interrupts  */
}


/**@brief  Get Interrupt Priority

    The function reads the priority of an interrupt. The interrupt
    number can be positive to specify an external (device specific)
    interrupt, or negative to specify an internal (core) interrupt.


    \param [in]   IRQn  Interrupt number.
    \return             Interrupt Priority. Value is aligned automatically to the implemented
                        priority bits of the microcontroller.
 */
static PORT_C_INLINE uint32_t NVIC_GetPriority(IRQn_Type IRQn)
{

  if(IRQn < 0) {
    return((uint32_t)(SCB->SHP[((uint32_t)(IRQn) & 0xF)-4] >> (8 - PORT_ISR_PRIO_BITS)));  } /* get priority for Cortex-M  system interrupts */
  else {
    return((uint32_t)(NVIC->IP[(uint32_t)(IRQn)]           >> (8 - PORT_ISR_PRIO_BITS)));  } /* get priority for device specific interrupts  */
}


/**@brief  Encode Priority

    The function encodes the priority for an interrupt with the given priority group,
    preemptive priority value, and subpriority value.
    In case of a conflict between priority grouping and available
    priority bits (PORT_ISR_PRIO_BITS), the samllest possible priority group is set.

    \param [in]     PriorityGroup  Used priority group.
    \param [in]   PreemptPriority  Preemptive priority value (starting from 0).
    \param [in]       SubPriority  Subpriority value (starting from 0).
    \return                        Encoded priority. Value can be used in the function \ref NVIC_SetPriority().
 */
static PORT_C_INLINE uint32_t NVIC_EncodePriority (uint32_t PriorityGroup, uint32_t PreemptPriority, uint32_t SubPriority)
{
  uint32_t PriorityGroupTmp = (PriorityGroup & 0x07);          /* only values 0..7 are used          */
  uint32_t PreemptPriorityBits;
  uint32_t SubPriorityBits;

  PreemptPriorityBits = ((7 - PriorityGroupTmp) > PORT_ISR_PRIO_BITS) ? PORT_ISR_PRIO_BITS : 7 - PriorityGroupTmp;
  SubPriorityBits     = ((PriorityGroupTmp + PORT_ISR_PRIO_BITS) < 7) ? 0 : PriorityGroupTmp - 7 + PORT_ISR_PRIO_BITS;

  return (
           ((PreemptPriority & ((1 << (PreemptPriorityBits)) - 1)) << SubPriorityBits) |
           ((SubPriority     & ((1 << (SubPriorityBits    )) - 1)))
         );
}


/**@brief  Decode Priority

    The function decodes an interrupt priority value with a given priority group to
    preemptive priority value and subpriority value.
    In case of a conflict between priority grouping and available
    priority bits (PORT_ISR_PRIO_BITS) the samllest possible priority group is set.

    \param [in]         Priority   Priority value, which can be retrieved with the function \ref NVIC_GetPriority().
    \param [in]     PriorityGroup  Used priority group.
    \param [out] pPreemptPriority  Preemptive priority value (starting from 0).
    \param [out]     pSubPriority  Subpriority value (starting from 0).
 */
static PORT_C_INLINE void NVIC_DecodePriority (uint32_t Priority, uint32_t PriorityGroup, uint32_t* pPreemptPriority, uint32_t* pSubPriority)
{
  uint32_t PriorityGroupTmp = (PriorityGroup & 0x07);          /* only values 0..7 are used          */
  uint32_t PreemptPriorityBits;
  uint32_t SubPriorityBits;

  PreemptPriorityBits = ((7 - PriorityGroupTmp) > PORT_ISR_PRIO_BITS) ? PORT_ISR_PRIO_BITS : 7 - PriorityGroupTmp;
  SubPriorityBits     = ((PriorityGroupTmp + PORT_ISR_PRIO_BITS) < 7) ? 0 : PriorityGroupTmp - 7 + PORT_ISR_PRIO_BITS;

  *pPreemptPriority = (Priority >> SubPriorityBits) & ((1 << (PreemptPriorityBits)) - 1);
  *pSubPriority     = (Priority                   ) & ((1 << (SubPriorityBits    )) - 1);
}


/**@brief  System Reset

    The function initiates a system reset request to reset the MCU.
 */
static PORT_C_INLINE void NVIC_SystemReset(void)
{
  __DSB();                                                     /* Ensure all outstanding memory accesses included
                                                                  buffered write are completed before reset */
  SCB->AIRCR  = ((0x5FA << SCB_AIRCR_VECTKEY_Pos)      |
                 (SCB->AIRCR & SCB_AIRCR_PRIGROUP_Msk) |
                 SCB_AIRCR_SYSRESETREQ_Msk);                   /* Keep priority group unchanged */
  __DSB();                                                     /* Ensure completion of memory access */
  while(1);                                                    /* wait until reset */
}

/*@} end of CMSIS_Core_NVICFunctions */



/* ##################################    SysTick function  ############################################ */
/** \ingroup  CMSIS_Core_FunctionInterface
    \defgroup CMSIS_Core_SysTickFunctions SysTick Functions
    @brief      Functions that configure the System.
  @{
 */

#if (__Vendor_SysTickConfig == 0)

/**@brief  System Tick Configuration

    The function initializes the System Timer and its interrupt, and starts the System Tick Timer.
    Counter is in free running mode to generate periodic interrupts.

    \param [in]  ticks  Number of ticks between two interrupts.

    \return          0  Function succeeded.
    \return          1  Function failed.

    \note     When the variable <b>__Vendor_SysTickConfig</b> is set to 1, then the
    function <b>SysTick_Config</b> is not included. In this case, the file <b><i>device</i>.h</b>
    must contain a vendor-specific implementation of this function.

 */
static PORT_C_INLINE uint32_t SysTick_Config(uint32_t ticks)
{
  if ((ticks - 1) > SysTick_LOAD_RELOAD_Msk)  return (1);      /* Reload value impossible */

  SysTick->LOAD  = ticks - 1;                                  /* set reload register */
  NVIC_SetPriority (SysTick_IRQn, (1<<PORT_ISR_PRIO_BITS) - 1);  /* set Priority for Systick Interrupt */
  SysTick->VAL   = 0;                                          /* Load the SysTick Counter Value */
  SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
                   SysTick_CTRL_TICKINT_Msk   |
                   SysTick_CTRL_ENABLE_Msk;                    /* Enable SysTick IRQ and SysTick Timer */
  return (0);                                                  /* Function successful */
}

#endif

/*@} end of CMSIS_Core_SysTickFunctions */



/* ##################################### Debug In/Output function ########################################### */
/** \ingroup  CMSIS_Core_FunctionInterface
    \defgroup CMSIS_core_DebugFunctions ITM Functions
    @brief   Functions that access the ITM debug interface.
  @{
 */

extern volatile int32_t ITM_RxBuffer;                    /**<@brief External variable to receive characters.                         */
#define                 ITM_RXBUFFER_EMPTY    0x5AA55AA5 /**<@brief Value identifying \ref ITM_RxBuffer is ready for next character. */


/**@brief  ITM Send Character

    The function transmits a character via the ITM channel 0, and
    \li Just returns when no debugger is connected that has booked the output.
    \li Is blocking when a debugger is connected, but the previous character sent has not been transmitted.

    \param [in]     ch  Character to transmit.

    \returns            Character to transmit.
 */
static PORT_C_INLINE uint32_t ITM_SendChar (uint32_t ch)
{
  if ((ITM->TCR & ITM_TCR_ITMENA_Msk)                  &&      /* ITM enabled */
      (ITM->TER & (1ul << 0)        )                    )     /* ITM Port #0 enabled */
  {
    while (ITM->PORT[0].u32 == 0);
    ITM->PORT[0].u8 = (uint8_t) ch;
  }
  return (ch);
}


/**@brief  ITM Receive Character

    The function inputs a character via the external variable \ref ITM_RxBuffer.

    \return             Received character.
    \return         -1  No character pending.
 */
static PORT_C_INLINE int32_t ITM_ReceiveChar (void) {
  int32_t ch = -1;                           /* no character available */

  if (ITM_RxBuffer != ITM_RXBUFFER_EMPTY) {
    ch = ITM_RxBuffer;
    ITM_RxBuffer = ITM_RXBUFFER_EMPTY;       /* ready for next character */
  }

  return (ch);
}


/**@brief  ITM Check Character

    The function checks whether a character is pending for reading in the variable \ref ITM_RxBuffer.

    \return          0  No character available.
    \return          1  Character available.
 */
static PORT_C_INLINE int32_t ITM_CheckChar (void) {

  if (ITM_RxBuffer == ITM_RXBUFFER_EMPTY) {
    return (0);                                 /* no character available */
  } else {
    return (1);                                 /*    character available */
  }
}

#endif

/*--------------------------------------------------------  C++ extern end  --*/
#ifdef __cplusplus
}
#endif

/*==================================================  CONFIGURATION ERRORS  ==*/
/******************************************************************************
 * END of cortex_m3.h
 ******************************************************************************/
#endif /* ES_CORTEX_M3_H_ */
