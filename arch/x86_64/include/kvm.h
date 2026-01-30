/****************************************************************************
* arch/x86_64/include/kvm.h
*
* Licensed to the Apache Software Foundation (ASF) under one or more
* contributor license agreements.  See the NOTICE file distributed with
* this work for additional information regarding copyright ownership.  The
* ASF licenses this file to you under the Apache License, Version 2.0 (the
* "License"); you may not use this file except in compliance with the
* License.  You may obtain a copy of the License at
*
*   http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
* WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
* License for the specific language governing permissions and limitations
* under the License.
*
****************************************************************************/

#ifndef __ARCH_X86_64_INCLUDE_KVM_H
#define __ARCH_X86_64_INCLUDE_KVM_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <stdint.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Hypercall numbers */

#define X86_64_KVM_HC_VAPIC_POLL_IRQ		            1
#define X86_64_KVM_HC_KICK_CPU			                5
#define X86_64_KVM_HC_CLOCK_PAIRING		              9
#define X86_64_KVM_HC_SEND_IPI		                  10
#define X86_64_KVM_HC_SCHED_YIELD		                11

/* Error codes */

#define X86_64_KVM_ENOSYS                           1000
#define X86_64_KVM_EFAULT                           14
#define X86_64_KVM_EINVAL                           22
#define X86_64_KVM_E2BIG                            7
#define X86_64_KVM_EPERM                            1
#define X86_64_KVM_EOPNOTSUPP                       95

/* Cpuid leaves */

#define X86_64_KVM_CPUID_SIGNATURE                  0x40000000
#define X86_64_KVM_CPUID_FEATURES                   0x40000001

/* Cpuid flags for X86_64_KVM_CPUID_SIGNATURE */

#define X86_64_KVM_SIGNATURE_B                      0x4b4d564b
#define X86_64_KVM_SIGNATURE_C                      0x564b4d56
#define X86_64_KVM_SIGNATURE_D                      0x4d

/* Cpuid flags for X86_64_KVM_CPUID_FEATURES.eax */

#define X86_64_KVM_FEATURE_CLOCKSOURCE              (1 << 0)
#define X86_64_KVM_FEATURE_NOP_IO_DELAY             (1 << 1)
#define X86_64_KVM_FEATURE_MMU_OP                   (1 << 2)
#define X86_64_KVM_FEATURE_CLOCKSOURCE2             (1 << 3)
#define X86_64_KVM_FEATURE_ASYNC_PF                 (1 << 4)
#define X86_64_KVM_FEATURE_STEAL_TIME               (1 << 5)
#define X86_64_KVM_FEATURE_PV_EOI                   (1 << 6)
#define X86_64_KVM_FEATURE_PV_UNHALT                (1 << 7)
#define X86_64_KVM_FEATURE_PV_TLB_FLUSH             (1 << 9)
#define X86_64_KVM_FEATURE_ASYNC_PF_VMEXIT          (1 << 10)
#define X86_64_KVM_FEATURE_PV_SEND_IPI              (1 << 11)
#define X86_64_KVM_FEATURE_POLL_CONTROL             (1 << 12)
#define X86_64_KVM_FEATURE_PV_SCHED_YIELD           (1 << 13)
#define X86_64_KVM_FEATURE_ASYNC_PF_INT             (1 << 14)
#define X86_64_KVM_FEATURE_MSI_EXT_DEST_ID          (1 << 15)
#define X86_64_KVM_FEATURE_HC_MAP_GPA_RANGE         (1 << 16)
#define X86_64_KVM_FEATURE_MIGRATION_CONTROL        (1 << 17)
#define X86_64_KVM_FEATURE_CLOCKSOURCE_STABLE_BIT   (1 << 24)

/* Cpuid flags for X86_64_KVM_CPUID_FEATURES.edx */

#define X86_64_KVM_HINTS_REALTIME                   (1 << 0)

/* Synthetic MSR's */

#define MSR_KVM_WALL_CLOCK                          0x11
#define MSR_KVM_SYSTEM_TIME                         0x12

#define MSR_KVM_WALL_CLOCK_NEW                      0x4b564d00
#define MSR_KVM_SYSTEM_TIME_NEW                     0x4b564d01
#define MSR_KVM_ASYNC_PF_EN                         0x4b564d02
#define MSR_KVM_STEAL_TIME                          0x4b564d03
#define MSR_KVM_EOI_EN                              0x4b564d04
#define MSR_KVM_POLL_CONTROL                        0x4b564d05
#define MSR_KVM_ASYNC_PF_INT                        0x4b564d06
#define MSR_KVM_ASYNC_PF_ACK                        0x4b564d07
#define MSR_KVM_MIGRATION_CONTROL                   0x4b564d08

/* MSR_KVM_SYSTEM_TIME_NEW flags */

#define X86_64_KVM_MP_MEASURES_MONOTONIC            (1 << 0)
#define X86_64_KVM_GUEST_VCPU_PAUSED                (1 << 1)

/* Hypercall encoding */

#ifdef CONFIG_ARCH_INTEL64
#   define X86_HYPERCALL ".byte 0x0f, 0x01, 0xc1"
#else 
#   define X86_HYPERCALL ".byte 0x0f, 0x01, 0xd9"
#endif

/****************************************************************************
 * Public Types
 ****************************************************************************/

/* MSR_KVM_WALL_CLOCK_NEW parameter */

begin_packed_struct struct kvm_pvclock_wall_clock_s
{
  uint32_t version;
  uint32_t sec;
  uint32_t nsec;
} end_packed_struct;

/* MSR_KVM_SYSTEM_TIME_NEW parameter */

begin_packed_struct struct kvm_pvclock_vcpu_time_info_s
{
  uint32_t version;
  uint32_t pad0;
  uint64_t tsc_timestamp;
  uint64_t system_time;
  uint32_t tsc_to_system_mul;
  char     tsc_shift;
  uint8_t  flags;
  uint8_t  pad[2];
} end_packed_struct;

/* MSR_KVM_STEAL_TIME parameter */

begin_packed_struct struct kvm_steal_time_s
{
  uint64_t steal;
  uint32_t version;
  uint32_t flags;
  uint8_t  preempted;
  uint8_t  u8_pad[3];
  uint32_t pad[11];
} end_packed_struct;

/****************************************************************************
 * Inline Functions
 ****************************************************************************/

/* KVM hypercall with hypercall number and 4 parameters */

static inline uintptr_t kvm_hypercall4(unsigned int nbr, uintptr_t parm1, 
                                       uintptr_t parm2, uintptr_t parm3, 
                                       uintptr_t parm4)
{
  uintptr_t ret;

  __asm__ volatile 
  (
    X86_HYPERCALL
    : "=a"(ret)
    : "a"(nbr), "b"(parm1), "c"(parm2), "d"(parm3), "S"(parm4)
    : "memory"
  );

  return ret;
}

/* KVM hypercall with hypercall number and 3 parameters */

static inline uintptr_t kvm_hypercall3(unsigned int nbr, uintptr_t parm1,
                                       uintptr_t parm2, uintptr_t parm3)
{
  return kvm_hypercall4(nbr, parm1, parm2, parm3, 0);
}

/* KVM hypercall with hypercall number and 2 parameters */

static inline uintptr_t kvm_hypercall2(unsigned int nbr, uintptr_t parm1,
                                       uintptr_t parm2)
{
  return kvm_hypercall4(nbr, parm1, parm2, 0, 0);
}

/* KVM hypercall with hypercall number and 1 parameter */

static inline uintptr_t kvm_hypercall1(unsigned int nbr, uintptr_t parm1)
{
  return kvm_hypercall4(nbr, parm1, 0, 0, 0);
}

/* KVM hypercall with hypercall number and no parameters */

static inline uintptr_t kvm_hypercall0(unsigned int nbr)
{
  return kvm_hypercall4(nbr, 0, 0, 0, 0);
}

#endif /* __ARCH_X86_64_INCLUDE_KVM_H */