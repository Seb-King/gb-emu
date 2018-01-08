#include <iostream>
#include <vector>
#include "cpu.hpp"

namespace CPU
{
    void init_op_codes(){
    }

    void runOPCode(u8 OP_code)
    {

    switch ( OP_code){
    	case 0x06: LDb_n(); break; // load n into c
        case 0x0E: LDc_n(); break;              // how do we even handle what n is? Where should it be stored?
        case 0x16: LDd_n(); break;
        case 0x1E: LDe_n(); break;
     	case 0x26: LDh_n(); break;
     	case 0x2E: LDl_n(); break;
        // Load  LD r1r2, load r2 into r1
     	case 0x7F: break; // loads A into A which is pointless and we should get away with this
          // although it does take 4 cpu cycles anyway, so maybe things break without it
      	case 0x78: LDrr_ab(); break;
      	case 0x79: LDrr_ac(); break;
      	case 0x7A: LDrr_ad(); break;
      	case 0x7B: LDrr_ae(); break;
      	case 0x7C: LDrr_ah(); break;
      	case 0x7D: LDrr_al(); break;
      case 0x7E: LDrr_aHL(); break; // unimplemented
      case 0x0A: LDrr_aBC(); break;
      case 0x1A: LDrr_aDE(); break;
      //case 0xFA: LDrr_ann(); break;
      case 0x3E: LDrr_a_hash(); break; // what the fuck is this op shiposed to be??????
      case 0x40: break; // B into B
      case 0x41: LDrr_bc(); break;
      case 0x42: LDrr_bd(); break; 
      case 0x43: LDrr_be(); break;
      case 0x44: LDrr_bh(); break;
      case 0x45: LDrr_bl(); break;
      case 0x46: LDrr_bHL(); break; // now i'm wondering where is the A into B OP Code
      case 0x47: LDrr_ba(); break;
      case 0x48: LDrr_cb(); break;
      case 0x49: break;
      case 0x4A: LDrr_cd(); break;
      case 0x4B: LDrr_ce(); break;
      case 0x4C: LDrr_ch(); break;
      case 0x4D: LDrr_cl(); break;
      case 0x4E: LDrr_cHL(); break;
      case 0x4F: LDrr_ca(); break;
      case 0x50: LDrr_db(); break;
      case 0x51: LDrr_dc(); break;
      case 0x52: break;
      case 0x53: LDrr_de(); break;
      case 0x54: LDrr_dh(); break;
      case 0x55: LDrr_dl(); break;
      case 0x56: LDrr_dHL(); break;
      case 0x57: LDrr_da(); break;
      case 0x58: LDrr_eb(); break;
      case 0x59: LDrr_ec(); break;
      case 0x5A: LDrr_ed(); break;
      case 0x5B: break;
      case 0x5C: LDrr_eh(); break;
      case 0x5D: LDrr_el(); break;
      case 0x5E: LDrr_eHL(); break;
      case 0x5F: LDrr_ea(); break;
      case 0x60: LDrr_hb(); break;
      case 0x61: LDrr_hc(); break;
      case 0x62: LDrr_hd(); break;
      case 0x63: LDrr_he(); break;
      case 0x64: break;
      case 0x65: LDrr_hl(); break;
      case 0x66: LDrr_hHL(); break;
      case 0x67: LDrr_ha(); break;
      case 0x68: LDrr_lb(); break;
      case 0x69: LDrr_lc(); break;
      case 0x6A: LDrr_ld(); break;
      case 0x6B: LDrr_le(); break;
      case 0x6C: LDrr_lh(); break;
      case 0x6D: break;
      case 0x6E: LDrr_lHL(); break;
      case 0x6F: LDrr_la(); break;
      //case 0x70: LDrr_HLb(); break;
      //case 0x71: LDrr_HLc(); break;
      //case 0x72: LDrr_HLd(); break;
      //case 0x73: LDrr_HLe(); break;
      //case 0x74: LDrr_HLh(); break;
      //case 0x75: LDrr_HLn(); break; //TODO: wtf is this n shit

      case 0xF2: LDa_c(); break; // put value at address 0xFF00 + reg C into A
      case 0xE2: LDc_a(); break; // put A into address (0xFF00 + reg C) what is this even doing
      case 0x3A: LDDaHL(); break; // put value at address HL into A then decrement HL
      case 0x32: LDDHLa(); break; // Put A into memory address HL then decrement HL
      //case 0x2A: LDI_aHL(); break; // Put value at address HL into A then incrememnt HL
      case 0x22: LDI_HLa(); break; // put A into mem address HL. Incrememnt HL
      // case 0xE0: LDH_nA(); break; // Put A into memory address 0xFF00 + n this is already implemented
      case 0xF0: LDH_a_ffn(); break; // Put memory address 0xFF00 + n into A


      // --------- 16-Bit Loads --------- //

      // LDnn,n put value nn into n
      case 0x01: LD_nn_BC(); break;
      case 0x11: LD_nn_DE(); break;
      case 0x21: LD_nn_HL(); break;
      case 0x31: LD_nn_SP(); break;

      // LD SP,HL put HL into SP
      case 0xF9: LD_SPHL(); break;
      case 0xF8: LDHL_SPn(); break; // Put SP+n effective address into HL, n is signed, flags are affected
      case 0x08: LD_nnSP(); break; //nn two byte immediate address, Put SP at address n (nn?)

        // Push register pair nn onto stack decrement SP twice
      case 0xF5: PUSH_AF(); break;
      case 0xC5: PUSH_BC(); break;
      case 0xD5: PUSH_DE(); break;
      case 0xE5: PUSH_HL(); break;

        // Pop nn, pop two bytes off stack into register pair nn, increment Stack pointer twice
      case 0xF1: POP_AF(); break;
      case 0xC1: POP_BC(); break;
      case 0xD1: POP_DE(); break;
      case 0xE1: POP_HL(); break;

        // 8-Bit ALU
        // ADD A,n -- add n to A

      case 0x87: ADD_aa(); break;
      case 0x80: ADD_ab(); break;
      case 0x81: ADD_ac(); break;
      case 0x82: ADD_ad(); break;
      case 0x83: ADD_ae(); break;
      case 0x84: ADD_ah(); break;
      case 0x85: ADD_al(); break;
      case 0x86: ADD_aH(); break;
      //case 0xC6: ADD_a_hash(); break;

        // ADC A,n -- add n + carry flag to A
      case 0x8f: ADC_aa(); break;
      case 0x88: ADC_ab(); break;
      case 0x89: ADC_ac(); break;
      case 0x8A: ADC_ad(); break;
      case 0x8B: ADC_ae(); break;
      case 0x8C: ADC_ah(); break;
      case 0x8D: ADC_al(); break;
      case 0x8E: ADC_aHL(); break;
      //case 0xCE: ADC_a_hash(); break;

        // SUB n, subtract n from A
      case 0x97: SUB_a(); break;
      case 0x90: SUB_b(); break;
      case 0x91: SUB_c(); break;
      case 0x92: SUB_d(); break;
      case 0x93: SUB_e(); break;
      case 0x94: SUB_h(); break;
      case 0x95: SUB_l(); break;
      case 0x96: SUB_HL(); break;
      //case 0xD6: SUB_hash(); break;

      case 0x9F: SBC_a(); break;
      case 0x98: SBC_b(); break;
      case 0x99: SBC_c(); break;
      case 0x9A: SBC_d(); break;
      case 0x9B: SBC_e(); break;
      case 0x9C: SBC_h(); break;
      case 0x9D: SBC_l(); break;
      case 0x9E: SBC_HL(); break;

      // ------ Logical Operations ------ //
      // Check flags in each operation

      // AND n, logically AND n with A, result in A
      case 0xA7: AND_a(); break;
      case 0xA0: AND_b(); break;
      case 0xA1: AND_c(); break;
      case 0xA2: AND_d(); break;
      case 0xA3: AND_e(); break;
      case 0xA4: AND_h(); break;
      case 0xA5: AND_l(); break;
      case 0xA6: AND_HL(); break;

      // Logical OR with register A, result in A
      case 0xB7: OR_a(); break;
      case 0xB0: OR_b(); break;
      case 0xB1: OR_c(); break;
      case 0xB2: OR_d(); break;
      case 0xB3: OR_e(); break;
      case 0xB4: OR_h(); break;
      case 0xB5: OR_l(); break;
      case 0xB6: OR_HL(); break;
      //case 0xF7: OR_hash(); break;

      // Logical exclusive OR n with register A, result in A

      case 0xAF: XOR_a(); break;
      case 0xA8: XOR_b(); break;
      case 0xA9: XOR_c(); break;
      case 0xAA: XOR_d(); break;
      case 0xAB: XOR_e(); break;
      case 0xAC: XOR_h(); break;
      case 0xAD: XOR_l(); break;
      case 0xAE: XOR_HL(); break;
      //case 0xEE: XOR_star(); break;

      // Copmare A with n. Basically an A - n subtraction instruct but no results
      // I think the point is to set flags here 
      case 0xBF: CP_a(); break;
      case 0xB8: CP_b(); break;
      case 0xB9: CP_c(); break;
      case 0xBA: CP_d(); break;
      case 0xBB: CP_e(); break;
      case 0xBC: CP_h(); break;
      case 0xBD: CP_l(); break;
      case 0xBE: CP_HL(); break;
      case 0xFE: CP_hash(); break;

      // increment, pretty self explanatory
      case 0x3C: INC_a(); break;
      case 0x04: INC_b(); break;
      case 0x0C: INC_c(); break;
      case 0x14: INC_d(); break;
      case 0x1C: INC_e(); break;
      case 0x24: INC_h(); break;
      case 0x2C: INC_l(); break;
      case 0x34: INC_HLad(); break;

      case 0x3D: DEC_a(); break;
      case 0x05: DEC_b(); break;
      case 0x0D: DEC_c(); break;
      case 0x15: DEC_d(); break;
      case 0x1D: DEC_e(); break;
      case 0x25: DEC_h(); break;
      case 0x2D: DEC_l(); break;
      case 0x35: DEC_HL(); break;

      // -------------------------------//
      // In this section i'm just going to implement opcodes until I can get the boot process of the gb going
      case 0xFF: RST_FF(); break;
      case 0xCB: CB(); break;
      case 0x20: JR_NZ(); break;
      case 0x28: JR_Z(); break;
      case 0x18: JR_n(); break;
      case 0x77: LD_HL_a(); break;
      case 0x02: LD_BC_a(); break;
      case 0x12: LD_DE_a(); break;
      case 0xE0: LDad_n_a(); break;
      case 0xCD: CALL_nn(); break;

      case 0x03: INC_BC(); break;
      case 0x13: INC_DE(); break;
      case 0x23: INC_HL(); break;
      case 0x33: INC_SP(); break;
      case 0xEA: LD_nn_a(); break; // put value A into address nn
      case 0xC9: RET(); break;
      case 0xC0: RET_NZ(); break;
      case 0xC8: RET_Z(); break;
      case 0xD0: RET_NC(); break;
      case 0xD8: RET_C(); break;
      case 0xD9: RETI(); break;
      case 0x17: RLA(); break;

      // -- 16-bit Arithmetic -- ??

      default: 
      std::cout << "opcode not implemented: ";
      std::cout << std::hex << unsigned( OP_code ) << std::endl;
      std::cout << "PC:" << std::hex << unsigned( PC ) << std::endl << std::endl;
      exit(0);
    }
  }


  void RET()
  {
    u8 n1 = RAM::readAt(SP);
    ++SP;
    u16 n2 = RAM::readAt(SP);
    ++SP;
    u16 addr = n1 + (n2 << 8);
    PC = addr;
      //std::cout << "n1: " << std::hex << unsigned(n1) << std::endl;
      //std::cout << "n2: " << std::hex <<  unsigned(n2) << std::endl;
      //std::cout << "dafsd: " << std::hex << unsigned(RAM::readAt(SP - 3)) << std::endl;

      //exit(0);
  }
  void RET_NZ()
  {
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

  void RET_Z()
  {
    if ((AF.lo >> 7) == 1)
    {
      u8 n1 = RAM::readAt(SP);
      ++SP;
      u16 n2 = RAM::readAt(SP);
      ++SP;
      u16 addr = n1 + (n2 << 8);
      PC = addr;      
    }
  }
  void RET_NC()
  {
    if ((AF.lo & 0b00000001) == 0)
    {
      u8 n1 = RAM::readAt(SP);
      ++SP;
      u16 n2 = RAM::readAt(SP);
      ++SP;
      u16 addr = n1 + (n2 << 8);
      PC = addr;  
    }
  }
  void RET_C()
  {
    if ((AF.lo & 0b00000001) == 1)
    {
      u8 n1 = RAM::readAt(SP);
      ++SP;
      u16 n2 = RAM::readAt(SP);
      ++SP;
      u16 addr = n1 + (n2 << 8);
      PC = addr;  
    }
  }

  void RETI()
  {
    u8 n1 = RAM::readAt(SP);
    ++SP;
    u16 n2 = RAM::readAt(SP);
    ++SP;
    u16 addr = n1 + (n2 << 8);
    PC = addr;
    RAM::write(0xFF, 0xFFFF);
  }

  void CB() 
  {
    u8 opcode = RAM::read(); 
    switch (opcode)
    {
      case 0x7c: BIT_7H(); break;
      case 0x17: RL_A(); break;
      case 0x10: RL_B(); break;
      case 0x11: RL_C(); break;
      case 0x12: RL_D(); break;
      case 0x13: RL_E(); break;
      case 0x14: RL_H(); break;
      case 0x15: RL_L(); break;
      case 0x16: RL_addr_HL(); break;
      default: std::cout << "CB opcode not implemented: " << std::hex << unsigned( opcode) << std::endl;
    }
  }

    //set z if bit 7 of reg H is zero 
  void BIT_7H() {
    u8 bit = (HL.hi >> 7);
      AF.lo &= 0b00010000; // TODO: figure out how AF.lo was being set in the first place
      
      
      if (bit == 0){AF.lo |= 0b10000000;} 
      AF.lo &= 0b10110000; 
      AF.lo |= 0b00100000; 
    }

    void RLA()
    {
      u8 carry = (AF.lo & 0b00010000) >> 4;
      AF.lo = (AF.hi & 0b10000000) >> 3;
      AF.lo &= 0b10010000;
      AF.hi = (AF.hi << 1) + carry;
      if (AF.hi == 0) {AF.lo |= 0b10000000;} else { AF.lo &= 0b01110000;}
    }
    void RL_A(){
      u8 carry = (AF.lo & 0b00010000) >> 4;
      AF.lo = (AF.hi & 0b10000000) >> 3;
      AF.lo &= 0b10010000;
      AF.hi = (AF.hi << 1) + carry;
      if (AF.hi == 0) {AF.lo |= 0b10000000;} else { AF.lo &= 0b01110000;}
    }
    void RL_B(){
      u8 carry = (AF.lo & 0b00010000) >> 4;
      AF.lo = (BC.hi & 0b10000000) >> 3;
      AF.lo &= 0b10010000;
      BC.hi = (BC.hi << 1) + carry;
      if (DE.hi == 0) {AF.lo |= 0b10000000;} else { AF.lo &= 0b01110000;}
    }
    void RL_C(){
      u8 carry = (AF.lo & 0b00010000) >> 4;
      AF.lo = (BC.lo & 0b10000000) >> 3;
      AF.lo &= 0b10010000;
      BC.lo = (BC.lo << 1) + carry;
      if (BC.lo == 0) {AF.lo |= 0b10000000;} else { AF.lo &= 0b01110000;}
    }

    void RL_D(){
      u8 carry = (AF.lo & 0b00010000) >> 4;
      AF.lo = (DE.hi & 0b10000000) >> 3;
      AF.lo &= 0b10010000;
      DE.hi = (DE.hi << 1) + carry;
      if (DE.hi == 0) {AF.lo |= 0b10000000;} else { AF.lo &= 0b01110000;}
    }
    void RL_E(){
      u8 carry = (AF.lo & 0b00010000) >> 4;
      AF.lo = (DE.lo & 0b10000000) >> 3;
      AF.lo &= 0b10010000;
      DE.lo = (DE.lo << 1) + carry;
      if (DE.lo == 0) {AF.lo |= 0b10000000;} else { AF.lo &= 0b01110000;}
    }
    void RL_H(){
      u8 carry = (AF.lo & 0b00010000) >> 4;
      AF.lo = (HL.hi & 0b10000000) >> 3;
      AF.lo &= 0b10010000;
      HL.hi = (HL.hi << 1) + carry;
      if (HL.hi == 0) {AF.lo |= 0b10000000;} else { AF.lo &= 0b01110000;}
    }
    void RL_L(){
      u8 carry = (AF.lo & 0b00010000) >> 4;
      AF.lo = (HL.lo & 0b10000000) >> 3;
      AF.lo &= 0b10010000;
      HL.lo = (HL.lo << 1) + carry;
      if (HL.lo == 0) {AF.lo |= 0b10000000;} else { AF.lo &= 0b01110000;}
    }
    void RL_addr_HL(){
      u8 x = RAM::readAt(HL.val());
      u8 carry = (AF.lo & 0b00010000) >> 4;
      AF.lo = (x & 0b10000000) >> 3;
      AF.lo &= 0b10010000;
      x = (x << 1 ) + carry;
      RAM::write(x, HL.val());
      if (x == 0) {AF.lo |= 0b10000000;} else { AF.lo &= 0b01110000;}
    }

    // if NZ (if Z is zero) then add n to current address and jump to that address
    // we have to treat the next byte as a signed variable
    void JR_NZ()
    {  
      if ((AF.lo >> 7) == 0)
      {
        s8 b = RAM::read();
        PC += b;
      } else {++PC;}
    }

    void JR_Z()
    {  
      if ((AF.lo >> 7 ) == 1)
      {
        s8 b = RAM::read();
        PC += b; 
      } else {++PC;}
    }

    void JR_n()
    {
      s8 jump = RAM::read();
      PC += jump;
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

    void LDH_nA()
    {
      u8 n = RAM::read();
      RAM::write(AF.hi, 0xFF00 +  n);
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
    void LD_BC_a(){ RAM::write(AF.hi, BC.val()); }
    void LD_DE_a(){ RAM::write(AF.hi, DE.val()); }

    void LDad_n_a() { u8 add = RAM::read(); u16 addr = add + 0xFF00; RAM::write(AF.hi, addr); }

    void CALL_nn() 
    {

      u8 n1 = RAM::read();
      u8 n2 = RAM:: read();

      u8 loNib = PC & 0xFF;
      u8 hiNib = (PC >> 8) & 0xFF;
      //std::cout << "PC :" << std::hex << unsigned(PC) << std::endl;
      //std::cout << "hiNib: " << std::hex << unsigned(loNib) << std::endl;
      //exit(0);

      --SP;
      RAM::write(hiNib, SP);
      --SP;
      RAM::write(loNib, SP);
      
      PC = n1 + (n2 << 8);
    }

    void INC_BC() { u16 x = BC.val()+1; BC.set(x); }
    void INC_DE() { u16 x = DE.val()+1; DE.set(x); }
    void INC_HL() { u16 x = HL.val()+1; HL.set(x); }
    void INC_SP() { ++SP; }

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
    }















    // let's start defining some opcodes now

    void LDrr_ab(){ AF.hi = BC.hi; }
    void LDrr_ac(){ AF.hi = BC.lo; }
    void LDrr_ad(){ AF.hi = DE.hi; }
    void LDrr_ae(){ AF.hi = DE.lo; }
    void LDrr_ah(){ AF.hi = HL.hi; }
    void LDrr_al(){ AF.hi = HL.lo; }
    void LDrr_aBC(){ AF.hi = RAM::readAt(BC.val()); }
    void LDrr_aDE(){ AF.hi = RAM::readAt(DE.val()); }
    void LDrr_aHL(){ AF.hi = RAM::readAt(HL.val()); }
    void LDrr_ba(){ BC.hi = AF.hi; }
    void LDrr_bc(){ BC.hi = BC.lo; }
    void LDrr_bd(){ BC.hi = DE.hi; }
    void LDrr_be(){ BC.hi = DE.lo; }
    void LDrr_bh(){ BC.hi = HL.hi; }
    void LDrr_bl(){ BC.hi = HL.lo; }
    void LDrr_bHL(){ BC.hi = RAM::readAt(HL.val()); }
    void LDrr_ca(){ BC.lo = AF.hi; }
    void LDrr_cb(){ BC.lo = BC.hi; }
    void LDrr_cd(){ BC.lo = DE.hi; }
    void LDrr_ce(){ BC.lo = DE.lo; }
    void LDrr_ch(){ BC.lo = HL.hi; }
    void LDrr_cl(){ BC.lo = HL.lo; }
    void LDrr_cHL(){ BC.lo = RAM::readAt(HL.val()); }
    void LDrr_da(){ DE.hi = AF.hi; }
    void LDrr_db(){ DE.hi = BC.hi; }
    void LDrr_dc(){ DE.hi = BC.lo; }
    void LDrr_de(){ DE.hi = DE.lo; }
    void LDrr_dh(){ DE.hi = HL.hi; }
    void LDrr_dl(){ DE.hi = HL.lo; }
    void LDrr_dHL(){ DE.hi = RAM::readAt(HL.val()); }
    void LDrr_ea(){ DE.lo = AF.hi; }
    void LDrr_eb(){ DE.lo = BC.hi; }
    void LDrr_ec(){ DE.lo = BC.lo; }
    void LDrr_ed(){ DE.lo = DE.hi; }
    void LDrr_eh(){ DE.lo = HL.hi; }
    void LDrr_el(){ DE.lo = HL.lo; }
    void LDrr_eHL(){ DE.lo = RAM::readAt(HL.val()); }
    void LDrr_ha(){ HL.hi = AF.hi; }
    void LDrr_hb(){ HL.hi = BC.hi; }
    void LDrr_hc(){ HL.hi = BC.lo; }
    void LDrr_hd(){ HL.hi = DE.hi; }
    void LDrr_he(){ HL.hi = DE.lo; }
    void LDrr_hl(){ HL.hi = HL.lo; }
    void LDrr_hHL(){ HL.hi = RAM::readAt(HL.val()); }
    void LDrr_la(){ HL.lo = AF.hi; }
    void LDrr_lb(){ HL.lo = BC.hi; }
    void LDrr_lc(){ HL.lo = BC.lo; }
    void LDrr_ld(){ HL.lo = DE.hi; }
    void LDrr_le(){ HL.lo = DE.lo; }
    void LDrr_lh(){ HL.lo = HL.hi; }
    void LDrr_lHL(){ HL.lo = RAM::readAt( HL.val() ) ; }

    void LDa_c(){ AF.hi = RAM::readAt(0xFF00 + BC.lo);}
    void LDc_a(){ RAM::write(AF.hi, 0xFF00 + BC.lo); }

    void LDDaHL(){ AF.hi = RAM::readAt(HL.val()); HL.set(HL.val() - 1);}
    void LDDHLa(){ RAM::write(AF.hi, HL.val()); HL.set(HL.val() - 1); }
    void LDIaHL(){ AF.hi = RAM::readAt(HL.val()); HL.set(HL.val()+ 1); }
    void LDIHLa(){ RAM::write(AF.hi, HL.val()); HL.set(HL.val() + 1); }

    void LDHnA(){ u8 n = RAM::read(); RAM::write(AF.hi, 0xFF00 + n); } 
    void LDHAn(){ u8 n = RAM::read(); RAM::readAt(0xFF00 + n); }
    //-- 16-Bit shit --//
    // going to need to read 16 bit values little endian style (I think)
    void LD_nn_BC()
    {
      u8 n1 = RAM::read();
      //PC++;
      u8 n2 = RAM::read();
      //PC++;
      u16 nn = n1 + (n2 << 8);
      BC.set(nn);
    }
    void LD_nn_DE(){ u8 n1 = RAM::read(); u8 n2 = RAM::read(); u16 nn = n1 + (n2 << 8); DE.set(nn); }
    void LD_nn_HL(){ u8 n1 = RAM::read(); u8 n2 = RAM::read(); u16 nn = n1 + (n2 << 8); HL.set(nn); }
    void LD_nn_SP(){ u8 n1 = RAM::read(); u8 n2 = RAM::read(); u16 nn = n1 + (n2 << 8); SP = nn; }

    // i'm not sure whether SP = HL.val() or the bytes at SP should be set to HL
    //TODO: fix this shit
    void LD_SPHL(){ SP = HL.val(); }

    // put SP + n effective address into HL
    void LDHL_SPn(){ u8 n = RAM::read(); HL.set(SP + n); AF.lo = AF.lo & 0x03;}
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
    void PUSH_AF(){ SP--; RAM::write(AF.hi, SP); SP--; RAM::write(AF.lo,SP); }
    void PUSH_BC(){ SP--; RAM::write(BC.hi, SP); SP--; RAM::write(BC.lo,SP); }
    void PUSH_DE(){ SP--; RAM::write(DE.hi, SP); SP--; RAM::write(DE.lo,SP); }
    void PUSH_HL(){ SP--; RAM::write(HL.hi, SP); SP--; RAM::write(HL.lo,SP); }

    // Pop two bytes off stack into register pair, increment SP twice
    void POP_AF(){ u8 n = RAM::readAt(SP); AF.lo = n; ++SP; u8 m = RAM::readAt(SP); AF.hi = m; ++SP; }
    void POP_BC(){ u8 n = RAM::readAt(SP); BC.lo = n; ++SP; u8 m = RAM::readAt(SP); BC.hi = m; ++SP; } //TODO: something is going wrong here
    void POP_DE(){ u8 n = RAM::readAt(SP); DE.lo = n; ++SP; u8 m = RAM::readAt(SP); DE.hi = m; ++SP; }
    void POP_HL(){ u8 n = RAM::readAt(SP); HL.lo = n; ++SP; u8 m = RAM::readAt(SP); HL.hi = m; ++SP; }

    // ADD_xy add y to x and set flags
    void ADD_aa()
    { 
      u8 x = AF.hi;
      u8 loNib = AF.hi & 0x0F;
      AF.hi += AF.hi;
      if (AF.hi == 0){ AF.lo = AF.lo | 0x80;} else { AF.lo = 0;} // set Z = 0 if == 0
      AF.lo = AF.lo & 0b10110000; // reset N (N = 0)
      if (AF.hi < x) { AF.lo = AF.lo | 0b00100000;} // Set H if overfloss
      if ( (AF.hi & 0x0F) < loNib){ AF.lo = AF.lo | 0b00010000; } // Set CY if loer nibble overflos

    }
    void ADD_ab()
    {
      u8 x = AF.hi;
      u8 loNib = AF.hi & 0x0F;
      AF.hi += BC.hi;
      if (AF.hi == 0){ AF.lo = AF.lo | 0x80;} else { AF.lo = 0;} // set Z = 0 if == 0
      AF.lo = AF.lo & 0b10110000; // reset N (N = 0)
      if (AF.hi < x) { AF.lo = AF.lo | 0b00100000;} // Set H if overfloss
      if ( (AF.hi & 0x0F) < loNib){ AF.lo = AF.lo | 0b00010000; } // Set CY if loer nibble overflos

    }
    void ADD_ac()
    {
      u8 x = AF.hi;
      u8 loNib = AF.hi & 0x0F;
      AF.hi += BC.lo;
      if (AF.hi == 0){ AF.lo = AF.lo | 0x80;} else { AF.lo = 0;} // set Z = 0 if == 0
      AF.lo = AF.lo & 0b10110000; // reset N (N = 0)
      if (AF.hi < x) { AF.lo = AF.lo | 0b00100000;} // Set H if overfloss
      if ( (AF.hi & 0x0F) < loNib){ AF.lo = AF.lo | 0b00010000; } // Set CY if loer nibble overflos

    }
    void ADD_ad()
    {
      u8 x = AF.hi;
      u8 loNib = AF.hi & 0x0F;
      AF.hi += DE.hi;
      if (AF.hi == 0){ AF.lo = AF.lo | 0x80;} else { AF.lo = 0;} // set Z = 0 if == 0
      AF.lo = AF.lo & 0b10110000; // reset N (N = 0)
      if (AF.hi < x) { AF.lo = AF.lo | 0b00100000;} // Set H if overfloss
      if ( (AF.hi & 0x0F) < loNib){ AF.lo = AF.lo | 0b00010000; } // Set CY if loer nibble overflos

    }
    void ADD_ae()
    {
      u8 x = AF.hi;
      u8 loNib = DE.lo & 0x0F;
      AF.hi += BC.hi;
      if (AF.hi == 0){ AF.lo = AF.lo | 0x80;} else { AF.lo = 0;} // set Z = 0 if == 0
      AF.lo = AF.lo & 0b10110000; // reset N (N = 0)
      if (AF.hi < x) { AF.lo = AF.lo | 0b00100000;} // Set H if overfloss
      if ( (AF.hi & 0x0F) < loNib){ AF.lo = AF.lo | 0b00010000; } // Set CY if loer nibble overflos

    }
    void ADD_ah()
    {
      u8 x = AF.hi;
      u8 loNib = AF.hi & 0x0F;
      AF.hi += HL.hi;
      if (AF.hi == 0){ AF.lo = AF.lo | 0x80;} else { AF.lo = 0;} // set Z = 0 if == 0
      AF.lo = AF.lo & 0b10110000; // reset N (N = 0)
      if (AF.hi < x) { AF.lo = AF.lo | 0b00100000;} // Set H if overfloss
      if ( (AF.hi & 0x0F) < loNib){ AF.lo = AF.lo | 0b00010000; } // Set CY if loer nibble overflos

    }
    void ADD_al()
    {
      u8 x = AF.hi;
      u8 loNib = HL.lo & 0x0F;
      AF.hi += BC.hi;
      if (AF.hi == 0){ AF.lo = AF.lo | 0x80;} else { AF.lo = 0;} // set Z = 0 if == 0
      AF.lo = AF.lo & 0b10110000; // reset N (N = 0)
      if (AF.hi < x) { AF.lo = AF.lo | 0b00100000;} // Set H if overfloss
      if ( (AF.hi & 0x0F) < loNib){ AF.lo = AF.lo | 0b00010000; } // Set CY if loer nibble overflos

    }
    void ADD_aH()
    {
      u8 x = AF.hi;
      u8 loNib = AF.hi & 0x0F;
      AF.hi += RAM::readAt(HL.val());
      if (AF.hi == 0){ AF.lo = AF.lo | 0x80;} else { AF.lo = 0;} // set Z = 0 if == 0
      AF.lo = AF.lo & 0b10110000; // reset N (N = 0)
      if (AF.hi < x) { AF.lo = AF.lo | 0b00100000;} // Set H if overfloss
      if ( (AF.hi & 0x0F) < loNib){ AF.lo = AF.lo | 0b00010000; } // Set CY if loer nibble overflos

    }
    //void ADD_a_hash(){ return void;} //TODO: wtf is this

    void ADC_aa()
    { 
      u8 x = AF.hi;
      u8 loNib = AF.hi & 0x0F;
      AF.hi += AF.hi + (AF.lo & 0b00010000);
      if (AF.hi == 0){ AF.lo = AF.lo | 0x80;} else { AF.lo = 0;} // set Z = 0 if == 0
      AF.lo = AF.lo & 0b10110000; // reset N (N = 0)
      if (AF.hi < x) { AF.lo = AF.lo | 0b00100000;} // Set H if overfloss
      if ( (AF.hi & 0x0F) < loNib){ AF.lo = AF.lo | 0b00010000; } // Set CY if loer nibble overflos

    }
    void ADC_ab()
    {
      u8 x = AF.hi;
      u8 loNib = AF.hi & 0x0F;
      AF.hi += BC.hi + (AF.lo & 0b00010000);
      if (AF.hi == 0){ AF.lo = AF.lo | 0x80;} else { AF.lo = 0;} // set Z = 0 if == 0
      AF.lo = AF.lo & 0b10110000; // reset N (N = 0)
      if (AF.hi < x) { AF.lo = AF.lo | 0b00100000;} // Set H if overfloss
      if ( (AF.hi & 0x0F) < loNib){ AF.lo = AF.lo | 0b00010000; } // Set CY if loer nibble overflos

    }
    void ADC_ac()
    {
      u8 x = AF.hi;
      u8 loNib = AF.hi & 0x0F;
      AF.hi += BC.lo + (AF.lo & 0b00010000);
      if (AF.hi == 0){ AF.lo = AF.lo | 0x80;} else { AF.lo = 0;} // set Z = 0 if == 0
      AF.lo = AF.lo & 0b10110000; // reset N (N = 0)
      if (AF.hi < x) { AF.lo = AF.lo | 0b00100000;} // Set H if overfloss
      if ( (AF.hi & 0x0F) < loNib){ AF.lo = AF.lo | 0b00010000; } // Set CY if loer nibble overflos

    }
    void ADC_ad()
    {
      u8 x = AF.hi;
      u8 loNib = AF.hi & 0x0F;
      AF.hi += DE.hi + (AF.lo & 0b00010000);
      if (AF.hi == 0){ AF.lo = AF.lo | 0x80;} else { AF.lo = 0;} // set Z = 0 if == 0
      AF.lo = AF.lo & 0b10110000; // reset N (N = 0)
      if (AF.hi < x) { AF.lo = AF.lo | 0b00100000;} // Set H if overfloss
      if ( (AF.hi & 0x0F) < loNib){ AF.lo = AF.lo | 0b00010000; } // Set CY if loer nibble overflos

    }
    void ADC_ae()
    {
      u8 x = AF.hi;
      u8 loNib = DE.lo & 0x0F;
      AF.hi += BC.hi + (AF.lo & 0b00010000);
      if (AF.hi == 0){ AF.lo = AF.lo | 0x80;} else { AF.lo = 0;} // set Z = 0 if == 0
      AF.lo = AF.lo & 0b10110000; // reset N (N = 0)
      if (AF.hi < x) { AF.lo = AF.lo | 0b00100000;} // Set H if overfloss
      if ( (AF.hi & 0x0F) < loNib){ AF.lo = AF.lo | 0b00010000; } // Set CY if loer nibble overflos

    }
    void ADC_ah()
    {
      u8 x = AF.hi;
      u8 loNib = AF.hi & 0x0F;
      AF.hi += HL.hi + (AF.lo & 0b00010000);
      if (AF.hi == 0){ AF.lo = AF.lo | 0x80;} else { AF.lo = 0;} // set Z = 0 if == 0
      AF.lo = AF.lo & 0b10110000; // reset N (N = 0)
      if (AF.hi < x) { AF.lo = AF.lo | 0b00100000;} // Set H if overfloss
      if ( (AF.hi & 0x0F) < loNib){ AF.lo = AF.lo | 0b00010000; } // Set CY if loer nibble overflos

    }
    void ADC_al()
    {
      u8 x = AF.hi;
      u8 loNib = HL.lo & 0x0F;
      AF.hi += BC.hi + (AF.lo & 0b00010000);
      if (AF.hi == 0){ AF.lo = AF.lo | 0x80;} else { AF.lo = 0;} // set Z = 0 if == 0
      AF.lo = AF.lo & 0b10110000; // reset N (N = 0)
      if (AF.hi < x) { AF.lo = AF.lo | 0b00100000;} // Set H if overfloss
      if ( (AF.hi & 0x0F) < loNib){ AF.lo = AF.lo | 0b00010000; } // Set CY if loer nibble overflos

    }
    void ADC_aHL()
    {
      u8 x = AF.hi;
      u8 loNib = AF.hi & 0x0F;
      AF.hi += RAM::readAt(HL.val()) + (AF.lo & 0b00010000);
      if (AF.hi == 0){ AF.lo = AF.lo | 0x80;} else { AF.lo = 0;} // set Z = 0 if == 0
      AF.lo = AF.lo & 0b10110000; // reset N (N = 0)
      if (AF.hi < x) { AF.lo = AF.lo | 0b00100000;} // Set H if overfloss
      if ( (AF.hi & 0x0F) < loNib){ AF.lo = AF.lo | 0b00010000; } // Set CY if loer nibble overflos

    }
    void ADC_a_hash();

    // have to change it to flag setting only when the there is NO underflo isntead of the opposite

    // TODO:: for the CY flag, have to figure out whether it is a <= or a < in the if statement

    void SUB_a()
    { 
      u8 x = AF.hi;
      u8 loNib = AF.hi & 0x0F;
      AF.hi -= AF.hi;
      if (AF.hi == 0){ AF.lo = AF.lo | 0x80;} else { AF.lo = 0;} // set Z = 0 if == 0
      AF.lo = AF.lo | 0b01000000; // set N
      if ((AF.hi & 0x0F) <= x) { AF.lo = AF.lo | 0b00100000;} // Set H if loer nibble does not underflo
      if ( AF.hi <= loNib){ AF.lo = AF.lo | 0b00010000; } // Set CY if whole thing does not underflo underflos
    }
    void SUB_b()
    { 
      u8 x = AF.hi;
      u8 loNib = AF.hi & 0x0F;
      AF.hi -= BC.hi;
      if (AF.hi == 0){ AF.lo = AF.lo | 0x80;} else { AF.lo = 0;} // set Z = 0 if == 0
      AF.lo = AF.lo | 0b01000000; // set N
      if ((AF.hi & 0x0F) <= loNib) { AF.lo = AF.lo | 0b00100000;} // Set H if loer nibble does not underflo
      if ( AF.hi <= BC.hi){ AF.lo = AF.lo | 0b00010000; } // Set CY if whole thing does not underflo
    }
    void SUB_c()
    { 
      u8 x = AF.hi;
      u8 loNib = AF.hi & 0x0F;
      AF.hi -= BC.lo;
      if (AF.hi == 0){ AF.lo = AF.lo | 0x80;} else { AF.lo = 0;} // set Z = 0 if == 0
      AF.lo = AF.lo | 0b01000000; // set N
      if ((AF.hi & 0x0F) <= loNib) { AF.lo = AF.lo | 0b00100000;} // Set H if loer nibble does not underflo
      if ( AF.hi < BC.lo){ AF.lo = AF.lo | 0b00010000; } // Set CY if whole thing does not underflo underflos
    }
    void SUB_d()
    { 
      u8 x = AF.hi;
      u8 loNib = AF.hi & 0x0F;
      AF.hi -= DE.hi;
      if (AF.hi == 0){ AF.lo = AF.lo | 0x80;} else { AF.lo = 0;} // set Z = 0 if == 0
      AF.lo = AF.lo | 0b01000000; // set N
      if ((AF.hi & 0x0F) <= loNib) { AF.lo = AF.lo | 0b00100000;} // Set H if loer nibble does not underflo
      if ( AF.hi < DE.hi){ AF.lo = AF.lo | 0b00010000; } // Set CY if whole thing does not underflo underflos
    }
    void SUB_e()
    { 
      u8 x = AF.hi;
      u8 loNib = AF.hi & 0x0F;
      AF.hi -= DE.lo;
      if (AF.hi == 0){ AF.lo = AF.lo | 0x80;} else { AF.lo = 0;} // set Z = 0 if == 0
      AF.lo = AF.lo | 0b01000000; // set N
      if ((AF.hi & 0x0F) <= loNib ) { AF.lo = AF.lo | 0b00100000;} // Set H if loer nibble does not underflo
      if ( AF.hi < DE.lo){ AF.lo = AF.lo | 0b00010000; } // Set CY if whole thing does not underflo underflos
    }
    void SUB_h()
    { 
      u8 x = AF.hi;
      u8 loNib = AF.hi & 0x0F;
      AF.hi -= HL.hi;
      if (AF.hi == 0){ AF.lo = AF.lo | 0x80;} else { AF.lo = 0;} // set Z = 0 if == 0
      AF.lo = AF.lo | 0b01000000; // set N
      if ((AF.hi & 0x0F) <= loNib) { AF.lo = AF.lo | 0b00100000;} // Set H if loer nibble does not underflo
      if ( AF.hi < HL.hi){ AF.lo = AF.lo | 0b00010000; } // Set CY if whole thing does not underflo underflos
    }
    void SUB_l()
    { 
      u8 x = AF.hi;
      u8 loNib = AF.hi & 0x0F;
      AF.hi -= HL.lo;
      if (AF.hi == 0){ AF.lo = AF.lo | 0x80;} else { AF.lo = 0;} // set Z = 0 if == 0
      AF.lo = AF.lo | 0b01000000; // set N
      if ((AF.hi & 0x0F) <= loNib) { AF.lo = AF.lo | 0b00100000;} // Set H if loer nibble does not underflo
      if ( AF.hi < HL.lo){ AF.lo = AF.lo | 0b00010000; } // Set CY if whole thing does not underflo underflos
    }
    void SUB_HL()
    { 
      u8 x = RAM::readAt(HL.val());
      u8 loNib = AF.hi & 0x0F;
      AF.hi -= x;
      if (AF.hi == 0){ AF.lo = AF.lo | 0x80;} else { AF.lo = 0;} // set Z = 0 if == 0
      AF.lo = AF.lo | 0b01000000; // set N
      if ((AF.hi & 0x0F) <= loNib) { AF.lo = AF.lo | 0b00100000;} // Set H if loer nibble does not underflo
      if ( AF.hi <= x){ AF.lo = AF.lo | 0b00010000; } // Set CY if whole thing does not underflo underflos
    }
    void SUB_hash()
    { 
      u8 x = RAM::read();
      u8 loNib = AF.hi & 0x0F;
      AF.hi -= x;
      if (AF.hi == 0){ AF.lo = AF.lo | 0x80;} else { AF.lo = 0;} // set Z = 0 if == 0
      AF.lo = AF.lo | 0b01000000; // set N
      if ((AF.hi & 0x0F) <= loNib) { AF.lo = AF.lo | 0b00100000;} // Set H if loer nibble does not underflo
      if ( AF.hi <= x){ AF.lo = AF.lo | 0b00010000; } // Set CY if whole thing does not underflo underflos
    }

    void SBC_a()
    { 
      u8 x = AF.hi;
      u8 loNib = AF.hi & 0x0F;
      AF.hi -= AF.hi + (AF.lo & 0b00010000);
      if (AF.hi == 0){ AF.lo = AF.lo | 0x80;} else { AF.lo = 0;} // set Z = 0 if == 0
      AF.lo = AF.lo | 0b01000000; // set N
      if ((AF.hi & 0x0F) <= x) { AF.lo = AF.lo | 0b00100000;} // Set H if loer nibble does not underflo
      if ( AF.hi <= loNib){ AF.lo = AF.lo | 0b00010000; } // Set CY if whole thing does not underflo underflos
    }
    void SBC_b()
    { 
      u8 x = AF.hi;
      u8 loNib = AF.hi & 0x0F;
      AF.hi -= BC.hi + (AF.lo & 0b00010000);
      if (AF.hi == 0){ AF.lo = AF.lo | 0x80;} else { AF.lo = 0;} // set Z = 0 if == 0
      AF.lo = AF.lo | 0b01000000; // set N
      if ((AF.hi & 0x0F) <= x) { AF.lo = AF.lo | 0b00100000;} // Set H if loer nibble does not underflo
      if ( AF.hi <= loNib){ AF.lo = AF.lo | 0b00010000; } // Set CY if whole thing does not underflo underflos
    }
    void SBC_c()
    { 
      u8 x = AF.hi;
      u8 loNib = AF.hi & 0x0F;
      AF.hi -= BC.lo + (AF.lo & 0b00010000);
      if (AF.hi == 0){ AF.lo = AF.lo | 0x80;} else { AF.lo = 0;} // set Z = 0 if == 0
      AF.lo = AF.lo | 0b01000000; // set N
      if ((AF.hi & 0x0F) <= x) { AF.lo = AF.lo | 0b00100000;} // Set H if loer nibble does not underflo
      if ( AF.hi <= loNib){ AF.lo = AF.lo | 0b00010000; } // Set CY if whole thing does not underflo underflos
    }
    void SBC_d()
    { 
      u8 x = AF.hi;
      u8 loNib = AF.hi & 0x0F;
      AF.hi -= DE.hi + (AF.lo & 0b00010000);
      if (AF.hi == 0){ AF.lo = AF.lo | 0x80;} else { AF.lo = 0;} // set Z = 0 if == 0
      AF.lo = AF.lo | 0b01000000; // set N
      if ((AF.hi & 0x0F) <= x) { AF.lo = AF.lo | 0b00100000;} // Set H if loer nibble does not underflo
      if ( AF.hi <= loNib){ AF.lo = AF.lo | 0b00010000; } // Set CY if whole thing does not underflo underflos
    }
    void SBC_e()
    { 
      u8 x = AF.hi;
      u8 loNib = AF.hi & 0x0F;
      AF.hi -= DE.lo + (AF.lo & 0b00010000);
      if (AF.hi == 0){ AF.lo = AF.lo | 0x80;} else { AF.lo = 0;} // set Z = 0 if == 0
      AF.lo = AF.lo | 0b01000000; // set N
      if ((AF.hi & 0x0F) <= x) { AF.lo = AF.lo | 0b00100000;} // Set H if loer nibble does not underflo
      if ( AF.hi <= loNib){ AF.lo = AF.lo | 0b00010000; } // Set CY if whole thing does not underflo underflos
    }
    void SBC_h()
    { 
        u8 x = AF.hi;
        u8 loNib = AF.hi & 0x0F;
        AF.hi -= HL.hi + (AF.lo & 0b00010000);
        if (AF.hi == 0){ AF.lo = AF.lo | 0x80;} else { AF.lo = 0;} // set Z = 0 if == 0
        AF.lo = AF.lo | 0b01000000; // set N
        if ((AF.hi & 0x0F) <= x) { AF.lo = AF.lo | 0b00100000;} // Set H if loer nibble does not underflo
        if ( AF.hi <= loNib){ AF.lo = AF.lo | 0b00010000; } // Set CY if whole thing does not underflo underflos
    }
    void SBC_l()
    { 
        u8 x = AF.hi;
        u8 loNib = AF.hi & 0x0F;
        AF.hi -= HL.lo + (AF.lo & 0b00010000);
        if (AF.hi == 0){ AF.lo = AF.lo | 0x80;} else { AF.lo = 0;} // set Z = 0 if == 0
        AF.lo = AF.lo | 0b01000000; // set N
        if ((AF.hi & 0x0F) <= x) { AF.lo = AF.lo | 0b00100000;} // Set H if loer nibble does not underflo
        if ( AF.hi <= loNib){ AF.lo = AF.lo | 0b00010000; } // Set CY if whole thing does not underflo underflos
    }
    void SBC_HL()
    { 
        u8 x = AF.hi;
        u8 loNib = AF.hi & 0x0F;
        AF.hi -= RAM::readAt(HL.val()) + (AF.lo & 0b00010000);
        if (AF.hi == 0){ AF.lo = AF.lo | 0x80;} else { AF.lo = 0;} // set Z = 0 if == 0
        AF.lo = AF.lo | 0b01000000; // set N
        if ((AF.hi & 0x0F) <= x) { AF.lo = AF.lo | 0b00100000;} // Set H if loer nibble does not underflo
        if ( AF.hi <= loNib){ AF.lo = AF.lo | 0b00010000; } // Set CY if whole thing does not underflo underflos
    }

    void AND_a(){ AF.hi = AF.hi & AF.hi; if (AF.hi == 0){AF.lo |= 0b10000000;} AF.lo &= 0b10100000;AF.lo |= 0b00100000;}
    void AND_b(){ AF.hi = AF.hi & BC.hi; if (AF.hi == 0){AF.lo |= 0b10000000;} AF.lo &= 0b10100000;AF.lo |= 0b00100000;}
    void AND_c(){ AF.hi = AF.hi & BC.lo; if (AF.hi == 0){AF.lo |= 0b10000000;} AF.lo &= 0b10100000;AF.lo |= 0b00100000;}
    void AND_d(){ AF.hi = AF.hi & DE.hi; if (AF.hi == 0){AF.lo |= 0b10000000;} AF.lo &= 0b10100000;AF.lo |= 0b00100000;}
    void AND_e(){ AF.hi = AF.hi & DE.lo; if (AF.hi == 0){AF.lo |= 0b10000000;} AF.lo &= 0b10100000;AF.lo |= 0b00100000;}
    void AND_h(){ AF.hi = AF.hi & HL.hi; if (AF.hi == 0){AF.lo |= 0b10000000;} AF.lo &= 0b10100000;AF.lo |= 0b00100000;}
    void AND_l(){ AF.hi = AF.hi & HL.lo; if (AF.hi == 0){AF.lo |= 0b10000000;} AF.lo &= 0b10100000;AF.lo |= 0b00100000;}
    void AND_HL(){ AF.hi = AF.hi & (RAM::readAt(HL.val())); if (AF.hi == 0){AF.lo |= 0b10000000;} AF.lo &= 0b10100000;AF.lo |= 0b00100000;}
    //void AND_hash(){}

    void OR_a(){ AF.hi = AF.hi | AF.hi; if (AF.hi == 0){AF.lo |= 0b10000000;} AF.lo &= 0b10100000;}
    void OR_b(){ AF.hi = AF.hi | BC.hi; if (AF.hi == 0){AF.lo |= 0b10000000;} AF.lo &= 0b10100000;}
    void OR_c(){ AF.hi = AF.hi | BC.lo; if (AF.hi == 0){AF.lo |= 0b10000000;} AF.lo &= 0b10100000;}
    void OR_d(){ AF.hi = AF.hi | DE.hi; if (AF.hi == 0){AF.lo |= 0b10000000;} AF.lo &= 0b10100000;}
    void OR_e(){ AF.hi = AF.hi | DE.lo; if (AF.hi == 0){AF.lo |= 0b10000000;} AF.lo &= 0b10100000;}
    void OR_h(){ AF.hi = AF.hi | HL.hi; if (AF.hi == 0){AF.lo |= 0b10000000;} AF.lo &= 0b10100000;}
    void OR_l(){ AF.hi = AF.hi | HL.lo; if (AF.hi == 0){AF.lo |= 0b10000000;} AF.lo &= 0b10100000;}
    void OR_HL(){ AF.hi = AF.hi | (RAM::readAt(HL.val())); if (AF.hi == 0){AF.lo |= 0b10000000;} AF.lo &= 0b10100000;}

    void XOR_a(){ AF.hi = AF.hi ^ AF.hi; if (AF.hi == 0){AF.lo |= 0b10000000;} AF.lo &= 0b10100000;}
    void XOR_b(){ AF.hi = AF.hi ^ BC.hi; if (AF.hi == 0){AF.lo |= 0b10000000;} AF.lo &= 0b10100000;}
    void XOR_c(){ AF.hi = AF.hi ^ BC.lo; if (AF.hi == 0){AF.lo |= 0b10000000;} AF.lo &= 0b10100000;}
    void XOR_d(){ AF.hi = AF.hi ^ DE.hi; if (AF.hi == 0){AF.lo |= 0b10000000;} AF.lo &= 0b10100000;}
    void XOR_e(){ AF.hi = AF.hi ^ DE.lo; if (AF.hi == 0){AF.lo |= 0b10000000;} AF.lo &= 0b10100000;}
    void XOR_h(){ AF.hi = AF.hi ^ HL.hi; if (AF.hi == 0){AF.lo |= 0b10000000;} AF.lo &= 0b10100000;}
    void XOR_l(){ AF.hi = AF.hi ^ HL.lo; if (AF.hi == 0){AF.lo |= 0b10000000;} AF.lo &= 0b10100000;}
    void XOR_HL(){ AF.hi = AF.hi ^ (RAM::readAt(HL.val())); if (AF.hi == 0){AF.lo |= 0b10000000;} AF.lo &= 0b10100000;}

    void CP_a()
    { 
        u8 x = AF.hi;
        u8 loNib = AF.hi & 0x0F;
        u8 z = AF.hi - AF.hi;
        if (z == 0){ AF.lo |= 0x80;} else {AF.lo &= 0b01110000;} // set Z = 1 if == 0
        AF.lo = AF.lo | 0b01000000; // set N
        if ((z & 0x0F) <= loNib) { AF.lo = AF.lo | 0b00100000;} else { AF.lo &= 0b11010000; } // Set H if loer nibble does not underflo
        if ( AF.hi < AF.hi){ AF.lo = AF.lo | 0b00010000; } else { AF.lo &= 0b11101111; } // Set CY if whole thing does not underflo 
    }
    void CP_b()
    { 
        u8 x = AF.hi;
        u8 loNib = AF.hi & 0x0F;
        u8 z = AF.hi - BC.hi;
        if (z == 0){ AF.lo = AF.lo | 0x80;} else {AF.lo &= 0b01110000;} // set Z = 0 if == 0
        AF.lo = AF.lo | 0b01000000; // set N
        if ((z & 0x0F) <= loNib) { AF.lo = AF.lo | 0b00100000;} else { AF.lo &= 0b11010000; } // Set H if loer nibble does not underflo
        if ( AF.hi < BC.hi){ AF.lo = AF.lo | 0b00010000; } else { AF.lo &= 0b11101111; } // Set CY if whole thing does not underflo
    }
    void CP_c()
    { 
        u8 x = AF.hi;
        u8 loNib = AF.hi & 0x0F;
        u8 z = AF.hi - BC.lo;
        if (z == 0){ AF.lo = AF.lo | 0x80;} else {AF.lo &= 0b01110000;} // set Z = 0 if == 0
        AF.lo = AF.lo | 0b01000000; // set N
        if ((z & 0x0F) <= loNib) { AF.lo = AF.lo | 0b00100000;} else { AF.lo &= 0b11010000; } // Set H if loer nibble does not underflo
        if ( AF.hi < BC.lo){ AF.lo = AF.lo | 0b00010000; } else { AF.lo &= 0b11101111; } // Set CY if whole thing does not underflo
    }
    void CP_d()
    { 
        u8 x = AF.hi;
        u8 loNib = AF.hi & 0x0F;
        u8 z = AF.hi - DE.hi;
        if (z == 0){ AF.lo = AF.lo | 0x80;} else {AF.lo &= 0b01110000;} // set Z = 0 if == 0
        AF.lo = AF.lo | 0b01000000; // set N
        if ((z & 0x0F) <= loNib) { AF.lo = AF.lo | 0b00100000;} else { AF.lo &= 0b11010000; } // Set H if loer nibble does not underflo
        if ( AF.hi < DE.hi){ AF.lo = AF.lo | 0b00010000; } else { AF.lo &= 0b11101111; } // Set CY if whole thing does not underflo 
    }

    void CP_e()
    { 
        u8 x = AF.hi;
        u8 loNib = AF.hi & 0x0F;
        u8 z = AF.hi - DE.lo;
        if (z == 0){ AF.lo = AF.lo | 0x80;} else {AF.lo &= 0b01110000;} // set Z = 0 if == 0
        AF.lo = AF.lo | 0b01000000; // set N
        if ((z & 0x0F) <= loNib) { AF.lo = AF.lo | 0b00100000;} else { AF.lo &= 0b11010000; } // Set H if loer nibble does not underflo
        if ( AF.hi < DE.lo){ AF.lo = AF.lo | 0b00010000; } else { AF.lo &= 0b11101111; } // Set CY if whole thing does not underflo underflos
    }
    void CP_h()
    { 
      u8 x = AF.hi;
      u8 loNib = AF.hi & 0x0F;
      u8 z = AF.hi - HL.hi;
      if (z == 0){ AF.lo = AF.lo | 0x80;} else {AF.lo &= 0b01110000;} // set Z = 0 if == 0
      AF.lo = AF.lo | 0b01000000; // set N
      if ((z & 0x0F) <= loNib) { AF.lo = AF.lo | 0b00100000;} else { AF.lo &= 0b11010000; } // Set H if loer nibble does not underflo
      if ( AF.hi < HL.hi){ AF.lo = AF.lo | 0b00010000; } else { AF.lo &= 0b11101111; } // Set CY if whole thing does not underflo underflos
    }
    void CP_l()
    { 
      u8 x = AF.hi;
      u8 loNib = AF.hi & 0x0F;
      u8 z = AF.hi - HL.lo;
      if (z == 0){ AF.lo = AF.lo | 0x80;} else {AF.lo &= 0b01110000;}// set Z = 1 if == 0
      AF.lo = AF.lo | 0b01000000; // set N
      if ((z & 0x0F) <= loNib) { AF.lo = AF.lo | 0b00100000;} else { AF.lo &= 0b11010000; } // Set H if loer nibble does not underflo
      if ( AF.hi < HL.lo){ AF.lo = AF.lo | 0b00010000; } else { AF.lo &= 0b11101111; } // Set CY if whole thing does not underflo underflos
      //TODO: i'm setting this last one incorrectly I think
    }
    void CP_HL()
    { 
      u8 x = RAM::readAt(HL.val());
      u8 loNib = AF.hi & 0x0F;
      u8 z = AF.hi - x;
      if (z == 0){ AF.lo = AF.lo | 0x80;} else {AF.lo &= 0b01110000;} // set Z = 1 if == 0
      AF.lo = AF.lo | 0b01000000; // set N
      if ((z & 0x0F) <= loNib) { AF.lo = AF.lo | 0b00100000;} else { AF.lo &= 0b11010000; } // Set H if loer nibble does not underflo
      if (AF.hi < x){ AF.lo = AF.lo | 0b00010000; } else { AF.lo &= 0b11101111; } // Set CY if whole thing does not underflo underflos
    }

    void CP_hash()
    { 
      u8 x = RAM::read();
      u8 loNib = AF.hi & 0x0F;
      u8 z = AF.hi - x;
      if (z == 0){ AF.lo |= 0b10000000;} else {AF.lo &= 0b01110000;} // set Z = 1 if result is == 0
      AF.lo |= 0b01000000; // set N
      if ((z & 0x0F) <= loNib) { AF.lo = AF.lo | 0b00100000;} else { AF.lo &= 0b11010000; } // Set H if loer nibble does not underflo
      if (AF.hi < x){ AF.lo = AF.lo | 0b00010000; } else { AF.lo &= 0b11101111; }// Set CY if whole thing does not underflo underflos

      //std::cout << "COMPARE: = " << std::hex << unsigned(z) << std::endl;
    }
    void INC_a()
    {
      u8 loNib = AF.hi & 0x0F;
      ++AF.hi;
      if(AF.hi == 0){ AF.lo |= 0b10000000;} else {AF.lo &= 0b01110000;}
      AF.lo &= 0b10110000;
      if (loNib > (AF.hi & 0x0F)) {AF.lo |= 0b00100000;} else {AF.lo &= 0b11010000;}
    }
    void INC_b()
    {
      u8 loNib = BC.hi & 0x0F;
      ++BC.hi;
      if(BC.hi == 0){ AF.lo |= 0b10000000;} else {AF.lo &= 0b01110000;}
      AF.lo &= 0b10110000;
      if (loNib > (BC.hi & 0x0F)) {AF.lo |= 0b00100000;} else {AF.lo &= 0b11010000;}
    }
    void INC_c()
    {
      u8 loNib = BC.lo & 0x0F;
      ++BC.lo;
      if(BC.lo == 0){ AF.lo |= 0b10000000;} else {AF.lo &= 0b01110000;}
      AF.lo &= 0b10110000;
      if (loNib > (BC.lo & 0x0F)) {AF.lo |= 0b00100000;} else {AF.lo &= 0b11010000;}
    }
    void INC_d()
    {
      u8 loNib = DE.hi & 0x0F;
      ++DE.hi;
      if(DE.hi == 0){ AF.lo |= 0b10000000;} else {AF.lo &= 0b01110000;}
      AF.lo &= 0b10110000;
      if (loNib > (DE.hi & 0x0F)) {AF.lo |= 0b00100000;} else {AF.lo &= 0b11010000;}
    }
    void INC_e()
    {
      u8 loNib = DE.lo & 0x0F;
      ++AF.hi;
      if(DE.lo == 0){ AF.lo |= 0b10000000;} else {AF.lo &= 0b01110000;}
      AF.lo &= 0b10110000;
      if (loNib > (DE.lo & 0x0F)) {AF.lo |= 0b00100000;} else {AF.lo &= 0b11010000;}
    }
    void INC_h()
    {
      u8 loNib = HL.hi & 0x0F;
      ++HL.hi;
      if(HL.hi == 0){ AF.lo |= 0b10000000;} else {AF.lo &= 0b01110000;}
      AF.lo &= 0b10110000;
      if (loNib > (HL.hi & 0x0F)) {AF.lo |= 0b00100000;} else {AF.lo &= 0b11010000;}
    }
    void INC_l()
    {
      u8 loNib = HL.lo & 0x0F;
      ++HL.lo;
      if(HL.lo == 0){ AF.lo |= 0b10000000;} else {AF.lo &= 0b01110000;}
      AF.lo &= 0b10110000;
      if (loNib > (HL.lo & 0x0F)) {AF.lo |= 0b00100000;} else {AF.lo &= 0b11010000;}
    }
    void INC_HLad()
    {
      u8 x = RAM::readAt(HL.val());
      u8 loNib = x & 0x0F;
      ++x;
      RAM::write(x, HL.val());
      if(x == 0){ AF.lo |= 0b10000000;} else {AF.lo &= 0b01110000;}
      AF.lo &= 0b10110000;
      if (loNib > (x & 0x0F)) {AF.lo |= 0b00100000;} else {AF.lo &= 0b11010000;}
    }

    void DEC_a()
    {
      u8 loNib = AF.hi & 0x0F;
      --AF.hi;
      if(AF.hi == 0){ AF.lo |= 0b10000000;} else {AF.lo &= 0b01110000;}
      AF.lo |= 0b01000000;
      if (loNib >= (AF.hi & 0x0F)) {AF.lo |= 0b00100000;} else {AF.lo &= 0b11010000;}
    }
    void DEC_b()
    {
        u8 loNib = BC.hi & 0x0F;
        --BC.hi;
        if(BC.hi == 0){ AF.lo |= 0b10000000;} else {AF.lo &= 0b01110000;}
        AF.lo |= 0b01000000;
        if (loNib >= (BC.hi & 0x0F)) {AF.lo |= 0b00100000;} else {AF.lo &= 0b11010000;}
    }
    void DEC_c()
    {
        u8 loNib = BC.lo & 0x0F;
        --BC.lo;
        if(BC.lo == 0){ AF.lo |= 0b10000000;} else {AF.lo &= 0b01110000;} // Z set if the result is zero
        AF.lo |= 0b01000000;
        if (loNib >= (BC.lo & 0x0F)) {AF.lo |= 0b00100000;} else {AF.lo &= 0b11010000;}
    }
    void DEC_d()
    {
        u8 loNib = DE.hi & 0x0F;
        --DE.hi;
        if(DE.hi == 0){ AF.lo |= 0b10000000;} else {AF.lo &= 0b01110000;}
        AF.lo |= 0b01000000;
        if (loNib >= (DE.hi & 0x0F)) {AF.lo |= 0b00100000;} else {AF.lo &= 0b11010000;}
    }
    void DEC_e()
    {
        u8 loNib = DE.lo & 0x0F;
        --DE.lo;
        if(DE.lo == 0){ AF.lo |= 0b10000000;} else {AF.lo &= 0b01110000;}
        AF.lo |= 0b01000000;
        if (loNib >= (DE.lo & 0x0F)) {AF.lo |= 0b00100000;} else {AF.lo &= 0b11010000;}
    }
    void DEC_h()
    {
      u8 loNib = HL.hi & 0x0F;
      --HL.hi;
      if(HL.hi == 0){ AF.lo |= 0b10000000;} else {AF.lo &= 0b01110000;}
      AF.lo |= 0b01000000;
      if (loNib >= (HL.hi & 0x0F)) {AF.lo |= 0b00100000;} else {AF.lo &= 0b11010000;}
    }
    void DEC_l()
    {
      u8 loNib = HL.lo & 0x0F;
      HL.lo--;
      if(HL.lo == 0){ AF.lo |= 0b10000000;} else {AF.lo &= 0b01110000;}
      AF.lo |= 0b01000000;
      if (loNib > (HL.lo & 0x0F)) {AF.lo |= 0b00100000;} else {AF.lo &= 0b11010000;}
    }
    void DEC_HL()
    {
      u8 x = RAM::readAt(HL.val());
      u8 loNib = x & 0x0F;
      x--;
      RAM::write(x, HL.val());
      if(x == 0){ AF.lo |= 0b10000000;} else {AF.lo &= 0b01110000;}
      AF.lo |= 0b01000000;
      if (loNib > (x & 0x0F)) {AF.lo |= 0b00100000;} else {AF.lo &= 0b11010000;}
    }

    // whatever the fuck i need to get this shit working
    void RST_FF()
    {
      u8 loNibble = PC & 0b00001111;
      u8 hiNibble = PC >> 4;
      SP--;
      RAM::write(loNibble, SP);
      SP--;
      RAM::write(hiNibble, SP);

      PC = 0x38;
    }
  }

