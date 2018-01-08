#include <iostream>
#include <vector>
typedef  uint8_t   u8;
typedef   int8_t   s8;
typedef uint16_t  u16;

class reg {
public:
  u8 hi, lo;
  u16 val();
  void set(u16);
};

namespace CPU
{
	std::vector<void(*)()> op_code;
	void init_op_codes();
	u16 PC = 0; //Program Counter: holds the address of the current instruction being fetched from memory
	// Needs to be incremented after reading any opcode (note that opcodes can change the counter

	u16 SP; //Stack Pointer: holds the address of the current top of stack located in external RAM
	u16 IX, IY; //Index registers: addresses where we need to store or retrieve data,
	// Could be signed integers, I'm not sure, not even sure that the GB uses these registers

	// F is the the flag register Z N H C 0 0 0 0, Z - zero flag, N - subtract flag, H - half carry, C - carry flag
	reg AF, BC, DE, HL;

	int cycles = 1;
	int timing = 0;

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
	void XOR_star();
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
	void DEC_HL();
	void INC_BC();
	void INC_DE();
	void INC_HLad();
	void INC_SP();

	// Piecemeal
	void RST_FF();
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

	void JR_NZ();
	void JR_Z();
	void JR_n();
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
    void LDI_HLa();
    void LDH_nA();

    void LD_nn_a();
    void LDH_nA();
    void LDH_a_ffn();
    void RET();
	void RET_NZ();
	void RET_Z();
	void RET_NC();
	void RET_C();
	void RETI();

	std::vector<void (*)()> op_codes;
}