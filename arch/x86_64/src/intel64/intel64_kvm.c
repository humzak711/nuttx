/****************************************************************************
 * arch/x86_64/src/intel64/intel64_kvm.c
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

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <arch/kvm.h>

#include <errno.h>
#include <stdint.h>

#include <nuttx/sched.h>
#include "x86_internal.h"
#include "intel64_cpu.h"
#include "intel64_kvm.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define X86_KVM_APREEMPTIVE     (1 << 0)
#define X86_KVM_PV_SPINLOCKS    (1 << 1)
#define X86_KVM_PV_IPIS         (1 << 2)
#define X86_KVM_PV_SCHED_YIELD  (1 << 3)

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct kvm_s 
{
  bool kvm_active;
  uint8_t kvm_features;
};

/****************************************************************************
 * Private Data
 ****************************************************************************/

static struct kvm_s g_kvm_priv[CONFIG_SMP_NCPUS];
static bool g_kvm_pv_enabled;

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void x86_64_kvm_priv_init(void)
{
  uint32_t max;
  uint32_t sig_b;
  uint32_t sig_c;
  uint32_t sig_d;
  uint32_t eax;
  uint32_t ebx;
  uint32_t ecx;
  uint32_t edx;
  int kvm_apreemptive_en;
  int kvm_pv_spinlocks_en;
  int kvm_pv_ipis_en;
  int kvm_pv_sched_yield_en;
  struct kvm_s *g_kvm;

  x86_64_cpuid(X86_64_KVM_CPUID_SIGNATURE, 0, &max, &sig_b, &sig_c, &sig_d);

  if (sig_b != X86_64_KVM_SIGNATURE_B || sig_c != X86_64_KVM_SIGNATURE_C ||
      sig_d != X86_64_KVM_SIGNATURE_D || max < X86_64_KVM_CPUID_FEATURES)
    {
      return;    
    }

  x86_64_cpuid(X86_64_KVM_CPUID_FEATURES, 0, &eax, &ebx, &ecx, &edx);

  kvm_apreemptive_en = edx & X86_64_KVM_HINTS_REALTIME;
  kvm_pv_spinlocks_en = eax & X86_64_KVM_FEATURE_PV_UNHALT;
  kvm_pv_ipis_en = eax & X86_64_KVM_FEATURE_PV_SEND_IPI;
  kvm_pv_sched_yield_en = eax & X86_64_KVM_FEATURE_PV_SCHED_YIELD;

  g_kvm = &g_kvm_priv[this_cpu()];

  g_kvm->kvm_active = true;

  g_kvm->kvm_features |= (kvm_apreemptive_en << X86_KVM_APREEMPTIVE);
  g_kvm->kvm_features |= (kvm_pv_spinlocks_en << X86_KVM_PV_SPINLOCKS);
  g_kvm->kvm_features |= (kvm_pv_ipis_en << X86_KVM_PV_IPIS);
  g_kvm->kvm_features |= (kvm_pv_sched_yield_en << X86_KVM_PV_SCHED_YIELD);
}

int x86_64_kvm_sched_yield(void)
{
  struct kvm_s *g_kvm; 
  uint8_t features;

  g_kvm = &g_kvm_priv[this_cpu()];
  features = g_kvm->kvm_features;

  if ((features & X86_KVM_APREEMPTIVE) != 0 || 
      (features & X86_KVM_PV_SCHED_YIELD) != 0)
    {
      return -EOPNOTSUPP;
    }

  kvm_hypercall0(X86_64_KVM_HC_SCHED_YIELD);

  return 0;
}