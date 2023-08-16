/*
 * ex1.c
 *
 * RISC-V ISA: RV64I
 *
 * Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
 * All Rights Reserved
 * contact@tactcomplabs.com
 *
 * See LICENSE in the top level directory for licensing details
 *
 */

#include <stdlib.h>
#include <stdint.h>

#define _PAN_COMPLETION_ADDR_ 0x30000000

int main(int argc, char **argv){
  uint64_t *ptr  = (uint64_t *)(_PAN_COMPLETION_ADDR_);
  volatile uint64_t value = *ptr;
  asm volatile("xor x0,x0,x0");  // trace off 0x4033
  while( value == 0x00ull){
    value = *ptr;
  }
  asm volatile("xor x0,x0,x0");  // trace on 0x4033

  return 0;
}
