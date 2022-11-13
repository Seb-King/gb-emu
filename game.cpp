#include <SDL.h>
#include <iostream>
#include <stdint.h>
#include <vector>
#include <bitset>
#include <iomanip>
#include "emu.hpp"
#include "game.hpp"

int flag = 0;
const int clockrate = 4194304;

u16 LCDC = 0xFF40;
u16 STAT = 0xFF41;  // LCDC status (interrupts and memory access)
u16 SCY = 0xFF42; 	// - Scroll Y
u16 SCX = 0xFF43; 	// - Scroll X
u16 LY = 0xFF44;		//0xFF44 - LCDC Y-Coordinate (the vertical line to which present data is transferred to the LCD driver)
u16 LYC = 0xFF45;		//0xFF45 - LY Compare : Compares itself with the LY. If the values ar ethe same, set the coincedent flag 
// That is Bit 6 of STAT

u16 DMA = 0xFF46; 	//0xFF46 - Direct Memory Access Transfer
u16 BGP = 0xFF47; 	//0xFF47 - Background Palette Data  0b11 10 01 00

// wrapper to allow two 8 bit registers to be used as a single 16 bit register
u16 reg::val() { return lo + (hi << 8); }

void reg::set(u16 x) {
    hi = x >> 8;
    lo = (x << 8) >> 8; // this should chop off the most significant 4 bits
}

namespace RUPS {
    u8 IF;

    //  Interrupt Enable 0xFFFF if a bit is 0 it disables the corresponding interrupt in IF
    u8 IE;
}

namespace LCD {
    int scanline_count = 456;
}

namespace RAM {
    u8 ie = 0;
}

namespace CPU
{
    u8 IME = 0;
    std::vector<std::string> decoder(256);
    std::vector<void (*)()> op_codes(256, op_not_imp);
    std::vector<void (*)()> cb_codes(256, cb_not_imp);


    bool halt = false;
    extern u16 PC = 0; //Program Counter: holds the address of the current instruction being fetched from memory
    // Needs to be incremented after reading any opcode (note that opcodes can change the counter

    u16 SP; //Stack Pointer: holds the address of the current top of stack located in external RAM

    // Could be signed integers, I'm not sure, not even sure that the GB uses these registers

    // F is the the flag register Z N H C 0 0 0 0, Z - zero flag, N - subtract flag, H - half carry, C - carry flag
    reg AF, BC, DE, HL;

    int interrupt_disable = 0;

    int cycles = 4;
    int timing = 0;

    void STAT() {

        RAM::write(CPU::PC & 0x0F, CPU::SP);
        CPU::SP--;
        RAM::write((CPU::PC & 0xF0) >> 8, CPU::SP);
        CPU::PC = 0x0048;

        // now get rid of request flag IF
        RUPS::IF = RAM::readAt(0xFF0F);
        RUPS::IF &= 0b11111101;
        RAM::write(RUPS::IF, 0xFF0F);

    }

    void op_not_imp() {
        u8 opcode = RAM::readAt(PC - 1);
        std::cout << "Opcode not implemented : OP = " << std::hex << unsigned(opcode) << std::endl;
        std::cout << "PC:" << std::hex << unsigned(PC) << std::endl;
        exit(0);
    }


    void cb_not_imp() {
        u8 opcode = RAM::readAt(PC - 1);
        std::cout << "CB Opcode not implemented : OP = " << std::hex << unsigned(opcode) << std::endl;
        exit(0);
    }

    void run_cb() {
        u8 op_code = RAM::read();
        cb_codes[op_code]();
    }

    void runOPCode(u8 op_code) {
        op_codes[op_code]();

        if (interrupt_disable > 0) {
            if (interrupt_disable == 1)
            {
                IME = 0;
                interrupt_disable = 0;
                //std::cout << "Interrupts Disabled. PC: " << std::hex << PC << std::endl << std::endl;
            }
            else if (interrupt_disable == 2) {
                interrupt_disable--;
            }

            if (interrupt_disable == 3) {
                IME = 1;
                //std::cout << "Interrupts Enabled. PC: " << std::hex << PC << std::endl << std::endl;
                interrupt_disable = 0;
            }
            else if (interrupt_disable == 4) {
                interrupt_disable--;
            }
        }
    }

    void init_opcodes() {
        op_codes[0x00] = NOP;
        op_codes[0x06] = LDb_n; // load 
        op_codes[0x0E] = LDc_n;              // how do we even handle what n is? Where should it  
        op_codes[0x16] = LDd_n;
        op_codes[0x1E] = LDe_n;
        op_codes[0x26] = LDh_n;
        op_codes[0x2E] = LDl_n;
        // Load  LD r1r2, load r2 into r1
        op_codes[0x7F] = op_not_imp; // loads A into A which is pointless and we should get aw 
        // although it does take 4 cpu cycles anyway, so maybe things break without it
        op_codes[0x78] = LDrr_ab;
        op_codes[0x79] = LDrr_ac;
        op_codes[0x7A] = LDrr_ad;
        op_codes[0x7B] = LDrr_ae;
        op_codes[0x7C] = LDrr_ah;
        op_codes[0x7D] = LDrr_al;
        op_codes[0x7E] = LDrr_aHL;
        op_codes[0x0A] = LDrr_aBC;
        op_codes[0x1A] = LDrr_aDE;
        op_codes[0xFA] = LDrr_ann;
        op_codes[0x3E] = LDrr_a_hash; // what the fuck is this op shiposed  
        op_codes[0x40] = op_not_imp; // 
        op_codes[0x41] = LDrr_bc;
        op_codes[0x42] = LDrr_bd;
        op_codes[0x43] = LDrr_be;
        op_codes[0x44] = LDrr_bh;
        op_codes[0x45] = LDrr_bl;
        op_codes[0x46] = LDrr_bHL; // now i'm wondering where is the A into 
        op_codes[0x47] = LDrr_ba;
        op_codes[0x48] = LDrr_cb;
        op_codes[0x49] = op_not_imp;
        op_codes[0x4A] = LDrr_cd;
        op_codes[0x4B] = LDrr_ce;
        op_codes[0x4C] = LDrr_ch;
        op_codes[0x4D] = LDrr_cl;
        op_codes[0x4E] = LDrr_cHL;
        op_codes[0x4F] = LDrr_ca;
        op_codes[0x50] = LDrr_db;
        op_codes[0x51] = LDrr_dc;
        op_codes[0x52] = op_not_imp;
        op_codes[0x53] = LDrr_de;
        op_codes[0x54] = LDrr_dh;
        op_codes[0x55] = LDrr_dl;
        op_codes[0x56] = LDrr_dHL;
        op_codes[0x57] = LDrr_da;
        op_codes[0x58] = LDrr_eb;
        op_codes[0x59] = LDrr_ec;
        op_codes[0x5A] = LDrr_ed;
        op_codes[0x5B] = op_not_imp;
        op_codes[0x5C] = LDrr_eh;
        op_codes[0x5D] = LDrr_el;
        op_codes[0x5E] = LDrr_eHL;
        op_codes[0x5F] = LDrr_ea;
        op_codes[0x60] = LDrr_hb;
        op_codes[0x61] = LDrr_hc;
        op_codes[0x62] = LDrr_hd;
        op_codes[0x63] = LDrr_he;
        op_codes[0x64] = op_not_imp;
        op_codes[0x65] = LDrr_hl;
        op_codes[0x66] = LDrr_hHL;
        op_codes[0x67] = LDrr_ha;
        op_codes[0x68] = LDrr_lb;
        op_codes[0x69] = LDrr_lc;
        op_codes[0x6A] = LDrr_ld;
        op_codes[0x6B] = LDrr_le;
        op_codes[0x6C] = LDrr_lh;
        op_codes[0x6D] = op_not_imp;
        op_codes[0x6E] = LDrr_lHL;
        op_codes[0x6F] = LDrr_la;
        op_codes[0x70] = LDrr_HLb;
        op_codes[0x71] = LDrr_HLc;
        op_codes[0x72] = LDrr_HLd;
        op_codes[0x73] = LDrr_HLe;
        op_codes[0x74] = LDrr_HLh;
        op_codes[0x75] = LDrr_HLl;
        op_codes[0x36] = LDrr_HLn;

        op_codes[0xF2] = LDa_c; // put value at address 0xFF00 + reg 
        op_codes[0xE2] = LDc_a; // put A into address (0xFF00 + reg C)
        op_codes[0x3A] = LDDaHL; // put value at address HL into A th 
        op_codes[0x32] = LDDHLa; // Put A into memory address HL th 
        op_codes[0x2A] = LDI_aHL; // Put value at address HL into A then increment HL
        op_codes[0x22] = LDI_HLa; // put A into mem address . 
        op_codes[0xE0] = LDH_nA; // Put A into memory address 0xFF00 + n
        op_codes[0xF0] = LDH_a_ffn; // Put memory address 0xFF00 + 


        // --------- 16-Bit Loads --------- //

          // LDnn,n put value nn into n
        op_codes[0x01] = LD_nn_BC;
        op_codes[0x11] = LD_nn_DE;
        op_codes[0x21] = LD_nn_HL;
        op_codes[0x31] = LD_nn_SP;

        // LD SP,HL put HL into SP
        op_codes[0xF9] = LD_SPHL;
        op_codes[0xF8] = LDHL_SPn; // Put SP+n effective address into HL, n is signed, fla 
        op_codes[0x08] = LD_nnSP; //nn two byte immediate address, Put SP at address n

        // Push register pair nn onto stack decrement SP twice
        op_codes[0xF5] = PUSH_AF;
        op_codes[0xC5] = PUSH_BC;
        op_codes[0xD5] = PUSH_DE;
        op_codes[0xE5] = PUSH_HL;

        // Pop nn, pop two bytes off stack into register pair nn, increment Stack pointer twice
        op_codes[0xF1] = POP_AF;
        op_codes[0xC1] = POP_BC;
        op_codes[0xD1] = POP_DE;
        op_codes[0xE1] = POP_HL;

        // 8-Bit ALU
        // ADD A,n -- add n to A

        op_codes[0x87] = ADD_aa;
        op_codes[0x80] = ADD_ab;
        op_codes[0x81] = ADD_ac;
        op_codes[0x82] = ADD_ad;
        op_codes[0x83] = ADD_ae;
        op_codes[0x84] = ADD_ah;
        op_codes[0x85] = ADD_al;
        op_codes[0x86] = ADD_aH;
        op_codes[0xC6] = ADD_a_hash;

        // ADC A,n -- add n + carry flag to A
        op_codes[0x8f] = ADC_aa;
        op_codes[0x88] = ADC_ab;
        op_codes[0x89] = ADC_ac;
        op_codes[0x8A] = ADC_ad;
        op_codes[0x8B] = ADC_ae;
        op_codes[0x8C] = ADC_ah;
        op_codes[0x8D] = ADC_al;
        op_codes[0x8E] = ADC_aHL;
        op_codes[0xCE] = ADC_a_hash;

        // SUB n, subtract n from A
        op_codes[0x97] = SUB_a;
        op_codes[0x90] = SUB_b;
        op_codes[0x91] = SUB_c;
        op_codes[0x92] = SUB_d;
        op_codes[0x93] = SUB_e;
        op_codes[0x94] = SUB_h;
        op_codes[0x95] = SUB_l;
        op_codes[0x96] = SUB_HL;
        op_codes[0xD6] = SUB_hash;

        op_codes[0x9F] = SBC_a;
        op_codes[0x98] = SBC_b;
        op_codes[0x99] = SBC_c;
        op_codes[0x9A] = SBC_d;
        op_codes[0x9B] = SBC_e;
        op_codes[0x9C] = SBC_h;
        op_codes[0x9D] = SBC_l;
        op_codes[0x9E] = SBC_HL;

        // ------ Logical Operations ------ //
        // Check flags in each operation

        // AND n, logically AND n with A, result in A
        op_codes[0xA7] = AND_a;
        op_codes[0xA0] = AND_b;
        op_codes[0xA1] = AND_c;
        op_codes[0xA2] = AND_d;
        op_codes[0xA3] = AND_e;
        op_codes[0xA4] = AND_h;
        op_codes[0xA5] = AND_l;
        op_codes[0xA6] = AND_HL;
        op_codes[0xE6] = AND_hash;

        // Logical OR with register A, result in A
        op_codes[0xB7] = OR_a;
        op_codes[0xB0] = OR_b;
        op_codes[0xB1] = OR_c;
        op_codes[0xB2] = OR_d;
        op_codes[0xB3] = OR_e;
        op_codes[0xB4] = OR_h;
        op_codes[0xB5] = OR_l;
        op_codes[0xB6] = OR_HL;
        op_codes[0xF6] = OR_hash;

        // Logical exclusive OR n with register A, result in A

        op_codes[0xAF] = XOR_a;
        op_codes[0xA8] = XOR_b;
        op_codes[0xA9] = XOR_c;
        op_codes[0xAA] = XOR_d;
        op_codes[0xAB] = XOR_e;
        op_codes[0xAC] = XOR_h;
        op_codes[0xAD] = XOR_l;
        op_codes[0xAE] = XOR_HL;
        op_codes[0xEE] = XOR_hash;

        // Copmare A with n. Basically an A - n subtraction instruct but no results
        // I think the point is to set flags here 
        op_codes[0xBF] = CP_a;
        op_codes[0xB8] = CP_b;
        op_codes[0xB9] = CP_c;
        op_codes[0xBA] = CP_d;
        op_codes[0xBB] = CP_e;
        op_codes[0xBC] = CP_h;
        op_codes[0xBD] = CP_l;
        op_codes[0xBE] = CP_HL;
        op_codes[0xFE] = CP_hash;

        // increment, pretty self explanatory
        op_codes[0x3C] = INC_a;
        op_codes[0x04] = INC_b;
        op_codes[0x0C] = INC_c;
        op_codes[0x14] = INC_d;
        op_codes[0x1C] = INC_e;
        op_codes[0x24] = INC_h;
        op_codes[0x2C] = INC_l;
        op_codes[0x34] = INC_HLad;

        op_codes[0x3D] = DEC_a;
        op_codes[0x05] = DEC_b;
        op_codes[0x0D] = DEC_c;
        op_codes[0x15] = DEC_d;
        op_codes[0x1D] = DEC_e;
        op_codes[0x25] = DEC_h;
        op_codes[0x2D] = DEC_l;
        op_codes[0x35] = DEC_HLad;

        // --16-bit arith-- //

        // add n to HL and set some flags
        op_codes[0x09] = ADD_BC;
        op_codes[0x19] = ADD_DE;
        op_codes[0x29] = ADD_HL;
        op_codes[0x39] = ADD_SP;


        // -------------------------------//
        // In this section i'm just going to implement opcodes until I can get the boot process of the gb going
        op_codes[0xC7] = RST_00;
        op_codes[0xCF] = RST_08;
        op_codes[0xD7] = RST_10;
        op_codes[0xDF] = RST_18;
        op_codes[0xE7] = RST_20;
        op_codes[0xEF] = RST_28;
        op_codes[0xF7] = RST_30;
        op_codes[0xFF] = RST_38;

        op_codes[0xCB] = run_cb;
        op_codes[0x20] = JR_NZ;
        op_codes[0x28] = JR_Z;
        op_codes[0x18] = JR_n;
        op_codes[0xC3] = JP;
        op_codes[0xC2] = JP_NZ;
        op_codes[0xCA] = JP_Z;
        op_codes[0xD2] = JP_NC;
        op_codes[0xDA] = JP_C;
        op_codes[0xE9] = JP_HL;


        op_codes[0x77] = LD_HL_a;
        op_codes[0x02] = LD_BC_a;
        op_codes[0x12] = LD_DE_a;
        op_codes[0xE0] = LDad_n_a;
        op_codes[0xCD] = CALL_nn;
        op_codes[0xC4] = CALL_NZ;
        op_codes[0xCC] = CALL_Z;
        op_codes[0xD4] = CALL_NC;
        op_codes[0xDc] = CALL_C;

        op_codes[0xEA] = LD_nn_a; // put value A into the address nn 
        op_codes[0xC9] = RET;
        op_codes[0xC0] = RET_NZ;
        op_codes[0xC8] = RET_Z;
        op_codes[0xD0] = RET_NC;
        op_codes[0xD8] = RET_C;
        op_codes[0xD9] = RETI;
        op_codes[0x17] = RLA;
        op_codes[0x07] = RLCA;
        op_codes[0x0F] = RRCA;
        op_codes[0xF3] = DI; // TODO: Disable interrupts after the next  instruction
        op_codes[0xFB] = EI;
        op_codes[0xE8] = ADD_n_SP;
        op_codes[0x03] = INC_BC;
        op_codes[0x13] = INC_DE;
        op_codes[0x23] = INC_HL;
        op_codes[0x33] = INC_SP;
        op_codes[0x0B] = DEC_BC;
        op_codes[0x1B] = DEC_DE;
        op_codes[0x2B] = DEC_HL;
        op_codes[0x3B] = DEC_SP;
        op_codes[0x76] = HALT;
        op_codes[0x27] = DAA;
        op_codes[0x2F] = CPL;
        op_codes[0x3F] = CCF;
        op_codes[0x37] = SCF;

        cb_codes[0x00] = RLC_B;
        cb_codes[0x01] = RLC_C;
        cb_codes[0x02] = RLC_D;
        cb_codes[0x03] = RLC_E;
        cb_codes[0x04] = RLC_H;
        cb_codes[0x05] = RLC_L;
        cb_codes[0x06] = RLC_HL;
        cb_codes[0x07] = RLC_A;

        cb_codes[0x40] = BIT_0B;
        cb_codes[0x41] = BIT_0C;
        cb_codes[0x42] = BIT_0D;
        cb_codes[0x43] = BIT_0E;
        cb_codes[0x44] = BIT_0H;
        cb_codes[0x45] = BIT_0L;
        //cb_codes[0x46] = BIT_0HL;
        cb_codes[0x48] = BIT_1B;
        cb_codes[0x49] = BIT_1C;
        cb_codes[0x4A] = BIT_1D;
        cb_codes[0x4B] = BIT_1E;
        cb_codes[0x4C] = BIT_1H;
        cb_codes[0x4D] = BIT_1L;
        //cb_codes[0x4E] = BIT_1HL;
        cb_codes[0x4F] = BIT_2A;
        cb_codes[0x50] = BIT_2B;
        cb_codes[0x51] = BIT_2C;
        cb_codes[0x52] = BIT_2D;
        cb_codes[0x53] = BIT_2E;
        cb_codes[0x54] = BIT_2H;
        cb_codes[0x55] = BIT_2L;
        //cb_codes[0x56] = BIT_2HL;
        cb_codes[0x57] = BIT_3A;
        cb_codes[0x58] = BIT_3B;
        cb_codes[0x59] = BIT_3C;
        cb_codes[0x5A] = BIT_3D;
        cb_codes[0x5B] = BIT_3E;
        cb_codes[0x5C] = BIT_3H;
        cb_codes[0x5D] = BIT_3L;
        //cb_codes[0x5E] = BIT_3HL;
        cb_codes[0x5F] = BIT_4A;
        cb_codes[0x60] = BIT_4B;
        cb_codes[0x61] = BIT_4C;
        cb_codes[0x62] = BIT_4D;
        cb_codes[0x63] = BIT_4E;
        cb_codes[0x64] = BIT_4H;
        cb_codes[0x65] = BIT_4L;
        //cb_codes[0x66] = BIT_4HL;
        cb_codes[0x67] = BIT_5A;
        cb_codes[0x68] = BIT_5B;
        cb_codes[0x69] = BIT_5C;
        cb_codes[0x6A] = BIT_5D;
        cb_codes[0x6B] = BIT_5E;
        cb_codes[0x6C] = BIT_5H;
        cb_codes[0x6D] = BIT_5L;
        //cb_codes[0x6E] = BIT_5HL;
        cb_codes[0x6F] = BIT_6A;
        cb_codes[0x70] = BIT_6B;
        cb_codes[0x71] = BIT_6C;
        cb_codes[0x72] = BIT_6D;
        cb_codes[0x73] = BIT_6E;
        cb_codes[0x74] = BIT_6H;
        cb_codes[0x75] = BIT_6L;
        //cb_codes[0x76] = BIT_6HL;
        cb_codes[0x77] = BIT_7A;
        cb_codes[0x78] = BIT_7B;
        cb_codes[0x79] = BIT_7C;
        cb_codes[0x7A] = BIT_7D;
        cb_codes[0x7B] = BIT_7E;
        cb_codes[0x7C] = BIT_7H;
        cb_codes[0x7D] = BIT_7L;
        //cb_codes[0x7E] = BIT_7HL;
        cb_codes[0x7F] = BIT_7A;

        //cb_codes[0x7c] = BIT_7H; 

        cb_codes[0x17] = RL_A;
        cb_codes[0x10] = RL_B;
        cb_codes[0x11] = RL_C;
        cb_codes[0x12] = RL_D;
        cb_codes[0x13] = RL_E;
        cb_codes[0x14] = RL_H;
        cb_codes[0x15] = RL_L;
        cb_codes[0x16] = RL_addr_HL;
        cb_codes[0x37] = SWAP_a;
        cb_codes[0x30] = SWAP_b;
        cb_codes[0x31] = SWAP_c;
        cb_codes[0x32] = SWAP_d;
        cb_codes[0x33] = SWAP_e;
        cb_codes[0x34] = SWAP_h;
        cb_codes[0x35] = SWAP_l;
        cb_codes[0x36] = SWAP_HL;

        cb_codes[0x87] = RES_0_a;

    }

    void CCF() {
        AF.lo &= 0b10010000;
        if ((AF.lo & 0b00010000) == 0b00010000) {
            AF.lo -= 0b00010000;
        }
        else {
            AF.lo += 0b00010000;
        }
        cycles = 4;
    }

    void SCF() {
        AF.lo &= 0b10010000;
        AF.lo |= 0b00010000;
        cycles = 4;
    }

    void NOP() {
        cycles = 4;
    }

    void RET() {
        u8 n1 = RAM::readAt(SP);
        ++SP;
        u16 n2 = RAM::readAt(SP);
        ++SP;
        u16 addr = n1 + (n2 << 8);
        PC = addr;
    }

    void RET_NZ() {
        if ((AF.lo >> 7) == 0)
        {
            u8 n1 = RAM::readAt(SP);
            ++SP;
            u16 n2 = RAM::readAt(SP);
            ++SP;
            u16 addr = n1 + (n2 << 8);
            PC = addr;
        }
    }

    void RET_Z() {
        if ((AF.lo >> 7) == 1) {
            u8 n1 = RAM::readAt(SP);
            ++SP;
            u16 n2 = RAM::readAt(SP);
            ++SP;
            u16 addr = n1 + (n2 << 8);
            PC = addr;
        }
    }
    void RET_NC() {
        if ((AF.lo & 0b00000001) == 0) {
            u8 n1 = RAM::readAt(SP);
            ++SP;
            u16 n2 = RAM::readAt(SP);
            ++SP;
            u16 addr = n1 + (n2 << 8);
            PC = addr;
        }
    }
    void RET_C() {
        if ((AF.lo & 0b00000001) == 1) {
            u8 n1 = RAM::readAt(SP);
            ++SP;
            u16 n2 = RAM::readAt(SP);
            ++SP;
            u16 addr = n1 + (n2 << 8);
            PC = addr;
        }
    }

    void RETI() {
        u8 n1 = RAM::readAt(SP);
        ++SP;
        u16 n2 = RAM::readAt(SP);
        ++SP;
        u16 addr = n1 + (n2 << 8);
        PC = addr;
        RAM::write(0xFF, 0xFFFF);
    }

    void CB() {
        u8 opcode = RAM::read();
        switch (opcode) {
        default: std::cout << "CB opcode not implemented: " << std::hex << unsigned(opcode) << std::endl;
        }
    }

    //  //set z if bit 7 of reg H is zero 
     // void BIT_7H() {
    //  	u8 bit = (HL.hi >> 7);
    //    	AF.lo &= 0b00010000; // TODO: figure out how AF.lo was being set in the first place
    //    	if (bit == 0){AF.lo |= 0b10000000;} 
    //    	AF.lo &= 0b10110000; 
    //    	AF.lo |= 0b00100000; 
    //  }

    void BIT(u8, u8);
    // Test bit (in binary representation so bit 3 = 0b00001000) n in register nn
    void BIT(u8 bit, u8 reg_) {
        u8 test = reg_ & bit;
        if (test == 0) {
            AF.lo |= 0b10000000;
        }
        else {
            AF.lo &= 0b01111111;
        }
        AF.lo &= 0b10110000;
        AF.lo |= 0b00100000;
        cycles = 8;
    }

    void BIT_0A() {
        BIT(1, AF.hi);
    }
    void BIT_1A() {
        BIT(0b00000010, AF.hi);
    }
    void BIT_2A() {
        BIT(0b00000100, AF.hi);
    }
    void BIT_3A() {
        BIT(0b00001000, AF.hi);
    }
    void BIT_4A() {
        BIT(0b00010000, AF.hi);
    }
    void BIT_5A() {
        BIT(0b00100000, AF.hi);
    }
    void BIT_6A() {
        BIT(0b01000000, AF.hi);
    }
    void BIT_7A() {
        BIT(0b10000000, AF.hi);
    }

    void BIT_0B() { BIT(1, BC.hi); }
    void BIT_1B() { BIT(0b00000010, BC.hi); }
    void BIT_2B() { BIT(0b00000100, BC.hi); }
    void BIT_3B() { BIT(0b00001000, BC.hi); }
    void BIT_4B() { BIT(0b00010000, BC.hi); }
    void BIT_5B() { BIT(0b00100000, BC.hi); }
    void BIT_6B() { BIT(0b01000000, BC.hi); }
    void BIT_7B() { BIT(0b10000000, BC.hi); }

    void BIT_0C() { BIT(1, BC.lo); }
    void BIT_1C() { BIT(0b00000010, BC.lo); }
    void BIT_2C() { BIT(0b00000100, BC.lo); }
    void BIT_3C() { BIT(0b00001000, BC.lo); }
    void BIT_4C() { BIT(0b00010000, BC.lo); }
    void BIT_5C() { BIT(0b00100000, BC.lo); }
    void BIT_6C() { BIT(0b01000000, BC.lo); }
    void BIT_7C() { BIT(0b10000000, BC.lo); }

    void BIT_0D() { BIT(1, DE.hi); }
    void BIT_1D() { BIT(0b00000010, DE.hi); }
    void BIT_2D() { BIT(0b00000100, DE.hi); }
    void BIT_3D() { BIT(0b00001000, DE.hi); }
    void BIT_4D() { BIT(0b00010000, DE.hi); }
    void BIT_5D() { BIT(0b00100000, DE.hi); }
    void BIT_6D() { BIT(0b01000000, DE.hi); }
    void BIT_7D() { BIT(0b10000000, DE.hi); }

    void BIT_0E() { BIT(1, DE.lo); }
    void BIT_1E() { BIT(0b00000010, DE.lo); }
    void BIT_2E() { BIT(0b00000100, DE.lo); }
    void BIT_3E() { BIT(0b00001000, DE.lo); }
    void BIT_4E() { BIT(0b00010000, DE.lo); }
    void BIT_5E() { BIT(0b00100000, DE.lo); }
    void BIT_6E() { BIT(0b01000000, DE.lo); }
    void BIT_7E() { BIT(0b10000000, DE.lo); }

    void BIT_0H() { BIT(1, HL.hi); }
    void BIT_1H() { BIT(0b00000010, HL.hi); }
    void BIT_2H() { BIT(0b00000100, HL.hi); }
    void BIT_3H() { BIT(0b00001000, HL.hi); }
    void BIT_4H() { BIT(0b00010000, HL.hi); }
    void BIT_5H() { BIT(0b00100000, HL.hi); }
    void BIT_6H() { BIT(0b01000000, HL.hi); }
    void BIT_7H() { BIT(0b10000000, HL.hi); }

    void BIT_0L() { BIT(1, HL.lo); }
    void BIT_1L() { BIT(0b00000010, HL.lo); }
    void BIT_2L() { BIT(0b00000100, HL.lo); }
    void BIT_3L() { BIT(0b00001000, HL.lo); }
    void BIT_4L() { BIT(0b00010000, HL.lo); }
    void BIT_5L() { BIT(0b00100000, HL.lo); }
    void BIT_6L() { BIT(0b01000000, HL.lo); }
    void BIT_7L() { BIT(0b10000000, HL.lo); }

    void RLA() {
        u8 carry = (AF.lo & 0b00010000) >> 4;
        AF.lo = (AF.hi & 0b10000000) >> 3;
        AF.lo &= 0b10010000;
        AF.hi = (AF.hi << 1) + carry;
        if (AF.hi == 0) { AF.lo |= 0b10000000; }
        else { AF.lo &= 0b01110000; }
        cycles = 4;
    }
    void RLCA() {
        // just rotate left
        u8 bit7 = AF.hi & 0b10000000;
        AF.lo = (bit7 >> 3);
        AF.lo &= 0b10010000;
        AF.hi = (AF.hi << 1) + (bit7 >> 7);
        if (AF.hi == 0) { AF.lo |= 0b10000000; }
        else { AF.lo &= 0b01110000; }
        cycles = 4;
    }

    void RRCA() {
        u8 bit0 = AF.hi & 0b00000001;
        AF.lo = (bit0 << 4);
        AF.lo &= 0b10010000;
        AF.hi = (AF.hi >> 1) + (bit0 << 7);
        if (AF.hi == 0) { AF.lo |= 0b10000000; }
        else { AF.lo &= 0b01110000; }
        cycles = 4;
    }

    void RRA() {
        u8 carry = (AF.lo & 0b00010000) << 3;
        AF.lo = (AF.hi & 0b00000001) << 4;
        AF.lo &= 0b10010000;
        AF.hi = (AF.hi >> 1) + carry;
        if (AF.hi == 0) { AF.lo |= 0b10000000; }
        else { AF.lo &= 0b01110000; }
        cycles = 4;
    }
    void RL_A() {
        u8 carry = (AF.lo & 0b00010000) >> 4;
        AF.lo = (AF.hi & 0b10000000) >> 3;
        AF.lo &= 0b10010000;
        AF.hi = (AF.hi << 1) + carry;
        if (AF.hi == 0) { AF.lo |= 0b10000000; }
        else { AF.lo &= 0b01110000; }
        cycles = 4;
    }
    // rotate left (but not through the carry flag)
    void RLC_A() {
        u8 carry = (AF.hi & 0b10000000) >> 7;
        AF.lo = (AF.hi & 0b10000000) >> 3;
        AF.lo &= 0b10010000;
        AF.hi = (AF.hi << 1) + carry;
        if (AF.hi == 0) { AF.lo |= 0b10000000; }
        else { AF.lo &= 0b01110000; }
        cycles = 8;
    }
    void RLC_B() {
        u8 carry = (BC.hi & 0b10000000) >> 7;
        AF.lo = (BC.hi & 0b10000000) >> 3;
        AF.lo &= 0b10010000;
        BC.hi = (BC.hi << 1) + carry;
        if (BC.hi == 0) { AF.lo |= 0b10000000; }
        else { AF.lo &= 0b01110000; }
        cycles = 8;
    }
    void RLC_C() {
        u8 carry = (BC.lo & 0b10000000) >> 7;
        AF.lo = (BC.lo & 0b10000000) >> 3;
        AF.lo &= 0b10010000;
        BC.lo = (BC.lo << 1) + carry;
        if (BC.lo == 0) { AF.lo |= 0b10000000; }
        else { AF.lo &= 0b01110000; }
        cycles = 8;
    }
    void RLC_D() {
        u8 carry = (DE.hi & 0b10000000) >> 7;
        AF.lo = (DE.hi & 0b10000000) >> 3;
        AF.lo &= 0b10010000;
        DE.hi = (DE.hi << 1) + carry;
        if (DE.hi == 0) { AF.lo |= 0b10000000; }
        else { AF.lo &= 0b01110000; }
        cycles = 8;
    }
    void RLC_E() {
        u8 carry = (DE.lo & 0b10000000) >> 7;
        AF.lo = (DE.lo & 0b10000000) >> 3;
        AF.lo &= 0b10010000;
        DE.lo = (DE.lo << 1) + carry;
        if (DE.lo == 0) { AF.lo |= 0b10000000; }
        else { AF.lo &= 0b01110000; }
        cycles = 8;
    }
    void RLC_H() {
        u8 carry = (HL.hi & 0b10000000) >> 7;
        AF.lo = (HL.hi & 0b10000000) >> 3;
        AF.lo &= 0b10010000;
        HL.hi = (HL.hi << 1) + carry;
        if (HL.hi == 0) { AF.lo |= 0b10000000; }
        else { AF.lo &= 0b01110000; }
        cycles = 8;
    }
    void RLC_L() {
        u8 carry = (HL.lo & 0b10000000) >> 7;
        AF.lo = (HL.lo & 0b10000000) >> 3;
        AF.lo &= 0b10010000;
        HL.lo = (HL.lo << 1) + carry;
        if (HL.lo == 0) { AF.lo |= 0b10000000; }
        else { AF.lo &= 0b01110000; }
        cycles = 8;
    }
    void RL_B() {
        u8 carry = (AF.lo & 0b00010000) >> 4;
        AF.lo = (BC.hi & 0b10000000) >> 3;
        AF.lo &= 0b10010000;
        BC.hi = (BC.hi << 1) + carry;
        if (DE.hi == 0) { AF.lo |= 0b10000000; }
        else { AF.lo &= 0b01110000; }
        cycles = 8;
    }
    void RL_C() {
        u8 carry = (AF.lo & 0b00010000) >> 4;
        AF.lo = (BC.lo & 0b10000000) >> 3;
        AF.lo &= 0b10010000;
        BC.lo = (BC.lo << 1) + carry;
        if (BC.lo == 0) { AF.lo |= 0b10000000; }
        else { AF.lo &= 0b01110000; }
        cycles = 8;
    }

    void RL_D() {
        u8 carry = (AF.lo & 0b00010000) >> 4;
        AF.lo = (DE.hi & 0b10000000) >> 3;
        AF.lo &= 0b10010000;
        DE.hi = (DE.hi << 1) + carry;
        if (DE.hi == 0) { AF.lo |= 0b10000000; }
        else { AF.lo &= 0b01110000; }
        cycles = 8;
    }
    void RL_E() {
        u8 carry = (AF.lo & 0b00010000) >> 4;
        AF.lo = (DE.lo & 0b10000000) >> 3;
        AF.lo &= 0b10010000;
        DE.lo = (DE.lo << 1) + carry;
        if (DE.lo == 0) { AF.lo |= 0b10000000; }
        else { AF.lo &= 0b01110000; }
        cycles = 8;
    }
    void RL_H() {
        u8 carry = (AF.lo & 0b00010000) >> 4;
        AF.lo = (HL.hi & 0b10000000) >> 3;
        AF.lo &= 0b10010000;
        HL.hi = (HL.hi << 1) + carry;
        if (HL.hi == 0) { AF.lo |= 0b10000000; }
        else { AF.lo &= 0b01110000; }
        cycles = 8;
    }
    void RL_L() {
        u8 carry = (AF.lo & 0b00010000) >> 4;
        AF.lo = (HL.lo & 0b10000000) >> 3;
        AF.lo &= 0b10010000;
        HL.lo = (HL.lo << 1) + carry;
        if (HL.lo == 0) { AF.lo |= 0b10000000; }
        else { AF.lo &= 0b01110000; }
        cycles = 8;
    }
    void RL_addr_HL() {
        u8 x = RAM::readAt(HL.val());
        u8 carry = (AF.lo & 0b00010000) >> 4;
        AF.lo = (x & 0b10000000) >> 3;
        AF.lo &= 0b10010000;
        x = (x << 1) + carry;
        RAM::write(x, HL.val());
        if (x == 0) { AF.lo |= 0b10000000; }
        else { AF.lo &= 0b01110000; }
        cycles = 16;
    }
    void RLC_HL() {
        u8 x = RAM::readAt(HL.val());
        u8 carry = (x & 0b10000000) >> 7;
        AF.lo = (x & 0b10000000) >> 3;
        AF.lo &= 0b10010000;
        x = (x << 1) + carry;
        RAM::write(x, HL.val());
        if (x == 0) { AF.lo |= 0b10000000; }
        else { AF.lo &= 0b01110000; }
        cycles = 16;
    }

    // if NZ (if Z is zero) then add n to current address and jump to that address
    // we have to treat the next byte as a signed variable
    void JR_NZ()
    {
        if ((AF.lo >> 7) == 0)
        {
            s8 b = RAM::read();
            PC += b;
        }
        else { ++PC; }
    }

    void JR_Z()
    {
        if ((AF.lo >> 7) == 1)
        {
            s8 b = RAM::read();
            PC += b;
        }
        else { ++PC; }
    }

    void JR_n()
    {
        s8 jump = RAM::read();
        PC += jump;
    }

    void JP() {
        u8 n1, n2;
        n1 = RAM::read();
        n2 = RAM::read();
        PC = n1 + (n2 << 8);
        cycles = 12;
    }

    void JP_NZ() {
        u8 n1, n2;
        if ((AF.lo >> 7) == 0) {
            n1 = RAM::read();
            n2 = RAM::read();
            PC = n1 + (n2 << 8);
        }
        else { PC += 2; }
        cycles = 12;
    }

    void JP_Z() {
        u8 n1, n2;
        if ((AF.lo >> 7) == 1) {
            n1 = RAM::read();
            n2 = RAM::read();
            PC = n1 + (n2 << 8);
        }
        else { PC += 2; }
        cycles = 12;
    }

    void JP_NC() {
        u8 n1, n2;
        if (((AF.lo >> 4) & 1) == 0) {
            n1 = RAM::read();
            n2 = RAM::read();
            PC = n1 + (n2 << 8);
        }
        else { PC += 2; }
        cycles = 12;
    }
    void JP_C() {
        u8 n1, n2;
        if (((AF.lo >> 4) & 1) == 1) {
            n1 = RAM::read();
            n2 = RAM::read();
            PC = n1 + (n2 << 8);
        }
        else { PC += 2; }
        cycles = 12;
    }

    void JP_HL() {
        // u8 n = RAM::readAt(HL.val());
        PC = HL.val();
        cycles = 4;
    }

    // load C into n ()
    void LDc_n()
    {
        u8 n = RAM::read();
        BC.lo = n;
    }

    void LDb_n()
    {
        u8 n = RAM::read();
        BC.hi = n;
    }
    void LDd_n()
    {
        u8 n = RAM::read();
        DE.hi = n;
    }
    void LDe_n()
    {
        u8 n = RAM::read();
        DE.lo = n;
    }
    void LDh_n()
    {
        u8 n = RAM::read();
        HL.hi = n;
    }

    void LDl_n()
    {
        u8 n = RAM::read();
        HL.lo = n;
    }

    void  LDrr_a_hash()
    {
        u8 byte = RAM::read();
        AF.hi = byte;
    }

    void LD_HL_a()
    {
        RAM::write(AF.hi, HL.val());
    }
    void LD_BC_a() { RAM::write(AF.hi, BC.val()); }
    void LD_DE_a() { RAM::write(AF.hi, DE.val()); }

    void LDad_n_a() { u8 add = RAM::read(); u16 addr = add + 0xFF00; RAM::write(AF.hi, addr); cycles = 12; }

    void CALL_nn()
    {

        u8 n1 = RAM::read();
        u8 n2 = RAM::read();

        u8 loNib = PC & 0xFF;
        u8 hiNib = (PC >> 8) & 0xFF;

        --SP;
        RAM::write(hiNib, SP);
        --SP;
        RAM::write(loNib, SP);

        PC = n1 + (n2 << 8);
        cycles = 12;
    }

    void CALL_NZ() {
        u8 F = AF.lo;
        u8 Z = (F & 0b10000000) >> 7;
        if (Z == 0) {
            u8 n1 = RAM::read();
            u8 n2 = RAM::read();

            u8 loNib = PC & 0xFF;
            u8 hiNib = (PC >> 8) & 0xFF;

            --SP;
            RAM::write(hiNib, SP);
            --SP;
            RAM::write(loNib, SP);

            PC = n1 + (n2 << 8);
            cycles = 12;
        }
    }
    void CALL_Z() {
        u8 F = AF.lo;
        u8 Z = (F & 0b10000000) >> 7;
        if (Z == 1) {
            u8 n1 = RAM::read();
            u8 n2 = RAM::read();

            u8 loNib = PC & 0xFF;
            u8 hiNib = (PC >> 8) & 0xFF;

            --SP;
            RAM::write(hiNib, SP);
            --SP;
            RAM::write(loNib, SP);

            PC = n1 + (n2 << 8);
            cycles = 12;
        }
    }
    void CALL_NC() {
        u8 F = AF.lo;
        u8 C = (F & 0b00010000) >> 7;
        if (C == 0) {
            u8 n1 = RAM::read();
            u8 n2 = RAM::read();

            u8 loNib = PC & 0xFF;
            u8 hiNib = (PC >> 8) & 0xFF;

            --SP;
            RAM::write(hiNib, SP);
            --SP;
            RAM::write(loNib, SP);

            PC = n1 + (n2 << 8);
            cycles = 12;
        }
    }
    void CALL_C() {
        u8 F = AF.lo;
        u8 C = (F & 0b00010000) >> 7;
        if (C == 1) {
            u8 n1 = RAM::read();
            u8 n2 = RAM::read();

            u8 loNib = PC & 0xFF;
            u8 hiNib = (PC >> 8) & 0xFF;

            --SP;
            RAM::write(hiNib, SP);
            --SP;
            RAM::write(loNib, SP);

            PC = n1 + (n2 << 8);
            cycles = 12;
        }
    }
    void LDI_HLa()
    {
        RAM::write(AF.hi, HL.val());
        u16 x = HL.val() + 1;
        HL.set(x);
    }

    void LD_nn_a()
    {
        u8 n1 = RAM::read();
        u8 n2 = RAM::read();
        u16 addr = n1 + (n2 << 8);
        RAM::write(AF.hi, addr);
    }

    void LDH_a_ffn()
    {
        u8 n = RAM::read();
        AF.hi = RAM::readAt(0xFF00 + n);
        cycles = 12;
    }

    void LDrr_ab() { AF.hi = BC.hi; }
    void LDrr_ac() { AF.hi = BC.lo; }
    void LDrr_ad() { AF.hi = DE.hi; }
    void LDrr_ae() { AF.hi = DE.lo; }
    void LDrr_ah() { AF.hi = HL.hi; }
    void LDrr_al() { AF.hi = HL.lo; }
    void LDrr_aBC() { AF.hi = RAM::readAt(BC.val()); }
    void LDrr_aDE() { AF.hi = RAM::readAt(DE.val()); }
    void LDrr_aHL() { AF.hi = RAM::readAt(HL.val()); }
    void LDrr_ba() { BC.hi = AF.hi; }
    void LDrr_bc() { BC.hi = BC.lo; }
    void LDrr_bd() { BC.hi = DE.hi; }
    void LDrr_be() { BC.hi = DE.lo; }
    void LDrr_bh() { BC.hi = HL.hi; }
    void LDrr_bl() { BC.hi = HL.lo; }
    void LDrr_bHL() { BC.hi = RAM::readAt(HL.val()); }
    void LDrr_ca() { BC.lo = AF.hi; }
    void LDrr_cb() { BC.lo = BC.hi; }
    void LDrr_cd() { BC.lo = DE.hi; }
    void LDrr_ce() { BC.lo = DE.lo; }
    void LDrr_ch() { BC.lo = HL.hi; }
    void LDrr_cl() { BC.lo = HL.lo; }
    void LDrr_cHL() { BC.lo = RAM::readAt(HL.val()); }
    void LDrr_da() { DE.hi = AF.hi; }
    void LDrr_db() { DE.hi = BC.hi; }
    void LDrr_dc() { DE.hi = BC.lo; }
    void LDrr_de() { DE.hi = DE.lo; }
    void LDrr_dh() { DE.hi = HL.hi; }
    void LDrr_dl() { DE.hi = HL.lo; }
    void LDrr_dHL() { DE.hi = RAM::readAt(HL.val()); }
    void LDrr_ea() { DE.lo = AF.hi; }
    void LDrr_eb() { DE.lo = BC.hi; }
    void LDrr_ec() { DE.lo = BC.lo; }
    void LDrr_ed() { DE.lo = DE.hi; }
    void LDrr_eh() { DE.lo = HL.hi; }
    void LDrr_el() { DE.lo = HL.lo; }
    void LDrr_eHL() { DE.lo = RAM::readAt(HL.val()); }
    void LDrr_ha() { HL.hi = AF.hi; }
    void LDrr_hb() { HL.hi = BC.hi; }
    void LDrr_hc() { HL.hi = BC.lo; }
    void LDrr_hd() { HL.hi = DE.hi; }
    void LDrr_he() { HL.hi = DE.lo; }
    void LDrr_hl() { HL.hi = HL.lo; }
    void LDrr_hHL() { HL.hi = RAM::readAt(HL.val()); }
    void LDrr_la() { HL.lo = AF.hi; }
    void LDrr_lb() { HL.lo = BC.hi; }
    void LDrr_lc() { HL.lo = BC.lo; }
    void LDrr_ld() { HL.lo = DE.hi; }
    void LDrr_le() { HL.lo = DE.lo; }
    void LDrr_lh() { HL.lo = HL.hi; }
    void LDrr_lHL() { HL.lo = RAM::readAt(HL.val()); }
    void LDrr_HLb() { RAM::write(BC.hi, HL.val()); cycles = 8; }
    void LDrr_HLc() { RAM::write(BC.lo, HL.val()); cycles = 8; }
    void LDrr_HLd() { RAM::write(DE.hi, HL.val()); cycles = 8; }
    void LDrr_HLe() { RAM::write(DE.lo, HL.val()); cycles = 8; }
    void LDrr_HLh() { RAM::write(HL.hi, HL.val()); cycles = 8; }
    void LDrr_HLl() { RAM::write(HL.lo, HL.val()); cycles = 8; }
    void LDrr_HLn() { RAM::write(RAM::read(), HL.val()); cycles = 12; }
    void LDrr_ann() {
        u8 n1 = RAM::read(), n2 = RAM::read();
        u16 addr = n1 + (n2 << 8);
        AF.hi = RAM::readAt(addr);
        cycles = 16;
    }

    void LDa_c() { AF.hi = RAM::readAt(0xFF00 + BC.lo); }
    void LDc_a() { RAM::write(AF.hi, 0xFF00 + BC.lo); }

    void LDDaHL() { AF.hi = RAM::readAt(HL.val()); HL.set(HL.val() - 1); }
    void LDDHLa() { RAM::write(AF.hi, HL.val()); HL.set(HL.val() - 1); }
    void LDIaHL() { AF.hi = RAM::readAt(HL.val()); HL.set(HL.val() + 1); }
    void LDIHLa() { RAM::write(AF.hi, HL.val()); HL.set(HL.val() + 1); }

    void LDHnA() { u8 n = RAM::read(); RAM::write(AF.hi, 0xFF00 + n); }
    void LDHAn() { u8 n = RAM::read(); RAM::readAt(0xFF00 + n); }
    //-- 16-Bit shit --//
    // going to need to read 16 bit values little endian style (I think)
    void LD_nn_BC()
    {
        u8 n1 = RAM::read();
        u8 n2 = RAM::read();
        u16 nn = n1 + (n2 << 8);
        BC.set(nn);
    }
    void LD_nn_DE() { u8 n1 = RAM::read(); u8 n2 = RAM::read(); u16 nn = n1 + (n2 << 8); DE.set(nn); }
    void LD_nn_HL() { u8 n1 = RAM::read(); u8 n2 = RAM::read(); u16 nn = n1 + (n2 << 8); HL.set(nn); }
    void LD_nn_SP() { u8 n1 = RAM::read(); u8 n2 = RAM::read(); u16 nn = n1 + (n2 << 8); SP = nn; }

    // i'm not sure whether SP = HL.val() or the bytes at SP should be set to HL
    //TODO: fix this shit
    void LD_SPHL() { SP = HL.val(); }

    // put SP + n effective address into HL
    void LDHL_SPn() { u8 n = RAM::read(); HL.set(SP + n); AF.lo = AF.lo & 0x30; cycles = 12; }
    void LD_nnSP()
    {
        u8 n1 = RAM::read();
        u8 n2 = RAM::read();
        u16 nn = n1 + (n2 << 8);
        SP = RAM::readAt(nn);
    } // Put SP at address nn

    // Push register pair nn onto stack, decrement SP twice
    // TODO: figure out whether to set the value first or decrement SP first 
    // TODO: check that the endianess is correct (does it even matter??)
    void PUSH_AF() { SP--; RAM::write(AF.hi, SP); SP--; RAM::write(AF.lo, SP); cycles = 16; }
    void PUSH_BC() { SP--; RAM::write(BC.hi, SP); SP--; RAM::write(BC.lo, SP); cycles = 16; }
    void PUSH_DE() { SP--; RAM::write(DE.hi, SP); SP--; RAM::write(DE.lo, SP); cycles = 16; }
    void PUSH_HL() { SP--; RAM::write(HL.hi, SP); SP--; RAM::write(HL.lo, SP); cycles = 16; }

    // Pop two bytes off stack into register pair, increment SP twice
    void POP_AF() { u8 n = RAM::readAt(SP); AF.lo = n; ++SP; u8 m = RAM::readAt(SP); AF.hi = m; ++SP; cycles = 12; }
    void POP_BC() { u8 n = RAM::readAt(SP); BC.lo = n; ++SP; u8 m = RAM::readAt(SP); BC.hi = m; ++SP; cycles = 12; } //TODO: something is going wrong here
    void POP_DE() { u8 n = RAM::readAt(SP); DE.lo = n; ++SP; u8 m = RAM::readAt(SP); DE.hi = m; ++SP; cycles = 12; }
    void POP_HL() { u8 n = RAM::readAt(SP); HL.lo = n; ++SP; u8 m = RAM::readAt(SP); HL.hi = m; ++SP; cycles = 12; }


    // ADD_xy add y to x and set flags
    void ADD_aa()
    {
        u8 x = AF.hi;
        u8 loNib = AF.hi & 0x0F;
        AF.hi += AF.hi;
        if (AF.hi == 0) { AF.lo = AF.lo | 0x80; }
        else { AF.lo = 0; } // set Z = 0 if == 0
        AF.lo = AF.lo & 0b10110000; // reset N (N = 0)
        if (AF.hi < x) { AF.lo = AF.lo | 0b00100000; } // Set H if overfloss
        if ((AF.hi & 0x0F) < loNib) { AF.lo = AF.lo | 0b00010000; } // Set CY if loer nibble overflos
        cycles = 4;
    }
    void ADD_ab()
    {
        u8 x = AF.hi;
        u8 loNib = AF.hi & 0x0F;
        AF.hi += BC.hi;
        if (AF.hi == 0) { AF.lo = AF.lo | 0x80; }
        else { AF.lo = 0; } // set Z = 0 if == 0
        AF.lo = AF.lo & 0b10110000; // reset N (N = 0)
        if (AF.hi < x) { AF.lo = AF.lo | 0b00100000; } // Set H if overfloss
        if ((AF.hi & 0x0F) < loNib) { AF.lo = AF.lo | 0b00010000; } // Set CY if loer nibble overflos
        cycles = 4;
    }
    void ADD_ac()
    {
        u8 x = AF.hi;
        u8 loNib = AF.hi & 0x0F;
        AF.hi += BC.lo;
        if (AF.hi == 0) { AF.lo = AF.lo | 0x80; }
        else { AF.lo = 0; } // set Z = 0 if == 0
        AF.lo = AF.lo & 0b10110000; // reset N (N = 0)
        if (AF.hi < x) { AF.lo = AF.lo | 0b00100000; } // Set H if overfloss
        if ((AF.hi & 0x0F) < loNib) { AF.lo = AF.lo | 0b00010000; } // Set CY if loer nibble overflos
        cycles = 4;
    }
    void ADD_ad()
    {
        u8 x = AF.hi;
        u8 loNib = AF.hi & 0x0F;
        AF.hi += DE.hi;
        if (AF.hi == 0) { AF.lo = AF.lo | 0x80; }
        else { AF.lo = 0; } // set Z = 0 if == 0
        AF.lo = AF.lo & 0b10110000; // reset N (N = 0)
        if (AF.hi < x) { AF.lo = AF.lo | 0b00100000; } // Set H if overfloss
        if ((AF.hi & 0x0F) < loNib) { AF.lo = AF.lo | 0b00010000; } // Set CY if loer nibble overflos
        cycles = 4;
    }
    void ADD_ae()
    {
        u8 x = AF.hi;
        u8 loNib = DE.lo & 0x0F;
        AF.hi += BC.hi;
        if (AF.hi == 0) { AF.lo = AF.lo | 0x80; }
        else { AF.lo = 0; } // set Z = 0 if == 0
        AF.lo = AF.lo & 0b10110000; // reset N (N = 0)
        if (AF.hi < x) { AF.lo = AF.lo | 0b00100000; } // Set H if overfloss
        if ((AF.hi & 0x0F) < loNib) { AF.lo = AF.lo | 0b00010000; } // Set CY if loer nibble overflos
        cycles = 4;
    }
    void ADD_ah()
    {
        u8 x = AF.hi;
        u8 loNib = AF.hi & 0x0F;
        AF.hi += HL.hi;
        if (AF.hi == 0) { AF.lo = AF.lo | 0x80; }
        else { AF.lo = 0; } // set Z = 0 if == 0
        AF.lo = AF.lo & 0b10110000; // reset N (N = 0)
        if (AF.hi < x) { AF.lo = AF.lo | 0b00100000; } // Set H if overfloss
        if ((AF.hi & 0x0F) < loNib) { AF.lo = AF.lo | 0b00010000; } // Set CY if loer nibble overflos
        cycles = 4;
    }
    void ADD_al()
    {
        u8 x = AF.hi;
        u8 loNib = HL.lo & 0x0F;
        AF.hi += BC.hi;
        if (AF.hi == 0) { AF.lo = AF.lo | 0x80; }
        else { AF.lo = 0; } // set Z = 0 if == 0
        AF.lo = AF.lo & 0b10110000; // reset N (N = 0)
        if (AF.hi < x) { AF.lo = AF.lo | 0b00100000; } // Set H if overfloss
        if ((AF.hi & 0x0F) < loNib) { AF.lo = AF.lo | 0b00010000; } // Set CY if loer nibble overflos
        cycles = 4;
    }
    void ADD_aH()
    {
        u8 x = AF.hi;
        u8 loNib = AF.hi & 0x0F;
        AF.hi += RAM::readAt(HL.val());
        if (AF.hi == 0) { AF.lo = AF.lo | 0x80; }
        else { AF.lo = 0; } // set Z = 0 if == 0
        AF.lo = AF.lo & 0b10110000; // reset N (N = 0)
        if (AF.hi < x) { AF.lo = AF.lo | 0b00100000; } // Set H if overfloss
        if ((AF.hi & 0x0F) < loNib) { AF.lo = AF.lo | 0b00010000; } // Set CY if loer nibble overflos
        cycles = 8;
    }
    void ADD_a_hash() {
        u8 x = AF.hi;
        u8 loNib = AF.hi & 0x0F;
        AF.hi += RAM::read();
        if (AF.hi == 0) { AF.lo = AF.lo | 0x80; }
        else { AF.lo = 0; } // set Z = 0 if == 0
        AF.lo = AF.lo & 0b10110000; // reset N (N = 0)
        if (AF.hi < x) { AF.lo = AF.lo | 0b00100000; } // Set H if overfloss
        if ((AF.hi & 0x0F) < loNib) { AF.lo = AF.lo | 0b00010000; } // Set CY if loer nibble overflos
        cycles = 8;
    }

    void ADC_aa()
    {
        u8 x = AF.hi;
        u8 loNib = AF.hi & 0x0F;
        AF.hi += AF.hi + (AF.lo & 0b00010000);
        if (AF.hi == 0) { AF.lo = AF.lo | 0x80; }
        else { AF.lo = 0; } // set Z = 0 if == 0
        AF.lo = AF.lo & 0b10110000; // reset N (N = 0)
        if (AF.hi < x) { AF.lo = AF.lo | 0b00100000; } // Set H if overfloss
        if ((AF.hi & 0x0F) < loNib) { AF.lo = AF.lo | 0b00010000; } // Set CY if loer nibble overflos
        cycles = 4;

    }
    void ADC_ab()
    {
        u8 x = AF.hi;
        u8 loNib = AF.hi & 0x0F;
        AF.hi += BC.hi + (AF.lo & 0b00010000);
        if (AF.hi == 0) { AF.lo = AF.lo | 0x80; }
        else { AF.lo = 0; } // set Z = 0 if == 0
        AF.lo = AF.lo & 0b10110000; // reset N (N = 0)
        if (AF.hi < x) { AF.lo = AF.lo | 0b00100000; } // Set H if overfloss
        if ((AF.hi & 0x0F) < loNib) { AF.lo = AF.lo | 0b00010000; } // Set CY if loer nibble overflos
        cycles = 4;

    }
    void ADC_ac()
    {
        u8 x = AF.hi;
        u8 loNib = AF.hi & 0x0F;
        AF.hi += BC.lo + (AF.lo & 0b00010000);
        if (AF.hi == 0) { AF.lo = AF.lo | 0x80; }
        else { AF.lo = 0; } // set Z = 0 if == 0
        AF.lo = AF.lo & 0b10110000; // reset N (N = 0)
        if (AF.hi < x) { AF.lo = AF.lo | 0b00100000; } // Set H if overfloss
        if ((AF.hi & 0x0F) < loNib) { AF.lo = AF.lo | 0b00010000; } // Set CY if loer nibble overflos
        cycles = 4;

    }
    void ADC_ad()
    {
        u8 x = AF.hi;
        u8 loNib = AF.hi & 0x0F;
        AF.hi += DE.hi + (AF.lo & 0b00010000);
        if (AF.hi == 0) { AF.lo = AF.lo | 0x80; }
        else { AF.lo = 0; } // set Z = 0 if == 0
        AF.lo = AF.lo & 0b10110000; // reset N (N = 0)
        if (AF.hi < x) { AF.lo = AF.lo | 0b00100000; } // Set H if overfloss
        if ((AF.hi & 0x0F) < loNib) { AF.lo = AF.lo | 0b00010000; } // Set CY if loer nibble overflos
        cycles = 4;

    }
    void ADC_ae()
    {
        u8 x = AF.hi;
        u8 loNib = DE.lo & 0x0F;
        AF.hi += BC.hi + (AF.lo & 0b00010000);
        if (AF.hi == 0) { AF.lo = AF.lo | 0x80; }
        else { AF.lo = 0; } // set Z = 0 if == 0
        AF.lo = AF.lo & 0b10110000; // reset N (N = 0)
        if (AF.hi < x) { AF.lo = AF.lo | 0b00100000; } // Set H if overfloss
        if ((AF.hi & 0x0F) < loNib) { AF.lo = AF.lo | 0b00010000; } // Set CY if loer nibble overflos
        cycles = 4;

    }
    void ADC_ah()
    {
        u8 x = AF.hi;
        u8 loNib = AF.hi & 0x0F;
        AF.hi += HL.hi + (AF.lo & 0b00010000);
        if (AF.hi == 0) { AF.lo = AF.lo | 0x80; }
        else { AF.lo = 0; } // set Z = 0 if == 0
        AF.lo = AF.lo & 0b10110000; // reset N (N = 0)
        if (AF.hi < x) { AF.lo = AF.lo | 0b00100000; } // Set H if overfloss
        if ((AF.hi & 0x0F) < loNib) { AF.lo = AF.lo | 0b00010000; } // Set CY if loer nibble overflos
        cycles = 4;

    }
    void ADC_al()
    {
        u8 x = AF.hi;
        u8 loNib = HL.lo & 0x0F;
        AF.hi += BC.hi + (AF.lo & 0b00010000);
        if (AF.hi == 0) { AF.lo = AF.lo | 0x80; }
        else { AF.lo = 0; } // set Z = 0 if == 0
        AF.lo = AF.lo & 0b10110000; // reset N (N = 0)
        if (AF.hi < x) { AF.lo = AF.lo | 0b00100000; } // Set H if overfloss
        if ((AF.hi & 0x0F) < loNib) { AF.lo = AF.lo | 0b00010000; } // Set CY if loer nibble overflos
        cycles = 4;

    }
    void ADC_aHL()
    {
        u8 x = AF.hi;
        u8 loNib = AF.hi & 0x0F;
        AF.hi += RAM::readAt(HL.val()) + (AF.lo & 0b00010000);
        if (AF.hi == 0) { AF.lo = AF.lo | 0x80; }
        else { AF.lo = 0; } // set Z = 0 if == 0
        AF.lo = AF.lo & 0b10110000; // reset N (N = 0)
        if (AF.hi < x) { AF.lo = AF.lo | 0b00100000; } // Set H if overfloss
        if ((AF.hi & 0x0F) < loNib) { AF.lo = AF.lo | 0b00010000; } // Set CY if loer nibble overflos
        cycles = 8;

    }
    void ADC_a_hash() {
        u8 x = AF.hi;
        u8 loNib = AF.hi & 0x0F;
        AF.hi += RAM::read() + (AF.lo & 0b00010000);
        if (AF.hi == 0) { AF.lo = AF.lo | 0x80; }
        else { AF.lo = 0; } // set Z = 0 if == 0
        AF.lo = AF.lo & 0b10110000; // reset N (N = 0)
        if (AF.hi < x) { AF.lo = AF.lo | 0b00100000; } // Set H if overfloss
        if ((AF.hi & 0x0F) < loNib) { AF.lo = AF.lo | 0b00010000; } // Set CY if loer nibble overflos
        cycles = 8;
    }

    // have to change it to flag setting only when the there is NO underflo isntead of the opposite

    // TODO:: for the CY flag, have to figure out whether it is a <= or a < in the if statement

    void SUB_a()
    {
        u8 x = AF.hi;
        u8 loNib = AF.hi & 0x0F;
        AF.hi -= AF.hi;
        if (AF.hi == 0) { AF.lo = AF.lo | 0x80; }
        else { AF.lo = 0; } // set Z = 0 if == 0
        AF.lo = AF.lo | 0b01000000; // set N
        if ((AF.hi & 0x0F) <= x) { AF.lo = AF.lo | 0b00100000; }
        else { AF.lo &= 0b11011111; } // Set H if loer nibble does not underflo
        if (AF.hi <= loNib) { AF.lo = AF.lo | 0b00010000; }
        else { AF.lo &= 0b11101111; } // Set CY if whole thing does not underflo underflos
        cycles = 4;
    }
    void SUB_b()
    {
        u8 x = AF.hi;
        u8 loNib = AF.hi & 0x0F;
        AF.hi -= BC.hi;
        if (AF.hi == 0) { AF.lo = AF.lo | 0x80; }
        else { AF.lo = 0; } // set Z = 0 if == 0
        AF.lo = AF.lo | 0b01000000; // set N
        if ((AF.hi & 0x0F) <= loNib) { AF.lo = AF.lo | 0b00100000; }
        else { AF.lo &= 0b11011111; } // Set H if loer nibble does not underflo
        if (AF.hi <= BC.hi) { AF.lo = AF.lo | 0b00010000; }
        else { AF.lo &= 0b11101111; } // Set CY if whole thing does not underflo
        cycles = 4;
    }
    void SUB_c()
    {
        u8 x = AF.hi;
        u8 loNib = AF.hi & 0x0F;
        AF.hi -= BC.lo;
        if (AF.hi == 0) { AF.lo = AF.lo | 0x80; }
        else { AF.lo = 0; } // set Z = 0 if == 0
        AF.lo = AF.lo | 0b01000000; // set N
        if ((AF.hi & 0x0F) <= loNib) { AF.lo = AF.lo | 0b00100000; }
        else { AF.lo &= 0b11011111; } // Set H if loer nibble does not underflo
        if (AF.hi < BC.lo) { AF.lo = AF.lo | 0b00010000; }
        else { AF.lo &= 0b11101111; } // Set CY if whole thing does not underflo underflos
        cycles = 4;
    }
    void SUB_d()
    {
        u8 x = AF.hi;
        u8 loNib = AF.hi & 0x0F;
        AF.hi -= DE.hi;
        if (AF.hi == 0) { AF.lo = AF.lo | 0x80; }
        else { AF.lo = 0; } // set Z = 0 if == 0
        AF.lo = AF.lo | 0b01000000; // set N
        if ((AF.hi & 0x0F) <= loNib) { AF.lo = AF.lo | 0b00100000; }
        else { AF.lo &= 0b11011111; } // Set H if loer nibble does not underflo
        if (AF.hi < DE.hi) { AF.lo = AF.lo | 0b00010000; }
        else { AF.lo &= 0b11101111; } // Set CY if whole thing does not underflo underflos
        cycles = 4;
    }
    void SUB_e()
    {
        u8 x = AF.hi;
        u8 loNib = AF.hi & 0x0F;
        AF.hi -= DE.lo;
        if (AF.hi == 0) { AF.lo = AF.lo | 0x80; }
        else { AF.lo = 0; } // set Z = 0 if == 0
        AF.lo = AF.lo | 0b01000000; // set N
        if ((AF.hi & 0x0F) <= loNib) { AF.lo = AF.lo | 0b00100000; }
        else { AF.lo &= 0b11011111; } // Set H if loer nibble does not underflo
        if (AF.hi < DE.lo) { AF.lo = AF.lo | 0b00010000; }
        else { AF.lo &= 0b11101111; } // Set CY if whole thing does not underflo underflos
        cycles = 4;
    }
    void SUB_h()
    {
        u8 x = AF.hi;
        u8 loNib = AF.hi & 0x0F;
        AF.hi -= HL.hi;
        if (AF.hi == 0) { AF.lo = AF.lo | 0x80; }
        else { AF.lo = 0; } // set Z = 0 if == 0
        AF.lo = AF.lo | 0b01000000; // set N
        if ((AF.hi & 0x0F) <= loNib) { AF.lo = AF.lo | 0b00100000; }
        else { AF.lo &= 0b11011111; } // Set H if loer nibble does not underflo
        if (AF.hi < HL.hi) { AF.lo = AF.lo | 0b00010000; }
        else { AF.lo &= 0b11101111; } // Set CY if whole thing does not underflo underflos
        cycles = 4;
    }
    void SUB_l()
    {
        u8 x = AF.hi;
        u8 loNib = AF.hi & 0x0F;
        AF.hi -= HL.lo;
        if (AF.hi == 0) { AF.lo = AF.lo | 0x80; }
        else { AF.lo = 0; } // set Z = 0 if == 0
        AF.lo = AF.lo | 0b01000000; // set N
        if ((AF.hi & 0x0F) <= loNib) { AF.lo = AF.lo | 0b00100000; }
        else { AF.lo &= 0b11011111; } // Set H if loer nibble does not underflo
        if (AF.hi < HL.lo) { AF.lo = AF.lo | 0b00010000; }
        else { AF.lo &= 0b11101111; } // Set CY if whole thing does not underflo underflos
        cycles = 4;
    }
    void SUB_HL()
    {
        u8 x = RAM::readAt(HL.val());
        u8 loNib = AF.hi & 0x0F;
        AF.hi -= x;
        if (AF.hi == 0) { AF.lo = AF.lo | 0x80; }
        else { AF.lo = 0; } // set Z = 0 if == 0
        AF.lo = AF.lo | 0b01000000; // set N
        if ((AF.hi & 0x0F) <= loNib) { AF.lo = AF.lo | 0b00100000; }
        else { AF.lo &= 0b11011111; } // Set H if loer nibble does not underflo
        if (AF.hi <= x) { AF.lo = AF.lo | 0b00010000; }
        else { AF.lo &= 0b11101111; } // Set CY if whole thing does not underflo underflos
        cycles = 8;
    }
    void SUB_hash()
    {
        u8 x = RAM::read();
        u8 loNib = AF.hi & 0x0F;
        AF.hi -= x;
        if (AF.hi == 0) { AF.lo = AF.lo | 0x80; }
        else { AF.lo = 0; } // set Z = 0 if == 0
        AF.lo = AF.lo | 0b01000000; // set N
        if ((AF.hi & 0x0F) <= loNib) { AF.lo = AF.lo | 0b00100000; }
        else { AF.lo &= 0b11011111; } // Set H if loer nibble does not underflo
        if (AF.hi <= x) { AF.lo = AF.lo | 0b00010000; }
        else { AF.lo &= 0b11101111; } // Set CY if whole thing does not underflo underflos
        cycles = 4;
    }

    void SBC_a()
    {
        u8 x = AF.hi;
        u8 loNib = AF.hi & 0x0F;
        AF.hi -= AF.hi + (AF.lo & 0b00010000);
        if (AF.hi == 0) { AF.lo = AF.lo | 0x80; }
        else { AF.lo = 0; } // set Z = 0 if == 0
        AF.lo = AF.lo | 0b01000000; // set N
        if ((AF.hi & 0x0F) <= x) { AF.lo = AF.lo | 0b00100000; }
        else { AF.lo &= 0b11011111; } // Set H if loer nibble does not underflo
        if (AF.hi <= loNib) { AF.lo = AF.lo | 0b00010000; }
        else { AF.lo &= 0b11101111; } // Set CY if whole thing does not underflo underflos
        cycles = 4;
    }
    void SBC_b()
    {
        u8 x = AF.hi;
        u8 loNib = AF.hi & 0x0F;
        AF.hi -= BC.hi + (AF.lo & 0b00010000);
        if (AF.hi == 0) { AF.lo = AF.lo | 0x80; }
        else { AF.lo = 0; } // set Z = 0 if == 0
        AF.lo = AF.lo | 0b01000000; // set N
        if ((AF.hi & 0x0F) <= x) { AF.lo = AF.lo | 0b00100000; }
        else { AF.lo &= 0b11011111; } // Set H if loer nibble does not underflo
        if (AF.hi <= loNib) { AF.lo = AF.lo | 0b00010000; }
        else { AF.lo &= 0b11101111; } // Set CY if whole thing does not underflo underflos
        cycles = 4;
    }
    void SBC_c()
    {
        u8 x = AF.hi;
        u8 loNib = AF.hi & 0x0F;
        AF.hi -= BC.lo + (AF.lo & 0b00010000);
        if (AF.hi == 0) { AF.lo = AF.lo | 0x80; }
        else { AF.lo = 0; } // set Z = 0 if == 0
        AF.lo = AF.lo | 0b01000000; // set N
        if ((AF.hi & 0x0F) <= x) { AF.lo = AF.lo | 0b00100000; }
        else { AF.lo &= 0b11011111; } // Set H if loer nibble does not underflo
        if (AF.hi <= loNib) { AF.lo = AF.lo | 0b00010000; }
        else { AF.lo &= 0b11101111; } // Set CY if whole thing does not underflo underflos
        cycles = 4;
    }
    void SBC_d()
    {
        u8 x = AF.hi;
        u8 loNib = AF.hi & 0x0F;
        AF.hi -= DE.hi + (AF.lo & 0b00010000);
        if (AF.hi == 0) { AF.lo = AF.lo | 0x80; }
        else { AF.lo = 0; } // set Z = 0 if == 0
        AF.lo = AF.lo | 0b01000000; // set N
        if ((AF.hi & 0x0F) <= x) { AF.lo = AF.lo | 0b00100000; }
        else { AF.lo &= 0b11011111; } // Set H if loer nibble does not underflo
        if (AF.hi <= loNib) { AF.lo = AF.lo | 0b00010000; }
        else { AF.lo &= 0b11101111; } // Set CY if whole thing does not underflo underflos
        cycles = 4;
    }
    void SBC_e()
    {
        u8 x = AF.hi;
        u8 loNib = AF.hi & 0x0F;
        AF.hi -= DE.lo + (AF.lo & 0b00010000);
        if (AF.hi == 0) { AF.lo = AF.lo | 0x80; }
        else { AF.lo = 0; } // set Z = 0 if == 0
        AF.lo = AF.lo | 0b01000000; // set N
        if ((AF.hi & 0x0F) <= x) { AF.lo = AF.lo | 0b00100000; }
        else { AF.lo &= 0b11011111; } // Set H if loer nibble does not underflo
        if (AF.hi <= loNib) { AF.lo = AF.lo | 0b00010000; }
        else { AF.lo &= 0b11101111; } // Set CY if whole thing does not underflo underflos
        cycles = 4;
    }
    void SBC_h()
    {
        u8 x = AF.hi;
        u8 loNib = AF.hi & 0x0F;
        AF.hi -= HL.hi + (AF.lo & 0b00010000);
        if (AF.hi == 0) { AF.lo = AF.lo | 0x80; }
        else { AF.lo = 0; } // set Z = 0 if == 0
        AF.lo = AF.lo | 0b01000000; // set N
        if ((AF.hi & 0x0F) <= x) { AF.lo = AF.lo | 0b00100000; }
        else { AF.lo &= 0b11011111; } // Set H if loer nibble does not underflo
        if (AF.hi <= loNib) { AF.lo = AF.lo | 0b00010000; }
        else { AF.lo &= 0b11101111; } // Set CY if whole thing does not underflo underflos
        cycles = 4;
    }
    void SBC_l()
    {
        u8 x = AF.hi;
        u8 loNib = AF.hi & 0x0F;
        AF.hi -= HL.lo + (AF.lo & 0b00010000);
        if (AF.hi == 0) { AF.lo = AF.lo | 0x80; }
        else { AF.lo = 0; } // set Z = 0 if == 0
        AF.lo = AF.lo | 0b01000000; // set N
        if ((AF.hi & 0x0F) <= x) { AF.lo = AF.lo | 0b00100000; }
        else { AF.lo &= 0b11011111; } // Set H if loer nibble does not underflo
        if (AF.hi <= loNib) { AF.lo = AF.lo | 0b00010000; }
        else { AF.lo &= 0b11101111; } // Set CY if whole thing does not underflo underflos
        cycles = 4;
    }
    void SBC_HL()
    {
        u8 x = AF.hi;
        u8 loNib = AF.hi & 0x0F;
        AF.hi -= RAM::readAt(HL.val()) + (AF.lo & 0b00010000);
        if (AF.hi == 0) { AF.lo = AF.lo | 0x80; }
        else { AF.lo = 0; } // set Z = 0 if == 0
        AF.lo = AF.lo | 0b01000000; // set N
        if ((AF.hi & 0x0F) <= x) { AF.lo = AF.lo | 0b00100000; }
        else { AF.lo &= 0b11011111; } // Set H if loer nibble does not underflo
        if (AF.hi <= loNib) { AF.lo = AF.lo | 0b00010000; }
        else { AF.lo &= 0b11101111; } // Set CY if whole thing does not underflo underflos
        cycles = 8;
    }

    void AND_a() { AF.hi = AF.hi & AF.hi; if (AF.hi == 0) { AF.lo |= 0b10000000; } else { AF.lo &= 0b01111111; } AF.lo &= 0b10100000; AF.lo |= 0b00100000; cycles = 4; }
    void AND_b() { AF.hi = AF.hi & BC.hi; if (AF.hi == 0) { AF.lo |= 0b10000000; } else { AF.lo &= 0b01111111; } AF.lo &= 0b10100000; AF.lo |= 0b00100000; cycles = 4; }
    void AND_c() { AF.hi = AF.hi & BC.lo; if (AF.hi == 0) { AF.lo |= 0b10000000; } else { AF.lo &= 0b01111111; } AF.lo &= 0b10100000; AF.lo |= 0b00100000; cycles = 4; }
    void AND_d() { AF.hi = AF.hi & DE.hi; if (AF.hi == 0) { AF.lo |= 0b10000000; } else { AF.lo &= 0b01111111; } AF.lo &= 0b10100000; AF.lo |= 0b00100000; cycles = 4; }
    void AND_e() { AF.hi = AF.hi & DE.lo; if (AF.hi == 0) { AF.lo |= 0b10000000; } else { AF.lo &= 0b01111111; } AF.lo &= 0b10100000; AF.lo |= 0b00100000; cycles = 4; }
    void AND_h() { AF.hi = AF.hi & HL.hi; if (AF.hi == 0) { AF.lo |= 0b10000000; } else { AF.lo &= 0b01111111; } AF.lo &= 0b10100000; AF.lo |= 0b00100000; cycles = 4; }
    void AND_l() { AF.hi = AF.hi & HL.lo; if (AF.hi == 0) { AF.lo |= 0b10000000; } else { AF.lo &= 0b01111111; } AF.lo &= 0b10100000; AF.lo |= 0b00100000; cycles = 4; }
    void AND_HL() { AF.hi = AF.hi & RAM::readAt(HL.val()); if (AF.hi == 0) { AF.lo |= 0b10000000; } else { AF.lo &= 0b01111111; } AF.lo &= 0b10100000; AF.lo |= 0b00100000; cycles = 8; }
    void AND_hash() { AF.hi = AF.hi & RAM::read(); if (AF.hi == 0) { AF.lo |= 0b10000000; } else { AF.lo &= 0b01111111; } AF.lo &= 0b10100000; AF.lo |= 0b00100000; cycles = 8; }

    void OR_a() { AF.hi = AF.hi | AF.hi; if (AF.hi == 0) { AF.lo |= 0b10000000; } else { AF.lo &= 0b01111111; } AF.lo &= 0b10100000; cycles = 4; }
    void OR_b() { AF.hi = AF.hi | BC.hi; if (AF.hi == 0) { AF.lo |= 0b10000000; } else { AF.lo &= 0b01111111; } AF.lo &= 0b10100000; cycles = 4; }
    void OR_c() { AF.hi = AF.hi | BC.lo; if (AF.hi == 0) { AF.lo |= 0b10000000; } else { AF.lo &= 0b01111111; } AF.lo &= 0b10100000; cycles = 4; }
    void OR_d() { AF.hi = AF.hi | DE.hi; if (AF.hi == 0) { AF.lo |= 0b10000000; } else { AF.lo &= 0b01111111; } AF.lo &= 0b10100000; cycles = 4; }
    void OR_e() { AF.hi = AF.hi | DE.lo; if (AF.hi == 0) { AF.lo |= 0b10000000; } else { AF.lo &= 0b01111111; } AF.lo &= 0b10100000; cycles = 4; }
    void OR_h() { AF.hi = AF.hi | HL.hi; if (AF.hi == 0) { AF.lo |= 0b10000000; } else { AF.lo &= 0b01111111; } AF.lo &= 0b10100000; cycles = 4; }
    void OR_l() { AF.hi = AF.hi | HL.lo; if (AF.hi == 0) { AF.lo |= 0b10000000; } else { AF.lo &= 0b01111111; } AF.lo &= 0b10100000; cycles = 4; }
    void OR_HL() { AF.hi = AF.hi | (RAM::readAt(HL.val())); if (AF.hi == 0) { AF.lo |= 0b10000000; } else { AF.lo &= 0b01111111; } AF.lo &= 0b10100000; cycles = 8; }
    void OR_hash() { AF.hi = AF.hi | RAM::read(); if (AF.hi == 0) { AF.lo |= 0b10000000; } else { AF.lo &= 0b01111111; } AF.lo &= 0b10100000; cycles = 8; }


    void XOR_a() { AF.hi ^= AF.hi; if (AF.hi == 0) { AF.lo |= 0b10000000; } else { AF.lo &= 0b01111111; } AF.lo &= 0b10000000; cycles = 4; }
    void XOR_b() { AF.hi ^= BC.hi; if (AF.hi == 0) { AF.lo |= 0b10000000; } else { AF.lo &= 0b01111111; } AF.lo &= 0b10000000; cycles = 4; }
    void XOR_c() { AF.hi ^= BC.lo; if (AF.hi == 0) { AF.lo |= 0b10000000; } else { AF.lo &= 0b01111111; } AF.lo &= 0b10000000; cycles = 4; }
    void XOR_d() { AF.hi ^= DE.hi; if (AF.hi == 0) { AF.lo |= 0b10000000; } else { AF.lo &= 0b01111111; } AF.lo &= 0b10000000; cycles = 4; }
    void XOR_e() { AF.hi ^= DE.lo; if (AF.hi == 0) { AF.lo |= 0b10000000; } else { AF.lo &= 0b01111111; } AF.lo &= 0b10000000; cycles = 4; }
    void XOR_h() { AF.hi ^= HL.hi; if (AF.hi == 0) { AF.lo |= 0b10000000; } else { AF.lo &= 0b01111111; } AF.lo &= 0b10000000; cycles = 4; }
    void XOR_l() { AF.hi ^= HL.lo; if (AF.hi == 0) { AF.lo |= 0b10000000; } else { AF.lo &= 0b01111111; } AF.lo &= 0b10000000; cycles = 4; }
    void XOR_HL() { AF.hi ^= (RAM::readAt(HL.val())); if (AF.hi == 0) { AF.lo |= 0b10000000; } else { AF.lo &= 0b01111111; } AF.lo &= 0b10000000; cycles = 8; }
    void XOR_hash() { AF.hi ^= RAM::read(); if (AF.hi == 0) { AF.lo |= 0b10000000; } else { AF.lo &= 0b01111111; } AF.lo &= 0b10000000; cycles = 8; }

    void CP_a()
    {
        u8 x = AF.hi;
        u8 loNib = AF.hi & 0x0F;
        u8 z = AF.hi - AF.hi;
        if (z == 0) { AF.lo |= 0x80; }
        else { AF.lo &= 0b01110000; } // set Z = 1 if == 0
        AF.lo = AF.lo | 0b01000000; // set N
        if ((z & 0x0F) <= loNib) { AF.lo = AF.lo | 0b00100000; }
        else { AF.lo &= 0b11010000; } // Set H if loer nibble does not underflo
        if (AF.hi < AF.hi) { AF.lo = AF.lo | 0b00010000; }
        else { AF.lo &= 0b11101111; } // Set CY if whole thing does not underflo 
        cycles = 4;
    }
    void CP_b()
    {
        u8 x = AF.hi;
        u8 loNib = AF.hi & 0x0F;
        u8 z = AF.hi - BC.hi;
        if (z == 0) { AF.lo = AF.lo | 0x80; }
        else { AF.lo &= 0b01110000; } // set Z = 0 if == 0
        AF.lo = AF.lo | 0b01000000; // set N
        if ((z & 0x0F) <= loNib) { AF.lo = AF.lo | 0b00100000; }
        else { AF.lo &= 0b11010000; } // Set H if loer nibble does not underflo
        if (AF.hi < BC.hi) { AF.lo = AF.lo | 0b00010000; }
        else { AF.lo &= 0b11101111; } // Set CY if whole thing does not underflo
        cycles = 4;
    }
    void CP_c()
    {
        u8 x = AF.hi;
        u8 loNib = AF.hi & 0x0F;
        u8 z = AF.hi - BC.lo;
        if (z == 0) { AF.lo = AF.lo | 0x80; }
        else { AF.lo &= 0b01110000; } // set Z = 0 if == 0
        AF.lo = AF.lo | 0b01000000; // set N
        if ((z & 0x0F) <= loNib) { AF.lo = AF.lo | 0b00100000; }
        else { AF.lo &= 0b11010000; } // Set H if loer nibble does not underflo
        if (AF.hi < BC.lo) { AF.lo = AF.lo | 0b00010000; }
        else { AF.lo &= 0b11101111; } // Set CY if whole thing does not underflo
        cycles = 4;
    }
    void CP_d()
    {
        u8 x = AF.hi;
        u8 loNib = AF.hi & 0x0F;
        u8 z = AF.hi - DE.hi;
        if (z == 0) { AF.lo = AF.lo | 0x80; }
        else { AF.lo &= 0b01110000; } // set Z = 0 if == 0
        AF.lo = AF.lo | 0b01000000; // set N
        if ((z & 0x0F) <= loNib) { AF.lo = AF.lo | 0b00100000; }
        else { AF.lo &= 0b11010000; } // Set H if loer nibble does not underflo
        if (AF.hi < DE.hi) { AF.lo = AF.lo | 0b00010000; }
        else { AF.lo &= 0b11101111; } // Set CY if whole thing does not underflo 
        cycles = 4;
    }

    void CP_e()
    {
        u8 x = AF.hi;
        u8 loNib = AF.hi & 0x0F;
        u8 z = AF.hi - DE.lo;
        if (z == 0) { AF.lo = AF.lo | 0x80; }
        else { AF.lo &= 0b01110000; } // set Z = 0 if == 0
        AF.lo = AF.lo | 0b01000000; // set N
        if ((z & 0x0F) <= loNib) { AF.lo = AF.lo | 0b00100000; }
        else { AF.lo &= 0b11010000; } // Set H if loer nibble does not underflo
        if (AF.hi < DE.lo) { AF.lo = AF.lo | 0b00010000; }
        else { AF.lo &= 0b11101111; } // Set CY if whole thing does not underflo underflos
        cycles = 4;
    }
    void CP_h()
    {
        u8 x = AF.hi;
        u8 loNib = AF.hi & 0x0F;
        u8 z = AF.hi - HL.hi;
        if (z == 0) { AF.lo = AF.lo | 0x80; }
        else { AF.lo &= 0b01110000; } // set Z = 0 if == 0
        AF.lo = AF.lo | 0b01000000; // set N
        if ((z & 0x0F) <= loNib) { AF.lo = AF.lo | 0b00100000; }
        else { AF.lo &= 0b11010000; } // Set H if loer nibble does not underflo
        if (AF.hi < HL.hi) { AF.lo = AF.lo | 0b00010000; }
        else { AF.lo &= 0b11101111; } // Set CY if whole thing does not underflo underflos
        cycles = 4;
    }
    void CP_l()
    {
        u8 x = AF.hi;
        u8 loNib = AF.hi & 0x0F;
        u8 z = AF.hi - HL.lo;
        if (z == 0) { AF.lo = AF.lo | 0x80; }
        else { AF.lo &= 0b01110000; }// set Z = 1 if == 0
        AF.lo = AF.lo | 0b01000000; // set N
        if ((z & 0x0F) <= loNib) { AF.lo = AF.lo | 0b00100000; }
        else { AF.lo &= 0b11010000; } // Set H if loer nibble does not underflo
        if (AF.hi < HL.lo) { AF.lo = AF.lo | 0b00010000; }
        else { AF.lo &= 0b11101111; } // Set CY if whole thing does not underflo underflos
        cycles = 4;
        //TODO: i'm setting this last one incorrectly I think (the CPU manual thinks)
    }
    void CP_HL()
    {
        u8 x = RAM::readAt(HL.val());
        u8 loNib = AF.hi & 0x0F;
        u8 z = AF.hi - x;
        if (z == 0) { AF.lo = AF.lo | 0x80; }
        else { AF.lo &= 0b01110000; } // set Z = 1 if == 0
        AF.lo = AF.lo | 0b01000000; // set N
        if ((z & 0x0F) <= loNib) { AF.lo = AF.lo | 0b00100000; }
        else { AF.lo &= 0b11010000; } // Set H if loer nibble does not underflo
        if (AF.hi < x) { AF.lo = AF.lo | 0b00010000; }
        else { AF.lo &= 0b11101111; } // Set CY if whole thing does not underflo underflos
        cycles = 8;
    }

    void CP_hash()
    {
        u8 x = RAM::read();
        u8 loNib = AF.hi & 0x0F;
        u8 z = AF.hi - x;
        if (z == 0) { AF.lo |= 0b10000000; }
        else { AF.lo &= 0b01110000; } // set Z = 1 if result is == 0
        AF.lo |= 0b01000000; // set N
        if ((z & 0x0F) <= loNib) { AF.lo = AF.lo | 0b00100000; }
        else { AF.lo &= 0b11010000; } // Set H if loer nibble does not underflo
        if (AF.hi < x) { AF.lo = AF.lo | 0b00010000; }
        else { AF.lo &= 0b11101111; }// Set CY if whole thing does not underflo underflos
        cycles = 8;
    }

    void INC_a()
    {
        u8 loNib = AF.hi & 0x0F;
        ++AF.hi;
        if (AF.hi == 0) { AF.lo |= 0b10000000; }
        else { AF.lo &= 0b01110000; }
        AF.lo &= 0b10110000;
        if (loNib > (AF.hi & 0x0F)) { AF.lo |= 0b00100000; }
        else { AF.lo &= 0b11010000; }
        cycles = 4;
    }
    void INC_b()
    {
        u8 loNib = BC.hi & 0x0F;
        ++BC.hi;
        if (BC.hi == 0) { AF.lo |= 0b10000000; }
        else { AF.lo &= 0b01110000; }
        AF.lo &= 0b10110000;
        if (loNib > (BC.hi & 0x0F)) { AF.lo |= 0b00100000; }
        else { AF.lo &= 0b11010000; }
        cycles = 4;
    }
    void INC_c()
    {
        u8 loNib = BC.lo & 0x0F;
        ++BC.lo;
        if (BC.lo == 0) { AF.lo |= 0b10000000; }
        else { AF.lo &= 0b01110000; }
        AF.lo &= 0b10110000;
        if (loNib > (BC.lo & 0x0F)) { AF.lo |= 0b00100000; }
        else { AF.lo &= 0b11010000; }
        cycles = 4;
    }
    void INC_d()
    {
        u8 loNib = DE.hi & 0x0F;
        ++DE.hi;
        if (DE.hi == 0) { AF.lo |= 0b10000000; }
        else { AF.lo &= 0b01110000; }
        AF.lo &= 0b10110000;
        if (loNib > (DE.hi & 0x0F)) { AF.lo |= 0b00100000; }
        else { AF.lo &= 0b11010000; }
        cycles = 4;
    }
    void INC_e()
    {
        u8 loNib = DE.lo & 0x0F;
        ++AF.hi;
        if (DE.lo == 0) { AF.lo |= 0b10000000; }
        else { AF.lo &= 0b01110000; }
        AF.lo &= 0b10110000;
        if (loNib > (DE.lo & 0x0F)) { AF.lo |= 0b00100000; }
        else { AF.lo &= 0b11010000; }
        cycles = 4;
    }
    void INC_h()
    {
        u8 loNib = HL.hi & 0x0F;
        ++HL.hi;
        if (HL.hi == 0) { AF.lo |= 0b10000000; }
        else { AF.lo &= 0b01110000; }
        AF.lo &= 0b10110000;
        if (loNib > (HL.hi & 0x0F)) { AF.lo |= 0b00100000; }
        else { AF.lo &= 0b11010000; }
        cycles = 4;
    }
    void INC_l()
    {
        u8 loNib = HL.lo & 0x0F;
        ++HL.lo;
        if (HL.lo == 0) { AF.lo |= 0b10000000; }
        else { AF.lo &= 0b01110000; }
        AF.lo &= 0b10110000;
        if (loNib > (HL.lo & 0x0F)) { AF.lo |= 0b00100000; }
        else { AF.lo &= 0b11010000; }
        cycles = 4;
    }
    void INC_HLad()
    {
        u8 x = RAM::readAt(HL.val());
        u8 loNib = x & 0x0F;
        ++x;
        RAM::write(x, HL.val());
        if (x == 0) { AF.lo |= 0b10000000; }
        else { AF.lo &= 0b01110000; }
        AF.lo &= 0b10110000;
        if (loNib > (x & 0x0F)) { AF.lo |= 0b00100000; }
        else { AF.lo &= 0b11010000; }
        cycles = 12;
    }

    void DEC_a()
    {
        u8 loNib = AF.hi & 0x0F;
        --AF.hi;
        if (AF.hi == 0) { AF.lo |= 0b10000000; }
        else { AF.lo &= 0b01110000; }
        AF.lo |= 0b01000000;
        if (loNib >= (AF.hi & 0x0F)) { AF.lo |= 0b00100000; }
        else { AF.lo &= 0b11010000; }
        cycles = 4;
    }
    void DEC_b()
    {
        u8 loNib = BC.hi & 0x0F;
        --BC.hi;
        if (BC.hi == 0) { AF.lo |= 0b10000000; }
        else { AF.lo &= 0b01110000; }
        AF.lo |= 0b01000000;
        if (loNib >= (BC.hi & 0x0F)) { AF.lo |= 0b00100000; }
        else { AF.lo &= 0b11010000; }
        cycles = 4;
    }
    void DEC_c()
    {
        u8 loNib = BC.lo & 0x0F;
        --BC.lo;
        if (BC.lo == 0) { AF.lo |= 0b10000000; }
        else { AF.lo &= 0b01110000; } // Z set if the result is zero
        AF.lo |= 0b01000000;
        if (loNib >= (BC.lo & 0x0F)) { AF.lo |= 0b00100000; }
        else { AF.lo &= 0b11010000; }
        cycles = 4;
    }
    void DEC_d()
    {
        u8 loNib = DE.hi & 0x0F;
        --DE.hi;
        if (DE.hi == 0) { AF.lo |= 0b10000000; }
        else { AF.lo &= 0b01110000; }
        AF.lo |= 0b01000000;
        if (loNib >= (DE.hi & 0x0F)) { AF.lo |= 0b00100000; }
        else { AF.lo &= 0b11010000; }
        cycles = 4;
    }
    void DEC_e()
    {
        u8 loNib = DE.lo & 0x0F;
        --DE.lo;
        if (DE.lo == 0) { AF.lo |= 0b10000000; }
        else { AF.lo &= 0b01110000; }
        AF.lo |= 0b01000000;
        if (loNib >= (DE.lo & 0x0F)) { AF.lo |= 0b00100000; }
        else { AF.lo &= 0b11010000; }
        cycles = 4;
    }
    void DEC_h()
    {
        u8 loNib = HL.hi & 0x0F;
        --HL.hi;
        if (HL.hi == 0) { AF.lo |= 0b10000000; }
        else { AF.lo &= 0b01110000; }
        AF.lo |= 0b01000000;
        if (loNib >= (HL.hi & 0x0F)) { AF.lo |= 0b00100000; }
        else { AF.lo &= 0b11010000; }
        cycles = 4;
    }
    void DEC_l()
    {
        u8 loNib = HL.lo & 0x0F;
        HL.lo--;
        if (HL.lo == 0) { AF.lo |= 0b10000000; }
        else { AF.lo &= 0b01110000; }
        AF.lo |= 0b01000000;
        if (loNib > (HL.lo & 0x0F)) { AF.lo |= 0b00100000; }
        else { AF.lo &= 0b11010000; }
        cycles = 4;
    }
    void DEC_HLad()
    {
        u8 x = RAM::readAt(HL.val());
        u8 loNib = x & 0x0F;
        x--;
        RAM::write(x, HL.val());
        if (x == 0) { AF.lo |= 0b10000000; }
        else { AF.lo &= 0b01110000; }
        AF.lo |= 0b01000000;
        if (loNib > (x & 0x0F)) { AF.lo |= 0b00100000; }
        else { AF.lo &= 0b11010000; }
        cycles = 12;
    }


    void HALT() {
        halt = true;
        cycles = 4;
    }
    // whatever the fuck i need to get this shit working
    void RST_00() {
        u8 loNibble = PC & 0x00FF;
        u8 hiNibble = (PC & 0xFF00) >> 8;
        SP--;
        RAM::write(hiNibble, SP);
        SP--;
        RAM::write(loNibble, SP);

        PC = 0x00;
        cycles = 32;
    }
    void RST_08() {
        u8 loNibble = PC & 0x00FF;
        u8 hiNibble = (PC & 0xFF00) >> 8;
        SP--;
        RAM::write(hiNibble, SP);
        SP--;
        RAM::write(loNibble, SP);

        PC = 0x08;
        cycles = 32;
    }
    void RST_10() {
        u8 loNibble = PC & 0x00FF;
        u8 hiNibble = (PC & 0xFF00) >> 8;
        SP--;
        RAM::write(hiNibble, SP);
        SP--;
        RAM::write(loNibble, SP);

        PC = 0x10;
        cycles = 32;
    }
    void RST_18() {
        u8 loNibble = PC & 0x00FF;
        u8 hiNibble = (PC & 0xFF00) >> 8;
        SP--;
        RAM::write(hiNibble, SP);
        SP--;
        RAM::write(loNibble, SP);

        PC = 0x18;
        cycles = 32;
    }
    void RST_20() {
        u8 loNibble = PC & 0x00FF;
        u8 hiNibble = (PC & 0xFF00) >> 8;
        SP--;
        RAM::write(hiNibble, SP);
        SP--;
        RAM::write(loNibble, SP);

        PC = 0x20;
        cycles = 32;
    }
    void RST_28() {
        u8 loNibble = PC & 0x00FF;
        u8 hiNibble = (PC & 0xFF00) >> 8;
        SP--;
        RAM::write(hiNibble, SP);
        SP--;
        RAM::write(loNibble, SP);

        PC = 0x28;
        cycles = 32;
    }
    void RST_30() {
        u8 loNibble = PC & 0b00001111;
        u8 hiNibble = PC >> 4;
        SP--;
        RAM::write(hiNibble, SP);
        SP--;
        RAM::write(loNibble, SP);

        PC = 0x30;
        cycles = 32;
    }
    void RST_38() {
        u8 loNibble = PC & 0x00FF;
        u8 hiNibble = (PC & 0xFF00) >> 8;
        SP--;
        RAM::write(hiNibble, SP);
        SP--;
        RAM::write(loNibble, SP);

        PC = 0x38;
        cycles = 32;
    }

    void LDI_aHL() {
        AF.hi = RAM::readAt(HL.val());
        HL.set(HL.val() + 1);
        cycles = 8;
    }

    void LDH_nA() {
        u8 n = RAM::read();
        RAM::write(AF.hi, 0xFF00 + n);
        cycles = 12;
    }

    void ADD_BC() {
        u16 x = HL.val();
        HL.set(x + BC.val());
        AF.lo &= 0b10110000;
        // half carry - set H if carry from bit 11 (so the 12th place in the binary expression)
        if ((HL.val() & 0x0FFF) < (x & 0xFFF)) { AF.lo |= 0b00100000; }
        else { AF.lo &= 0b11010000; }
        //carry flag - we set C if there is carry, which is when the result is lower than to start
        if (HL.val() < x) { AF.lo |= 0b00010000; }
        else { AF.lo &= 0b11100000; }
        cycles = 8;
    }
    void ADD_DE() {
        u16 x = HL.val();
        HL.set(x + DE.val());
        AF.lo &= 0b10110000;
        // half carry - set H if carry from bit 11 (so the 12th place in the binary expression)
        if ((HL.val() & 0x0FFF) < (x & 0xFFF)) { AF.lo |= 0b00100000; }
        else { AF.lo &= 0b11010000; }
        //carry flag - we set C if there is carry, which is when the result is lower than to start
        if (HL.val() < x) { AF.lo |= 0b00010000; }
        else { AF.lo &= 0b11100000; }
        cycles = 8;
    }
    void ADD_HL() {
        u16 x = HL.val();
        HL.set(2 * x);
        AF.lo &= 0b10110000;
        // half carry - set H if carry from bit 11 (so the 12th place in the binary expression)
        if ((HL.val() & 0x0FFF) < (x & 0xFFF)) { AF.lo |= 0b00100000; }
        else { AF.lo &= 0b11010000; }
        //carry flag - we set C if there is carry, which is when the result is lower than to start
        if (HL.val() < x) { AF.lo |= 0b00010000; }
        else { AF.lo &= 0b11100000; }
        cycles = 8;
    }
    void ADD_SP() {
        u16 x = HL.val();
        HL.set(x + SP);
        AF.lo &= 0b10110000;
        // half carry - set H if carry from bit 11 (so the 12th place in the binary expression)
        if ((HL.val() & 0x0FFF) < (x & 0xFFF)) { AF.lo |= 0b00100000; }
        else { AF.lo &= 0b11010000; }
        //carry flag - we set C if there is carry, which is when the result is lower than to start
        if (HL.val() < x) { AF.lo |= 0b00010000; }
        else { AF.lo &= 0b11100000; }
        cycles = 8;
    }

    void DI() {
        interrupt_disable = 2;
        cycles = 4;
    }

    void EI() {
        interrupt_disable = 4;
        cycles = 4;
    }

    void ADD_n_SP() {
        u8 n = RAM::read();
        u8 x = SP;
        SP += n;
        AF.lo &= 0b00110000;

        // half carry - set H if carry from bit 11 (so the 12th place in the binary expression)
        if ((SP & 0x0FFF) < (x & 0xFFF)) { AF.lo |= 0b00100000; }
        else { AF.lo &= 0b11010000; }
        //carry flag - we set C if there is carry, which is when the result is lower than to start
        if (SP < x) { AF.lo |= 0b00010000; }
        else { AF.lo &= 0b11100000; }
        cycles = 16;
    }

    void INC_BC() {
        BC.set(BC.val() + 1);
        cycles = 8;
    }
    void INC_DE() {
        DE.set(DE.val() + 1);
        cycles = 8;
    }
    void INC_HL() {
        HL.set(HL.val() + 1);
        cycles = 8;
    }
    void INC_SP() {
        SP++;
        cycles = 8;
    }

    void DEC_BC() {
        BC.set(BC.val() - 1);
        cycles = 8;
    }
    void DEC_DE() {
        DE.set(DE.val() - 1);
        cycles = 8;
    }
    void DEC_HL() {
        HL.set(HL.val() - 1);
        cycles = 8;
    }
    void DEC_SP() {
        SP--;
        cycles = 8;
    }

    void CPL() {
        AF.hi = 0xFF - AF.hi;
        AF.lo |= 0b01100000;
        cycles = 4;
    }

    void SWAP_a() {
        AF.hi = ((AF.hi & 0xF0) >> 4) + ((AF.hi & 0x0F) << 4);
        AF.lo = 0;
        if (AF.hi == 0) { AF.lo |= 0b10000000; }
        cycles = 8;
    }
    void SWAP_b() {
        BC.hi = ((BC.hi & 0xF0) >> 4) + ((BC.hi & 0x0F) << 4);
        AF.lo = 0;
        if (BC.hi == 0) { AF.lo |= 0b10000000; }
        cycles = 8;
    }
    void SWAP_c() {
        BC.lo = ((BC.lo & 0xF0) >> 4) + ((BC.lo & 0x0F) << 4);
        AF.lo = 0;
        if (BC.lo == 0) { AF.lo |= 0b10000000; }
        cycles = 8;
    }

    void SWAP_d() {
        DE.hi = ((DE.hi & 0xF0) >> 4) + ((DE.hi & 0x0F) << 4);
        AF.lo = 0;
        if (DE.hi == 0) { AF.lo |= 0b10000000; }
        cycles = 8;
    }
    void SWAP_e() {
        DE.lo = ((DE.lo & 0xF0) >> 4) + ((DE.lo & 0x0F) << 4);
        AF.lo = 0;
        if (DE.lo == 0) { AF.lo |= 0b10000000; }
        cycles = 8;
    }
    void SWAP_h() {
        HL.hi = ((HL.hi & 0xF0) >> 4) + ((HL.hi & 0x0F) << 4);
        AF.lo = 0;
        if (HL.hi == 0) { AF.lo |= 0b10000000; }
        cycles = 8;
    }
    void SWAP_l() {
        HL.lo = ((HL.lo & 0xF0) >> 4) + ((HL.hi & 0x0F) << 4);
        AF.lo = 0;
        if (HL.lo == 0) { AF.lo |= 0b10000000; }
        cycles = 8;
    }
    void SWAP_HL() {
        u8 x = RAM::readAt(HL.val());
        x = ((x & 0xF0) >> 4) + ((x & 0x0F) << 4);
        RAM::write(x, HL.val());
        AF.lo = 0;
        if (HL.hi == 0) { AF.lo |= 0b10000000; }
        cycles = 16;
    }

    void RES_0_a() {
        AF.hi &= 0b11111110;
        cycles = 8;
    }

    void DAA() {
        std::cout << "What the fuck is this: opcode 0x27, DAA" << std::endl;
        exit(0);
    }

    void init_decoder() {
        decoder[0x00] = "NOP";
        decoder[0x01] = "LD_nn_BC";
        decoder[0x02] = "LD_BC_a";
        decoder[0x03] = "INC_BC";
        decoder[0x04] = "INC_b";
        decoder[0x05] = "DEC_b";
        decoder[0x06] = "LDb_n";
        decoder[0x07] = "RLCA";
        decoder[0x08] = "LD_nnSP";
        decoder[0x09] = "ADD_BC";
        decoder[0x0A] = "LDrr_aBC";
        decoder[0x0B] = "DEC_BC";
        decoder[0x0C] = "INC_c";
        decoder[0x0D] = "DEC_c";
        decoder[0x0E] = "LDc_n";
        decoder[0x0F] = "RRCA";
        decoder[0x11] = "LD_nn_DE";
        decoder[0x12] = "LD_DE_a";
        decoder[0x13] = "INC_DE";
        decoder[0x14] = "INC_d";
        decoder[0x15] = "DEC_d";
        decoder[0x16] = "LDd_n";
        decoder[0x17] = "RLA";
        decoder[0x18] = "JR_n";
        decoder[0x19] = "ADD_DE";
        decoder[0x1A] = "LDrr_aDE";
        decoder[0x1B] = "DEC_DE";
        decoder[0x1C] = "INC_e";
        decoder[0x1D] = "DEC_e";
        decoder[0x1E] = "LDe_n";

        decoder[0x20] = "JR_NZ";
        decoder[0x21] = "LD_nn_HL";
        decoder[0x22] = "LDI_HLa";
        decoder[0x23] = "INC_HL";
        decoder[0x24] = "INC_h";
        decoder[0x25] = "DEC_h";
        decoder[0x26] = "LDh_n";
        decoder[0x27] = "DAA";
        decoder[0x28] = "JR_Z";
        decoder[0x29] = "ADD_HL";
        decoder[0x2A] = "LDI_aHL";
        decoder[0x2B] = "DEC_HL";
        decoder[0x2C] = "INC_l";
        decoder[0x2D] = "DEC_l";
        decoder[0x2E] = "LDl_n";
        decoder[0x2F] = "CPL";

        decoder[0x7F] = "op_not_imp";
        decoder[0x78] = "LDrr_ab";
        decoder[0x79] = "LDrr_ac";
        decoder[0x7A] = "LDrr_ad";
        decoder[0x7B] = "LDrr_ae";
        decoder[0x7C] = "LDrr_ah";
        decoder[0x7D] = "LDrr_al";
        decoder[0x7E] = "LDrr_aHL";
        decoder[0xFA] = "LDrr_ann";
        decoder[0x3E] = "LDrr_a_hash";
        decoder[0x40] = "op_not_imp";
        decoder[0x41] = "LDrr_bc";
        decoder[0x42] = "LDrr_bd";
        decoder[0x43] = "LDrr_be";
        decoder[0x44] = "LDrr_bh";
        decoder[0x45] = "LDrr_bl";
        decoder[0x46] = "LDrr_bHL";
        decoder[0x47] = "LDrr_ba";
        decoder[0x48] = "LDrr_cb";
        decoder[0x49] = "op_not_imp";
        decoder[0x4A] = "LDrr_cd";
        decoder[0x4B] = "LDrr_ce";
        decoder[0x4C] = "LDrr_ch";
        decoder[0x4D] = "LDrr_cl";
        decoder[0x4E] = "LDrr_cHL";
        decoder[0x4F] = "LDrr_ca";
        decoder[0x50] = "LDrr_db";
        decoder[0x51] = "LDrr_dc";
        decoder[0x52] = "op_not_imp";
        decoder[0x53] = "LDrr_de";
        decoder[0x54] = "LDrr_dh";
        decoder[0x55] = "LDrr_dl";
        decoder[0x56] = "LDrr_dHL";
        decoder[0x57] = "LDrr_da";
        decoder[0x58] = "LDrr_eb";
        decoder[0x59] = "LDrr_ec";
        decoder[0x5A] = "LDrr_ed";
        decoder[0x5B] = "op_not_imp";
        decoder[0x5C] = "LDrr_eh";
        decoder[0x5D] = "LDrr_el";
        decoder[0x5E] = "LDrr_eHL";
        decoder[0x5F] = "LDrr_ea";
        decoder[0x60] = "LDrr_hb";
        decoder[0x61] = "LDrr_hc";
        decoder[0x62] = "LDrr_hd";
        decoder[0x63] = "LDrr_he";
        decoder[0x64] = "op_not_imp";
        decoder[0x65] = "LDrr_hl";
        decoder[0x66] = "LDrr_hHL";
        decoder[0x67] = "LDrr_ha";
        decoder[0x68] = "LDrr_lb";
        decoder[0x69] = "LDrr_lc";
        decoder[0x6A] = "LDrr_ld";
        decoder[0x6B] = "LDrr_le";
        decoder[0x6C] = "LDrr_lh";
        decoder[0x6D] = "op_not_imp";
        decoder[0x6E] = "LDrr_lHL";
        decoder[0x6F] = "LDrr_la";
        decoder[0x70] = "LDrr_HLb";
        decoder[0x71] = "LDrr_HLc";
        decoder[0x72] = "LDrr_HLd";
        decoder[0x73] = "LDrr_HLe";
        decoder[0x74] = "LDrr_HLh";
        decoder[0x75] = "LDrr_HLl";
        decoder[0x36] = "LDrr_HLn";
        decoder[0xF2] = "LDa_c";
        decoder[0xE2] = "LDc_a";
        decoder[0x3A] = "LDDaHL";
        decoder[0x32] = "LDDHLa";
        decoder[0xE0] = "LDH_nA";
        decoder[0xF0] = "LDH_a_ffn";
        decoder[0x31] = "LD_nn_SP";
        decoder[0xF9] = "LD_SPHL";
        decoder[0xF8] = "LDHL_SPn";
        decoder[0xF5] = "PUSH_AF";
        decoder[0xC5] = "PUSH_BC";
        decoder[0xD5] = "PUSH_DE";
        decoder[0xE5] = "PUSH_HL";
        decoder[0xF1] = "POP_AF";
        decoder[0xC1] = "POP_BC";
        decoder[0xD1] = "POP_DE";
        decoder[0xE1] = "POP_HL";
        decoder[0x87] = "ADD_aa";
        decoder[0x80] = "ADD_ab";
        decoder[0x81] = "ADD_ac";
        decoder[0x82] = "ADD_ad";
        decoder[0x83] = "ADD_ae";
        decoder[0x84] = "ADD_ah";
        decoder[0x85] = "ADD_al";
        decoder[0x86] = "ADD_aH";
        decoder[0xC6] = "ADD_a_hash";
        decoder[0x8f] = "ADC_aa";
        decoder[0x88] = "ADC_ab";
        decoder[0x89] = "ADC_ac";
        decoder[0x8A] = "ADC_ad";
        decoder[0x8B] = "ADC_ae";
        decoder[0x8C] = "ADC_ah";
        decoder[0x8D] = "ADC_al";
        decoder[0x8E] = "ADC_aHL";
        decoder[0xCE] = "ADC_a_hash";
        decoder[0x97] = "SUB_a";
        decoder[0x90] = "SUB_b";
        decoder[0x91] = "SUB_c";
        decoder[0x92] = "SUB_d";
        decoder[0x93] = "SUB_e";
        decoder[0x94] = "SUB_h";
        decoder[0x95] = "SUB_l";
        decoder[0x96] = "SUB_HL";
        decoder[0xD6] = "SUB_hash";
        decoder[0x9F] = "SBC_a";
        decoder[0x98] = "SBC_b";
        decoder[0x99] = "SBC_c";
        decoder[0x9A] = "SBC_d";
        decoder[0x9B] = "SBC_e";
        decoder[0x9C] = "SBC_h";
        decoder[0x9D] = "SBC_l";
        decoder[0x9E] = "SBC_HL";
        decoder[0xA7] = "AND_a";
        decoder[0xA0] = "AND_b";
        decoder[0xA1] = "AND_c";
        decoder[0xA2] = "AND_d";
        decoder[0xA3] = "AND_e";
        decoder[0xA4] = "AND_h";
        decoder[0xA5] = "AND_l";
        decoder[0xA6] = "AND_HL";
        decoder[0xE6] = "AND_hash";
        decoder[0xB7] = "OR_a";
        decoder[0xB0] = "OR_b";
        decoder[0xB1] = "OR_c";
        decoder[0xB2] = "OR_d";
        decoder[0xB3] = "OR_e";
        decoder[0xB4] = "OR_h";
        decoder[0xB5] = "OR_l";
        decoder[0xB6] = "OR_HL";
        decoder[0xAF] = "XOR_a";
        decoder[0xA8] = "XOR_b";
        decoder[0xA9] = "XOR_c";
        decoder[0xAA] = "XOR_d";
        decoder[0xAB] = "XOR_e";
        decoder[0xAC] = "XOR_h";
        decoder[0xAD] = "XOR_l";
        decoder[0xAE] = "XOR_HL";
        decoder[0xEE] = "XOR_hash";
        decoder[0xBF] = "CP_a";
        decoder[0xB8] = "CP_b";
        decoder[0xB9] = "CP_c";
        decoder[0xBA] = "CP_d";
        decoder[0xBB] = "CP_e";
        decoder[0xBC] = "CP_h";
        decoder[0xBD] = "CP_l";
        decoder[0xBE] = "CP_HL";
        decoder[0xFE] = "CP_hash";
        decoder[0x3C] = "INC_a";
        decoder[0x34] = "INC_HLad";
        decoder[0x3D] = "DEC_a";
        decoder[0x35] = "DEC_HLad";
        decoder[0x39] = "ADD_SP";
        decoder[0xC7] = "RST_00";
        decoder[0xCF] = "RST_08";
        decoder[0xD7] = "RST_10";
        decoder[0xDF] = "RST_18";
        decoder[0xE7] = "RST_20";
        decoder[0xEF] = "RST_28";
        decoder[0xF7] = "RST_30";
        decoder[0xFF] = "RST_38";
        decoder[0xCB] = "run_cb";
        decoder[0xC3] = "JP";
        decoder[0xC2] = "JP_NZ";
        decoder[0xCA] = "JP_Z";
        decoder[0xD2] = "JP_NC";
        decoder[0xDA] = "JP_C";
        decoder[0xE9] = "JP_HL";
        decoder[0x77] = "LD_HL_a";
        decoder[0xE0] = "LDad_n_a";
        decoder[0xCD] = "CALL_nn";
        decoder[0xC4] = "CALL_NZ";
        decoder[0xCC] = "CALL_Z";
        decoder[0xD4] = "CALL_NC";
        decoder[0xDC] = "CALL_C";
        decoder[0xEA] = "LD_nn_a"; // put value A into the address nn 
        decoder[0xC9] = "RET";
        decoder[0xC0] = "RET_NZ";
        decoder[0xC8] = "RET_Z";
        decoder[0xD0] = "RET_NC";
        decoder[0xD8] = "RET_C";
        decoder[0xD9] = "RETI";
        decoder[0xF3] = "DI";
        decoder[0xFB] = "EI";
        decoder[0xE8] = "ADD_n_SP";
        decoder[0x33] = "INC_SP";
        decoder[0x3B] = "DEC_SP";
    }
}

namespace RAM
{
    std::vector<u8> rom(0x8000, 0); // I have no idea what the size should be
    std::vector<u8> vRam(0xA000 - 0x8000, 0);
    std::vector<u8> iRam(0xC000 - 0xA000, 0);
    std::vector<u8> mbc(0xE000 - 0xC000, 0); // This size should be big enough to cover all RAM banks
    std::vector<u8> sRam(0xFE00 - 0xE000, 0);
    std::vector<u8> OAM(0xFEA0 - 0xFE00, 0);  // this one needs to be made smaller
    std::vector<u8> i2Ram(0xFFFF - 0xFF80, 0); // more internal ram
    std::vector<u8> bootRom = { 0x31, 0xfe, 0xff, 0xaf, 0x21, 0xff, 0x9f, 0x32, 0xcb, 0x7c, 0x20, 0xfb, 0x21, 0x26, 0xff, 0x0e, 0x11, 0x3e, 0x80, 0x32, 0xe2, 0x0c, 0x3e, 0xf3, 0xe2, 0x32, 0x3e, 0x77, 0x77, 0x3e, 0xfc, 0xe0, 0x47, 0x11, 0x04, 0x01, 0x21, 0x10, 0x80, 0x1a, 0xcd, 0x95, 0x00, 0xcd, 0x96, 0x00, 0x13, 0x7b, 0xfe, 0x34, 0x20, 0xf3, 0x11, 0xd8, 0x00, 0x06, 0x08, 0x1a, 0x13, 0x22, 0x23, 0x05, 0x20, 0xf9, 0x3e, 0x19, 0xea, 0x10, 0x99, 0x21, 0x2f, 0x99, 0x0e, 0x0c, 0x3d, 0x28, 0x08, 0x32, 0x0d, 0x20, 0xf9, 0x2e, 0x0f, 0x18, 0xf3, 0x67, 0x3e, 0x64, 0x57, 0xe0, 0x42, 0x3e, 0x91, 0xe0, 0x40, 0x04, 0x1e, 0x02, 0x0e, 0x0c, 0xf0, 0x44, 0xfe, 0x90, 0x20, 0xfa, 0x0d, 0x20, 0xf7, 0x1d, 0x20, 0xf2, 0x0e, 0x13, 0x24, 0x7c, 0x1e, 0x83, 0xfe, 0x62, 0x28, 0x06, 0x1e, 0xc1, 0xfe, 0x64, 0x20, 0x06, 0x7b, 0xe2, 0x0c, 0x3e, 0x87, 0xe2, 0xf0, 0x42, 0x90, 0xe0, 0x42, 0x15, 0x20, 0xd2, 0x05, 0x20, 0x4f, 0x16, 0x20, 0x18, 0xcb, 0x4f, 0x06, 0x04, 0xc5, 0xcb, 0x11, 0x17, 0xc1, 0xcb, 0x11, 0x17, 0x05, 0x20, 0xf5, 0x22, 0x23, 0x22, 0x23, 0xc9, 0xce, 0xed, 0x66, 0x66, 0xcc, 0x0d, 0x00, 0x0b, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0c, 0x00, 0x0d, 0x00, 0x08, 0x11, 0x1f, 0x88, 0x89, 0x00, 0x0e, 0xdc, 0xcc, 0x6e, 0xe6, 0xdd, 0xdd, 0xd9, 0x99, 0xbb, 0xbb, 0x67, 0x63, 0x6e, 0x0e, 0xec, 0xcc, 0xdd, 0xdc, 0x99, 0x9f, 0xbb, 0xb9, 0x33, 0x3e, 0x3c, 0x42, 0xb9, 0xa5, 0xb9, 0xa5, 0x42, 0x3c, 0x21, 0x04, 0x01, 0x11, 0xa8, 0x00, 0x1a, 0x13, 0xbe, 0x20, 0xfe, 0x23, 0x7d, 0xfe, 0x34, 0x20, 0xf5, 0x06, 0x19, 0x78, 0x86, 0x23, 0x05, 0x20, 0xfb, 0x86, 0x20, 0xfe, 0x3e, 0x01, 0xe0, 0x50 };
    std::vector<u8> moreRam(0xFF80 - 0xFEA0, 0); //TODO: needs a better name
    u8 readAt(u16 addr)
    {
        // we'll be assuming 32kB cartridge for now and add swapping in later
        if (addr < 0x8000) {
            if (addr < 0x0100) {
                if (moreRam[0xFF50 - 0xFEA0] == 0) // if the bootrom is enabled (which it is by default)
                {
                    return bootRom.at(addr);
                }
            }
            return rom.at(addr);
        }
        else if (addr < 0xA000) { return vRam.at(addr - 0x8000); }
        else if (addr < 0xC000) { return sRam.at(addr - 0xA000); }
        else if (addr < 0xE000) { return iRam.at(addr - 0xC000); }
        else if (addr < 0xFE00) { return iRam.at(addr - 0xE000); }
        else if (addr < 0xFEA0) { return OAM.at(addr - 0xFE00); }
        else if (addr < 0xFF80) { return moreRam.at(addr - 0xFEA0); }
        else if (0xFF80 <= addr && addr < 0xFFFF) { return i2Ram.at(addr - 0xFF80); }
        else if (addr == 0xFFFF) { return ie; }
        return -1;
    }

    void write(u8 val, u16 addr)
    {
        if (addr < 0x8000) { std::cout << "Tried writing to ROM!" << std::hex << unsigned(addr) << " " << std::hex << unsigned(val) << std::endl; }//rom.at(addr) = val; }
        else if (addr < 0xA000) { vRam.at(addr - 0x8000) = val; }
        else if (addr < 0xC000) { sRam.at(addr - 0xA000) = val; }
        else if (addr < 0xE000) { iRam.at(addr - 0xC000) = val; }
        else if (addr < 0xFE00) { iRam.at(addr - 0xE000) = val; }
        else if (addr < 0xFEA0) { OAM.at(addr - 0xFE00) = val; }
        else if (addr < 0xFF80) {
            moreRam.at(addr - 0xFEA0) = val;
            if (addr == DMA) {
                DMA_routine();
            }
        }
        else if (addr < 0xFFFF) { i2Ram.at(addr - 0xFF80) = val; } //std::cout << "Writing: " << std::hex <<  val << " to " << std::hex << addr << std::endl << std::endl;
        else if (addr == 0xFFFF) { ie = val; }
    }

    //could just make this a function call in CPU
    u8 read()
    {
        u8 byte = readAt(CPU::PC);
        ++CPU::PC;
        return byte;
    }

    void DMA_routine() {
        // the source of the DMA transfer is determined by the value written to the register, starting at XX00 where XX is the value in hex
        // the original gameboy could only take xx to be from 0x00-F1.

        std::cout << "DMA Time.\n\n";

        u16 source = (readAt(DMA) << 8);
        // std::cout << "SOURCE:" << std::hex << unsigned(readAt(0xFF46)) << std::endl;
        u8 data;
        for (int idx = 0; idx < 0xF1; idx++) {

            //std::cout << std::hex << unsigned(source + idx)<< std::endl;
            data = readAt(source + idx);

            // now write the data to OAM
            write(data, 0xFE00 + idx);
        }

        dump_oam();
        std::cout << "DO WE GET HERE";
        // exit(0);
    }

    void init() {
        FILE* rom_file;
        FILE* boot;
        char buffer[100];
        rom_file = fopen("Tetris.gb", "rb");
        //rom_file = fopen("Tetris.gb", "rb");
        boot = fopen("boot.rom", "rb");

        int c;
        int i = 0;
        while ((c = std::fgetc(rom_file)) != EOF) {
            RAM::rom[i] = c;
            ++i;
        }

        u8 d = RAM::readAt(0x147);
        // std::cout << std::hex << unsigned(d) << std::endl;
        // exit(0);
    }


    void dump_oam() {
        u16 addr = 0xFE00;
        u8 byte;
        for (int i = 0; i < 0xF1; i++) {
            byte = readAt(byte + i);
            std::cout << std::hex << unsigned(addr + i) << ": " << std::hex << unsigned(byte) << std::endl;
        }
    }

    //    void dump_mem(int in, int en){
    //        u16 addr = in;
    //        u8 byte;
    //        while(in <= en){
    //            byte = readAt(byte + i);
    //            std::cout << std::hex << unsigned(addr + i) << ": " << std::hex << unsigned(byte) << std::endl;
    //        }
    //    }

    void d_vram() {
        // print ram from 8000 to (idk what)
        std::cout << "BGP :" << std::bitset<8>(readAt(BGP)) << std::endl;
        std::cout << "LCDC :" << std::bitset<8>(readAt(LCDC)) << std::endl;
        // BG map
        std::cout << "BG 0x9882 :" << std::hex << unsigned(readAt(0x9882)) << std::endl;

        u16 addr = 0x8B00;
        for (int i = 0; i < 0x0CFF; i += 2) {
            std::cout << std::hex << unsigned(addr + i) << ": " << std::hex << unsigned(readAt(addr + i)) << unsigned(readAt(addr + i + 1)) << std::endl;
        }
        std::cout << std::endl;
    }
}

namespace LCD
{
    void dump_vram();
    void dump_vram() {
        //    u16 addr = 0x8010;
        u16 addr = 0x9000;
        // addr = 0x8010;
        int colour = 0; // colour of the pixel = 0,1,2 probably easiest to write it as a 2 bit binary number
        int tile_num = 0; // change this to character code later
        int x = 8, y = 8;

        s8 point = 0;
        u8 line1, line2;

        // Now we shuffle through each line and find each pixel along the way, let's just do the first tiel to begin with
        // the tile is 8x8, so we have to read 16 bytes (2 for each line) and there are 8 pixels in each horizontal line

        for (tile_num = 0; tile_num < 40; tile_num++) {
            for (int j = 0; j < 8; j++) {
                y = 7 - j + 8 * (tile_num / 20);
                line1 = RAM::readAt(addr);
                line2 = RAM::readAt(addr + 1);
                point += 2;
                addr = 0x9000 + point;

                // now we have to loop through each bit in the lines, extracting the colours and the x and y position
                for (int k = 0; k < 8; k++) {
                    colour = 2 * ((line1 >> k) & 0b1) + ((line2 >> k) & 0b1); // can make this a lot shorter but this way preserve readability (for me)

                    x = 7 - k + 8 * (tile_num % 20);
                    //std::cout << "Colour " <<  colour << std::endl;
                    EMU::setPix(x, y, colour);
                }
            }
        }
    }

    void update() {
        // -- Display -- //
        u8 IF = RAM::readAt(0xFFFF);

        // we need to check that the LCD is turned on
        if ((RAM::readAt(LCDC) >> 7) == 1) {
            LCD::scanline_count -= CPU::cycles;
        }

        if (LCD::scanline_count <= 0) {
            LCD::scanline_count = 456;

            // Increment LY
            u8 l = RAM::readAt(LY);
            RAM::write(l + 1, LY);
            u8 line = RAM::readAt(LY);

            if (line == 144) {
                if (flag == 1) {
                    // RAM::d_vram();    
                }
                RAM::write(RAM::readAt(0xFF0F) | 0x01, 0xFF0F); // request interrupt

                u8 thing = RAM::readAt(0xFF40);

                u8 LCDC_ = RAM::readAt(LCDC);
                //std::cout << "LCDC:" << std::bitset<8>(LCDC) << std::endl << std::endl;
                if (LCDC_ >> 7 == 1) {
                    if ((LCDC_ & 1) == 1) {
                        draw_BG();
                        if (((LCDC_ & 0b00100000) >> 5) == 1) {
                            draw_Window();
                        }
                    }

                    // Sprites
                    if ((LCDC_ & 2) == 2) {
                        draw_sprites();
                        std::cout << "we got here\n";
                    }

                }

                EMU::drawFrame();
                EMU::delay(5);

            }
            else if (line > 153) {
                // here we reset LY and reset the flag associated with v-blank
                RAM::write(0, 0xFF44);
                u8 IF = RAM::readAt(0xFF0F); // interrupt flag
                IF &= 0b11111110;
                RAM::write(IF, 0xFF0F);
            }
        }

        // LCDC Status interrupt
        if (CPU::IME == 1 && ((IF & 0x02) >> 1) == 1) {
            // if LY = LYC
            if (RAM::readAt(0xFF44) == RAM::readAt(0xFF45)) {

                // request interrupt
                RAM::write(RAM::readAt(0xFF0F) | 0b00000010, 0xFF0F);
                //CPU::STAT();
                //std::cout << "LY = LYC interrupt" << std::endl;
            }
        }
    }

    void v_blank() {
        std::cout << "V-Blank Time!\n\n";

        u8 STAT = RAM::readAt(0xFF41);
        STAT |= 0b00000001;
        STAT &= 0b11111101;
        RAM::write(STAT, 0xFF41);

        RAM::write(CPU::PC & 0x0F, CPU::SP);
        CPU::SP--;
        RAM::write((CPU::PC & 0xF0) >> 8, CPU::SP);
        CPU::PC = 0x0040;
        flag = 0;

        // now get rid of request flag IF
        RUPS::IF = RAM::readAt(0xFF0F);
        RUPS::IF &= 0b11111110;
        RAM::write(RUPS::IF, 0xFF0F);

        std::cout << "PC:" << std::hex << unsigned(CPU::PC) << std::endl;
        std::cout << "OPCODOE:" << std::hex << unsigned(RAM::rom[0x40]) << std::endl << std::endl;
    }


    void draw_BG() {
        // figure out where the tileset is located for the background
        u8 LCDC_ = RAM::readAt(LCDC);
        u16 addr, tile_addr;

        // TODO: investigate why this only works when ==1 (I thought it should be the other way around)
        // I think it's possible that the BG tilemap always takes the addresses that the window doesn't use
        // because i think that the 

        // this figures where the BG tilemap is 
        if (((LCDC_ & 0b00001000) >> 3) == 0) {
            addr = 0x9800;
        }
        else {
            addr = 0x9C00;
        }

        //addr = 0x9800;

        if (((LCDC_ & 0b00010000) >> 4) == 0) {
            tile_addr = 0x9000; // tiles are indexed by a signed 8 bit int
        }
        else {
            tile_addr = 0x8000; // tiles indexed by an unsigned 8-bit integer
        }

        u8 charcode;
        u8 scrollX, scrollY;

        scrollY = RAM::readAt(0xFF42);
        scrollX = RAM::readAt(0xFF43);

        u8 palette = RAM::readAt(0xFF47); //BGP

        u16 x;
        for (int j = 0; j < 32 * 32; j++) {
            charcode = RAM::readAt(addr);
            x = tile_addr;

            // TODO: handle signed integers
            if (x == 0x9000) {
                s8 charc = charcode;
                tile_addr = ((0x0900 + charc) << 4);
            }
            else {
                tile_addr += (charcode << 4);
            }

            draw_BGTile(j, scrollX, scrollY, tile_addr, palette);

            tile_addr = x;
            addr++;
        }
    }

    // have to check the LCDC to figure out hwo to handle the address of the charcodes
    // TODO: extend this to be a general tile drawing function
    void draw_BGTile(int tile_num, u8 scrollX, u8 scrollY, u16 addr, u8 palette) {
        u8 line1, line2;
        int x, y, colour;


        for (int j = 0; j < 8; j++) {
            y = (j + 8 * (tile_num / 32) - scrollY) % 256;
            line1 = RAM::readAt(addr);
            line2 = RAM::readAt(addr + 1);
            addr += 2;

            // now we have to loop through each bit in the lines, extracting the colours
            for (int k = 0; k < 8; k++) {

                //TODO: colour is also determined by the colour palette (same goes for OBJs) at 0xFF47
                colour = ((line1 >> k) & 1) + 2 * ((line2 >> k) & 1); // can make this a lot shorter but this way preserve readability (for me)
                if (colour == 0) {
                    colour = palette & 0b00000011;
                }
                else if (colour == 1) {
                    colour = (palette & 0b00001100) >> 2;
                }
                else if (colour == 2) {
                    colour = (palette & 0b00110000) >> 4;
                }
                else if (colour == 3) {
                    colour = (palette & 0b11000000) >> 6;
                }
                x = (7 - k + 8 * (tile_num % 32) - scrollX) % 256;

                if (x >= 0 && x < 160 && y >= 0 && y < 144) {
                    EMU::setPix(x, y, colour);
                }
            }
        }
    }

    void draw_Window() {

        // figure out where the tileset is located for the OBJ
        u8  LCDC_ = RAM::readAt(0xFF40);
        u16 addr, tile_addr;
        // address of the tilemap for objects

        // we are drawing the window here, so this gives us the WIndow tilemap
        if (((LCDC_ & 0b01000000) >> 6) == 0) {
            addr = 0x9800;
        }
        else {
            addr = 0x9C00;
        }

        if (((LCDC_ & 0b00001000) >> 3) == 1) {
            tile_addr = 0x9000; // tiles are indexed by a signed 8 bit int
        }
        else {
            tile_addr = 0x8000; // tiles indexed by an unsigned 8-bit integer
        }

        u8 charcode;
        u8 WindowX, WindowY;

        WindowY = RAM::readAt(0xFF4A);
        WindowX = RAM::readAt(0xFF4B) - 7;

        u8 palette = RAM::readAt(0xFF48);

        u16 x;
        for (int j = 0; j < 32 * 32; j++) {
            charcode = RAM::readAt(addr);
            x = tile_addr;

            if (x == 0x9000) {
                s8 s_char = charcode;
                tile_addr = ((0x0900 + s_char) << 4);
            }
            else {
                tile_addr += (charcode << 4);
            }

            draw_BGTile(j, WindowX, WindowY, tile_addr, palette);

            tile_addr = x;
            addr++;
        }
    }

    // we can probably merge this and draw_BGTile into the same function with another parameter in the arguments

    void render_sprite(u8 tile_x, u8 tile_y, u8 charcode, u8 palette) {
        u8 scrollY = RAM::readAt(0xFF42);
        u8 scrollX = RAM::readAt(0xFF43);
        u8 line1, line2;
        u16 addr = 0x8000 + (charcode << 4);
        int x = tile_x, y = tile_y, colour = 3;

        for (int i = 0; i < 8; i++) {
            y = (tile_y - 15 + i - scrollY) % 256;
            line1 = RAM::readAt(addr);
            line2 = RAM::readAt(addr + 2);

            addr += 2;

            for (int j = 0; j < 8; j++) {
                colour = ((line1 >> j) & 1) + 2 * ((line2 >> j) & 1);
                x = (-1 - j + tile_x - scrollX) % 256;

                if (x >= 0 && x < 160 && y >= 0 && y < 144) {
                    if (colour == 0) {
                        colour = 4;
                    }
                    else if (colour == 1) {
                        colour = (palette & 0b00001100) >> 2;
                    }
                    else if (colour == 2) {
                        colour = (palette & 0b00110000) >> 4;
                    }
                    else if (colour == 3) {
                        colour = (palette & 0b11000000) >> 6;
                    }
                    EMU::setPix(x, y, colour);
                }
            }
        }
    }

    void draw_sprites() {
        int fff = 0;

        // The first address in OAM
        u16 addr = 0xFE00;
        u8 palette;

        // x and y define the bottom right corner of the sprite (i think)
        for (int i = 0; i < 40; i++) {
            u8 y = RAM::readAt(addr);
            u8 x = RAM::readAt(addr + 1);
            u8 charcode = RAM::readAt(addr + 2);
            std::cout << "addr:" << std::hex << unsigned(addr) << std::endl;
            std::cout << "OBJcode: " << std::hex << unsigned(charcode) << std::endl;
            //exit(0);
            u8 attrib = RAM::readAt(addr + 3);

            if ((attrib & 0b00010000) == 0) {
                palette = RAM::readAt(0xFF48);
            }
            else {
                palette = RAM::readAt(0xFF49);
            }

            std::cout << "palette:" << std::bitset<8>(palette) << std::endl;

            // Sprites are all stored in v-ram from 0x8000-0x8BFF
            // TODO: add in the other parameters for drawing characters, such as the palette and the orientation
            std::cout << "x: " << std::hex << unsigned(x) << " y: " << std::hex << unsigned(y) << std::endl;
            if (y > 0 && y < 160 && x > 0 && x < 168) {

                std::cout << "x: " << x << " y: " << y << std::endl;
                // exit(0);
                render_sprite(x, y, charcode, palette);
            }
            addr += 4;
        }
    }
}

namespace TIMER {
    int counter = 0;

    /**
    * The gameboy has a timer with selectable frequency of 4096, 16384 65536, 262144 Hertz
    * when it overflows, it generates an interrupt it is then loaded with the contents of
    * timer modulo (TMA)
    */
    u8 TIMA; // 0xFF05 frequency specified by 0xFF07 
    u8 TMA; // 0xFF06 when TIMA overflows this data is loaded

    // Timer control:
    // Bit 2 = 0 : stop timer, = 1 : start timer
    // Bits 1+0 - Input clock select, = 00 4.096KHz, = 01 : 262.144KHz, = 10 65.536KHz, =11 : 16.384KHz 
    // u8 TAC; 
    // 1024 cycles, 16, 64, 256

    void inc(int amount) {
        u8 t = RAM::readAt(0xFF05);
        TIMA = t + amount;
        if (TIMA <= t) {
            RAM::write(RAM::readAt(0xFF0F) | 0b00000100, 0xFF0F);
        }
    }

    void update() {
        if (CPU::IME != 0) {
            counter += CPU::cycles;
            u8 rate_reg = RAM::readAt(0xFF07);
            int rate = 0;
            if ((rate & 0b00000011) == 0) {
                rate = 1024;
            }
            else if ((rate & 0b00000011) == 1) {
                rate = 16;
            }
            else if ((rate & 0b00000011) == 2) {
                rate = 64;
            }
            else if ((rate & 0b00000011) == 3) {
                rate = 256;
            }
            int amount = counter % rate;
            if (amount != 0) {
                counter = 0;
                TIMER::inc(amount);
            }
        }
    }

    void overflow() {
        RAM::write(CPU::PC & 0x0F, CPU::SP);
        CPU::SP--;
        RAM::write((CPU::PC & 0xF0) >> 8, CPU::SP);
        CPU::PC = 0x0050;

        RUPS::IF = RAM::readAt(0xFF0F);
        RUPS::IF &= 0b11111011;
        RAM::write(RUPS::IF, 0xFF0F);
    }
}

namespace RUPS {
    bool handle_interrupts() {
        // check that IME is set to 1
        if (CPU::IME == 0) {
            return false;
        }

        IF = RAM::readAt(0xFF0F);
        IE = RAM::readAt(0xFFFF);

        if ((IF & 1) == 1 && (IE & 1) == 1) {
            LCD::v_blank();
            CPU::halt = false;
        }
        return true;
    }
}

void game_loop() {
    std::cout << std::endl;

    CPU::init_opcodes();
    CPU::init_decoder();
    RAM::init();
    EMU::init();
    LCD::draw_BG();
    EMU::drawFrame();
    EMU::delay(1000);

    u16 debug = 0;

    u8 opcode = 0;

    int inp_time = 0;

    while (!EMU::quit) {
        inp_time++;

        if (inp_time == 100000) {
            inp_time = 0;
            for (int i = 0; i < 50; i++) {
                EMU::inputs();
            }
        }

        if (CPU::PC == 0x00FE) {
            std::cout << "--BOOT COMPLETE--\n\n";

            LCD::draw_BG();

            EMU::drawFrame();
        }

        if (!CPU::halt) {
            debug = CPU::PC;
            opcode = RAM::read();
            CPU::runOPCode(opcode);
        }

        LCD::update();

        TIMER::update();

        RUPS::handle_interrupts();
    }
}