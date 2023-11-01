#include <iostream>
#include <iomanip>
#include <string>
#include "typedefs.hpp"
#include <vector>
#include "cpu.hpp"
#include "ram.hpp"
#include "functional"

const int clockrate = 4194304;

u16 reg::val() { return lo + (hi << 8); }

namespace RUPS {
    u8 IF = 0;
    u8 IE = 0;
}

void reg::set(u16 x) {
    hi = x >> 8;
    lo = (x & 0x00FF);
}

u16 LCDC = 0xFF40;
u16 STAT = 0xFF41;
u16 SCY = 0xFF42;
u16 SCX = 0xFF43;
u16 LY = 0xFF44;
u16 LYC = 0xFF45;

u16 DMA = 0xFF46;
u16 BGP = 0xFF47;

GB_CPU::GB_CPU() {
    this->halt = false;
    this->halt_bug = false;
    this->IME = 0;
    this->PC = 0;
    this->interrupt_mode = 0;
    this->count = 1;
    this->cycles = 4;
    this->timing = 0;

    std::vector<std::string> decoder(256);

    this->init_codes();
};

int GB_CPU::get_cycles() {
    return this->cycles;
}

int GB_CPU::get_timing() {
    return this->timing;
}

void GB_CPU::init_codes() {
    std::vector<operation> op_codes(256, std::mem_fn(&GB_CPU::op_not_imp));
    std::vector<operation> cb_codes(256, std::mem_fn(&GB_CPU::cb_not_imp));

    this->op_codes = op_codes;
    this->cb_codes = cb_codes;

    this->init_opcodes();
    this->init_decoder();
}

void GB_CPU::init_opcodes() {
    throw std::runtime_error("Not implemented");
}

void GB_CPU::init_decoder() {
    throw std::runtime_error("Not implemented");
}

void GB_CPU::disable_boot_rom() {
    RAM::write(0x01, 0xFF50);
}

void GB_CPU::init_registers_to_skip_boot() {
    this->PC = 0x0100;
    this->SP = 0xFFFE;
    this->AF.set(0x01B0);
    this->BC.set(0x0013);
    this->DE.set(0x00D8);
    this->HL.set(0x014D);
    this->disable_boot_rom();
}

void GB_CPU::push_byte_onto_stack(u8 val) {
    this->SP--;
    this->write(val, this->SP);
}

void GB_CPU::push_onto_stack(u16 val) {
    u8 lowerByte = val & 0x00FF;
    u8 higherByte = (val & 0xFF00) >> 8;

    this->push_byte_onto_stack(higherByte);
    this->push_byte_onto_stack(lowerByte);
}

u8 GB_CPU::pop_byte_from_stack() {
    u8 value = RAM::readAt(this->SP);
    this->SP++;
    return value;
}

u16 GB_CPU::pop_from_stack() {
    return this->pop_byte_from_stack() + (this->pop_byte_from_stack() << 8);
}

void GB_CPU::write(u8 val, u16 addr) {
    RAM::write(val, addr);

    if (addr == DMA) {
        this->DMA_routine();
    }
}

u8 GB_CPU::read() {
    u8 byte = RAM::readAt(this->PC);
    ++this->PC;
    return byte;
}

void GB_CPU::DMA_routine() {
    u16 source = (RAM::readAt(DMA) << 8);
    u8 data;
    for (int idx = 0; idx < 0xF1; idx++) {
        data = RAM::readAt(source + idx);

        this->write(data, 0xFE00 + idx);
    }
}

bool GB_CPU::half_carry_add(u8 a, u8 b) {
    return ((((a & 0x0F) + (b & 0x0F)) & 0x10) == 0x10);
}

bool GB_CPU::half_carry_sub(u8 a, u8 b) {
    return ((a & 0x0F) < (b & 0x0F));
}

void GB_CPU::set_z(bool x) {
    if (x) this->AF.lo |= 0x80;
    else this->AF.lo &= 0x70;
}

void GB_CPU::set_n(bool x) {
    if (x) this->AF.lo |= 0x40;
    else this->AF.lo &= 0xBF;
}

void GB_CPU::set_h(bool x) {
    if (x) this->AF.lo |= 0x20;
    else this->AF.lo &= 0xDF;
}

void GB_CPU::set_c(bool x) {
    if (x) this->AF.lo |= 0x10;
    else this->AF.lo &= 0xEF;
}

bool GB_CPU::get_c() { return (this->AF.lo & 0b00010000) == 0b00010000; }
bool GB_CPU::get_z() { return (this->AF.lo & 0b10000000) == 0b10000000; }
bool GB_CPU::get_h() { return (this->AF.lo & 0b00100000) == 0b00100000; }
bool GB_CPU::get_n() { return (this->AF.lo & 0b01000000) == 0b01000000; }

void GB_CPU::print_registers() {
    std::cout << "A:";
    std::cout << std::uppercase << std::hex << std::setfill('0') << std::setw(2) << int(this->AF.hi);
    std::cout << " F:" << std::uppercase << std::hex << std::setfill('0') << std::setw(2) << int(this->AF.lo);

    std::cout << " B:" << std::hex << std::setfill('0') << std::setw(2) << std::uppercase << int(this->BC.hi);
    std::cout << " C:" << std::hex << std::setfill('0') << std::setw(2) << std::uppercase << int(this->BC.lo);

    std::cout << " D:" << std::hex << std::setfill('0') << std::setw(2) << std::uppercase << int(this->DE.hi);
    std::cout << " E:" << std::hex << std::setfill('0') << std::setw(2) << std::uppercase << int(this->DE.lo);

    std::cout << " H:" << std::hex << std::setfill('0') << std::setw(2) << std::uppercase << std::uppercase << int(this->HL.hi);
    std::cout << " L:" << std::hex << std::setfill('0') << std::setw(2) << std::uppercase << int(this->HL.lo);

    std::cout << " SP:" << std::hex << std::setfill('0') << std::setw(4) << std::uppercase << int(this->SP);
    std::cout << " PC:" << std::hex << std::setfill('0') << std::setw(4) << std::uppercase << int(this->PC);

    std::cout << " PCMEM:";
    std::cout << std::hex << std::setfill('0') << std::setw(2) << std::uppercase << int(RAM::readAt(this->PC));
    std::cout << "," << std::hex << std::setfill('0') << std::setw(2) << std::uppercase << int(RAM::readAt(this->PC + 1));
    std::cout << "," << std::hex << std::setfill('0') << std::setw(2) << std::uppercase << int(RAM::readAt(this->PC + 2));
    std::cout << "," << std::hex << std::setfill('0') << std::setw(2) << std::uppercase << int(RAM::readAt(this->PC + 3));

    std::cout << std::endl;
}

void GB_CPU::STAT() {
    this->push_onto_stack(this->PC);
    this->PC = 0x0048;

    this->IF = RAM::readAt(0xFF0F);
    this->IF &= 0b11111101;
    this->write(this->IF, 0xFF0F);
}

void GB_CPU::op_not_imp() {
    u8 opcode = RAM::readAt(this->PC - 1);
    std::cout << "Opcode not implemented : OP = " << std::hex << unsigned(opcode) << std::endl;
    std::cout << "PC:" << std::hex << unsigned(this->PC) << std::endl;
    exit(0);
}

void GB_CPU::cb_not_imp() {
    u8 opcode = RAM::readAt(this->PC - 1);
    std::cout << "CB Opcode not implemented : OP = " << std::hex << unsigned(opcode) << std::endl;
    exit(0);
}

void GB_CPU::execute_next_operation() {
    if (!this->halt) {
        if (halt_bug) {
            halt_bug = false;
            u16 prevPC = this->PC;
            u8 opcode = this->read();
            this->run_opcode(opcode);
            this->PC = prevPC;
        } else {
            u8 opcode = this->read();
            this->run_opcode(opcode);
        }
    }
}

void GB_CPU::run_opcode(u8 op_code) {
    this->op_code_switch(op_code);

    if (this->interrupt_mode > 0) {
        if (this->interrupt_mode == 1) {
            this->IME = 0;
            this->interrupt_mode = 0;
        } else if (this->interrupt_mode == 2) {
            this->interrupt_mode--;
        }

        if (this->interrupt_mode == 3) {
            this->IME = 1;
            this->interrupt_mode = 0;
        } else if (this->interrupt_mode == 4) {
            this->interrupt_mode--;
        }
    }
}

void GB_CPU::op_code_switch(u8 op_code) {
    switch (op_code) {
    default:
        this->op_not_imp();
    }
}

void GB_CPU::SRL_A() {
    u8 oldValue = this->AF.hi;
    u8 newValue = oldValue >> 1;
    this->AF.hi = newValue;

    this->set_c(oldValue & 0b00000001);
    this->set_z(newValue == 0);
    this->set_n(false);
    this->set_h(false);
}

void GB_CPU::SRL_B() {
    u8 oldValue = this->BC.hi;
    u8 newValue = oldValue >> 1;
    this->BC.hi = newValue;

    this->set_c(oldValue & 0b00000001);
    this->set_z(newValue == 0);
    this->set_n(false);
    this->set_h(false);
}

void GB_CPU::SRL_C() {
    u8 oldValue = this->BC.lo;
    u8 newValue = oldValue >> 1;
    this->BC.lo = newValue;

    this->set_c(oldValue & 0b00000001);
    this->set_z(newValue == 0);
    this->set_n(false);
    this->set_h(false);
}

void GB_CPU::SRL_D() {
    u8 oldValue = this->DE.hi;
    u8 newValue = oldValue >> 1;
    this->DE.hi = newValue;

    this->set_c(oldValue & 0b00000001);
    this->set_z(newValue == 0);
    this->set_n(false);
    this->set_h(false);
}

void GB_CPU::SRL_E() {
    u8 oldValue = this->DE.lo;
    u8 newValue = oldValue >> 1;
    this->DE.lo = newValue;

    this->set_c(oldValue & 0b00000001);
    this->set_z(newValue == 0);
    this->set_n(false);
    this->set_h(false);
}

void GB_CPU::SRL_H() {
    u8 oldValue = this->HL.hi;
    u8 newValue = oldValue >> 1;
    this->HL.hi = newValue;

    this->set_c(oldValue & 0b00000001);
    this->set_z(newValue == 0);
    this->set_n(false);
    this->set_h(false);
}

void GB_CPU::SRL_L() {
    u8 oldValue = this->HL.lo;
    u8 newValue = oldValue >> 1;
    this->HL.lo = newValue;

    this->set_c(oldValue & 0b00000001);
    this->set_z(newValue == 0);
    this->set_n(false);
    this->set_h(false);
}

void GB_CPU::SRL_HL() {
    u8 oldValue = RAM::readAt(this->HL.val());
    u8 newValue = oldValue >> 1;
    RAM::write(newValue, this->HL.val());

    this->set_c(oldValue & 0b00000001);
    this->set_z(newValue == 0);
    this->set_n(false);
    this->set_h(false);
}

void GB_CPU::RR_A() {
    u8 oldValue = this->AF.hi;
    u8 newValue = (oldValue >> 1);
    if (this->get_c()) {
        newValue |= 0b10000000;
    }
    this->AF.hi = newValue;

    this->set_c(oldValue & 0b00000001);
    this->set_z(false);
    this->set_n(false);
    this->set_h(false);
    this->cycles = 4;
}

void GB_CPU::CB_RR_A() {
    u8 oldValue = this->AF.hi;
    u8 newValue = (oldValue >> 1);
    if (this->get_c()) {
        newValue |= 0b10000000;
    }
    this->AF.hi = newValue;

    this->set_c(oldValue & 0b00000001);
    this->set_z(newValue == 0);
    this->set_n(false);
    this->set_h(false);
}

void GB_CPU::RR_B() {
    u8 oldValue = this->BC.hi;
    u8 newValue = (oldValue >> 1);
    if (this->get_c()) {
        newValue |= 0b10000000;
    }
    this->BC.hi = newValue;

    this->set_c(oldValue & 0b00000001);
    this->set_z(newValue == 0);
    this->set_n(false);
    this->set_h(false);
}

void GB_CPU::RR_C() {
    u8 oldValue = this->BC.lo;
    u8 newValue = (oldValue >> 1);
    if (this->get_c()) {
        newValue |= 0b10000000;
    }
    this->BC.lo = newValue;

    this->set_c(oldValue & 0b00000001);
    this->set_z(newValue == 0);
    this->set_n(false);
    this->set_h(false);
}

void GB_CPU::RR_D() {
    u8 oldValue = this->DE.hi;
    u8 newValue = (oldValue >> 1);
    if (this->get_c()) {
        newValue |= 0b10000000;
    }
    this->DE.hi = newValue;

    this->set_c(oldValue & 0b00000001);
    this->set_z(newValue == 0);
    this->set_n(false);
    this->set_h(false);
}

void GB_CPU::RR_E() {
    u8 oldValue = this->DE.lo;
    u8 newValue = (oldValue >> 1);
    if (this->get_c()) {
        newValue |= 0b10000000;
    }
    this->DE.lo = newValue;

    this->set_c(oldValue & 0b00000001);
    this->set_z(newValue == 0);
    this->set_n(false);
    this->set_h(false);
}

void GB_CPU::RR_H() {
    u8 oldValue = this->HL.hi;
    u8 newValue = (oldValue >> 1);
    if (this->get_c()) {
        newValue |= 0b10000000;
    }
    this->HL.hi = newValue;

    this->set_c(oldValue & 0b00000001);
    this->set_z(newValue == 0);
    this->set_n(false);
    this->set_h(false);
}

void GB_CPU::RR_L() {
    u8 oldValue = this->HL.lo;
    u8 newValue = (oldValue >> 1);
    if (this->get_c()) {
        newValue |= 0b10000000;
    }
    this->HL.lo = newValue;

    this->set_c(oldValue & 0b00000001);
    this->set_z(newValue == 0);
    this->set_n(false);
    this->set_h(false);
}

void GB_CPU::RR_HL() {
    u8 oldValue = RAM::readAt(this->HL.val());
    u8 newValue = (oldValue >> 1);
    if (this->get_c()) {
        newValue |= 0b10000000;
    }
    RAM::write(newValue, this->HL.val());

    this->set_c(oldValue & 0b00000001);
    this->set_z(newValue == 0);
    this->set_n(false);
    this->set_h(false);
}

void GB_CPU::CCF() {
    this->AF.lo &= 0b10010000;
    if ((this->AF.lo & 0b00010000) == 0b00010000) {
        this->AF.lo -= 0b00010000;
    } else {
        this->AF.lo += 0b00010000;
    }
    this->cycles = 4;
}

void GB_CPU::SCF() {
    this->AF.lo &= 0b10010000;
    this->AF.lo |= 0b00010000;
    this->cycles = 4;
}

void GB_CPU::NOP() {
    this->cycles = 4;
}

void GB_CPU::RET() {
    u8 n1 = RAM::readAt(SP);
    ++this->SP;
    u16 n2 = RAM::readAt(SP);
    ++this->SP;
    u16 addr = n1 + (n2 << 8);
    this->PC = addr;
    this->cycles = 16;
}

void GB_CPU::RET_NZ() {
    this->cycles = 8;
    if (!this->get_z()) {
        this->cycles = 20;
        u8 n1 = RAM::readAt(SP);
        ++this->SP;
        u16 n2 = RAM::readAt(SP);
        ++this->SP;
        u16 addr = n1 + (n2 << 8);
        this->PC = addr;
    }
}

void GB_CPU::RET_Z() {
    this->cycles = 8;
    if (this->get_z()) {
        this->cycles = 20;
        u8 n1 = RAM::readAt(SP);
        ++this->SP;
        u16 n2 = RAM::readAt(SP);
        ++this->SP;
        u16 addr = n1 + (n2 << 8);
        this->PC = addr;
    }
}
void GB_CPU::RET_NC() {
    this->cycles = 8;
    if (!this->get_c()) {
        this->cycles = 20;
        u8 n1 = RAM::readAt(SP);
        ++this->SP;
        u16 n2 = RAM::readAt(SP);
        ++this->SP;
        u16 addr = n1 + (n2 << 8);
        this->PC = addr;
    }
}
void GB_CPU::RET_C() {
    this->cycles = 8;
    if (this->get_c()) {
        this->cycles = 20;
        u8 n1 = RAM::readAt(SP);
        ++this->SP;
        u16 n2 = RAM::readAt(SP);
        ++this->SP;
        u16 addr = n1 + (n2 << 8);
        this->PC = addr;
    }
}

void GB_CPU::RETI() {
    u8 n1 = RAM::readAt(SP);
    ++this->SP;
    u16 n2 = RAM::readAt(SP);
    ++this->SP;
    u16 addr = n1 + (n2 << 8);
    this->PC = addr;
    this->write(0xFF, 0xFFFF);
    this->cycles = 16;
}

void GB_CPU::BIT(u8 bit, u8 reg_) {
    u8 test = reg_ & bit;

    this->set_z(test != bit);
    this->set_n(false);
    this->set_h(true);
    this->cycles = 8;
}

void GB_CPU::BIT_0A() { this->BIT(1, this->AF.hi); }
void GB_CPU::BIT_1A() { this->BIT(0b00000010, this->AF.hi); }
void GB_CPU::BIT_2A() { this->BIT(0b00000100, this->AF.hi); }
void GB_CPU::BIT_3A() { this->BIT(0b00001000, this->AF.hi); }
void GB_CPU::BIT_4A() { this->BIT(0b00010000, this->AF.hi); }
void GB_CPU::BIT_5A() { this->BIT(0b00100000, this->AF.hi); }
void GB_CPU::BIT_6A() { this->BIT(0b01000000, this->AF.hi); }
void GB_CPU::BIT_7A() { this->BIT(0b10000000, this->AF.hi); }

void GB_CPU::BIT_0B() { this->BIT(1, this->BC.hi); }
void GB_CPU::BIT_1B() { this->BIT(0b00000010, this->BC.hi); }
void GB_CPU::BIT_2B() { this->BIT(0b00000100, this->BC.hi); }
void GB_CPU::BIT_3B() { this->BIT(0b00001000, this->BC.hi); }
void GB_CPU::BIT_4B() { this->BIT(0b00010000, this->BC.hi); }
void GB_CPU::BIT_5B() { this->BIT(0b00100000, this->BC.hi); }
void GB_CPU::BIT_6B() { this->BIT(0b01000000, this->BC.hi); }
void GB_CPU::BIT_7B() { this->BIT(0b10000000, this->BC.hi); }

void GB_CPU::BIT_0C() { this->BIT(1, this->BC.lo); }
void GB_CPU::BIT_1C() { this->BIT(0b00000010, this->BC.lo); }
void GB_CPU::BIT_2C() { this->BIT(0b00000100, this->BC.lo); }
void GB_CPU::BIT_3C() { this->BIT(0b00001000, this->BC.lo); }
void GB_CPU::BIT_4C() { this->BIT(0b00010000, this->BC.lo); }
void GB_CPU::BIT_5C() { this->BIT(0b00100000, this->BC.lo); }
void GB_CPU::BIT_6C() { this->BIT(0b01000000, this->BC.lo); }
void GB_CPU::BIT_7C() { this->BIT(0b10000000, this->BC.lo); }

void GB_CPU::BIT_0D() { this->BIT(1, this->DE.hi); }
void GB_CPU::BIT_1D() { this->BIT(0b00000010, this->DE.hi); }
void GB_CPU::BIT_2D() { this->BIT(0b00000100, this->DE.hi); }
void GB_CPU::BIT_3D() { this->BIT(0b00001000, this->DE.hi); }
void GB_CPU::BIT_4D() { this->BIT(0b00010000, this->DE.hi); }
void GB_CPU::BIT_5D() { this->BIT(0b00100000, this->DE.hi); }
void GB_CPU::BIT_6D() { this->BIT(0b01000000, this->DE.hi); }
void GB_CPU::BIT_7D() { this->BIT(0b10000000, this->DE.hi); }

void GB_CPU::BIT_0E() { this->BIT(1, this->DE.lo); }
void GB_CPU::BIT_1E() { this->BIT(0b00000010, this->DE.lo); }
void GB_CPU::BIT_2E() { this->BIT(0b00000100, this->DE.lo); }
void GB_CPU::BIT_3E() { this->BIT(0b00001000, this->DE.lo); }
void GB_CPU::BIT_4E() { this->BIT(0b00010000, this->DE.lo); }
void GB_CPU::BIT_5E() { this->BIT(0b00100000, this->DE.lo); }
void GB_CPU::BIT_6E() { this->BIT(0b01000000, this->DE.lo); }
void GB_CPU::BIT_7E() { this->BIT(0b10000000, this->DE.lo); }

void GB_CPU::BIT_0H() { this->BIT(1, this->HL.hi); }
void GB_CPU::BIT_1H() { this->BIT(0b00000010, this->HL.hi); }
void GB_CPU::BIT_2H() { this->BIT(0b00000100, this->HL.hi); }
void GB_CPU::BIT_3H() { this->BIT(0b00001000, this->HL.hi); }
void GB_CPU::BIT_4H() { this->BIT(0b00010000, this->HL.hi); }
void GB_CPU::BIT_5H() { this->BIT(0b00100000, this->HL.hi); }
void GB_CPU::BIT_6H() { this->BIT(0b01000000, this->HL.hi); }
void GB_CPU::BIT_7H() { this->BIT(0b10000000, this->HL.hi); }

void GB_CPU::BIT_0L() { this->BIT(1, this->HL.lo); }
void GB_CPU::BIT_1L() { this->BIT(0b00000010, this->HL.lo); }
void GB_CPU::BIT_2L() { this->BIT(0b00000100, this->HL.lo); }
void GB_CPU::BIT_3L() { this->BIT(0b00001000, this->HL.lo); }
void GB_CPU::BIT_4L() { this->BIT(0b00010000, this->HL.lo); }
void GB_CPU::BIT_5L() { this->BIT(0b00100000, this->HL.lo); }
void GB_CPU::BIT_6L() { this->BIT(0b01000000, this->HL.lo); }
void GB_CPU::BIT_7L() { this->BIT(0b10000000, this->HL.lo); }

void GB_CPU::BIT_0HL() { this->BIT(1, RAM::readAt(this->HL.val())); this->cycles = 12; }
void GB_CPU::BIT_1HL() { this->BIT(0b00000010, RAM::readAt(this->HL.val())); this->cycles = 12; }
void GB_CPU::BIT_2HL() { this->BIT(0b00000100, RAM::readAt(this->HL.val())); this->cycles = 12; }
void GB_CPU::BIT_3HL() { this->BIT(0b00001000, RAM::readAt(this->HL.val())); this->cycles = 12; }
void GB_CPU::BIT_4HL() { this->BIT(0b00010000, RAM::readAt(this->HL.val())); this->cycles = 12; }
void GB_CPU::BIT_5HL() { this->BIT(0b00100000, RAM::readAt(this->HL.val())); this->cycles = 12; }
void GB_CPU::BIT_6HL() { this->BIT(0b01000000, RAM::readAt(this->HL.val())); this->cycles = 12; }
void GB_CPU::BIT_7HL() { this->BIT(0b10000000, RAM::readAt(this->HL.val())); this->cycles = 12; }

u8 GB_CPU::AssignReg(u8 mask, u8 reg_, bool val) {
    if (val) {
        return reg_ | mask;
    } else {
        return reg_ & ~mask;
    }
}

u8 GB_CPU::RES(u8 bit, u8 reg_) {
    this->cycles = 8;
    return AssignReg(bit, reg_, false);
}

void GB_CPU::RES_0A() { this->AF.hi = this->RES(1, this->AF.hi); }
void GB_CPU::RES_1A() { this->AF.hi = this->RES(0b00000010, this->AF.hi); }
void GB_CPU::RES_2A() { this->AF.hi = this->RES(0b00000100, this->AF.hi); }
void GB_CPU::RES_3A() { this->AF.hi = this->RES(0b00001000, this->AF.hi); }
void GB_CPU::RES_4A() { this->AF.hi = this->RES(0b00010000, this->AF.hi); }
void GB_CPU::RES_5A() { this->AF.hi = this->RES(0b00100000, this->AF.hi); }
void GB_CPU::RES_6A() { this->AF.hi = this->RES(0b01000000, this->AF.hi); }
void GB_CPU::RES_7A() { this->AF.hi = this->RES(0b10000000, this->AF.hi); }

void GB_CPU::RES_0B() { this->BC.hi = this->RES(1, this->BC.hi); }
void GB_CPU::RES_1B() { this->BC.hi = this->RES(0b00000010, this->BC.hi); }
void GB_CPU::RES_2B() { this->BC.hi = this->RES(0b00000100, this->BC.hi); }
void GB_CPU::RES_3B() { this->BC.hi = this->RES(0b00001000, this->BC.hi); }
void GB_CPU::RES_4B() { this->BC.hi = this->RES(0b00010000, this->BC.hi); }
void GB_CPU::RES_5B() { this->BC.hi = this->RES(0b00100000, this->BC.hi); }
void GB_CPU::RES_6B() { this->BC.hi = this->RES(0b01000000, this->BC.hi); }
void GB_CPU::RES_7B() { this->BC.hi = this->RES(0b10000000, this->BC.hi); }

void GB_CPU::RES_0C() { this->BC.lo = this->RES(1, this->BC.lo); }
void GB_CPU::RES_1C() { this->BC.lo = this->RES(0b00000010, this->BC.lo); }
void GB_CPU::RES_2C() { this->BC.lo = this->RES(0b00000100, this->BC.lo); }
void GB_CPU::RES_3C() { this->BC.lo = this->RES(0b00001000, this->BC.lo); }
void GB_CPU::RES_4C() { this->BC.lo = this->RES(0b00010000, this->BC.lo); }
void GB_CPU::RES_5C() { this->BC.lo = this->RES(0b00100000, this->BC.lo); }
void GB_CPU::RES_6C() { this->BC.lo = this->RES(0b01000000, this->BC.lo); }
void GB_CPU::RES_7C() { this->BC.lo = this->RES(0b10000000, this->BC.lo); }

void GB_CPU::RES_0D() { this->DE.hi = this->RES(1, this->DE.hi); }
void GB_CPU::RES_1D() { this->DE.hi = this->RES(0b00000010, this->DE.hi); }
void GB_CPU::RES_2D() { this->DE.hi = this->RES(0b00000100, this->DE.hi); }
void GB_CPU::RES_3D() { this->DE.hi = this->RES(0b00001000, this->DE.hi); }
void GB_CPU::RES_4D() { this->DE.hi = this->RES(0b00010000, this->DE.hi); }
void GB_CPU::RES_5D() { this->DE.hi = this->RES(0b00100000, this->DE.hi); }
void GB_CPU::RES_6D() { this->DE.hi = this->RES(0b01000000, this->DE.hi); }
void GB_CPU::RES_7D() { this->DE.hi = this->RES(0b10000000, this->DE.hi); }

void GB_CPU::RES_0E() { this->DE.lo = this->RES(1, this->DE.lo); }
void GB_CPU::RES_1E() { this->DE.lo = this->RES(0b00000010, this->DE.lo); }
void GB_CPU::RES_2E() { this->DE.lo = this->RES(0b00000100, this->DE.lo); }
void GB_CPU::RES_3E() { this->DE.lo = this->RES(0b00001000, this->DE.lo); }
void GB_CPU::RES_4E() { this->DE.lo = this->RES(0b00010000, this->DE.lo); }
void GB_CPU::RES_5E() { this->DE.lo = this->RES(0b00100000, this->DE.lo); }
void GB_CPU::RES_6E() { this->DE.lo = this->RES(0b01000000, this->DE.lo); }
void GB_CPU::RES_7E() { this->DE.lo = this->RES(0b10000000, this->DE.lo); }

void GB_CPU::RES_0H() { this->HL.hi = this->RES(1, this->HL.hi); }
void GB_CPU::RES_1H() { this->HL.hi = this->RES(0b00000010, this->HL.hi); }
void GB_CPU::RES_2H() { this->HL.hi = this->RES(0b00000100, this->HL.hi); }
void GB_CPU::RES_3H() { this->HL.hi = this->RES(0b00001000, this->HL.hi); }
void GB_CPU::RES_4H() { this->HL.hi = this->RES(0b00010000, this->HL.hi); }
void GB_CPU::RES_5H() { this->HL.hi = this->RES(0b00100000, this->HL.hi); }
void GB_CPU::RES_6H() { this->HL.hi = this->RES(0b01000000, this->HL.hi); }
void GB_CPU::RES_7H() { this->HL.hi = this->RES(0b10000000, this->HL.hi); }

void GB_CPU::RES_0L() { this->HL.lo = this->RES(1, this->HL.lo); }
void GB_CPU::RES_1L() { this->HL.lo = this->RES(0b00000010, this->HL.lo); }
void GB_CPU::RES_2L() { this->HL.lo = this->RES(0b00000100, this->HL.lo); }
void GB_CPU::RES_3L() { this->HL.lo = this->RES(0b00001000, this->HL.lo); }
void GB_CPU::RES_4L() { this->HL.lo = this->RES(0b00010000, this->HL.lo); }
void GB_CPU::RES_5L() { this->HL.lo = this->RES(0b00100000, this->HL.lo); }
void GB_CPU::RES_6L() { this->HL.lo = this->RES(0b01000000, this->HL.lo); }
void GB_CPU::RES_7L() { this->HL.lo = this->RES(0b10000000, this->HL.lo); }

void GB_CPU::RES_0HL() { RAM::write(this->RES(1, RAM::readAt(this->HL.val())), this->HL.val()); this->cycles = 16; }
void GB_CPU::RES_1HL() { RAM::write(this->RES(0b00000010, RAM::readAt(this->HL.val())), this->HL.val()); this->cycles = 16; }
void GB_CPU::RES_2HL() { RAM::write(this->RES(0b00000100, RAM::readAt(this->HL.val())), this->HL.val()); this->cycles = 16; }
void GB_CPU::RES_3HL() { RAM::write(this->RES(0b00001000, RAM::readAt(this->HL.val())), this->HL.val()); this->cycles = 16; }
void GB_CPU::RES_4HL() { RAM::write(this->RES(0b00010000, RAM::readAt(this->HL.val())), this->HL.val()); this->cycles = 16; }
void GB_CPU::RES_5HL() { RAM::write(this->RES(0b00100000, RAM::readAt(this->HL.val())), this->HL.val()); this->cycles = 16; }
void GB_CPU::RES_6HL() { RAM::write(this->RES(0b01000000, RAM::readAt(this->HL.val())), this->HL.val()); this->cycles = 16; }
void GB_CPU::RES_7HL() { RAM::write(this->RES(0b10000000, RAM::readAt(this->HL.val())), this->HL.val()); this->cycles = 16; }

u8 GB_CPU::SET(u8 bit, u8 reg_) {
    this->cycles = 8;
    return AssignReg(bit, reg_, true);
}

void GB_CPU::SET_0A() { this->AF.hi = this->SET(1, this->AF.hi); }
void GB_CPU::SET_1A() { this->AF.hi = this->SET(0b00000010, this->AF.hi); }
void GB_CPU::SET_2A() { this->AF.hi = this->SET(0b00000100, this->AF.hi); }
void GB_CPU::SET_3A() { this->AF.hi = this->SET(0b00001000, this->AF.hi); }
void GB_CPU::SET_4A() { this->AF.hi = this->SET(0b00010000, this->AF.hi); }
void GB_CPU::SET_5A() { this->AF.hi = this->SET(0b00100000, this->AF.hi); }
void GB_CPU::SET_6A() { this->AF.hi = this->SET(0b01000000, this->AF.hi); }
void GB_CPU::SET_7A() { this->AF.hi = this->SET(0b10000000, this->AF.hi); }

void GB_CPU::SET_0B() { this->BC.hi = this->SET(1, this->BC.hi); }
void GB_CPU::SET_1B() { this->BC.hi = this->SET(0b00000010, this->BC.hi); }
void GB_CPU::SET_2B() { this->BC.hi = this->SET(0b00000100, this->BC.hi); }
void GB_CPU::SET_3B() { this->BC.hi = this->SET(0b00001000, this->BC.hi); }
void GB_CPU::SET_4B() { this->BC.hi = this->SET(0b00010000, this->BC.hi); }
void GB_CPU::SET_5B() { this->BC.hi = this->SET(0b00100000, this->BC.hi); }
void GB_CPU::SET_6B() { this->BC.hi = this->SET(0b01000000, this->BC.hi); }
void GB_CPU::SET_7B() { this->BC.hi = this->SET(0b10000000, this->BC.hi); }

void GB_CPU::SET_0C() { this->BC.lo = this->SET(1, this->BC.lo); }
void GB_CPU::SET_1C() { this->BC.lo = this->SET(0b00000010, this->BC.lo); }
void GB_CPU::SET_2C() { this->BC.lo = this->SET(0b00000100, this->BC.lo); }
void GB_CPU::SET_3C() { this->BC.lo = this->SET(0b00001000, this->BC.lo); }
void GB_CPU::SET_4C() { this->BC.lo = this->SET(0b00010000, this->BC.lo); }
void GB_CPU::SET_5C() { this->BC.lo = this->SET(0b00100000, this->BC.lo); }
void GB_CPU::SET_6C() { this->BC.lo = this->SET(0b01000000, this->BC.lo); }
void GB_CPU::SET_7C() { this->BC.lo = this->SET(0b10000000, this->BC.lo); }

void GB_CPU::SET_0D() { this->DE.hi = this->SET(1, this->DE.hi); }
void GB_CPU::SET_1D() { this->DE.hi = this->SET(0b00000010, this->DE.hi); }
void GB_CPU::SET_2D() { this->DE.hi = this->SET(0b00000100, this->DE.hi); }
void GB_CPU::SET_3D() { this->DE.hi = this->SET(0b00001000, this->DE.hi); }
void GB_CPU::SET_4D() { this->DE.hi = this->SET(0b00010000, this->DE.hi); }
void GB_CPU::SET_5D() { this->DE.hi = this->SET(0b00100000, this->DE.hi); }
void GB_CPU::SET_6D() { this->DE.hi = this->SET(0b01000000, this->DE.hi); }
void GB_CPU::SET_7D() { this->DE.hi = this->SET(0b10000000, this->DE.hi); }

void GB_CPU::SET_0E() { this->DE.lo = this->SET(1, this->DE.lo); }
void GB_CPU::SET_1E() { this->DE.lo = this->SET(0b00000010, this->DE.lo); }
void GB_CPU::SET_2E() { this->DE.lo = this->SET(0b00000100, this->DE.lo); }
void GB_CPU::SET_3E() { this->DE.lo = this->SET(0b00001000, this->DE.lo); }
void GB_CPU::SET_4E() { this->DE.lo = this->SET(0b00010000, this->DE.lo); }
void GB_CPU::SET_5E() { this->DE.lo = this->SET(0b00100000, this->DE.lo); }
void GB_CPU::SET_6E() { this->DE.lo = this->SET(0b01000000, this->DE.lo); }
void GB_CPU::SET_7E() { this->DE.lo = this->SET(0b10000000, this->DE.lo); }

void GB_CPU::SET_0H() { this->HL.hi = this->SET(1, this->HL.hi); }
void GB_CPU::SET_1H() { this->HL.hi = this->SET(0b00000010, this->HL.hi); }
void GB_CPU::SET_2H() { this->HL.hi = this->SET(0b00000100, this->HL.hi); }
void GB_CPU::SET_3H() { this->HL.hi = this->SET(0b00001000, this->HL.hi); }
void GB_CPU::SET_4H() { this->HL.hi = this->SET(0b00010000, this->HL.hi); }
void GB_CPU::SET_5H() { this->HL.hi = this->SET(0b00100000, this->HL.hi); }
void GB_CPU::SET_6H() { this->HL.hi = this->SET(0b01000000, this->HL.hi); }
void GB_CPU::SET_7H() { this->HL.hi = this->SET(0b10000000, this->HL.hi); }

void GB_CPU::SET_0L() { this->HL.lo = this->SET(1, this->HL.lo); }
void GB_CPU::SET_1L() { this->HL.lo = this->SET(0b00000010, this->HL.lo); }
void GB_CPU::SET_2L() { this->HL.lo = this->SET(0b00000100, this->HL.lo); }
void GB_CPU::SET_3L() { this->HL.lo = this->SET(0b00001000, this->HL.lo); }
void GB_CPU::SET_4L() { this->HL.lo = this->SET(0b00010000, this->HL.lo); }
void GB_CPU::SET_5L() { this->HL.lo = this->SET(0b00100000, this->HL.lo); }
void GB_CPU::SET_6L() { this->HL.lo = this->SET(0b01000000, this->HL.lo); }
void GB_CPU::SET_7L() { this->HL.lo = this->SET(0b10000000, this->HL.lo); }

void GB_CPU::SET_0HL() { RAM::write(this->SET(1, RAM::readAt(this->HL.val())), this->HL.val()); this->cycles = 16; }
void GB_CPU::SET_1HL() { RAM::write(this->SET(0b00000010, RAM::readAt(this->HL.val())), this->HL.val()); this->cycles = 16; }
void GB_CPU::SET_2HL() { RAM::write(this->SET(0b00000100, RAM::readAt(this->HL.val())), this->HL.val()); this->cycles = 16; }
void GB_CPU::SET_3HL() { RAM::write(this->SET(0b00001000, RAM::readAt(this->HL.val())), this->HL.val()); this->cycles = 16; }
void GB_CPU::SET_4HL() { RAM::write(this->SET(0b00010000, RAM::readAt(this->HL.val())), this->HL.val()); this->cycles = 16; }
void GB_CPU::SET_5HL() { RAM::write(this->SET(0b00100000, RAM::readAt(this->HL.val())), this->HL.val()); this->cycles = 16; }
void GB_CPU::SET_6HL() { RAM::write(this->SET(0b01000000, RAM::readAt(this->HL.val())), this->HL.val()); this->cycles = 16; }
void GB_CPU::SET_7HL() { RAM::write(this->SET(0b10000000, RAM::readAt(this->HL.val())), this->HL.val()); this->cycles = 16; }

void GB_CPU::RRA() {
    u8 carry = (this->AF.lo & 0b00010000) << 3;
    this->AF.lo = (this->AF.hi & 0b00000001) << 4;
    this->AF.lo &= 0b10010000;
    this->AF.hi = (this->AF.hi >> 1) + carry;
    if (this->AF.hi == 0) { this->AF.lo |= 0b10000000; } else { this->AF.lo &= 0b01110000; }
    this->cycles = 4;
}

void GB_CPU::RLC_A() {
    u8 carry = (this->AF.hi & 0b10000000) >> 7;
    this->AF.lo = (this->AF.hi & 0b10000000) >> 3;
    this->AF.lo &= 0b10010000;
    this->AF.hi = (this->AF.hi << 1) + carry;
    if (this->AF.hi == 0) { this->AF.lo |= 0b10000000; } else { this->AF.lo &= 0b01110000; }
    this->cycles = 8;
}
void GB_CPU::RLC_B() {
    u8 carry = (this->BC.hi & 0b10000000) >> 7;
    this->AF.lo = (this->BC.hi & 0b10000000) >> 3;
    this->AF.lo &= 0b10010000;
    this->BC.hi = (this->BC.hi << 1) + carry;
    if (this->BC.hi == 0) { this->AF.lo |= 0b10000000; } else { this->AF.lo &= 0b01110000; }
    this->cycles = 8;
}
void GB_CPU::RLC_C() {
    u8 carry = (this->BC.lo & 0b10000000) >> 7;
    this->AF.lo = (this->BC.lo & 0b10000000) >> 3;
    this->AF.lo &= 0b10010000;
    this->BC.lo = (this->BC.lo << 1) + carry;
    if (this->BC.lo == 0) { this->AF.lo |= 0b10000000; } else { this->AF.lo &= 0b01110000; }
    this->cycles = 8;
}
void GB_CPU::RLC_D() {
    u8 carry = (this->DE.hi & 0b10000000) >> 7;
    this->AF.lo = (this->DE.hi & 0b10000000) >> 3;
    this->AF.lo &= 0b10010000;
    this->DE.hi = (this->DE.hi << 1) + carry;
    if (this->DE.hi == 0) { this->AF.lo |= 0b10000000; } else { this->AF.lo &= 0b01110000; }
    this->cycles = 8;
}
void GB_CPU::RLC_E() {
    u8 carry = (this->DE.lo & 0b10000000) >> 7;
    this->AF.lo = (this->DE.lo & 0b10000000) >> 3;
    this->AF.lo &= 0b10010000;
    this->DE.lo = (this->DE.lo << 1) + carry;
    if (this->DE.lo == 0) { this->AF.lo |= 0b10000000; } else { this->AF.lo &= 0b01110000; }
    this->cycles = 8;
}
void GB_CPU::RLC_H() {
    u8 carry = (this->HL.hi & 0b10000000) >> 7;
    this->AF.lo = (this->HL.hi & 0b10000000) >> 3;
    this->AF.lo &= 0b10010000;
    this->HL.hi = (this->HL.hi << 1) + carry;
    if (this->HL.hi == 0) { this->AF.lo |= 0b10000000; } else { this->AF.lo &= 0b01110000; }
    this->cycles = 8;
}
void GB_CPU::RLC_L() {
    u8 carry = (this->HL.lo & 0b10000000) >> 7;
    this->AF.lo = (this->HL.lo & 0b10000000) >> 3;
    this->AF.lo &= 0b10010000;
    this->HL.lo = (this->HL.lo << 1) + carry;
    if (this->HL.lo == 0) { this->AF.lo |= 0b10000000; } else { this->AF.lo &= 0b01110000; }
    this->cycles = 8;
}

u8 GB_CPU::RL_generic(u8 val) {
    u8 bit7 = val & 0b10000000;
    u8 newVal = (val << 1) + this->get_c();
    this->set_z(newVal == 0);
    this->set_n(false);
    this->set_h(false);
    this->set_c(bit7 == 0b10000000);
    return newVal;
}

void GB_CPU::RL_A() {
    this->AF.hi = this->RL_generic(this->AF.hi);
    this->cycles = 8;
}

void GB_CPU::RL_B() {
    this->BC.hi = this->RL_generic(this->BC.hi);
    this->cycles = 8;
}

void GB_CPU::RL_C() {
    this->BC.lo = this->RL_generic(this->BC.lo);
    this->cycles = 8;
}

void GB_CPU::RL_D() {
    this->DE.hi = this->RL_generic(this->DE.hi);
    this->cycles = 8;
}
void GB_CPU::RL_E() {
    this->DE.lo = this->RL_generic(this->DE.lo);
    this->cycles = 8;
}
void GB_CPU::RL_H() {
    this->HL.hi = this->RL_generic(this->HL.hi);
    this->cycles = 8;
}
void GB_CPU::RL_L() {
    this->HL.lo = this->RL_generic(this->HL.lo);
    this->cycles = 8;
}
void GB_CPU::RL_addr_HL() {
    u8 readVal = RAM::readAt(this->HL.val());
    RAM::write(this->RL_generic(readVal), this->HL.val());
    this->cycles = 16;
}
void GB_CPU::RLC_HL() {
    u8 x = RAM::readAt(this->HL.val());
    u8 carry = (x & 0b10000000) >> 7;
    this->AF.lo = (x & 0b10000000) >> 3;
    this->AF.lo &= 0b10010000;
    x = (x << 1) + carry;
    this->write(x, this->HL.val());
    if (x == 0) { this->AF.lo |= 0b10000000; } else { this->AF.lo &= 0b01110000; }
    this->cycles = 16;
}


void GB_CPU::JR_NZ() {
    if (!this->get_z()) {
        s8 b = this->read();
        this->PC += b;
        this->cycles = 12;

    } else {
        ++this->PC;
        this->cycles = 8;
    }
}

void GB_CPU::JR_Z() {
    if ((this->AF.lo >> 7) == 1) {
        s8 b = this->read();
        this->PC += b;
        this->cycles = 12;
    } else {
        ++this->PC;
        this->cycles = 8;
    }
}

void GB_CPU::JR_NC() {
    if (!this->get_c()) {
        s8 b = this->read();
        this->PC += b;
        this->cycles = 12;
    } else {
        ++this->PC;
        this->cycles = 8;
    }
}

void GB_CPU::JR_C() {
    if (this->get_c()) {
        s8 b = this->read();
        this->PC += b;
        this->cycles = 12;
    } else {
        ++this->PC;
        this->cycles = 8;
    }
}

void GB_CPU::JR_n() {
    s8 jump = this->read();
    this->PC += jump;
    this->cycles = 12;
}

void GB_CPU::JP() {
    u8 n1, n2;
    n1 = this->read();
    n2 = this->read();
    this->PC = n1 + (n2 << 8);
    this->cycles = 12;
}

void GB_CPU::JP_NZ() {
    u8 n1, n2;
    if (!this->get_z()) {
        n1 = this->read();
        n2 = this->read();
        this->PC = n1 + (n2 << 8);
    } else { this->PC += 2; }
    this->cycles = 12;
}

void GB_CPU::JP_Z() {
    u8 n1, n2;
    if (this->get_z()) {
        n1 = this->read();
        n2 = this->read();
        this->PC = n1 + (n2 << 8);
    } else { this->PC += 2; }
    this->cycles = 12;
}

void GB_CPU::JP_NC() {
    u8 n1, n2;
    if (!this->get_c()) {
        n1 = this->read();
        n2 = this->read();
        this->PC = n1 + (n2 << 8);
    } else { this->PC += 2; }
    this->cycles = 12;
}
void GB_CPU::JP_C() {
    u8 n1, n2;
    if (this->get_c()) {
        n1 = this->read();
        n2 = this->read();
        this->PC = n1 + (n2 << 8);
    } else { this->PC += 2; }
    this->cycles = 12;
}

void GB_CPU::JP_HL() {
    this->PC = this->HL.val();
    this->cycles = 4;
}

void GB_CPU::LDc_n() {
    u8 n = this->read();
    this->BC.lo = n;
}

void GB_CPU::LDb_n() {
    u8 n = this->read();
    this->BC.hi = n;
}
void GB_CPU::LDd_n() {
    u8 n = this->read();
    this->DE.hi = n;
}
void GB_CPU::LDe_n() {
    u8 n = this->read();
    this->DE.lo = n;
}
void GB_CPU::LDh_n() {
    u8 n = this->read();
    this->HL.hi = n;
}

void GB_CPU::LDl_n() {
    u8 n = this->read();
    this->HL.lo = n;
}

void  GB_CPU::LDrr_a_hash() {
    u8 byte = this->read();
    this->AF.hi = byte;
}

void GB_CPU::LD_HL_a() {
    this->write(this->AF.hi, this->HL.val());
}
void GB_CPU::LD_BC_a() { this->write(this->AF.hi, this->BC.val()); }
void GB_CPU::LD_DE_a() { this->write(this->AF.hi, this->DE.val()); }

void GB_CPU::LDad_n_a() { u8 add = this->read(); u16 addr = add + 0xFF00; this->write(this->AF.hi, addr); this->cycles = 12; }

void GB_CPU::CALL_nn() {
    u8 n1 = this->read();
    u8 n2 = this->read();

    u8 loNib = this->PC & 0xFF;
    u8 hiNib = (this->PC >> 8) & 0xFF;

    --this->SP;
    this->write(hiNib, this->SP);
    --this->SP;
    this->write(loNib, this->SP);

    this->PC = n1 + (n2 << 8);
    this->cycles = 12;
}

void GB_CPU::CALL_NZ() {
    u8 F = this->AF.lo;
    u8 Z = (F & 0b10000000) >> 7;
    u8 n1 = this->read();
    u8 n2 = this->read();
    if (Z == 0) {
        u8 loNib = this->PC & 0xFF;
        u8 hiNib = (this->PC >> 8) & 0xFF;

        --this->SP;
        this->write(hiNib, this->SP);
        --this->SP;
        this->write(loNib, this->SP);

        this->PC = n1 + (n2 << 8);
        this->cycles = 12;
    }
}
void GB_CPU::CALL_Z() {
    u8 F = this->AF.lo;
    u8 Z = (F & 0b10000000) >> 7;
    u8 n1 = this->read();
    u8 n2 = this->read();
    if (Z == 1) {
        u8 loNib = this->PC & 0xFF;
        u8 hiNib = (this->PC >> 8) & 0xFF;

        --this->SP;
        this->write(hiNib, this->SP);
        --this->SP;
        this->write(loNib, this->SP);

        this->PC = n1 + (n2 << 8);
        this->cycles = 12;
    }
}
void GB_CPU::CALL_NC() {
    u8 n1 = this->read();
    u8 n2 = this->read();
    if (!this->get_c()) {
        u8 loNib = this->PC & 0xFF;
        u8 hiNib = (this->PC >> 8) & 0xFF;

        --this->SP;
        this->write(hiNib, this->SP);
        --this->SP;
        this->write(loNib, this->SP);

        this->PC = n1 + (n2 << 8);
        this->cycles = 12;
    }
}

void GB_CPU::CALL_C() {
    u8 n1 = this->read();
    u8 n2 = this->read();
    if (this->get_c()) {
        u8 loNib = this->PC & 0xFF;
        u8 hiNib = (this->PC >> 8) & 0xFF;

        --this->SP;
        this->write(hiNib, this->SP);
        --this->SP;
        this->write(loNib, this->SP);

        this->PC = n1 + (n2 << 8);
        this->cycles = 12;
    }
}

void GB_CPU::LDI_HLa() {
    this->write(this->AF.hi, this->HL.val());
    u16 x = this->HL.val() + 1;
    this->HL.set(x);
}

void GB_CPU::LD_nn_a() {
    u8 n1 = this->read();
    u8 n2 = this->read();
    u16 addr = n1 + (n2 << 8);
    this->write(this->AF.hi, addr);
}

void GB_CPU::LDH_a_ffn() {
    u8 n = this->read();
    this->AF.hi = RAM::readAt(0xFF00 + n);
    this->cycles = 12;
}

void GB_CPU::LDrr_aa() { this->cycles = 4; }
void GB_CPU::LDrr_ab() { this->AF.hi = this->BC.hi; this->cycles = 4; }
void GB_CPU::LDrr_ac() { this->AF.hi = this->BC.lo; this->cycles = 4; }
void GB_CPU::LDrr_ad() { this->AF.hi = this->DE.hi; this->cycles = 4; }
void GB_CPU::LDrr_ae() { this->AF.hi = this->DE.lo; this->cycles = 4; }
void GB_CPU::LDrr_ah() { this->AF.hi = this->HL.hi; this->cycles = 4; }
void GB_CPU::LDrr_al() { this->AF.hi = this->HL.lo; this->cycles = 4; }
void GB_CPU::LDrr_aBC() { this->AF.hi = RAM::readAt(this->BC.val()); }
void GB_CPU::LDrr_aDE() { this->AF.hi = RAM::readAt(this->DE.val()); }
void GB_CPU::LDrr_aHL() { this->AF.hi = RAM::readAt(this->HL.val()); }
void GB_CPU::LDrr_bb() { this->cycles = 4; }
void GB_CPU::LDrr_ba() { this->BC.hi = this->AF.hi; this->cycles = 4; }
void GB_CPU::LDrr_bc() { this->BC.hi = this->BC.lo; this->cycles = 4; }
void GB_CPU::LDrr_bd() { this->BC.hi = this->DE.hi; this->cycles = 4; }
void GB_CPU::LDrr_be() { this->BC.hi = this->DE.lo; this->cycles = 4; }
void GB_CPU::LDrr_bh() { this->BC.hi = this->HL.hi; this->cycles = 4; }
void GB_CPU::LDrr_bl() { this->BC.hi = this->HL.lo; this->cycles = 4; }
void GB_CPU::LDrr_bHL() { this->BC.hi = RAM::readAt(this->HL.val()); }
void GB_CPU::LDrr_cc() { this->cycles = 4; }
void GB_CPU::LDrr_ca() { this->BC.lo = this->AF.hi; this->cycles = 4; }
void GB_CPU::LDrr_cb() { this->BC.lo = this->BC.hi; this->cycles = 4; }
void GB_CPU::LDrr_cd() { this->BC.lo = this->DE.hi; this->cycles = 4; }
void GB_CPU::LDrr_ce() { this->BC.lo = this->DE.lo; this->cycles = 4; }
void GB_CPU::LDrr_ch() { this->BC.lo = this->HL.hi; this->cycles = 4; }
void GB_CPU::LDrr_cl() { this->BC.lo = this->HL.lo; this->cycles = 4; }
void GB_CPU::LDrr_cHL() { this->BC.lo = RAM::readAt(this->HL.val()); }
void GB_CPU::LDrr_dd() { this->cycles = 4; }
void GB_CPU::LDrr_da() { this->DE.hi = this->AF.hi; this->cycles = 4; }
void GB_CPU::LDrr_db() { this->DE.hi = this->BC.hi; this->cycles = 4; }
void GB_CPU::LDrr_dc() { this->DE.hi = this->BC.lo; this->cycles = 4; }
void GB_CPU::LDrr_de() { this->DE.hi = this->DE.lo; this->cycles = 4; }
void GB_CPU::LDrr_dh() { this->DE.hi = this->HL.hi; this->cycles = 4; }
void GB_CPU::LDrr_dl() { this->DE.hi = this->HL.lo; this->cycles = 4; }
void GB_CPU::LDrr_dHL() { this->DE.hi = RAM::readAt(this->HL.val()); }
void GB_CPU::LDrr_ee() { this->cycles = 4; }
void GB_CPU::LDrr_ea() { this->DE.lo = this->AF.hi; this->cycles = 4; }
void GB_CPU::LDrr_eb() { this->DE.lo = this->BC.hi; this->cycles = 4; }
void GB_CPU::LDrr_ec() { this->DE.lo = this->BC.lo; this->cycles = 4; }
void GB_CPU::LDrr_ed() { this->DE.lo = this->DE.hi; this->cycles = 4; }
void GB_CPU::LDrr_eh() { this->DE.lo = this->HL.hi; this->cycles = 4; }
void GB_CPU::LDrr_el() { this->DE.lo = this->HL.lo; this->cycles = 4; }
void GB_CPU::LDrr_eHL() { this->DE.lo = RAM::readAt(this->HL.val()); }
void GB_CPU::LDrr_hh() { this->cycles = 4; }
void GB_CPU::LDrr_ha() { this->HL.hi = this->AF.hi; this->cycles = 4; }
void GB_CPU::LDrr_hb() { this->HL.hi = this->BC.hi; this->cycles = 4; }
void GB_CPU::LDrr_hc() { this->HL.hi = this->BC.lo; this->cycles = 4; }
void GB_CPU::LDrr_hd() { this->HL.hi = this->DE.hi; this->cycles = 4; }
void GB_CPU::LDrr_he() { this->HL.hi = this->DE.lo; this->cycles = 4; }
void GB_CPU::LDrr_hl() { this->HL.hi = this->HL.lo; this->cycles = 4; }
void GB_CPU::LDrr_hHL() { this->HL.hi = RAM::readAt(this->HL.val()); }
void GB_CPU::LDrr_ll() { this->cycles = 4; }
void GB_CPU::LDrr_la() { this->HL.lo = this->AF.hi; this->cycles = 4; }
void GB_CPU::LDrr_lb() { this->HL.lo = this->BC.hi; this->cycles = 4; }
void GB_CPU::LDrr_lc() { this->HL.lo = this->BC.lo; this->cycles = 4; }
void GB_CPU::LDrr_ld() { this->HL.lo = this->DE.hi; this->cycles = 4; }
void GB_CPU::LDrr_le() { this->HL.lo = this->DE.lo; this->cycles = 4; }
void GB_CPU::LDrr_lh() { this->HL.lo = this->HL.hi; this->cycles = 4; }
void GB_CPU::LDrr_lHL() { this->HL.lo = RAM::readAt(this->HL.val()); }
void GB_CPU::LDrr_HLb() { this->write(this->BC.hi, this->HL.val()); this->cycles = 8; }
void GB_CPU::LDrr_HLc() { this->write(this->BC.lo, this->HL.val()); this->cycles = 8; }
void GB_CPU::LDrr_HLd() { this->write(this->DE.hi, this->HL.val()); this->cycles = 8; }
void GB_CPU::LDrr_HLe() { this->write(this->DE.lo, this->HL.val()); this->cycles = 8; }
void GB_CPU::LDrr_HLh() { this->write(this->HL.hi, this->HL.val()); this->cycles = 8; }
void GB_CPU::LDrr_HLl() { this->write(this->HL.lo, this->HL.val()); this->cycles = 8; }
void GB_CPU::LDrr_HLn() { this->write(read(), this->HL.val()); this->cycles = 12; }
void GB_CPU::LDrr_ann() {
    u8 n1 = this->read(), n2 = this->read();
    u16 addr = n1 + (n2 << 8);
    this->AF.hi = RAM::readAt(addr);
    this->cycles = 16;
}

void GB_CPU::LDa_c() { this->AF.hi = RAM::readAt(0xFF00 + this->BC.lo); }
void GB_CPU::LDc_a() { this->write(this->AF.hi, 0xFF00 + this->BC.lo); }

void GB_CPU::LDDaHL() { this->AF.hi = RAM::readAt(this->HL.val()); this->HL.set(this->HL.val() - 1); }
void GB_CPU::LDDHLa() { this->write(this->AF.hi, this->HL.val()); this->HL.set(this->HL.val() - 1); }

void GB_CPU::LD_nn_BC() {
    u8 n1 = this->read();
    u8 n2 = this->read();
    u16 nn = n1 + (n2 << 8);
    this->BC.set(nn);
}
void GB_CPU::LD_nn_DE() { u8 n1 = this->read(); u8 n2 = this->read(); u16 nn = n1 + (n2 << 8); this->DE.set(nn); }
void GB_CPU::LD_nn_HL() { u8 n1 = this->read(); u8 n2 = this->read(); u16 nn = n1 + (n2 << 8); this->HL.set(nn); }
void GB_CPU::LD_nn_SP() { u8 n1 = this->read(); u8 n2 = this->read(); u16 nn = n1 + (n2 << 8); this->SP = nn; }

void GB_CPU::LD_SPHL() { this->SP = this->HL.val(); }

void GB_CPU::LDHL_SPn() {
    s8 n = this->read();
    this->HL.set(SP + n);
    u8 point = this->SP + n;

    this->set_z(false);
    this->set_n(false);
    this->set_c((point & 0xFF) < (SP & 0xFF));
    this->set_h((SP & 0x0F) + (n & 0x0F) > 0x0F);

    this->cycles = 12;
}

void GB_CPU::LD_nnSP() {
    u8 n1 = this->read();
    u8 n2 = this->read();
    u16 nn = n1 + (n2 << 8);
    RAM::write(SP & 0x00FF, nn);
    RAM::write((SP & 0xFF00) >> 8, nn + 1);


    this->cycles = 20;
}

void GB_CPU::PUSH_AF() {
    this->SP--;
    this->write(this->AF.hi, this->SP);
    this->SP--;
    this->write(this->AF.lo & 0xF0, this->SP);
    this->cycles = 16;
}
void GB_CPU::PUSH_BC() { this->SP--; this->write(this->BC.hi, this->SP); this->SP--; this->write(this->BC.lo, this->SP); this->cycles = 16; }
void GB_CPU::PUSH_DE() { this->SP--; this->write(this->DE.hi, this->SP); this->SP--; this->write(this->DE.lo, this->SP); this->cycles = 16; }
void GB_CPU::PUSH_HL() { this->SP--; this->write(this->HL.hi, this->SP); this->SP--; this->write(this->HL.lo, this->SP); this->cycles = 16; }

void GB_CPU::POP_AF() {
    u8 n = RAM::readAt(SP);
    this->AF.lo = n & 0xF0;
    ++this->SP;
    u8 m = RAM::readAt(SP);
    this->AF.hi = m;
    ++this->SP;

    this->cycles = 12;
}
void GB_CPU::POP_BC() { u8 n = RAM::readAt(SP); this->BC.lo = n; ++this->SP; u8 m = RAM::readAt(SP); this->BC.hi = m; ++this->SP; this->cycles = 12; }
void GB_CPU::POP_DE() { u8 n = RAM::readAt(SP); this->DE.lo = n; ++this->SP; u8 m = RAM::readAt(SP); this->DE.hi = m; ++this->SP; this->cycles = 12; }
void GB_CPU::POP_HL() { u8 n = RAM::readAt(SP); this->HL.lo = n; ++this->SP; u8 m = RAM::readAt(SP); this->HL.hi = m; ++this->SP; this->cycles = 12; }



void GB_CPU::ADD_aa() {
    u8 x = this->AF.hi;
    u8 loNib = this->AF.hi & 0x0F;
    this->AF.hi += this->AF.hi;
    if (this->AF.hi == 0) { this->AF.lo = this->AF.lo | 0x80; } else { this->AF.lo = 0; }
    this->AF.lo = this->AF.lo & 0b10110000;
    this->set_h((this->AF.hi & 0x0F) < loNib);
    this->set_c(this->AF.hi < x);
    this->cycles = 4;
}
void GB_CPU::ADD_ab() {
    u8 x = this->AF.hi;
    u8 loNib = this->AF.hi & 0x0F;
    this->AF.hi += this->BC.hi;
    if (this->AF.hi == 0) { this->AF.lo = this->AF.lo | 0x80; } else { this->AF.lo = 0; }
    this->AF.lo = this->AF.lo & 0b10110000;
    this->set_h((this->AF.hi & 0x0F) < loNib);
    this->set_c(this->AF.hi < x);
    this->cycles = 4;
}
void GB_CPU::ADD_ac() {
    u8 x = this->AF.hi;
    u8 loNib = this->AF.hi & 0x0F;
    this->AF.hi += this->BC.lo;
    if (this->AF.hi == 0) { this->AF.lo = this->AF.lo | 0x80; } else { this->AF.lo = 0; }
    this->AF.lo = this->AF.lo & 0b10110000;
    this->set_h((this->AF.hi & 0x0F) < loNib);
    this->set_c(this->AF.hi < x);
    this->cycles = 4;
}
void GB_CPU::ADD_ad() {
    u8 x = this->AF.hi;
    u8 loNib = this->AF.hi & 0x0F;
    this->AF.hi += this->DE.hi;
    if (this->AF.hi == 0) { this->AF.lo = this->AF.lo | 0x80; } else { this->AF.lo = 0; }
    this->AF.lo = this->AF.lo & 0b10110000;
    this->set_h((this->AF.hi & 0x0F) < loNib);
    this->set_c(this->AF.hi < x);
    this->cycles = 4;
}
void GB_CPU::ADD_ae() {
    u8 x = this->AF.hi;
    u8 loNib = this->DE.lo & 0x0F;
    this->AF.hi += this->DE.lo;

    this->set_z(this->AF.hi == 0);
    this->set_n(false);
    this->set_h((this->AF.hi & 0x0F) < loNib);
    this->set_c(this->AF.hi < x);
    this->cycles = 4;
}
void GB_CPU::ADD_ah() {
    u8 x = this->AF.hi;
    u8 loNib = this->AF.hi & 0x0F;
    this->AF.hi += this->HL.hi;
    if (this->AF.hi == 0) { this->AF.lo = this->AF.lo | 0x80; } else { this->AF.lo = 0; }
    this->set_n(false);
    this->set_h((this->AF.hi & 0x0F) < loNib);
    this->set_c(this->AF.hi < x);
    this->cycles = 4;
}
void GB_CPU::ADD_al() {
    u8 x = this->AF.hi;
    u8 loNib = this->HL.lo & 0x0F;
    this->AF.hi += this->HL.lo;
    if (this->AF.hi == 0) { this->AF.lo = this->AF.lo | 0x80; } else { this->AF.lo = 0; }
    this->set_n(false);
    this->set_h((this->AF.hi & 0x0F) < loNib);
    this->set_c(this->AF.hi < x);
    this->cycles = 4;
}
void GB_CPU::ADD_aH() {
    u8 x = this->AF.hi;
    u8 loNib = this->AF.hi & 0x0F;
    this->AF.hi += RAM::readAt(this->HL.val());
    if (this->AF.hi == 0) { this->AF.lo = this->AF.lo | 0x80; } else { this->AF.lo = 0; }
    this->set_n(false);
    this->set_h((this->AF.hi & 0x0F) < loNib);
    this->set_c(this->AF.hi < x);
    this->cycles = 8;
}
void GB_CPU::ADD_a_hash() {
    u8 x = this->AF.hi;
    u8 loNib = this->AF.hi & 0x0F;
    this->AF.hi += this->read();
    if (this->AF.hi == 0) { this->AF.lo = this->AF.lo | 0x80; } else { this->AF.lo = 0; }
    this->AF.lo = this->AF.lo & 0b10110000;
    this->set_h((this->AF.hi & 0x0F) < loNib);
    this->set_c(this->AF.hi < x);
    this->cycles = 8;
}

void GB_CPU::ADC_generic(u8 add) {
    u8 a = this->AF.hi;
    u8 b = add;
    u8 c = this->get_c();
    bool carry = false;
    u8 res = a + b + c;

    this->set_z(res == 0);
    this->set_n(false);
    this->set_h((a & 0x0F) + (b & 0x0F) + (c & 0x0F) > 0x0F);

    if (b + c > 0xFF) {
        carry = true;
    }

    if (res < a) {
        carry = true;
    }
    this->set_c(carry);

    this->AF.hi = res;
}

void GB_CPU::ADC_a_hash() {
    u8 readValue = this->read();
    ADC_generic(readValue);

    this->cycles = 8;
}

void GB_CPU::ADC_aa() {
    ADC_generic(this->AF.hi);
    this->cycles = 4;

}
void GB_CPU::ADC_ab() {
    ADC_generic(this->BC.hi);
    this->cycles = 4;
}

void GB_CPU::ADC_ac() {
    ADC_generic(this->BC.lo);
    this->cycles = 4;

}
void GB_CPU::ADC_ad() {
    ADC_generic(this->DE.hi);
    this->cycles = 4;

}
void GB_CPU::ADC_ae() {
    ADC_generic(this->DE.lo);
    this->cycles = 4;

}
void GB_CPU::ADC_ah() {
    ADC_generic(this->HL.hi);
    this->cycles = 4;

}
void GB_CPU::ADC_al() {
    ADC_generic(this->HL.lo);
    this->cycles = 4;

}
void GB_CPU::ADC_aHL() {
    u8 readValue = RAM::readAt(this->HL.val());
    ADC_generic(readValue);

    this->cycles = 8;

}

void GB_CPU::SUB_a() {
    this->set_z(true);
    this->set_n(true);
    this->set_h(false);
    this->set_c(false);

    this->AF.hi = 0;
    this->cycles = 4;
}
void GB_CPU::SUB_b() {
    u8 x = this->BC.hi;
    u8 oldValue = this->AF.hi;
    this->AF.hi -= x;

    this->set_z(this->AF.hi == 0);
    this->set_n(true);
    this->set_h((oldValue & 0x0F) < (x & 0x0F));
    this->set_c(oldValue < x);

    this->cycles = 4;
}
void GB_CPU::SUB_c() {
    u8 x = this->BC.lo;
    u8 oldValue = this->AF.hi;
    this->AF.hi -= x;

    this->set_z(this->AF.hi == 0);
    this->set_n(true);
    this->set_h((oldValue & 0x0F) < (x & 0x0F));
    this->set_c(oldValue < x);

    this->cycles = 4;
}
void GB_CPU::SUB_d() {
    u8 x = this->DE.hi;
    u8 oldValue = this->AF.hi;
    this->AF.hi -= x;

    this->set_z(this->AF.hi == 0);
    this->set_n(true);
    this->set_h((oldValue & 0x0F) < (x & 0x0F));
    this->set_c(oldValue < x);

    this->cycles = 4;
}
void GB_CPU::SUB_e() {
    u8 x = this->DE.lo;
    u8 oldValue = this->AF.hi;
    this->AF.hi -= x;

    this->set_z(this->AF.hi == 0);
    this->set_n(true);
    this->set_h((oldValue & 0x0F) < (x & 0x0F));
    this->set_c(oldValue < x);

    this->cycles = 4;
}
void GB_CPU::SUB_h() {
    u8 x = this->HL.hi;
    u8 oldValue = this->AF.hi;
    this->AF.hi -= x;

    this->set_z(this->AF.hi == 0);
    this->set_n(true);
    this->set_h((oldValue & 0x0F) < (x & 0x0F));
    this->set_c(oldValue < x);

    this->cycles = 4;
}
void GB_CPU::SUB_l() {
    u8 x = this->HL.lo;
    u8 oldValue = this->AF.hi;
    this->AF.hi -= x;

    this->set_z(this->AF.hi == 0);
    this->set_n(true);
    this->set_h((oldValue & 0x0F) < (x & 0x0F));
    this->set_c(oldValue < x);

    this->cycles = 4;
}
void GB_CPU::SUB_HL() {
    u8 x = RAM::readAt(this->HL.val());
    u8 oldValue = this->AF.hi;

    this->AF.hi -= x;

    this->set_z(this->AF.hi == 0);
    this->set_n(true);
    this->set_h((oldValue & 0x0F) < (x & 0x0F));
    this->set_c(oldValue < x);

    this->cycles = 8;
}
void GB_CPU::SUB_hash() {
    u8 x = this->read();
    u8 oldValue = this->AF.hi;
    this->AF.hi -= x;

    this->set_z(this->AF.hi == 0);
    this->set_n(true);
    this->set_h((oldValue & 0x0F) < (x & 0x0F));
    this->set_c(oldValue < x);

    this->cycles = 4;
}


void GB_CPU::SBC_generic(u8 sub) {
    u8 a = this->AF.hi;
    u8 b = sub;
    u8 c = this->get_c();
    bool halfCarry = false;
    bool carry = false;

    if (a < b || a - b < c) {
        carry = true;
    }

    this->AF.hi -= (b + c);


    this->set_z(this->AF.hi == 0);
    this->set_n(true);
    this->set_h(half_carry_sub(a, b) || half_carry_sub(a - b, c));
    this->set_c(carry);
}

void GB_CPU::SBC_a() {
    SBC_generic(this->AF.hi);
    this->cycles = 4;
}
void GB_CPU::SBC_b() {
    SBC_generic(this->BC.hi);
    this->cycles = 4;
}
void GB_CPU::SBC_c() {
    SBC_generic(this->BC.lo);
    this->cycles = 4;
}
void GB_CPU::SBC_d() {
    SBC_generic(this->DE.hi);
    this->cycles = 4;
}
void GB_CPU::SBC_e() {
    SBC_generic(this->DE.lo);
    this->cycles = 4;
}
void GB_CPU::SBC_h() {
    SBC_generic(this->HL.hi);
    this->cycles = 4;
}
void GB_CPU::SBC_l() {
    SBC_generic(this->HL.lo);
    this->cycles = 4;
}
void GB_CPU::SBC_HL() {
    SBC_generic(RAM::readAt(this->HL.val()));
    this->cycles = 8;
}

void GB_CPU::SBC_hash() {
    u8 oldValue = this->AF.hi;
    u8 readValue = this->read();
    u8 change = this->get_c() + readValue;
    bool halfCarry = false;
    bool carry = false;

    if (oldValue < readValue || oldValue - readValue < this->get_c()) {
        carry = true;
    }

    if ((((oldValue) & 0x0F) - (this->get_c() & 0x0F) - (readValue & 0x0F) & 0x10) == 0x10) {
        halfCarry = true;
    }

    this->AF.hi -= change;

    this->set_z(this->AF.hi == 0);
    this->set_n(true);
    this->set_h(half_carry_sub(oldValue, readValue) || half_carry_sub(oldValue - readValue, this->get_c()));
    this->set_c(carry);
    this->cycles = 8;
}

void GB_CPU::AND_a() { this->AF.hi = this->AF.hi & this->AF.hi; if (this->AF.hi == 0) { this->AF.lo |= 0b10000000; } else { this->AF.lo &= 0b01111111; }  this->AF.lo &= 0b10100000;  this->AF.lo |= 0b00100000; this->cycles = 4; }
void GB_CPU::AND_b() { this->AF.hi = this->AF.hi & this->BC.hi; if (this->AF.hi == 0) { this->AF.lo |= 0b10000000; } else { this->AF.lo &= 0b01111111; }  this->AF.lo &= 0b10100000;  this->AF.lo |= 0b00100000; this->cycles = 4; }
void GB_CPU::AND_c() { this->AF.hi = this->AF.hi & this->BC.lo; if (this->AF.hi == 0) { this->AF.lo |= 0b10000000; } else { this->AF.lo &= 0b01111111; }  this->AF.lo &= 0b10100000;  this->AF.lo |= 0b00100000; this->cycles = 4; }
void GB_CPU::AND_d() { this->AF.hi = this->AF.hi & this->DE.hi; if (this->AF.hi == 0) { this->AF.lo |= 0b10000000; } else { this->AF.lo &= 0b01111111; }  this->AF.lo &= 0b10100000;  this->AF.lo |= 0b00100000; this->cycles = 4; }
void GB_CPU::AND_e() { this->AF.hi = this->AF.hi & this->DE.lo; if (this->AF.hi == 0) { this->AF.lo |= 0b10000000; } else { this->AF.lo &= 0b01111111; }  this->AF.lo &= 0b10100000;  this->AF.lo |= 0b00100000; this->cycles = 4; }
void GB_CPU::AND_h() { this->AF.hi = this->AF.hi & this->HL.hi; if (this->AF.hi == 0) { this->AF.lo |= 0b10000000; } else { this->AF.lo &= 0b01111111; }  this->AF.lo &= 0b10100000;  this->AF.lo |= 0b00100000; this->cycles = 4; }
void GB_CPU::AND_l() { this->AF.hi = this->AF.hi & this->HL.lo; if (this->AF.hi == 0) { this->AF.lo |= 0b10000000; } else { this->AF.lo &= 0b01111111; }  this->AF.lo &= 0b10100000;  this->AF.lo |= 0b00100000; this->cycles = 4; }
void GB_CPU::AND_HL() { this->AF.hi = this->AF.hi & RAM::readAt(this->HL.val()); if (this->AF.hi == 0) { this->AF.lo |= 0b10000000; } else { this->AF.lo &= 0b01111111; }  this->AF.lo &= 0b10100000;  this->AF.lo |= 0b00100000; this->cycles = 8; }
void GB_CPU::AND_hash() { this->AF.hi = this->AF.hi & this->read(); if (this->AF.hi == 0) { this->AF.lo |= 0b10000000; } else { this->AF.lo &= 0b01111111; }  this->AF.lo &= 0b10100000;  this->AF.lo |= 0b00100000; this->cycles = 8; }

void GB_CPU::OR_a() {
    this->AF.hi = this->AF.hi | this->AF.hi;
    this->set_n(false);
    this->set_h(false);
    this->set_c(false);
    this->set_z(this->AF.hi == 0);
    this->cycles = 4;
}
void GB_CPU::OR_b() {
    this->AF.hi = this->AF.hi | this->BC.hi;
    this->set_n(false);
    this->set_h(false);
    this->set_c(false);
    this->set_z(this->AF.hi == 0);
    this->cycles = 4;
}
void GB_CPU::OR_c() {
    this->AF.hi = this->AF.hi | this->BC.lo;
    this->set_n(false);
    this->set_h(false);
    this->set_c(false);
    this->set_z(this->AF.hi == 0);
    this->cycles = 4;
}
void GB_CPU::OR_d() {
    this->AF.hi = this->AF.hi | this->DE.hi;
    this->set_n(false);
    this->set_h(false);
    this->set_c(false);
    this->set_z(this->AF.hi == 0);
    this->cycles = 4;
}
void GB_CPU::OR_e() {
    this->AF.hi = this->AF.hi | this->DE.lo;
    this->set_n(false);
    this->set_h(false);
    this->set_c(false);
    this->set_z(this->AF.hi == 0);
    this->cycles = 4;
}
void GB_CPU::OR_h() {
    this->AF.hi = this->AF.hi | this->HL.hi;
    this->set_n(false);
    this->set_h(false);
    this->set_c(false);
    this->set_z(this->AF.hi == 0);
    this->cycles = 4;
}
void GB_CPU::OR_l() {
    this->AF.hi = this->AF.hi | this->HL.lo;
    this->set_n(false);
    this->set_h(false);
    this->set_c(false);
    this->set_z(this->AF.hi == 0);
    this->cycles = 4;
}
void GB_CPU::OR_HL() {
    this->AF.hi = this->AF.hi | RAM::readAt(this->HL.val());
    this->set_n(false);
    this->set_h(false);
    this->set_c(false);
    this->set_z(this->AF.hi == 0);
    this->cycles = 8;
}
void GB_CPU::OR_hash() {
    this->AF.hi = this->AF.hi | this->read();
    this->set_n(false);
    this->set_h(false);
    this->set_c(false);
    this->set_z(this->AF.hi == 0);
    this->cycles = 8;
}


void GB_CPU::XOR_a() { this->AF.hi ^= this->AF.hi; if (this->AF.hi == 0) { this->AF.lo |= 0b10000000; } else { this->AF.lo &= 0b01111111; }  this->AF.lo &= 0b10000000; this->cycles = 4; }
void GB_CPU::XOR_b() { this->AF.hi ^= this->BC.hi; if (this->AF.hi == 0) { this->AF.lo |= 0b10000000; } else { this->AF.lo &= 0b01111111; }  this->AF.lo &= 0b10000000; this->cycles = 4; }
void GB_CPU::XOR_c() { this->AF.hi ^= this->BC.lo; if (this->AF.hi == 0) { this->AF.lo |= 0b10000000; } else { this->AF.lo &= 0b01111111; }  this->AF.lo &= 0b10000000; this->cycles = 4; }
void GB_CPU::XOR_d() { this->AF.hi ^= this->DE.hi; if (this->AF.hi == 0) { this->AF.lo |= 0b10000000; } else { this->AF.lo &= 0b01111111; }  this->AF.lo &= 0b10000000; this->cycles = 4; }
void GB_CPU::XOR_e() { this->AF.hi ^= this->DE.lo; if (this->AF.hi == 0) { this->AF.lo |= 0b10000000; } else { this->AF.lo &= 0b01111111; }  this->AF.lo &= 0b10000000; this->cycles = 4; }
void GB_CPU::XOR_h() { this->AF.hi ^= this->HL.hi; if (this->AF.hi == 0) { this->AF.lo |= 0b10000000; } else { this->AF.lo &= 0b01111111; }  this->AF.lo &= 0b10000000; this->cycles = 4; }
void GB_CPU::XOR_l() { this->AF.hi ^= this->HL.lo; if (this->AF.hi == 0) { this->AF.lo |= 0b10000000; } else { this->AF.lo &= 0b01111111; }  this->AF.lo &= 0b10000000; this->cycles = 4; }
void GB_CPU::XOR_HL() { this->AF.hi ^= (RAM::readAt(this->HL.val())); if (this->AF.hi == 0) { this->AF.lo |= 0b10000000; } else { this->AF.lo &= 0b01111111; }  this->AF.lo &= 0b10000000; this->cycles = 8; }
void GB_CPU::XOR_hash() { this->AF.hi ^= this->read(); if (this->AF.hi == 0) { this->AF.lo |= 0b10000000; } else { this->AF.lo &= 0b01111111; }  this->AF.lo &= 0b10000000; this->cycles = 8; }

void GB_CPU::CP_a() {
    this->set_z(true);
    this->set_n(true);
    this->set_h(false);
    this->set_c(false);
    this->cycles = 4;
}
void GB_CPU::CP_b() {
    u8 x = this->BC.hi;
    u8 oldValue = this->AF.hi;
    u8 comparison = this->AF.hi - x;

    this->set_z(comparison == 0);
    this->set_n(true);
    this->set_h((oldValue & 0x0F) < (x & 0x0F));
    this->set_c(oldValue < x);

    this->cycles = 4;
}
void GB_CPU::CP_c() {
    u8 x = this->BC.lo;
    u8 oldValue = this->AF.hi;
    u8 comparison = this->AF.hi - x;

    this->set_z(comparison == 0);
    this->set_n(true);
    this->set_h((oldValue & 0x0F) < (x & 0x0F));
    this->set_c(oldValue < x);

    this->cycles = 4;
}
void GB_CPU::CP_d() {
    u8 x = this->DE.hi;
    u8 oldValue = this->AF.hi;
    u8 comparison = this->AF.hi - x;

    this->set_z(comparison == 0);
    this->set_n(true);
    this->set_h((oldValue & 0x0F) < (x & 0x0F));
    this->set_c(oldValue < x);

    this->cycles = 4;
}

void GB_CPU::CP_e() {
    u8 x = this->DE.lo;
    u8 oldValue = this->AF.hi;
    u8 comparison = this->AF.hi - x;

    this->set_z(comparison == 0);
    this->set_n(true);
    this->set_h((oldValue & 0x0F) < (x & 0x0F));
    this->set_c(oldValue < x);

    this->cycles = 4;
}
void GB_CPU::CP_h() {
    u8 x = this->HL.hi;
    u8 oldValue = this->AF.hi;
    u8 comparison = this->AF.hi - x;

    this->set_z(comparison == 0);
    this->set_n(true);
    this->set_h((oldValue & 0x0F) < (x & 0x0F));
    this->set_c(oldValue < x);

    this->cycles = 4;
}
void GB_CPU::CP_l() {
    u8 x = this->HL.lo;
    u8 oldValue = this->AF.hi;
    u8 comparison = this->AF.hi - x;

    this->set_z(comparison == 0);
    this->set_n(true);
    this->set_h((oldValue & 0x0F) < (x & 0x0F));
    this->set_c(oldValue < x);

    this->cycles = 4;
}
void GB_CPU::CP_HL() {
    u8 x = RAM::readAt(this->HL.val());
    u8 oldValue = this->AF.hi;
    u8 comparison = this->AF.hi - x;

    this->set_z(comparison == 0);
    this->set_n(true);
    this->set_h((oldValue & 0x0F) < (x & 0x0F));
    this->set_c(oldValue < x);

    this->cycles = 8;
}

void GB_CPU::CP_hash() {
    u8 x = this->read();
    u8 z = this->AF.hi - x;
    this->set_z(z == 0);
    this->set_n(true);
    this->set_h((this->AF.hi & 0x0F) < (x & 0x0F));
    this->set_c(this->AF.hi < x);
    this->cycles = 8;
}

void GB_CPU::INC_a() {
    u8 loNib = this->AF.hi & 0x0F;
    ++this->AF.hi;
    if (this->AF.hi == 0) { this->AF.lo |= 0b10000000; } else { this->AF.lo &= 0b01110000; }
    this->AF.lo &= 0b10110000;
    if (loNib > (this->AF.hi & 0x0F)) { this->AF.lo |= 0b00100000; } else { this->AF.lo &= 0b11010000; }
    this->cycles = 4;
}
void GB_CPU::INC_b() {
    u8 loNib = this->BC.hi & 0x0F;
    ++this->BC.hi;
    if (this->BC.hi == 0) { this->AF.lo |= 0b10000000; } else { this->AF.lo &= 0b01110000; }
    this->AF.lo &= 0b10110000;
    if (loNib > (this->BC.hi & 0x0F)) { this->AF.lo |= 0b00100000; } else { this->AF.lo &= 0b11010000; }
    this->cycles = 4;
}
void GB_CPU::INC_c() {
    u8 loNib = this->BC.lo & 0x0F;
    ++this->BC.lo;
    if (this->BC.lo == 0) { this->AF.lo |= 0b10000000; } else { this->AF.lo &= 0b01110000; }
    this->AF.lo &= 0b10110000;
    if (loNib > (this->BC.lo & 0x0F)) { this->AF.lo |= 0b00100000; } else { this->AF.lo &= 0b11010000; }
    this->cycles = 4;
}
void GB_CPU::INC_d() {
    u8 loNib = this->DE.hi & 0x0F;
    ++this->DE.hi;
    if (this->DE.hi == 0) { this->AF.lo |= 0b10000000; } else { this->AF.lo &= 0b01110000; }
    this->AF.lo &= 0b10110000;
    if (loNib > (this->DE.hi & 0x0F)) { this->AF.lo |= 0b00100000; } else { this->AF.lo &= 0b11010000; }
    this->cycles = 4;
}
void GB_CPU::INC_e() {
    u8 loNib = this->DE.lo & 0x0F;
    ++this->DE.lo;
    if (this->DE.lo == 0) { this->AF.lo |= 0b10000000; } else { this->AF.lo &= 0b01110000; }
    this->AF.lo &= 0b10110000;
    if (loNib > (this->DE.lo & 0x0F)) { this->AF.lo |= 0b00100000; } else { this->AF.lo &= 0b11010000; }
    this->cycles = 4;
}
void GB_CPU::INC_h() {
    u8 loNib = this->HL.hi & 0x0F;
    ++this->HL.hi;
    if (this->HL.hi == 0) { this->AF.lo |= 0b10000000; } else { this->AF.lo &= 0b01110000; }
    this->AF.lo &= 0b10110000;
    if (loNib > (this->HL.hi & 0x0F)) { this->AF.lo |= 0b00100000; } else { this->AF.lo &= 0b11010000; }
    this->cycles = 4;
}
void GB_CPU::INC_l() {
    u8 loNib = this->HL.lo & 0x0F;
    ++this->HL.lo;
    if (this->HL.lo == 0) { this->AF.lo |= 0b10000000; } else { this->AF.lo &= 0b01110000; }
    this->AF.lo &= 0b10110000;
    if (loNib > (this->HL.lo & 0x0F)) { this->AF.lo |= 0b00100000; } else { this->AF.lo &= 0b11010000; }
    this->cycles = 4;
}
void GB_CPU::INC_HLad() {
    u8 x = RAM::readAt(this->HL.val());
    u8 loNib = x & 0x0F;
    ++x;
    this->write(x, this->HL.val());
    if (x == 0) { this->AF.lo |= 0b10000000; } else { this->AF.lo &= 0b01110000; }
    this->AF.lo &= 0b10110000;
    if (loNib > (x & 0x0F)) { this->AF.lo |= 0b00100000; } else { this->AF.lo &= 0b11010000; }
    this->cycles = 12;
}

void GB_CPU::DEC_a() {
    u8 lowerNib = this->AF.hi & 0x0F;
    u8 newValue = -- this->AF.hi;
    this->set_n(true);
    this->set_z(newValue == 0);
    this->set_h(lowerNib == 0x00);
    this->cycles = 4;
}
void GB_CPU::DEC_b() {
    u8 lowerNib = this->BC.hi & 0x0F;
    u8 newValue = --this->BC.hi;
    this->set_n(true);
    this->set_z(newValue == 0);
    this->set_h(lowerNib == 0x00);
    this->cycles = 4;
}
void GB_CPU::DEC_c() {
    u8 lowerNib = this->BC.lo & 0x0F;
    u8 newValue = --this->BC.lo;
    this->set_n(true);
    this->set_z(newValue == 0);
    this->set_h(lowerNib == 0x00);
    this->cycles = 4;
}
void GB_CPU::DEC_d() {
    u8 lowerNib = this->DE.hi & 0x0F;
    u8 newValue = --this->DE.hi;
    this->set_n(true);
    this->set_z(newValue == 0);
    this->set_h(lowerNib == 0x00);
    this->cycles = 4;
}
void GB_CPU::DEC_e() {
    u8 lowerNib = this->DE.lo & 0x0F;
    u8 newValue = --this->DE.lo;
    this->set_n(true);
    this->set_z(newValue == 0);
    this->set_h(lowerNib == 0x00);
    this->cycles = 4;
}
void GB_CPU::DEC_h() {
    u8 lowerNib = this->HL.hi & 0x0F;
    u8 newValue = --this->HL.hi;
    this->set_n(true);
    this->set_z(newValue == 0);
    this->set_h(lowerNib == 0x00);
    this->cycles = 4;
}
void GB_CPU::DEC_l() {
    u8 lowerNib = this->HL.lo & 0x0F;
    u8 newValue = --this->HL.lo;
    this->set_n(true);
    this->set_z(newValue == 0);
    this->set_h(lowerNib == 0x00);
    this->cycles = 4;
}
void GB_CPU::DEC_HLad() {
    u8 x = RAM::readAt(this->HL.val());
    u8 lowerNib = x & 0x0F;
    u8 newValue = --x;
    this->write(x, this->HL.val());

    this->set_n(true);
    this->set_z(newValue == 0);
    this->set_h(lowerNib == 0x00);
    this->cycles = 12;
}

void GB_CPU::HALT() {
    halt = true;
    this->cycles = 4;
}

void GB_CPU::RST_00() {
    u8 loNibble = this->PC & 0x00FF;
    u8 hiNibble = (this->PC & 0xFF00) >> 8;
    this->SP--;
    this->write(hiNibble, this->SP);
    this->SP--;
    this->write(loNibble, this->SP);

    this->PC = 0x00;
    this->cycles = 32;
}
void GB_CPU::RST_08() {
    u8 loNibble = this->PC & 0x00FF;
    u8 hiNibble = (this->PC & 0xFF00) >> 8;
    this->SP--;
    this->write(hiNibble, this->SP);
    this->SP--;
    this->write(loNibble, this->SP);

    this->PC = 0x08;
    this->cycles = 32;
}
void GB_CPU::RST_10() {
    u8 loNibble = this->PC & 0x00FF;
    u8 hiNibble = (this->PC & 0xFF00) >> 8;
    this->SP--;
    this->write(hiNibble, this->SP);
    this->SP--;
    this->write(loNibble, this->SP);

    this->PC = 0x10;
    this->cycles = 32;
}
void GB_CPU::RST_18() {
    u8 loNibble = this->PC & 0x00FF;
    u8 hiNibble = (this->PC & 0xFF00) >> 8;
    this->SP--;
    this->write(hiNibble, this->SP);
    this->SP--;
    this->write(loNibble, this->SP);

    this->PC = 0x18;
    this->cycles = 32;
}
void GB_CPU::RST_20() {
    u8 loNibble = this->PC & 0x00FF;
    u8 hiNibble = (this->PC & 0xFF00) >> 8;
    this->SP--;
    this->write(hiNibble, this->SP);
    this->SP--;
    this->write(loNibble, this->SP);

    this->PC = 0x20;
    this->cycles = 32;
}
void GB_CPU::RST_28() {
    u8 loNibble = this->PC & 0x00FF;
    u8 hiNibble = (this->PC & 0xFF00) >> 8;
    this->SP--;
    this->write(hiNibble, this->SP);
    this->SP--;
    this->write(loNibble, this->SP);

    this->PC = 0x28;
    this->cycles = 32;
}
void GB_CPU::RST_30() {
    u8 loNibble = this->PC & 0b00001111;
    u8 hiNibble = this->PC >> 4;
    this->SP--;
    this->write(hiNibble, this->SP);
    this->SP--;
    this->write(loNibble, this->SP);

    this->PC = 0x30;
    this->cycles = 32;
}
void GB_CPU::RST_38() {
    u8 loNibble = this->PC & 0x00FF;
    u8 hiNibble = (this->PC & 0xFF00) >> 8;
    this->SP--;
    this->write(hiNibble, this->SP);
    this->SP--;
    this->write(loNibble, this->SP);

    this->PC = 0x38;
    this->cycles = 32;
}

void GB_CPU::LDI_aHL() {
    this->AF.hi = RAM::readAt(this->HL.val());
    this->HL.set(this->HL.val() + 1);
    this->cycles = 8;
}

void GB_CPU::LDH_nA() {
    u8 n = this->read();
    this->write(this->AF.hi, 0xFF00 + n);
    this->cycles = 12;
}

void GB_CPU::ADD_BC() {
    u16 x = this->HL.val();
    this->HL.set(x + this->BC.val());
    this->AF.lo &= 0b10110000;

    if ((this->HL.val() & 0x0FFF) < (x & 0xFFF)) { this->AF.lo |= 0b00100000; } else { this->AF.lo &= 0b11010000; }

    if (this->HL.val() < x) { this->AF.lo |= 0b00010000; } else { this->AF.lo &= 0b11100000; }
    this->cycles = 8;
}
void GB_CPU::ADD_DE() {
    u16 x = this->HL.val();
    this->HL.set(x + this->DE.val());
    this->AF.lo &= 0b10110000;

    if ((this->HL.val() & 0x0FFF) < (x & 0xFFF)) { this->AF.lo |= 0b00100000; } else { this->AF.lo &= 0b11010000; }

    if (this->HL.val() < x) { this->AF.lo |= 0b00010000; } else { this->AF.lo &= 0b11100000; }
    this->cycles = 8;
}
void GB_CPU::ADD_HL() {
    u16 x = this->HL.val();
    this->HL.set(2 * x);
    this->AF.lo &= 0b10110000;

    if ((this->HL.val() & 0x0FFF) < (x & 0xFFF)) { this->AF.lo |= 0b00100000; } else { this->AF.lo &= 0b11010000; }

    if (this->HL.val() < x) { this->AF.lo |= 0b00010000; } else { this->AF.lo &= 0b11100000; }
    this->cycles = 8;
}
void GB_CPU::ADD_SP() {
    u16 x = this->HL.val();
    this->HL.set(x + this->SP);
    this->AF.lo &= 0b10110000;

    if ((this->HL.val() & 0x0FFF) < (x & 0xFFF)) { this->AF.lo |= 0b00100000; } else { this->AF.lo &= 0b11010000; }

    if (this->HL.val() < x) { this->AF.lo |= 0b00010000; } else { this->AF.lo &= 0b11100000; }
    this->cycles = 8;
}

void GB_CPU::DI() {
    this->interrupt_mode = 2;
    this->cycles = 4;
}

void GB_CPU::EI() {
    this->interrupt_mode = 4;
    this->cycles = 4;
}

void GB_CPU::ADD_n_SP() {
    s8 readValue = this->read();
    u16 oldValue = this->SP;
    this->AF.lo &= 0b00110000;

    u16 result = this->SP + readValue;

    this->set_z(0);
    this->set_n(0);
    this->set_h((SP & 0x0F) + (readValue & 0x0F) > 0x0F);
    this->set_c((result & 0xFF) < (oldValue & 0xFF));

    this->SP = result;
    this->cycles = 16;
}

void GB_CPU::INC_BC() {
    this->BC.set(this->BC.val() + 1);
    this->cycles = 8;
}
void GB_CPU::INC_DE() {
    this->DE.set(this->DE.val() + 1);
    this->cycles = 8;
}
void GB_CPU::INC_HL() {
    this->HL.set(this->HL.val() + 1);
    this->cycles = 8;
}
void GB_CPU::INC_SP() {
    this->SP++;
    this->cycles = 8;
}

void GB_CPU::DEC_BC() {
    this->BC.set(this->BC.val() - 1);
    this->cycles = 8;
}
void GB_CPU::DEC_DE() {
    this->DE.set(this->DE.val() - 1);
    this->cycles = 8;
}
void GB_CPU::DEC_HL() {
    this->HL.set(this->HL.val() - 1);
    this->cycles = 8;
}
void GB_CPU::DEC_SP() {
    this->SP--;
    this->cycles = 8;
}

void GB_CPU::CPL() {
    this->AF.hi = 0xFF - this->AF.hi;
    this->AF.lo |= 0b01100000;
    this->cycles = 4;
}

u8 GB_CPU::SWAP_gen(u8 val) {
    u8 res = ((val >> 4) & 0x0F) + ((val << 4) & 0xF0);
    this->set_z(res == 0);
    this->set_n(false);
    this->set_h(false);
    this->set_c(false);
    return res;
}

void GB_CPU::SWAP_a() {
    this->AF.hi = SWAP_gen(this->AF.hi);
    this->cycles = 8;
}
void GB_CPU::SWAP_b() {
    this->BC.hi = SWAP_gen(this->BC.hi);
    this->cycles = 8;
}
void GB_CPU::SWAP_c() {
    this->BC.lo = SWAP_gen(this->BC.lo);
    this->cycles = 8;
}

void GB_CPU::SWAP_d() {
    this->DE.hi = SWAP_gen(this->DE.hi);
    this->cycles = 8;
}
void GB_CPU::SWAP_e() {
    this->DE.lo = SWAP_gen(this->DE.lo);
    this->cycles = 8;
}
void GB_CPU::SWAP_h() {
    this->HL.hi = SWAP_gen(this->HL.hi);
    this->cycles = 8;
}
void GB_CPU::SWAP_l() {
    this->HL.lo = SWAP_gen(this->HL.lo);
    this->cycles = 8;
}
void GB_CPU::SWAP_HL() {
    u8 value = RAM::readAt(this->HL.val());
    this->write(SWAP_gen(value), this->HL.val());
    this->cycles = 16;
}

void GB_CPU::RES_0_a() {
    this->AF.hi &= 0b11111110;
    this->cycles = 8;
}

void GB_CPU::DAA() {
    bool carry = false;

    if (!this->get_n()) {
        if (this->get_c() || this->AF.hi > 0x99) {
            this->AF.hi += 0x60;
            carry = true;
        }
        if (this->get_h() || (this->AF.hi & 0x0F) > 0x09) {
            this->AF.hi += 0x06;
        }
    } else {
        if (this->get_c()) {
            this->AF.hi -= 0x60;
            carry = true;
        }
        if (this->get_h()) {
            this->AF.hi -= 0x06;
        }
    }

    this->set_c(carry);
    this->set_z(this->AF.hi == 0);
    this->set_h(false);

    this->cycles = 4;
}

u8 GB_CPU::RRC_generic(u8 val) {
    bool bit0 = val & 1;
    u8 newVal = (val >> 1) + (bit0 << 7);
    this->set_z(newVal == 0);
    this->set_n(false);
    this->set_h(false);
    this->set_c(bit0);
    return newVal;
}

void GB_CPU::RRC_A() {
    this->AF.hi = RRC_generic(this->AF.hi);
    this->cycles = 8;
}

void GB_CPU::RRC_B() {
    this->BC.hi = RRC_generic(this->BC.hi);
    this->cycles = 8;
}

void GB_CPU::RRC_C() {
    this->BC.lo = RRC_generic(this->BC.lo);
    this->cycles = 8;
}

void GB_CPU::RRC_D() {
    this->DE.hi = RRC_generic(this->DE.hi);
    this->cycles = 8;
}

void GB_CPU::RRC_E() {
    this->DE.lo = RRC_generic(this->DE.lo);
    this->cycles = 8;
}

void GB_CPU::RRC_H() {
    this->HL.hi = RRC_generic(this->HL.hi);
    this->cycles = 8;
}

void GB_CPU::RRC_L() {
    this->HL.lo = RRC_generic(this->HL.lo);
    this->cycles = 8;
}

void GB_CPU::RRC_HL() {
    u8 readVal = RAM::readAt(this->HL.val());
    RAM::write(RRC_generic(readVal), this->HL.val());
    this->cycles = 16;
}

u8 GB_CPU::SLA_generic(u8 val) {
    u8 bit7 = val & 0b10000000;
    u8 newVal = val << 1;
    this->set_z(newVal == 0);
    this->set_n(false);
    this->set_h(false);
    this->set_c(bit7 == 0b10000000);
    return newVal;
}

void GB_CPU::SLA_A() {
    this->AF.hi = this->SLA_generic(this->AF.hi);
    this->cycles = 8;
}

void GB_CPU::SLA_B() {
    this->BC.hi = this->SLA_generic(this->BC.hi);
    this->cycles = 8;
}

void GB_CPU::SLA_C() {
    this->BC.lo = this->SLA_generic(this->BC.lo);
    this->cycles = 8;
}

void GB_CPU::SLA_D() {
    this->DE.hi = this->SLA_generic(this->DE.hi);
    this->cycles = 8;
}

void GB_CPU::SLA_E() {
    this->DE.lo = this->SLA_generic(this->DE.lo);
    this->cycles = 8;
}

void GB_CPU::SLA_H() {
    this->HL.hi = this->SLA_generic(this->HL.hi);
    this->cycles = 8;
}

void GB_CPU::SLA_L() {
    this->HL.lo = this->SLA_generic(this->HL.lo);
    this->cycles = 8;
}

void GB_CPU::SLA_HL() {
    u8 readVal = RAM::readAt(this->HL.val());
    RAM::write(this->SLA_generic(readVal), this->HL.val());
    this->cycles = 16;
}

u8 GB_CPU::SRA_generic(u8 val) {
    bool bit0 = val & 1;
    u8 bit7 = val & 0b10000000;
    u8 newVal = (val >> 1) + bit7;
    this->set_z(newVal == 0);
    this->set_n(false);
    this->set_h(false);
    this->set_c(bit0);
    return newVal;
}

void GB_CPU::SRA_A() {
    this->AF.hi = SRA_generic(this->AF.hi);
    this->cycles = 8;
}

void GB_CPU::SRA_B() {
    this->BC.hi = SRA_generic(this->BC.hi);
    this->cycles = 8;
}

void GB_CPU::SRA_C() {
    this->BC.lo = SRA_generic(this->BC.lo);
    this->cycles = 8;
}

void GB_CPU::SRA_D() {
    this->DE.hi = SRA_generic(this->DE.hi);
    this->cycles = 8;
}

void GB_CPU::SRA_E() {
    this->DE.lo = SRA_generic(this->DE.lo);
    this->cycles = 8;
}

void GB_CPU::SRA_H() {
    this->HL.hi = SRA_generic(this->HL.hi);
    this->cycles = 8;
}

void GB_CPU::SRA_L() {
    this->HL.lo = SRA_generic(this->HL.lo);
    this->cycles = 8;
}

void GB_CPU::SRA_HL() {
    u8 readVal = RAM::readAt(this->HL.val());
    RAM::write(SRA_generic(readVal), this->HL.val());
    this->cycles = 16;
}
