#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
#include <errno.h>

#include "debug.h"
#include "box86context.h"
#include "dynarec.h"
#include "emu/x86emu_private.h"
#include "emu/x86run_private.h"
#include "x86run.h"
#include "x86emu.h"
#include "box86stack.h"
#include "callback.h"
#include "emu/x86run_private.h"
#include "x86trace.h"
#include "dynarec_arm.h"
#include "dynarec_arm_private.h"
#include "arm_printer.h"

#include "dynarec_arm_functions.h"
#include "dynarec_arm_helper.h"


uintptr_t dynarecDC(dynarec_arm_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, int* ok, int* need_epilog)
{
    uint8_t nextop = F8;
    uint8_t wback;
    int v1, v2;
    int d1;
    int fixedaddress;
    int parity;
    uint8_t u8;

    MAYUSE(d1);
    MAYUSE(v2);
    MAYUSE(v1);

    switch(nextop) {
        case 0xC0:
        case 0xC1:
        case 0xC2:
        case 0xC3:
        case 0xC4:
        case 0xC5:
        case 0xC6:
        case 0xC7:
            INST_NAME("FADD STx, ST0");
            v2 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v1 = x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_COMBINE(0, nextop&7));
            if(!box86_dynarec_fastround)
                u8 = x87_setround(dyn, ninst, x1, x2, x14);
            if(ST_IS_F(0)) {
                VADD_F32(v1, v1, v2);
            } else {
                VADD_F64(v1, v1, v2);
            }
            if(!box86_dynarec_fastround)
                x87_restoreround(dyn, ninst, u8);
            break;
        case 0xC8:
        case 0xC9:
        case 0xCA:
        case 0xCB:
        case 0xCC:
        case 0xCD:
        case 0xCE:
        case 0xCF:
            INST_NAME("FMUL STx, ST0");
            v2 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v1 = x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_COMBINE(0, nextop&7));
            if(!box86_dynarec_fastround)
                u8 = x87_setround(dyn, ninst, x1, x2, x14);
            if(ST_IS_F(0)) {
                VMUL_F32(v1, v1, v2);
            } else {
                VMUL_F64(v1, v1, v2);
            }
            if(!box86_dynarec_fastround)
                x87_restoreround(dyn, ninst, u8);
            break;
        case 0xD0:
        case 0xD1:
        case 0xD2:
        case 0xD3:
        case 0xD4:
        case 0xD5:
        case 0xD6:
        case 0xD7:
            INST_NAME("FCOM ST0, STx"); //yep
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_COMBINE(0, nextop&7));
            if(ST_IS_F(0)) {
                VCMP_F32(v1, v2);
            } else {
                VCMP_F64(v1, v2);
            }
            FCOM(x1, x2, x3, x14, v1, v2, ST_IS_F(0));
            break;
        case 0xD8:
        case 0xD9:
        case 0xDA:
        case 0xDB:
        case 0xDC:
        case 0xDD:
        case 0xDE:
        case 0xDF:
            INST_NAME("FCOMP ST0, STx");
            v1 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v2 = x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_COMBINE(0, nextop&7));
            if(ST_IS_F(0)) {
                VCMP_F32(v1, v2);
            } else {
                VCMP_F64(v1, v2);
            }
            FCOM(x1, x2, x3, x14, v1, v2, ST_IS_F(0));
            X87_POP_OR_FAIL(dyn, ninst, x3);
            break;
        case 0xE0:
        case 0xE1:
        case 0xE2:
        case 0xE3:
        case 0xE4:
        case 0xE5:
        case 0xE6:
        case 0xE7:
            INST_NAME("FSUBR STx, ST0");
            v2 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v1 = x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_COMBINE(0, nextop&7));
            if(!box86_dynarec_fastround)
                u8 = x87_setround(dyn, ninst, x1, x2, x14);
            if(ST_IS_F(0)) {
                VSUB_F32(v1, v2, v1);
            } else {
                VSUB_F64(v1, v2, v1);
            }
            if(!box86_dynarec_fastround)
                x87_restoreround(dyn, ninst, u8);
            break;
        case 0xE8:
        case 0xE9:
        case 0xEA:
        case 0xEB:
        case 0xEC:
        case 0xED:
        case 0xEE:
        case 0xEF:
            INST_NAME("FSUB STx, ST0");
            v2 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v1 = x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_COMBINE(0, nextop&7));
            if(!box86_dynarec_fastround)
                u8 = x87_setround(dyn, ninst, x1, x2, x14);
            if(ST_IS_F(0)) {
                VSUB_F32(v1, v1, v2);
            } else {
                VSUB_F64(v1, v1, v2);
            }
            if(!box86_dynarec_fastround)
                x87_restoreround(dyn, ninst, u8);
            break;
        case 0xF0:
        case 0xF1:
        case 0xF2:
        case 0xF3:
        case 0xF4:
        case 0xF5:
        case 0xF6:
        case 0xF7:
            INST_NAME("FDIVR STx, ST0");
            v2 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v1 = x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_COMBINE(0, nextop&7));
            if(!box86_dynarec_fastround)
                u8 = x87_setround(dyn, ninst, x1, x2, x14);
            if(ST_IS_F(0)) {
                VDIV_F32(v1, v2, v1);
            } else {
                VDIV_F64(v1, v2, v1);
            }
            if(!box86_dynarec_fastround)
                x87_restoreround(dyn, ninst, u8);
            break;       
        case 0xF8:
        case 0xF9:
        case 0xFA:
        case 0xFB:
        case 0xFC:
        case 0xFD:
        case 0xFE:
        case 0xFF:
            INST_NAME("FDIV STx, ST0");
            v2 = x87_get_st(dyn, ninst, x1, x2, 0, X87_COMBINE(0, nextop&7));
            v1 = x87_get_st(dyn, ninst, x1, x2, nextop&7, X87_COMBINE(0, nextop&7));
            if(!box86_dynarec_fastround)
                u8 = x87_setround(dyn, ninst, x1, x2, x14);
            if(ST_IS_F(0)) {
                VDIV_F32(v1, v1, v2);
            } else {
                VDIV_F64(v1, v1, v2);
            }
            if(!box86_dynarec_fastround)
                x87_restoreround(dyn, ninst, u8);
            break;
        default:
            switch((nextop>>3)&7) {
                case 0:
                    INST_NAME("FADD ST0, double[ED]");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
                    d1 = fpu_get_scratch_double(dyn);
                    parity = getedparity(dyn, ninst, addr, nextop, 2);
                    if(parity) {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 1023, 3, 0, NULL);
                        VLDR_64(d1, wback, fixedaddress);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x1, &fixedaddress, 4095-4, 0, 0, NULL);
                        LDR_IMM9(x2, wback, fixedaddress);
                        LDR_IMM9(x3, wback, fixedaddress+4);
                        VMOVtoV_D(d1, x2, x3);
                    }
                    if(!box86_dynarec_fastround)
                        u8 = x87_setround(dyn, ninst, x1, x2, x14);
                    VADD_F64(v1, v1, d1);
                    if(!box86_dynarec_fastround)
                        x87_restoreround(dyn, ninst, u8);
                    break;
                case 1:
                    INST_NAME("FMUL ST0, double[ED]");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
                    d1 = fpu_get_scratch_double(dyn);
                    parity = getedparity(dyn, ninst, addr, nextop, 2);
                    if(parity) {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 1023, 3, 0, NULL);
                        VLDR_64(d1, wback, fixedaddress);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x1, &fixedaddress, 4095-4, 0, 0, NULL);
                        LDR_IMM9(x2, wback, fixedaddress);
                        LDR_IMM9(x3, wback, fixedaddress+4);
                        VMOVtoV_D(d1, x2, x3);
                    }
                    if(!box86_dynarec_fastround)
                        u8 = x87_setround(dyn, ninst, x1, x2, x14);
                    VMUL_F64(v1, v1, d1);
                    if(!box86_dynarec_fastround)
                        x87_restoreround(dyn, ninst, u8);
                    break;
                case 2:
                    INST_NAME("FCOM ST0, double[ED]");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
                    d1 = fpu_get_scratch_double(dyn);
                    parity = getedparity(dyn, ninst, addr, nextop, 2);
                    if(parity) {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 1023, 3, 0, NULL);
                        VLDR_64(d1, wback, fixedaddress);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x1, &fixedaddress, 4095-4, 0, 0, NULL);
                        LDR_IMM9(x2, wback, fixedaddress);
                        LDR_IMM9(x3, wback, fixedaddress+4);
                        VMOVtoV_D(d1, x2, x3);
                    }
                    VCMP_F64(v1, d1);
                    FCOM(x1, x2, x3, x14, v1, d1, 0);
                    break;
                case 3:
                    INST_NAME("FCOMP ST0, double[ED]");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
                    d1 = fpu_get_scratch_double(dyn);
                    parity = getedparity(dyn, ninst, addr, nextop, 2);
                    if(parity) {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 1023, 3, 0, NULL);
                        VLDR_64(d1, wback, fixedaddress);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x1, &fixedaddress, 4095-4, 0, 0, NULL);
                        LDR_IMM9(x2, wback, fixedaddress);
                        LDR_IMM9(x3, wback, fixedaddress+4);
                        VMOVtoV_D(d1, x2, x3);
                    }
                    VCMP_F64(v1, d1);
                    FCOM(x1, x2, x3, x14, v1, d1, 0);
                    X87_POP_OR_FAIL(dyn, ninst, x3);
                    break;
                case 4:
                    INST_NAME("FSUB ST0, double[ED]");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
                    d1 = fpu_get_scratch_double(dyn);
                    parity = getedparity(dyn, ninst, addr, nextop, 2);
                    if(parity) {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 1023, 3, 0, NULL);
                        VLDR_64(d1, wback, fixedaddress);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x1, &fixedaddress, 4095-4, 0, 0, NULL);
                        LDR_IMM9(x2, wback, fixedaddress);
                        LDR_IMM9(x3, wback, fixedaddress+4);
                        VMOVtoV_D(d1, x2, x3);
                    }
                    if(!box86_dynarec_fastround)
                        u8 = x87_setround(dyn, ninst, x1, x2, x14);
                    VSUB_F64(v1, v1, d1);
                    if(!box86_dynarec_fastround)
                        x87_restoreround(dyn, ninst, u8);
                    break;
                case 5:
                    INST_NAME("FSUBR ST0, double[ED]");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
                    d1 = fpu_get_scratch_double(dyn);
                    parity = getedparity(dyn, ninst, addr, nextop, 2);
                    if(parity) {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 1023, 3, 0, NULL);
                        VLDR_64(d1, wback, fixedaddress);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x1, &fixedaddress, 4095-4, 0, 0, NULL);
                        LDR_IMM9(x2, wback, fixedaddress);
                        LDR_IMM9(x3, wback, fixedaddress+4);
                        VMOVtoV_D(d1, x2, x3);
                    }
                    if(!box86_dynarec_fastround)
                        u8 = x87_setround(dyn, ninst, x1, x2, x14);
                    VSUB_F64(v1, d1, v1);
                    if(!box86_dynarec_fastround)
                        x87_restoreround(dyn, ninst, u8);
                    break;
                case 6:
                    INST_NAME("FDIV ST0, double[ED]");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
                    d1 = fpu_get_scratch_double(dyn);
                    parity = getedparity(dyn, ninst, addr, nextop, 2);
                    if(parity) {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 1023, 3, 0, NULL);
                        VLDR_64(d1, wback, fixedaddress);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x1, &fixedaddress, 4095-4, 0, 0, NULL);
                        LDR_IMM9(x2, wback, fixedaddress);
                        LDR_IMM9(x3, wback, fixedaddress+4);
                        VMOVtoV_D(d1, x2, x3);
                    }
                    if(!box86_dynarec_fastround)
                        u8 = x87_setround(dyn, ninst, x1, x2, x14);
                    VDIV_F64(v1, v1, d1);
                    if(!box86_dynarec_fastround)
                        x87_restoreround(dyn, ninst, u8);
                    break;
                case 7:
                    INST_NAME("FDIVR ST0, double[ED]");
                    v1 = x87_get_st(dyn, ninst, x1, x2, 0, NEON_CACHE_ST_D);
                    d1 = fpu_get_scratch_double(dyn);
                    parity = getedparity(dyn, ninst, addr, nextop, 2);
                    if(parity) {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x3, &fixedaddress, 1023, 3, 0, NULL);
                        VLDR_64(d1, wback, fixedaddress);
                    } else {
                        addr = geted(dyn, addr, ninst, nextop, &wback, x1, &fixedaddress, 4095-4, 0, 0, NULL);
                        LDR_IMM9(x2, wback, fixedaddress);
                        LDR_IMM9(x3, wback, fixedaddress+4);
                        VMOVtoV_D(d1, x2, x3);
                    }
                    if(!box86_dynarec_fastround)
                        u8 = x87_setround(dyn, ninst, x1, x2, x14);
                    VDIV_F64(v1, d1, v1);
                    if(!box86_dynarec_fastround)
                        x87_restoreround(dyn, ninst, u8);
                    break;
            }
    }
    return addr;
}

