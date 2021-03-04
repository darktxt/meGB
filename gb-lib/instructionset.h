#pragma once
#include"common.h"
#include"mmu.h"
constexpr auto ZeroFlag = 7;
constexpr auto SubtractFlag = 6;
constexpr auto HalfCarryFlag = 5;
constexpr auto CarryFlag = 4;
constexpr auto INT40 = 0x40;  // VBlank
constexpr auto INT48 = 0x48; // STAT
constexpr auto INT50 = 0x50; // Timer
constexpr auto INT58 = 0x58; // Serial
constexpr auto INT60 = 0x60; // Joypad
class InstructionSet {
    friend class CPU;
public:
    InstructionSet();
	ulong operator ()(const byte opcode) {

	}
private:
    std::shared_ptr<MMU> m_MMU;
    ushort m_AF; // Accumulator & flags
    ushort m_BC; // General purpose
    ushort m_DE; // General purpose
    ushort m_HL; // General purpose
    ushort m_SP; // Stack pointer
    ushort m_PC; // Program counter
    byte* m_ByteRegisterMap[0x07 + 1];
    ushort* m_UShortRegisterMap[0x03 + 1];

    // Interrupts
    byte m_IME; // Interrupt master enable

    unsigned long m_cycles; // The current number of cycles
    bool m_isHalted;

    OPF m_operationMap[0xFF + 1];
    OPF m_operationMapCB[0xFF + 1];
    byte GetHighByte(ushort dest);
    byte GetLowByte(ushort dest);

    byte* GetByteRegister(byte val);
    ushort* GetUShortRegister(byte val, bool useAF);

    void SetHighByte(ushort* dest, byte val);
    void SetLowByte(ushort* dest, byte val);

    void SetFlag(byte flag);
    void ClearFlag(byte flag);
    bool IsFlagSet(byte flag);

    void PushByteToSP(byte val);
    void PushUShortToSP(ushort val);
    ushort PopUShort();
    byte PopByte();
    byte ReadBytePC();
    ushort ReadUShortPC();

    byte AddByte(byte b1, byte b2);
    ushort AddUShort(ushort u1, ushort u2);
    void ADC(byte val);
    void SBC(byte val);

    void HandleInterrupts();

    // TODO: Organize the following...
    // Z80 Instruction Set
    ulong NOP(const byte& opCode);             // 0x00

    ulong LDrn(const byte& opCode);
    ulong LDrR(const byte& opCode);
    ulong LDrrnn(const byte& opCode);
    ulong INCr(const byte& opCode);
    ulong INCrr(const byte& opCode);
    ulong DECrr(const byte& opCode);
    ulong ORr(const byte& opCode);
    ulong XORr(const byte& opCode);
    ulong PUSHrr(const byte& opCode);
    ulong POPrr(const byte& opCode);
    ulong DECr(const byte& opCode);
    ulong SUBr(const byte& opCode);
    ulong SBCAr(const byte& opCode);
    ulong CALLccnn(const byte& opCode);
    ulong LDr_HL_(const byte& opCode);
    ulong LD_HL_r(const byte& opCode);
    ulong RETcc(const byte& opCode);
    ulong ADDHLss(const byte& opCode);
    ulong JPccnn(const byte& opCode);
    ulong ADDAr(const byte& opCode);
    ulong ADCAr(const byte& opCode);
    ulong JRcce(const byte& opCode);
    ulong ANDr(const byte& opCode);
    ulong CPr(const byte& opCode);
    ulong RSTn(const byte& opCode);

    ulong LD_BC_A(const byte& opCode);         // 0x02
    ulong RLCA(const byte& opCode);            // 0x07
    ulong LD_nn_SP(const byte& opCode);        // 0x08
    ulong LDA_BC_(const byte& opCode);         // 0x0A
    ulong RRCA(const byte& opCode);            // 0x0F
    ulong STOP(const byte& opCode);            // 0x10
    ulong LD_DE_A(const byte& opCode);         // 0x12
    ulong RLA(const byte& opCode);             // 0x17
    ulong JRe(const byte& opCode);             // 0x18
    ulong LDA_DE_(const byte& opCode);         // 0x1A
    ulong RRA(const byte& opCode);             // 0x1F
    ulong LDI_HL_A(const byte& opCode);        // 0x22
    ulong DAA(const byte& opCode);             // 0x27
    ulong LDIA_HL_(const byte& opCode);        // 0x2A
    ulong CPL(const byte& opCode);             // 0x2F
    ulong LDD_HL_A(const byte& opCode);        // 0x32
    ulong INC_HL_(const byte& opCode);         // 0x34
    ulong DEC_HL_(const byte& opCode);         // 0x35
    ulong LD_HL_n(const byte& opCode);         // 0x36
    ulong SCF(const byte& opCode);             // 0x37
    ulong LDDA_HL_(const byte& opCode);        // 0x3A
    ulong CCF(const byte& opCode);             // 0x3F
    ulong HALT(const byte& opCode);            // 0x76
    ulong ADDA_HL_(const byte& opCode);        // 0x86
    ulong ADCA_HL_(const byte& opCode);        // 0x8E
    ulong SUB_HL_(const byte& opCode);         // 0x96
    ulong SBCA_HL_(const byte& opCode);        // 0x9E
    ulong AND_HL_(const byte& opCode);         // 0xA6
    ulong XOR_HL_(const byte& opCode);         // 0xAE
    ulong OR_HL_(const byte& opCode);          // 0xB6
    ulong CP_HL_(const byte& opCode);          // 0xBE
    ulong JPnn(const byte& opCode);            // 0xC3
    ulong ADDAn(const byte& opCode);           // 0xC6
    ulong RET(const byte& opCode);             // 0xC9
    ulong CALLnn(const byte& opCode);          // 0xCD
    ulong ADCAn(const byte& opCode);           // 0xCE
    ulong SUBn(const byte& opCode);            // 0xD6
    ulong RETI(const byte& opCode);            // 0xD9
    ulong SBCAn(const byte& opCode);           // 0xDE
    ulong LD_0xFF00n_A(const byte& opCode);    // 0xE0
    ulong LD_0xFF00C_A(const byte& opCode);    // 0xE2
    ulong ANDn(const byte& opCode);            // 0xE6
    ulong ADDSPdd(const byte& opCode);         // 0xE8
    ulong JP_HL_(const byte& opCode);          // 0xE9
    ulong LD_nn_A(const byte& opCode);         // 0xEA
    ulong XORn(const byte& opCode);            // 0xEE
    ulong LDA_0xFF00n_(const byte& opCode);    // 0xF0
    ulong LDA_0xFF00C_(const byte& opCode);    // 0xF2
    ulong DI(const byte& opCode);              // 0xF3
    ulong ORn(const byte& opCode);             // 0xF6
    ulong LDHLSPe(const byte& opCode);         // 0xF8
    ulong LDSPHL(const byte& opCode);          // 0xF9
    ulong LDA_nn_(const byte& opCode);         // 0xFA
    ulong EI(const byte& opCode);              // 0xFB
    ulong CPn(const byte& opCode);             // 0xFE

    // Z80 Instruction Set - CB
    ulong RLCr(const byte& opCode);
    ulong RLC_HL_(const byte& opCode);
    ulong RRCr(const byte& opCode);
    ulong RRC_HL_(const byte& opCode);
    ulong RLr(const byte& opCode);
    ulong RL_HL_(const byte& opCode);
    ulong RRr(const byte& opCode);
    ulong RR_HL_(const byte& opCode);
    ulong SLAr(const byte& opCode);
    ulong SLA_HL_(const byte& opCode);
    ulong SRLr(const byte& opCode);
    ulong SRL_HL_(const byte& opCode);
    ulong SRAr(const byte& opCode);
    ulong SRA_HL_(const byte& opCode);
    ulong BITbr(const byte& opCode);
    ulong BITb_HL_(const byte& opCode);
    ulong RESbr(const byte& opCode);
    ulong RESb_HL_(const byte& opCode);
    ulong SETbr(const byte& opCode);
    ulong SETb_HL_(const byte& opCode);
    ulong SWAPr(const byte& opCode);
    ulong SWAP_HL_(const byte& opCode);
};