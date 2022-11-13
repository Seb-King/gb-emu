#include <SDL.h>
#include <iostream>
#include <stdint.h>
#include <vector>
#include <cstring>
//#include "common.cpp"

typedef  uint8_t   u8;
typedef   int8_t   s8;
typedef uint16_t  u16;



void game_loop();

void dump_vram();
// void draw_chars();

class reg {
public:
    u8 hi, lo;
    u16 val();
    void set(u16);
};

namespace RUPS { 
    bool handle_interupts();
    // Interrupt Flag 0xFF0F,
    // Bit 4: Transition from High to Low of Pin number P10-P13
    // Bit 3: Serial I/O Transfer complete
    // Bit 2: Timer overflow
    // Bit 1: LCDC (see STAT)
    // Bit 0: V-Blank

    // Lowest bits have highest priority
}

namespace TIMER {
    void inc(int);
    void update();
    void overflow();
}

namespace LCD {
    void draw_sprites();
    void draw_BG();
    void draw_BGTile(int, u8, u8, u16, u8 palette);
    void render_sprite(u8, u8, u8, u8);
    void draw_Window();
    void draw_WindowTile();
    void  update();
    void v_blank();
}



namespace RAM {
    u8 readAt(u16);
    u8 read();              // look at the next byte and return it?
    void write(u8, u16);
    void init();
    void dump_oam();
    void DMA_routine();
    void d_vram();
}

namespace CPU {
    // void handle_interrupts();
    void cb_not_imp();
    void op_not_imp();
    void init_decoder();
    void STAT();

    void run_cb(u8);

    void NOP();
    void LDrr_ab();
    void LDrr_ac();
    void LDrr_ad();
    void LDrr_ae();
    void LDrr_ah();
    void LDrr_al();
    void LDrr_aBC();
    void LDrr_aDE();
    void LDrr_ann();
    void LDrr_a_hash();
    void LDrr_aHL();
    void LDrr_ba();
    void LDrr_bc();
    void LDrr_bd();
    void LDrr_be();
    void LDrr_bh();
    void LDrr_bl();
    void LDrr_bHL();
    void LDrr_ca();
    void LDrr_cb();
    void LDrr_cd();
    void LDrr_ce();
    void LDrr_ch();
    void LDrr_cl();
    void LDrr_cHL();
    void LDrr_da();
    void LDrr_db();
    void LDrr_dc();
    void LDrr_de();
    void LDrr_dh();
    void LDrr_dl();
    void LDrr_dHL();
    void LDrr_ea();
    void LDrr_eb();
    void LDrr_ec();
    void LDrr_ed();
    void LDrr_eh();
    void LDrr_el();
    void LDrr_eHL();
    void LDrr_ha();
    void LDrr_hb();
    void LDrr_hc();
    void LDrr_hd();
    void LDrr_he();
    void LDrr_hl();
    void LDrr_hHL();
    void LDrr_la();
    void LDrr_lb();
    void LDrr_lc();
    void LDrr_ld();
    void LDrr_le();
    void LDrr_lh();
    void LDrr_lHL();
    void LDrr_HLa();
    void LDrr_HLb();
    void LDrr_HLc();
    void LDrr_HLd();
    void LDrr_HLe();
    void LDrr_HLh();
    void LDrr_HLl();
    void LDrr_HLn();

    void LDa_c();
    void LDc_a();
    void LDDaHL();
    void LDDHLa();
    void LDDaHL();
    void LDDHLa();
    void LDHnA();
    void LDHAn();
    void LD_nn_BC();
    void LD_nn_DE();
    void LD_nn_HL();
    void LD_nn_SP();
    void LD_SPHL();
    void LDHL_SPn();
    void LD_nnSP();
    void PUSH_AF();
    void PUSH_BC();
    void PUSH_DE();
    void PUSH_HL();
    void POP_AF();
    void POP_BC();
    void POP_DE();
    void POP_HL();
    void ADD_aa();
    void ADD_ab();
    void ADD_ac();
    void ADD_ad();
    void ADD_ae();
    void ADD_ah();
    void ADD_al();
    void ADD_aH();
    void ADD_a_hash();
    void ADC_aa();
    void ADC_ab();
    void ADC_ac();
    void ADC_ad();
    void ADC_ae();
    void ADC_ah();
    void ADC_al();
    void ADC_aHL();
    void ADC_a_hash();
    void SUB_a();
    void SUB_b();
    void SUB_c();
    void SUB_d();
    void SUB_e();
    void SUB_h();
    void SUB_l();
    void SUB_HL();
    void SBC_a();
    void SBC_b();
    void SBC_c();
    void SBC_d();
    void SBC_e();
    void SBC_h();
    void SBC_l();
    void SBC_HL();
    void SUB_hash();
    void AND_a();
    void AND_b();
    void AND_c();
    void AND_d();
    void AND_e();
    void AND_h();
    void AND_l();
    void AND_HL();
    void AND_hash();
    void OR_a();
    void OR_b();
    void OR_c();
    void OR_d();
    void OR_e();
    void OR_h();
    void OR_l();
    void OR_HL();
    void OR_hash();
    void XOR_a();
    void XOR_b();
    void XOR_c();
    void XOR_d();
    void XOR_e();
    void XOR_h();
    void XOR_l();
    void XOR_HL();
    void XOR_hash();
    void CP_a();
    void CP_b();
    void CP_c();
    void CP_d();
    void CP_e();
    void CP_h();
    void CP_l();
    void CP_HL();
    void CP_hash();
    void INC_a();
    void INC_b();
    void INC_c();
    void INC_d();
    void INC_e();
    void INC_h();
    void INC_l();
    void INC_HL();
    void DEC_a();
    void DEC_b();
    void DEC_c();
    void DEC_d();
    void DEC_e();
    void DEC_h();
    void DEC_l();
    void DEC_HLad();

    void INC_HLad();

    // Piecemeal
    void RST_00();
    void RST_08();
    void RST_10();
    void RST_18();
    void RST_20();
    void RST_28();
    void RST_30();
    void RST_38();
    void RLA();

    void CB();
    void BIT_7H();
    void RL_A();
    void RL_B();
    void RL_C();
    void RL_D();
    void RL_E();
    void RL_H();
    void RL_L();
    void RL_addr_HL();
    void RLCA();
    void RLC_A();
    void RLC_B();
    void RLC_C();
    void RLC_D();
    void RLC_E();
    void RLC_H();
    void RLC_L();
    void RLC_HL();
    void RRA();
    void RRCA();

    void JR_NZ();
    void JR_Z();
    void JR_n();
    void JP();
    void JP_NZ();
    void JP_Z();
    void JP_NC();
    void JP_C();
    void JP_HL();
    void LDb_n();
    void LDc_n();
    void LDd_n();
    void LDe_n();
    void LDh_n();
    void LDl_n();
    void LDrr_a_hash();
    void LD_HL_a();
    void LD_BC_a();
    void LD_DE_a();
    void LDad_n_a();
    void CALL_nn();
    void CALL_NZ();
    void CALL_Z();
    void CALL_NC();
    void CALL_C();
    void LDI_HLa();
    void LDH_nA();

    void LD_nn_a();
    void LDH_a_ffn();
    void RET();
    void RET_NZ();
    void RET_Z();
    void RET_NC();
    void RET_C();
    void RETI();
    void DI();
    void EI();
    void LDI_aHL();

    void ADD_BC();
    void ADD_DE();
    void ADD_HL();
    void ADD_SP();

    void INC_BC();
    void INC_DE();
    void INC_HL();
    void INC_SP();
    void DEC_BC();
    void DEC_DE();
    void DEC_HL();
    void DEC_SP();
    void DAA();

    void ADD_n_SP();
    void CPL();
    void CCF();
    void SCF();
    void BIT_0A();
    void BIT_1A();
    void BIT_2A();
    void BIT_3A();
    void BIT_4A();
    void BIT_5A();
    void BIT_6A();
    void BIT_7A();
    void BIT_0B();
    void BIT_1B();
    void BIT_2B();
    void BIT_3B();
    void BIT_4B();
    void BIT_5B();
    void BIT_6B();
    void BIT_7B();
    void BIT_0C();
    void BIT_1C();
    void BIT_2C();
    void BIT_3C();
    void BIT_4C();
    void BIT_5C();
    void BIT_6C();
    void BIT_7C();
    void BIT_0D();
    void BIT_1D();
    void BIT_2D();
    void BIT_3D();
    void BIT_4D();
    void BIT_5D();
    void BIT_6D();
    void BIT_7D();
    void BIT_0E();
    void BIT_1E();
    void BIT_2E();
    void BIT_3E();
    void BIT_4E();
    void BIT_5E();
    void BIT_6E();
    void BIT_7E();
    void BIT_0H();
    void BIT_1H();
    void BIT_2H();
    void BIT_3H();
    void BIT_4H();
    void BIT_5H();
    void BIT_6H();
    void BIT_7H();
    void BIT_0L();
    void BIT_1L();
    void BIT_2L();
    void BIT_3L();
    void BIT_4L();
    void BIT_5L();
    void BIT_6L();
    void BIT_7L();

    void SWAP_a();
    void SWAP_b();
    void SWAP_c();
    void SWAP_d();
    void SWAP_e();
    void SWAP_h();
    void SWAP_l();
    void SWAP_HL();
    void RES_0_a();
    void HALT();
}