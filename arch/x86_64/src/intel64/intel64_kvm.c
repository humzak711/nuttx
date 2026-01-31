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

#include <arch/kvm.h>

#include <errno.h>
#include <stdint.h>

#include <nuttx/sched.h>
#include <nuttx/arch.h>
#include <nuttx/compiler.h>

#include <unistd.h>
#include <assert.h>
#include "x86_64_internal.h"
#include "intel64_cpu.h"
#include "intel64_kvm.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define INTEL64_KVM_APREEMPTIVE     (1 << 0)
#define INTEL64_KVM_PV_SCHED_YIELD  (1 << 1)

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct intel64_kvm_priv_s 
{
  uint32_t spin_count;
  uint8_t kvm_features;
};

/****************************************************************************
 * Private Data
 ****************************************************************************/

static struct intel64_kvm_priv_s g_kvm_priv[CONFIG_SMP_NCPUS];

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
  struct intel64_kvm_priv_s *priv;

  x86_64_cpuid(X86_64_KVM_CPUID_SIGNATURE, 0, &max, &sig_b, &sig_c, &sig_d);

  if (sig_b != X86_64_KVM_SIGNATURE_B || sig_c != X86_64_KVM_SIGNATURE_C ||
      sig_d != X86_64_KVM_SIGNATURE_D || max < X86_64_KVM_CPUID_FEATURES)
    {
      return;
    }

  x86_64_cpuid(X86_64_KVM_CPUID_FEATURES, 0, &eax, &ebx, &ecx, &edx);

  UNUSED(ebx);
  UNUSED(ecx);

  priv = &g_kvm_priv[this_cpu()];

  if ((edx & X86_64_KVM_HINTS_REALTIME) != 0)
    {
      priv->kvm_features |= INTEL64_KVM_APREEMPTIVE;
    }

  if ((eax & X86_64_KVM_FEATURE_PV_SCHED_YIELD) != 0)
    {
      priv->kvm_features |= INTEL64_KVM_PV_SCHED_YIELD;
    }
}

#ifdef CONFIG_ARCH_INTEL64_KVM_PV_SPINLOCKS

void x86_64_kvm_pv_spin_wait(void)
{
  struct intel64_kvm_priv_s *priv; 
  uint8_t features;

  priv = &g_kvm_priv[this_cpu()];
  features = priv->kvm_features;

  if ((features & INTEL64_KVM_APREEMPTIVE) != 0 || 
      (features & INTEL64_KVM_PV_SCHED_YIELD) == 0)
    {
      __asm__ volatile ("pause");
    }
  else if (priv->spin_count < CONFIG_ARCH_INTEL64_KVM_PV_SPIN_THRESHOLD)
    {
      priv->spin_count++;
      __asm__ volatile ("pause");
    }
  else
    {
      kvm_hypercall0(X86_64_KVM_HC_SCHED_YIELD);
      priv->spin_count = 0;
    }
}

#endif