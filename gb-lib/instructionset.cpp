#include"instructionset.h"

InstructionSet::InstructionSet()
{
}

byte InstructionSet::GetHighByte(ushort dest)
{
    return ((dest >> 8) & 0xFF);
}

byte InstructionSet::GetLowByte(ushort dest)
{
    return (dest & 0xFF);
}

byte* InstructionSet::GetByteRegister(byte val)
{
    // Bottom 3 bits only
    return m_ByteRegisterMap[val & 0x07];
}

ushort* InstructionSet::GetUShortRegister(byte val, bool useAF)
{
    // Some instructions (PUSH rr and POP rr) use a dumb alternate mapping,
    // which replaces 0x03 (normally SP) with AF.
    if ((val & 0x03) == 0x03)
    {
        return useAF ? &m_AF : m_UShortRegisterMap[0x03];
    }
    else
    {
        // Bottom 2 bits only
        return m_UShortRegisterMap[val & 0x03];
    }
}

void InstructionSet::SetHighByte(ushort* dest, byte val)
{
    byte low = GetLowByte(*dest);
    *dest = (val << 8) | low;
}

void InstructionSet::SetLowByte(ushort* dest, byte val)
{
    byte high = GetHighByte(*dest);
    *dest = (high << 8) | val;
}

void InstructionSet::SetFlag(byte flag)
{
    // This shifts the bit to the left to where the flag is
    // Then ORs it with the Flag register.
    // Finally it filters out the lower 4 bits, as they aren't used on the Gameboy
    SetLowByte(&m_AF, SETBIT(GetLowByte(m_AF), flag) & 0xF0);
}

void InstructionSet::ClearFlag(byte flag)
{
    // This shifts the bit to the left to where the flag is
    // Then it inverts all of the bits
    // Then ANDs it with the Flag register.
    // Finally it filters out the lower 4 bits, as they aren't used on the Gameboy
    SetLowByte(&m_AF, CLEARBIT(GetLowByte(m_AF), flag) & 0xF0);
}

bool InstructionSet::IsFlagSet(byte flag)
{
    return ISBITSET(GetLowByte(m_AF), flag);
}

void InstructionSet::PushByteToSP(byte val)
{
    m_SP--;
    m_MMU->Write(m_SP, val);
}

void InstructionSet::PushUShortToSP(ushort val)
{
    PushByteToSP(GetHighByte(val));
    PushByteToSP(GetLowByte(val));
}

ushort InstructionSet::PopUShort()
{
    ushort val = m_MMU->ReadUShort(m_SP);
    m_SP += 2;
    return val;
}

byte InstructionSet::PopByte()
{
    byte val = m_MMU->Read(m_SP);
    m_SP++;
    return val;
}

byte InstructionSet::ReadBytePC()
{
    byte val = m_MMU->Read(m_PC);
    m_PC++;
    return val;
}

ushort InstructionSet::ReadUShortPC()
{
    ushort val = m_MMU->ReadUShort(m_PC);
    m_PC += 2;
    return val;
}

byte InstructionSet::AddByte(byte b1, byte b2)
{
    byte val = b1 + b2;

    ClearFlag(SubtractFlag);
    (val == 0x00) ? SetFlag(ZeroFlag) : ClearFlag(ZeroFlag);
    (((val ^ b2 ^ b1) & 0x10) == 0x10) ? SetFlag(HalfCarryFlag) : ClearFlag(HalfCarryFlag);
    (val < b1) ? SetFlag(CarryFlag) : ClearFlag(CarryFlag);

    return val;
}

ushort InstructionSet::AddUShort(ushort u1, ushort u2)
{
    ushort result = u1 + u2;

    ClearFlag(SubtractFlag);
    (result < u1) ? SetFlag(CarryFlag) : ClearFlag(CarryFlag);
    ((result ^ u1 ^ u2) & 0x1000) ? SetFlag(HalfCarryFlag) : ClearFlag(HalfCarryFlag);

    return result;
}

void InstructionSet::ADC(byte val)
{
    byte A = GetHighByte(m_AF);
    byte C = (IsFlagSet(CarryFlag)) ? 0x01 : 0x00;

    ClearFlag(SubtractFlag);

    if (((int)(A & 0x0F) + (int)(val & 0x0F) + (int)C) > 0x0F)
    {
        SetFlag(HalfCarryFlag);
    }
    else
    {
        ClearFlag(HalfCarryFlag);
    }

    if (((int)(A & 0xFF) + (int)(val & 0xFF) + (int)C) > 0xFF)
    {
        SetFlag(CarryFlag);
    }
    else
    {
        ClearFlag(CarryFlag);
    }

    byte result = A + val + C;
    SetHighByte(&m_AF, result);

    (result == 0x00) ? SetFlag(ZeroFlag) : ClearFlag(ZeroFlag);
}

void InstructionSet::SBC(byte val)
{
    int un = (int)val & 0xFF;
    int tmpa = (int)GetHighByte(m_AF) & 0xFF;
    int ua = tmpa;

    ua -= un;

    if (IsFlagSet(CarryFlag))
    {
        ua -= 1;
    }

    SetFlag(SubtractFlag);
    (ua < 0) ? SetFlag(CarryFlag) : ClearFlag(CarryFlag);

    ua &= 0xFF;

    (ua == 0x00) ? SetFlag(ZeroFlag) : ClearFlag(ZeroFlag);

    if (((ua ^ un ^ tmpa) & 0x10) == 0x10)
    {
        SetFlag(HalfCarryFlag);
    }
    else
    {
        ClearFlag(HalfCarryFlag);
    }

    SetHighByte(&m_AF, (byte)ua);
}

void InstructionSet::HandleInterrupts()
{
    // If the IME is enabled, some interrupts are enabled in IE, and
    // an interrupt flag is set, handle the interrupt.
    if (m_IME == 0x01)
    {
        byte IE = m_MMU->Read(0xFFFF);
        byte IF = m_MMU->Read(0xFF0F);

        // This will only match valid interrupts
        byte activeInterrupts = ((IE & IF) & 0x0F);
        if (activeInterrupts > 0x00)
        {
            m_IME = 0x00; // Disable further interrupts

            PushUShortToSP(m_PC); // Push current PC onto stack

            // Jump to the correct handler
            if (ISBITSET(activeInterrupts, 0))
            {
                // VBlank
                m_PC = INT40;
                IF = CLEARBIT(IF, 0);
            }
            else if (ISBITSET(activeInterrupts, 1))
            {
                // LCD status
                m_PC = INT48;
                IF = CLEARBIT(IF, 1);
            }
            else if (ISBITSET(activeInterrupts, 2))
            {
                // Timer
                m_PC = INT50;
                IF = CLEARBIT(IF, 2);
            }
            else if (ISBITSET(activeInterrupts, 3))
            {
                // Serial
                m_PC = INT58;
                IF = CLEARBIT(IF, 3);
            }
            else if (ISBITSET(activeInterrupts, 4))
            {
                // Joypad
                m_PC = INT60;
                IF = CLEARBIT(IF, 4);
            }

            m_MMU->Write(0xFF0F, IF);
        }
    }
}

/*
    CPU INSTRUCTION MAP
*/

// 0x00 (NOP)
ulong InstructionSet::NOP(const byte& opCode)
{
    // No flags affected
    return 4;
}

/*
    LD (bc), a
    00000010

    The contents of the accumulator are loaded to the memory location specified by
    the contents of the register pair BC.

    8 Cycles

    Flags affected(znhc): ----
*/
ulong InstructionSet::LD_BC_A(const byte& opCode)
{
    m_MMU->Write(m_BC, GetHighByte(m_AF));
    return 8;
}

/*
    RLCA
    00000111

    The contents of the accumulator are rotated left 1-bit position. Bit 7
    is copied to the carry flag and also to bit 0.

    4 Cycles

    Flags affected(znhc): 000c
*/
ulong InstructionSet::RLCA(const byte& opCode)
{
    byte r = GetHighByte(m_AF);

    // Grab bit 7 and store it in the carryflag
    ISBITSET(r, 7) ? SetFlag(CarryFlag) : ClearFlag(CarryFlag);

    // Shift r left
    r = r << 1;

    // Set bit 0 of r to the old CarryFlag
    r = IsFlagSet(CarryFlag) ? SETBIT(r, 0) : CLEARBIT(r, 0);

    SetHighByte(&m_AF, r);

    // Clear sZ, clears N, clears H, affects C
    ClearFlag(ZeroFlag);
    ClearFlag(SubtractFlag);
    ClearFlag(HalfCarryFlag);

    return 4;
}

/*
    LD r, n
    00rrr110 nnnnnnnn

    The 8-bit integer n is loaded to any register r, where r identifies register
    A, B, C, D, E, H, or L.

    8 Cycles

    Flags affected(znhc): ----
*/
ulong InstructionSet::LDrn(const byte& opCode)
{
    byte n = ReadBytePC();
    byte* r = GetByteRegister(opCode >> 3);
    (*r) = n;

    return 8;
}

/*
    LD r, R
    01rrrRRR

    The contents of any register R are loaded into another other register r, where
    R and r identify a register A, B, C, D, E, H, or L.

    4 Cycles

    Flags affected(znhc): ----
*/
ulong InstructionSet::LDrR(const byte& opCode)
{
    byte* r = GetByteRegister(opCode >> 3);
    byte* R = GetByteRegister(opCode);
    (*r) = *R;

    return 4;
}

/*
    LD r, (hl)
    01rrr110

    The contents of the memory location (HL) are loaded into register r, where
    r identifies a register A, B, C, D, E, H, or L.

    8 Cycles

    Flags affected(znhc): ----
*/
ulong InstructionSet::LDr_HL_(const byte& opCode)
{
    byte* r = GetByteRegister(opCode >> 3);

    (*r) = m_MMU->Read(m_HL);

    return 8;
}

/*
    LD (HL), r
    01110rrr

    The contents of register r are loaded into the memory locoation specifed by the
    contents of the HL register pair. The operand r identifies register A, B, C, D, E,
    H, or L.

    8 Cycles

    Flags affected(znhc): ----
*/
ulong InstructionSet::LD_HL_r(const byte& opCode)
{
    byte* r = GetByteRegister(opCode);
    m_MMU->Write(m_HL, (*r)); // Load r into the address pointed at by HL.

    return 8;
}


/*
    LD rr, nn
    00rr0001 nnnnnnnn nnnnnnnn

    The 2-byte integer nn is loaded into the rr register pair, where rr defines
    the BC, DL, HL, or SP register pairs.

    12 Cycles

    Flags affected(znhc): ----
*/
ulong InstructionSet::LDrrnn(const byte& opCode)
{
    ushort* rr = GetUShortRegister(opCode >> 4, false);
    ushort nn = ReadUShortPC(); // Read nn
    (*rr) = nn;

    return 12;
}

/*
    INC r
    00rrr100

    Register r is incremented, where r identifies register A, B, C, D, E, H, or L.

    4 Cycles

    Flags affected(znhc): z0h-
*/
ulong InstructionSet::INCr(const byte& opCode)
{
    byte* r = GetByteRegister(opCode >> 3);
    bool isBit3Before = ISBITSET(*r, 3);
    *r += 1;
    bool isBit3After = ISBITSET(*r, 3);

    if (*r == 0x00)
    {
        SetFlag(ZeroFlag);
    }
    else
    {
        ClearFlag(ZeroFlag);
    }

    ClearFlag(SubtractFlag);

    if (isBit3Before && !isBit3After)
    {
        SetFlag(HalfCarryFlag);
    }
    else
    {
        ClearFlag(HalfCarryFlag);
    }

    return 4;
}

/*
    CALL cc, nn
    11ccc100

    000 NZ
    001 Z
    010 NC
    011 C

    24 Cycles if taken
    12 Cycles if not taken

    Flags affected(znhc): ----
*/
ulong InstructionSet::CALLccnn(const byte& opCode)
{
    ushort nn = ReadUShortPC();

    bool check = false;
    switch ((opCode >> 3) & 0x03)
    {
    case 0x00:  // NZ
        check = !IsFlagSet(ZeroFlag);
        break;
    case 0x01:  // Z
        check = IsFlagSet(ZeroFlag);
        break;
    case 0x02:  // NC
        check = !IsFlagSet(CarryFlag);
        break;
    case 0x03:  // C
        check = IsFlagSet(CarryFlag);
        break;
    }

    if (check)
    {
        PushUShortToSP(m_PC);
        m_PC = nn;
        return 24;
    }
    else
    {
        return 12;
    }
}

/*
    RET cc
    11ccc000

    000 NZ
    001 Z
    010 NC
    011 C

    20 Cycles if taken
    8 Cycles if not taken

    Flags affected(znhc): ----
*/
ulong InstructionSet::RETcc(const byte& opCode)
{
    bool check = false;
    switch ((opCode >> 3) & 0x03)
    {
    case 0x00:  // NZ
        check = !IsFlagSet(ZeroFlag);
        break;
    case 0x01:  // Z
        check = IsFlagSet(ZeroFlag);
        break;
    case 0x02:  // NC
        check = !IsFlagSet(CarryFlag);
        break;
    case 0x03:  // C
        check = IsFlagSet(CarryFlag);
        break;
    }

    if (check)
    {
        m_PC = PopUShort();
        return 20;
    }
    else
    {
        return 8;
    }
}

/*
    LD (nn), SP - 0x08

    The contents of the stack pointer are loaded into the address specified by the
    operand nn.

    20 Cycles

    Flags affected(znhc): ----
*/
ulong InstructionSet::LD_nn_SP(const byte& opCode)
{
    ushort nn = ReadUShortPC();

    // Load A into (nn)
    m_MMU->Write(nn + 1, GetHighByte(m_SP));
    m_MMU->Write(nn, GetLowByte(m_SP));

    return 20;
}

/*
    ADD HL, ss
    00ss1001

    The contents of the register pair ss (BC, DE, HL, SP) are added to the contents
    of the register pair HL and the result is stored in HL.

    8 Cycles

    Flags affected(znhc): -0hc
*/
ulong InstructionSet::ADDHLss(const byte& opCode)
{
    ushort* ss = GetUShortRegister(opCode >> 4, false);

    m_HL = AddUShort(m_HL, *ss);

    return 8;
}

/*
    ADD SP, dd
    0xE8

    16 Cycles

    Flags affected(znhc): 00hc
*/
ulong InstructionSet::ADDSPdd(const byte& opCode)
{
    sbyte arg = static_cast<sbyte>(ReadBytePC());
    ushort result = (m_SP + arg);

    ClearFlag(ZeroFlag);
    ClearFlag(SubtractFlag);
    ((result & 0xF) < (m_SP & 0xF)) ? SetFlag(HalfCarryFlag) : ClearFlag(HalfCarryFlag);
    ((result & 0xFF) < (m_SP & 0xFF)) ? SetFlag(CarryFlag) : ClearFlag(CarryFlag);

    m_SP = result;

    return 16;
}

/*
    JP cc, nn
    11ccc010

    000 NZ
    001 Z
    010 NC
    011 C

    16 Cycles if taken
    12 Cycles if not taken

    Flags affected(znhc): ----
*/
ulong InstructionSet::JPccnn(const byte& opCode)
{
    ushort nn = ReadUShortPC();

    bool check = false;
    switch ((opCode >> 3) & 0x03)
    {
    case 0x00:  // NZ
        check = !IsFlagSet(ZeroFlag);
        break;
    case 0x01:  // Z
        check = IsFlagSet(ZeroFlag);
        break;
    case 0x02:  // NC
        check = !IsFlagSet(CarryFlag);
        break;
    case 0x03:  // C
        check = IsFlagSet(CarryFlag);
        break;
    }

    if (check)
    {
        m_PC = nn;
        return 16;
    }
    else
    {
        return 12;
    }
}

/*
    ADD A, r
    10000rrr

    The contents of the r register are added to the contents of the accumulator,
    and the result is stored in the accumulator. The operand r identifies the registers
    A, B, C, D, E, H, or L.

    4 Cycles

    Flags affected(znhc): z0hc
*/
ulong InstructionSet::ADDAr(const byte& opCode)
{
    byte A = GetHighByte(m_AF);
    byte* r = GetByteRegister(opCode);

    SetHighByte(&m_AF, AddByte(A, *r));

    return 4;
}

/*
    ADC A, r
    10001rrr

    The contents of the r register, and the contents of the carry flag,
    are added to the contents of the accumulator, and the result is stored
    in the accumulator. The operand r identifies the registers A, B, C, D, E, H, or L.

    4 Cycles

    Flags affected(znhc): z0hc
*/
ulong InstructionSet::ADCAr(const byte& opCode)
{
    byte* r = GetByteRegister(opCode);
    ADC(*r);
    return 4;
}

/*
    JR cc, nn
    001cc000

    00 NZ
    01 Z
    10 NC
    11 C

    12 Cycles if taken
    8 Cycles if not taken

    Flags affected(znhc): ----
*/
ulong InstructionSet::JRcce(const byte& opCode)
{
    sbyte arg = static_cast<sbyte>(ReadBytePC());

    bool check = false;
    switch ((opCode >> 3) & 0x03)
    {
    case 0x00:  // NZ
        check = !IsFlagSet(ZeroFlag);
        break;
    case 0x01:  // Z
        check = IsFlagSet(ZeroFlag);
        break;
    case 0x02:  // NC
        check = !IsFlagSet(CarryFlag);
        break;
    case 0x03:  // C
        check = IsFlagSet(CarryFlag);
        break;
    }

    if (check)
    {
        m_PC += arg;
        return 12;
    }
    else
    {
        return 8;
    }
}

/*
    RST
    11ttt111

    000 0x00
    001 0x08
    010 0x10
    011 0x18
    100 0x20
    101 0x28
    110 0x30
    111 0x38

    16 Cycles if taken

    Flags affected(znhc): ----
*/
ulong InstructionSet::RSTn(const byte& opCode)
{
    byte t = ((opCode >> 3) & 0x07);

    PushUShortToSP(m_PC);
    m_PC = (ushort)(t * 0x08);
    return 16;
}

/*
    AND r
    10100rrr

    The logical AND operation is performed between the register specified in the r
    operand and the byte contained in the accumulator. The result is stored in the accumulator.
    Register r can be A, B, C, D, E, H, or L.

    4 Cycles

    Flags affected(znhc): z010
*/
ulong InstructionSet::ANDr(const byte& opCode)
{
    byte* r = GetByteRegister(opCode);
    byte result = (*r) & GetHighByte(m_AF);
    SetHighByte(&m_AF, result);

    if (result == 0x00)
    {
        SetFlag(ZeroFlag);
    }
    else
    {
        ClearFlag(ZeroFlag);
    }

    ClearFlag(SubtractFlag);
    SetFlag(HalfCarryFlag);
    ClearFlag(CarryFlag);

    return 4;
}

/*
    CP r
    10111rrr

    The contents of 8-bit register r are compared with the contents of the accumulator.
    If there is a true compare, the Z flag is set. The execution of this instruction
    does not affect the contents of the accumulator.

    4 Cycles

    Flags affected(znhc): z1hc
*/
ulong InstructionSet::CPr(const byte& opCode)
{
    byte* r = GetByteRegister(opCode);
    byte A = GetHighByte(m_AF);
    byte result = A - (*r);

    (result == 0x00) ? SetFlag(ZeroFlag) : ClearFlag(ZeroFlag);
    SetFlag(SubtractFlag);
    ((A & 0xFF) < ((*r) & 0xFF)) ? SetFlag(CarryFlag) : ClearFlag(CarryFlag);
    ((A & 0x0F) < ((*r) & 0x0F)) ? SetFlag(HalfCarryFlag) : ClearFlag(HalfCarryFlag);

    return 4;
}

/*
    INC rr
    00rr0011

    16-bit register rr is incremented, where rr identifies register pairs BC, DE, HL, or SP.

    8 Cycles

    Flags affected(znhc): ----
*/
ulong InstructionSet::INCrr(const byte& opCode)
{
    ushort* rr = GetUShortRegister(opCode >> 4, false);
    *rr += 1;

    return 8;
}

/*
    DEC rr
    00rr1011

    16-bit register rr is decremented, where rr identifies register pairs BC, DE, HL, or SP.

    8 Cycles

    Flags affected(znhc): ----
*/
ulong InstructionSet::DECrr(const byte& opCode)
{
    ushort* rr = GetUShortRegister(opCode >> 4, false);
    *rr -= 1;

    return 8;
}

/*
    XOR r
    10101rrr

    The logical exclusive-OR operation is performed between the register specified in the r
    operand and the byte contained in the accumulator. The result is stored in the accumulator.
    Register r can be A, B, C, D, E, H, or L.

    4 Cycles

    Flags affected(znhc): z000
*/
ulong InstructionSet::XORr(const byte& opCode)
{
    byte* r = GetByteRegister(opCode);
    SetHighByte(&m_AF, *r ^ GetHighByte(m_AF));

    // Affects Z and clears NHC
    if (GetHighByte(m_AF) == 0x00)
    {
        SetFlag(ZeroFlag);
    }
    else
    {
        ClearFlag(ZeroFlag);
    }

    ClearFlag(SubtractFlag);
    ClearFlag(HalfCarryFlag);
    ClearFlag(CarryFlag);

    return 4;
}

/*
    XOR (HL) - 0xAE

    The logical exclusive-OR operation is performed between the byte pointed to by the HL register
    and the byte contained in the accumulator. The result is stored in the accumulator.

    8 Cycles

    Flags affected(znhc): z000
*/
ulong InstructionSet::XOR_HL_(const byte& opCode)
{
    byte r = m_MMU->Read(m_HL);
    SetHighByte(&m_AF, r ^ GetHighByte(m_AF));

    // Affects Z and clears NHC
    if (GetHighByte(m_AF) == 0x00)
    {
        SetFlag(ZeroFlag);
    }
    else
    {
        ClearFlag(ZeroFlag);
    }

    ClearFlag(SubtractFlag);
    ClearFlag(HalfCarryFlag);
    ClearFlag(CarryFlag);

    return 8;
}

/*
    OR r
    10110rrr

    The logical OR operation is performed between the register specified in the r
    operand and the byte contained in the accumulator. The result is stored in the accumulator.
    Register r can be A, B, C, D, E, H, or L.

    4 Cycles

    Flags affected(znhc): z000
*/
ulong InstructionSet::ORr(const byte& opCode)
{
    byte* r = GetByteRegister(opCode);
    SetHighByte(&m_AF, *r | GetHighByte(m_AF));

    // Affects Z and clears NHC
    if (GetHighByte(m_AF) == 0x00)
    {
        SetFlag(ZeroFlag);
    }
    else
    {
        ClearFlag(ZeroFlag);
    }

    ClearFlag(SubtractFlag);
    ClearFlag(HalfCarryFlag);
    ClearFlag(CarryFlag);

    return 4;
}

/*
    OR (HL) - 0xB6

    The logical OR operation is performed between the value at memory location (HL)
    and the byte contained in the accumulator. The result is stored in the accumulator.

    8 Cycles

    Flags affected(znhc): z000
*/
ulong InstructionSet::OR_HL_(const byte& opCode)
{
    byte r = m_MMU->Read(m_HL);
    SetHighByte(&m_AF, r | GetHighByte(m_AF));

    // Affects Z and clears NHC
    if (GetHighByte(m_AF) == 0x00)
    {
        SetFlag(ZeroFlag);
    }
    else
    {
        ClearFlag(ZeroFlag);
    }

    ClearFlag(SubtractFlag);
    ClearFlag(HalfCarryFlag);
    ClearFlag(CarryFlag);

    return 8;
}

/*
    PUSH rr
    11qq0101

    The contents of the register pair rr are pushed to the external memory stack.
    The stack pointer holds the 16-bit address of the current top of the stack.
    This instruction first decrements SP and loads the high order byte of register
    pair rr to the memory address specified by the SP. The SP is decremented again,
    and then the low order byte is then loaded to the new memory address. The operand
    rr identifies register pair BC, DE, HL, or AF.

    16 Cycles

    Flags affected(znhc): ----
*/
ulong InstructionSet::PUSHrr(const byte& opCode)
{
    ushort* rr = GetUShortRegister(opCode >> 4, true);
    PushUShortToSP(*rr);

    return 16;
}

/*
    AND n
    11100110 (0xE6)

    The logical AND operation is performed between the byte specified in n and the byte contained
    in the accumulator. The resutl is stored in the accumulator.

    8 Cycles

    Flags affected(znhc): z010
*/
ulong InstructionSet::ANDn(const byte& opCode)
{
    byte n = ReadBytePC();

    byte result = GetHighByte(m_AF) & n;
    SetHighByte(&m_AF, result);

    (result == 0x00) ? SetFlag(ZeroFlag) : ClearFlag(ZeroFlag);
    ClearFlag(SubtractFlag);
    SetFlag(HalfCarryFlag);
    ClearFlag(CarryFlag);

    return 8;
}

/*
    JP HL - 0xE9

    The PC is loaded with the value of HL.

    4 Cycles

    Flags affected(znhc): ----
*/
ulong InstructionSet::JP_HL_(const byte& opCode)
{
    m_PC = m_HL;
    return 4;
}

/*
    POP rr
    11qq0001

    The top two bytes of the external memory stack are popped into register pair qq.
    The stack pointer holds the 16-bit address of the current top of the stack. This
    instruction first loads to the low order portion of rr. The SP is incremented and
    the contents of the corresponding adjacent memory ocation are loaded into the high
    order portion of rr. The SP is then incremented again. The operand rr identifies
    register pair BC, DE, HL, or AF.

    12 Cycles

    Flags affected(znhc): ----
*/
ulong InstructionSet::POPrr(const byte& opCode)
{
    ushort* rr = GetUShortRegister(opCode >> 4, true);
    (*rr) = PopUShort();

    if (((opCode >> 4) & 0x03) == 0x03)
    {
        (*rr) &= 0xFFF0;
    }

    return 12;
}

/*
    DEC r
    00rrr101

    Register r is decremented, where r identifies register A, B, C, D, E, H, or L.

    4 Cycles

    Flags affected(znhc): z1h-
*/
ulong InstructionSet::DECr(const byte& opCode)
{
    byte* r = GetByteRegister(opCode >> 3);
    byte calc = (*r - 1);

    SetFlag(SubtractFlag);
    (calc == 0x00) ? SetFlag(ZeroFlag) : ClearFlag(ZeroFlag);

    if (((calc ^ 0x01 ^ *r) & 0x10) == 0x10)
    {
        SetFlag(HalfCarryFlag);
    }
    else
    {
        ClearFlag(HalfCarryFlag);
    }

    *r = calc;

    return 4;
}

/*
    INC (HL) - 0x34

    The byte at the address (HL) is incremented.

    12 Cycles

    Flags affected(znhc): z0h-
*/
ulong InstructionSet::INC_HL_(const byte& opCode)
{
    byte HL = m_MMU->Read(m_HL);
    bool isBit3Before = ISBITSET(HL, 3);
    HL += 1;
    bool isBit3After = ISBITSET(HL, 3);

    m_MMU->Write(m_HL, HL);

    if (HL == 0x00)
    {
        SetFlag(ZeroFlag);
    }
    else
    {
        ClearFlag(ZeroFlag);
    }

    ClearFlag(SubtractFlag);

    if (isBit3Before && !isBit3After)
    {
        SetFlag(HalfCarryFlag);
    }
    else
    {
        ClearFlag(HalfCarryFlag);
    }

    return 12;
}

/*
    DEC (hl) - 0x35

    The byte at the address (HL) is decremented.

    12 Cycles

    Flags affected(znhc): z1h-
*/
ulong InstructionSet::DEC_HL_(const byte& opCode)
{
    byte val = m_MMU->Read(m_HL);
    byte calc = (val - 1);

    SetFlag(SubtractFlag);
    (calc == 0x00) ? SetFlag(ZeroFlag) : ClearFlag(ZeroFlag);

    if (((calc ^ 0x01 ^ val) & 0x10) == 0x10)
    {
        SetFlag(HalfCarryFlag);
    }
    else
    {
        ClearFlag(HalfCarryFlag);
    }

    m_MMU->Write(m_HL, calc);

    return 12;
}

/*
    LD (HL), n - 0x36

    The contents of n are loaded into the memory location specifed by the
    contents of the HL register pair.

    12 Cycles

    Flags affected(znhc): ----
*/
ulong InstructionSet::LD_HL_n(const byte& opCode)
{
    byte n = ReadBytePC();
    m_MMU->Write(m_HL, n); // Load n into the address pointed at by HL.

    return 12;
}

/*
    SCF - 0x37

    Sets the carry flag.

    4 Cycles

    Flags affected(znhc): -001
*/
ulong InstructionSet::SCF(const byte& opCode)
{
    ClearFlag(SubtractFlag);
    ClearFlag(HalfCarryFlag);
    SetFlag(CarryFlag);

    return 4;
}

/*
    CCF - 0x3F

    Toggle the carry flag.

    4 Cycles

    Flags affected(znhc): -00c
*/
ulong InstructionSet::CCF(const byte& opCode)
{
    ClearFlag(SubtractFlag);
    ClearFlag(HalfCarryFlag);
    IsFlagSet(CarryFlag) ? ClearFlag(CarryFlag) : SetFlag(CarryFlag);

    return 4;
}

/*
    SUB r
    10010rrr

    The 8-bit register r is subtracted from the contents of the Accumulator, and the
    result is stored in the accumulator.

    4 Cycles

    Flags affected(znhc): z1hc
*/
ulong InstructionSet::SUBr(const byte& opCode)
{
    byte* r = GetByteRegister(opCode);
    byte A = GetHighByte(m_AF);
    byte result = A - (*r);
    SetHighByte(&m_AF, result);

    (result == 0x00) ? SetFlag(ZeroFlag) : ClearFlag(ZeroFlag);
    SetFlag(SubtractFlag);
    ((A & 0x0F) < ((*r) & 0x0F)) ? SetFlag(HalfCarryFlag) : ClearFlag(HalfCarryFlag);
    ((A & 0xFF) < ((*r) & 0xFF)) ? SetFlag(CarryFlag) : ClearFlag(CarryFlag);

    return 4;
}

/*
    SBC A, r
    10011rrr

    The 8-bit register r and the contents of the carry flag are subtracted from the
    contents of the Accumulator, and the result is stored in the accumulator.

    4 Cycles

    Flags affected(znhc): z1hc
*/
ulong InstructionSet::SBCAr(const byte& opCode)
{
    byte* r = GetByteRegister(opCode);
    SBC(*r);
    return 4;
}

/*
    STOP - 0x10

    For the purposes of this emulator, this is identical to HALT.

    0 Cycles

    Flags affected(znhc): ----
*/
ulong InstructionSet::STOP(const byte& opCode)
{
    return HALT(opCode);
}

/*
    LD (de), a
    00010010

    The contents of the accumulator are loaded to the memory location specified by
    the contents of the register pair DE.

    8 Cycles

    Flags affected(znhc): ----
*/
ulong InstructionSet::LD_DE_A(const byte& opCode)
{
    m_MMU->Write(m_DE, GetHighByte(m_AF));
    return 8;
}

/*
    RL A - 0x17
*/
ulong InstructionSet::RLA(const byte& opCode)
{
    // Grab the current CarryFlag val
    bool carry = IsFlagSet(CarryFlag);

    // Grab bit 7 and store it in the carryflag
    ISBITSET(GetHighByte(m_AF), 7) ? SetFlag(CarryFlag) : ClearFlag(CarryFlag);

    // Shift A left
    SetHighByte(&m_AF, GetHighByte(m_AF) << 1);

    // Set bit 0 of A to the old CarryFlag
    SetHighByte(&m_AF, carry ? SETBIT(GetHighByte(m_AF), 0) : CLEARBIT(GetHighByte(m_AF), 0));

    // Clears Z, clears N, clears H, affects C
    ClearFlag(ZeroFlag);
    ClearFlag(SubtractFlag);
    ClearFlag(HalfCarryFlag);

    return 4;
}

/*
    JR Z, e - 0x18

    Jump relative, to the offset e.

    12 cycles

    Flags affected(znhc): ----
*/
ulong InstructionSet::JRe(const byte& opCode)
{
    sbyte e = static_cast<sbyte>(ReadBytePC());

    m_PC += e;
    return 12;
}

/*
    LD A, (DE) - 0x1A

    Loads the value stored at the address pointed to by DE and stores in the A register.

    8 Cycles

    Flags affected(znhc): ----
*/
ulong InstructionSet::LDA_DE_(const byte& opCode)
{
    byte val = m_MMU->Read(m_DE);
    SetHighByte(&m_AF, val);
    return 8;
}

/*
    LD A, (BC) - 0x0A

    Loads the value stored at the address pointed to by BC into the accumulator.

    8 Cycles

    Flags affected(znhc): ----
*/
ulong InstructionSet::LDA_BC_(const byte& opCode)
{
    byte val = m_MMU->Read(m_BC);
    SetHighByte(&m_AF, val);

    return 8;
}

/*
    RRCA - 0x0F

    The contents off the accumulator are roated right 1-bit. Bit 0 is copied to the carry
    flag and also to bit 7.

    4 Cycles

    Flags affected(znhc): 000c
*/
ulong InstructionSet::RRCA(const byte& opCode)
{
    byte A = GetHighByte(m_AF);
    bool carry = ISBITSET(A, 0);

    A = A >> 1;

    if (carry)
    {
        SetFlag(CarryFlag);
        A = SETBIT(A, 7);
    }
    else
    {
        ClearFlag(CarryFlag);
        A = CLEARBIT(A, 7);
    }

    SetHighByte(&m_AF, A);

    ClearFlag(ZeroFlag);
    ClearFlag(SubtractFlag);
    ClearFlag(HalfCarryFlag);

    return 4;
}

/*
    RR A - 0x17

    4 Cycles

    Flags affected(znhc): 000c
*/
ulong InstructionSet::RRA(const byte& opCode)
{
    // Grab the current CarryFlag val
    bool carry = IsFlagSet(CarryFlag);

    // Grab bit 0 and store it in the carryflag
    ISBITSET(GetHighByte(m_AF), 0) ? SetFlag(CarryFlag) : ClearFlag(CarryFlag);

    // Shift A right
    SetHighByte(&m_AF, GetHighByte(m_AF) >> 1);

    // Set bit 7 of A to the old CarryFlag
    SetHighByte(&m_AF, carry ? SETBIT(GetHighByte(m_AF), 7) : CLEARBIT(GetHighByte(m_AF), 7));

    // Affects Z, clears N, clears H, affects C
    ClearFlag(ZeroFlag);
    ClearFlag(SubtractFlag);
    ClearFlag(HalfCarryFlag);

    return 4;
}

/*
    LDI (HL), A - 0x22

    Loads A into the address pointed at by HL, then increment HL.

    8 Cycles

    Flags affected(znhc): ----
*/
ulong InstructionSet::LDI_HL_A(const byte& opCode)
{
    m_MMU->Write(m_HL, GetHighByte(m_AF)); // Load A into the address pointed at by HL.

    m_HL++;

    return 8;
}

/*
    DAA - 0x27

    This instruction conditionally adjust the accumulator for BCD addition and
    subtraction operations. For addition (ADD, ADC, INC) or subtraction (SUB, SBC, DEC, NEG),
    the following table indicates the operation performed:

    OP      C Before    U       H Before    L       Add     C After
            0           0-9     0           0-9     00      0
    ADD     0           0-8     0           A-F     06      0
    ADC     0           A-F     0           0-9     60      1
    INC     0           9-F     0           A-F     66      1
            0           0-9     1           0-3     06      0*
            0           A-F     1           0-3     66      1*
            1           0-2     0           0-9     60      1*
            1           0-2     0           A-F     66      1*
            1           0-3     1           0-3     66      1*
    ----------------------
    SUB     0           0-9     0           0-9     00      0*
    SBC     0           0-8     1           6-F     FA      0*
    DEC     1           7-F     0           0-9     A0      1*
    NEG     1           6-7     1           6-F     9A      1*

    4 Cycles

    Flags affected(znhc): z-0x
*/
ulong InstructionSet::DAA(const byte& opCode)
{
    int aVal = GetHighByte(m_AF);

    if (!IsFlagSet(SubtractFlag))
    {
        if (IsFlagSet(HalfCarryFlag) || (aVal & 0xF) > 9)
        {
            aVal += 0x06;
        }

        if (IsFlagSet(CarryFlag) || (aVal > 0x9F))
        {
            aVal += 0x60;
        }
    }
    else
    {
        if (IsFlagSet(HalfCarryFlag))
        {
            aVal = (aVal - 0x06) & 0xFF;
        }

        if (IsFlagSet(CarryFlag))
        {
            aVal -= 0x60;
        }
    }

    ClearFlag(HalfCarryFlag);

    if ((aVal & 0x100) == 0x100)
    {
        SetFlag(CarryFlag);
    }

    aVal &= 0xFF;

    (aVal == 0x00) ? SetFlag(ZeroFlag) : ClearFlag(ZeroFlag);
    SetHighByte(&m_AF, (byte)aVal);

    return 4;
}

/*
    LDI A, (HL) - 0x2A

    Loads the address pointed at by HL into A, then increment HL.

    8 Cycles

    Flags affected(znhc): ----
*/
ulong InstructionSet::LDIA_HL_(const byte& opCode)
{
    SetHighByte(&m_AF, m_MMU->Read(m_HL));
    m_HL++;

    return 8;
}

/*
    CPL - 0x2F

    The contents of the accumulator are inverted.

    4 Cycles

    Flags affected(znhc): -11-
*/
ulong InstructionSet::CPL(const byte& opCode)
{
    byte A = GetHighByte(m_AF);
    byte result = A ^ 0xFF;
    SetHighByte(&m_AF, result);

    SetFlag(SubtractFlag);
    SetFlag(HalfCarryFlag);

    return 4;
}

/*
    LDD (HL), A - 0x32

    Load A into the address pointed at by HL.

    8 Cycles

    Flags affected(znhc): ----
*/
ulong InstructionSet::LDD_HL_A(const byte& opCode)
{
    m_MMU->Write(m_HL, GetHighByte(m_AF));

    m_HL--;
    return 8;
}

/*
    LDD A, (HL) - 0x3A

    Load the byte at the address specified in the HL register into A, and decrement HL.

    8 Cycles

    Flags affected(znhc): ----
*/
ulong InstructionSet::LDDA_HL_(const byte& opCode)
{
    byte HL = m_MMU->Read(m_HL);
    SetHighByte(&m_AF, HL);

    m_HL--;
    return 8;
}

/*
    HALT - 0x76

    0 Cycles

    Flags affected(znhc): ----
*/
ulong InstructionSet::HALT(const byte& opCode)
{
    m_isHalted = true;
    //m_IFWhenHalted = m_MMU->Read(0xFF0F);
    return 0;
}

/*
    ADD A, (HL) - 0x86

    The byte at the memory address specified by the contents of the HL register pair
    is added to the contents of the accumulator, and the result is stored in the
    accumulator.

    8 Cycles

    Flags affected(znhc): z0hc
*/
ulong InstructionSet::ADDA_HL_(const byte& opCode)
{
    byte A = GetHighByte(m_AF);
    byte HL = m_MMU->Read(m_HL);
    SetHighByte(&m_AF, AddByte(A, HL));

    return 8;
}

/*
    ADC A, (HL) - 0x8E

    The contents specified by the address of the HL register pair and the contents of
    the carry flag are added to the contents of the accumulator, and the result is
    stored in the accumulator.

    8 Cycles

    Flags affected(znhc): z0hc
*/
ulong InstructionSet::ADCA_HL_(const byte& opCode)
{
    byte HL = m_MMU->Read(m_HL);
    ADC(HL);
    return 8;
}

/*
    SUB (HL) - 0x96

    The byte at the memory address specified by the contents of the HL register pair
    is subtracted to the contents of the accumulator, and the result is stored in the
    accumulator.

    8 Cycles

    Flags affected(znhc): z1hc
*/
ulong InstructionSet::SUB_HL_(const byte& opCode)
{
    byte A = GetHighByte(m_AF);
    byte HL = m_MMU->Read(m_HL);
    byte result = A - HL;
    SetHighByte(&m_AF, result);

    (result == 0x00) ? SetFlag(ZeroFlag) : ClearFlag(ZeroFlag);
    SetFlag(SubtractFlag);
    ((A & 0x0F) < (result & 0x0F)) ? SetFlag(HalfCarryFlag) : ClearFlag(HalfCarryFlag);
    ((A & 0xFF) < (result & 0xFF)) ? SetFlag(CarryFlag) : ClearFlag(CarryFlag);

    return 8;
}

/*
    SBC A, (HL) - 0x9E

    The byte at the memory address specified by the contents of the HL register pair
    and the accumulator is subtracted from the contents of the accumulator, and
    the result is stored in the accumulator.

    8 Cycles

    Flags affected(znhc): z1hc
*/
ulong InstructionSet::SBCA_HL_(const byte& opCode)
{
    byte HL = m_MMU->Read(m_HL);
    SBC(HL);
    return 8;
}

/*
    AND (HL) - 0xA6

    The logical AND operation is performed between the byte contained at
    the memory address specified by the HL register  and the byte contained
    in the accumulator. The result is stored in the accumulator.

    8 Cycles

    Flags affected(znhc): z010
*/
ulong InstructionSet::AND_HL_(const byte& opCode)
{
    byte HL = m_MMU->Read(m_HL);
    byte result = HL & GetHighByte(m_AF);
    SetHighByte(&m_AF, result);

    if (result == 0x00)
    {
        SetFlag(ZeroFlag);
    }
    else
    {
        ClearFlag(ZeroFlag);
    }

    ClearFlag(SubtractFlag);
    SetFlag(HalfCarryFlag);
    ClearFlag(CarryFlag);

    return 8;
}

/*
    CP (HL) - 0xBE

    The contents of 8-bit register HL are compared with the contents of the accumulator.
    If there is a true compare, the Z flag is set. The execution of this instruction
    does not affect the contents of the accumulator.

    8 Cycles

    Flags affected(znhc): z1hc
*/
ulong InstructionSet::CP_HL_(const byte& opCode)
{
    byte HL = m_MMU->Read(m_HL);
    byte A = GetHighByte(m_AF);
    byte result = A - HL;

    (result == 0x00) ? SetFlag(ZeroFlag) : ClearFlag(ZeroFlag);
    SetFlag(SubtractFlag);
    ((A & 0x0F) < (HL & 0x0F)) ? SetFlag(HalfCarryFlag) : ClearFlag(HalfCarryFlag);
    ((A & 0xFF) < (HL & 0xFF)) ? SetFlag(CarryFlag) : ClearFlag(CarryFlag);

    return 8;
}

/*
    JP nn - 0xC3

    Jump to nn

    16 Cycles

    Flags affected(znhc): ----
*/
ulong InstructionSet::JPnn(const byte& opCode)
{
    ushort nn = ReadUShortPC();
    m_PC = nn;

    return 16;
}

/*
    ADD A, n - 0xC6

    The integer n is added to the contents of the accumulator, and the reults are
    stored in the accumulator.

    8 Cycles

    Flags affected(znhc): z0hc
    Affects Z, clears n, affects h, affects c
*/
ulong InstructionSet::ADDAn(const byte& opCode)
{
    byte n = ReadBytePC();
    byte A = GetHighByte(m_AF);

    SetHighByte(&m_AF, AddByte(A, n));

    return 8;
}

/*
    RET - 0xC9

    return, PC=(SP), SP=SP+2

    16 Cycles

    Flags affected(znhc): ----
*/
ulong InstructionSet::RET(const byte& opCode)
{
    m_PC = PopUShort();

    return 16;
}

/*
    CALL nn - 0xCD

    Pushes the PC to the SP, then sets the PC to the target address(nn).

    24 Cycles

    Flags affected(znhc): ----
*/
ulong InstructionSet::CALLnn(const byte& opCode)
{
    ushort nn = ReadUShortPC(); // Read nn
    PushUShortToSP(m_PC); // Push PC to SP
    m_PC = nn; // Set the PC to the target address

    return 24;
}

/*
    ADC A, n - 0xCE

    The 8-bit n operand, along with the carry flag, is added to the contents of the
    accumulator, and the result is stored in the accumulator.

    8 Cycles

    Flags affected(znhc): z0hc
*/
ulong InstructionSet::ADCAn(const byte& opCode)
{
    ADC(ReadBytePC());
    return 8;
}

/*
    SUB n - 0xD6

    The 8-bit value n is subtracted from the contents of the Accumulator, and the
    result is stored in the accumulator.

    8 Cycles

    Flags affected(znhc): z1hc
*/
ulong InstructionSet::SUBn(const byte& opCode)
{
    byte n = ReadBytePC();
    byte A = GetHighByte(m_AF);
    byte result = A - n;
    SetHighByte(&m_AF, result);

    (result == 0x00) ? SetFlag(ZeroFlag) : ClearFlag(ZeroFlag);
    SetFlag(SubtractFlag);
    ((A & 0x0F) < (n & 0x0F)) ? SetFlag(HalfCarryFlag) : ClearFlag(HalfCarryFlag);
    ((A & 0xFF) < (n & 0xFF)) ? SetFlag(CarryFlag) : ClearFlag(CarryFlag);

    return 8;
}

/*
    RETI - 0xD9

    Return and enable interrupts.

    16 Cycles

    Flags affected(znhc): ----
*/
ulong InstructionSet::RETI(const byte& opCode)
{
    m_IME = 0x01; // Restore interrupts
    m_PC = PopUShort(); // Return

    return 16;
}

/*
    SBC A, n - 0xDE

    The 8-bit value n, along with the carry flag, is subtracted from the contents
    of the Accumulator, and the result is stored in the accumulator.

    8 Cycles

    Flags affected(znhc): z1hc
*/
ulong InstructionSet::SBCAn(const byte& opCode)
{
    byte n = ReadBytePC();
    SBC(n);
    return 8;
}

/*
    LD (0xFF00 + n), A - 0xE0

    Loads the contents of the A register into 0xFF00 + n.

    12 Cycles

    Flags affected(znhc): ----
*/
ulong InstructionSet::LD_0xFF00n_A(const byte& opCode)
{
    byte n = ReadBytePC(); // Read n

    m_MMU->Write(0xFF00 + n, GetHighByte(m_AF)); // Load A into 0xFF00 + n

    return 12;
}

/*
    LD (0xFF00 + C), A - 0xE2

    Loads the contents of the A register into 0xFF00 + C.

    8 Cycles

    Flags affected(znhc): ----
*/
ulong InstructionSet::LD_0xFF00C_A(const byte& opCode)
{
    m_MMU->Write(0xFF00 + GetLowByte(m_BC), GetHighByte(m_AF)); // Load A into 0xFF00 + C

    return 8;
}

/*
    LD (nn), A - 0xEA

    The contents of the accumulator are loaded into the address specified by the
    operand nn.

    16 Cycles

    Flags affected(znhc): ----
*/
ulong InstructionSet::LD_nn_A(const byte& opCode)
{
    ushort nn = ReadUShortPC();

    m_MMU->Write(nn, GetHighByte(m_AF)); // Load A into (nn)

    return 16;
}

/*
    XOR n - 0xEE

    The logical exclusive-OR operation is performed between the 8-bit operand and
    the byte contained in the accumulator. The result is stored in the accumulator.

    8 Cycles

    Flags affected(znhc): z000
*/
ulong InstructionSet::XORn(const byte& opCode)
{
    byte n = ReadBytePC();
    SetHighByte(&m_AF, n ^ GetHighByte(m_AF));

    // Affects Z and clears NHC
    if (GetHighByte(m_AF) == 0x00)
    {
        SetFlag(ZeroFlag);
    }
    else
    {
        ClearFlag(ZeroFlag);
    }

    ClearFlag(SubtractFlag);
    ClearFlag(HalfCarryFlag);
    ClearFlag(CarryFlag);

    return 8;
}

/*
    LD A, (0xFF00 + n) - 0xF0

    Read from io-port n

    12 Cycles

    Flags affected(znhc): ----
*/
ulong InstructionSet::LDA_0xFF00n_(const byte& opCode)
{
    byte n = ReadBytePC(); // Read n
    SetHighByte(&m_AF, m_MMU->Read(0xFF00 + n));

    return 12;
}

/*
    LD A, (0xFF00 + C) - 0xF2

    Read from io-port C

    8 Cycles

    Flags affected(znhc): ----
*/
ulong InstructionSet::LDA_0xFF00C_(const byte& opCode)
{
    SetHighByte(&m_AF, m_MMU->Read(0xFF00 + GetLowByte(m_BC)));

    return 8;
}

/*
    DI - 0xF3

    Disable interrupts

    4 Cycles

    Flags affected(znhc): ----
*/
ulong InstructionSet::DI(const byte& opCode)
{
    m_IME = 0x00;

    return 4;
}

/*
    OR n - 0xF6

    The logical OR operation is performed between the byte in n and the byte contained in the
    accumulator. The result is stored in the accumulator.

    8 Cycles

    Flags affected(znhc): z000
*/
ulong InstructionSet::ORn(const byte& opCode)
{
    byte n = ReadBytePC();
    SetHighByte(&m_AF, n | GetHighByte(m_AF));

    // Affects Z and clears NHC
    if (GetHighByte(m_AF) == 0x00)
    {
        SetFlag(ZeroFlag);
    }
    else
    {
        ClearFlag(ZeroFlag);
    }

    ClearFlag(SubtractFlag);
    ClearFlag(HalfCarryFlag);
    ClearFlag(CarryFlag);

    return 8;
}

/*
    LD SP, HL - 0xF9

    Load HL into SP.

    8 Cycles

    Flags affected(znhc): ----
*/
ulong InstructionSet::LDSPHL(const byte& opCode)
{
    m_SP = m_HL;

    return 8;
}

/*
    LD HL, SP+e - 0xF8

    ld   HL,SP+dd  F8          12 00hc HL = SP +/- dd ;dd is 8bit signed number

    12 Cycles

    Flags affected(znhc): 00hc
*/
ulong InstructionSet::LDHLSPe(const byte& opCode)
{
    sbyte e = static_cast<sbyte>(ReadBytePC());

    ushort result = m_SP + e;

    ushort check = m_SP ^ e ^ ((m_SP + e) & 0xFFFF);

    ((check & 0x100) == 0x100) ? SetFlag(CarryFlag) : ClearFlag(CarryFlag);
    ((check & 0x10) == 0x10) ? SetFlag(HalfCarryFlag) : ClearFlag(HalfCarryFlag);

    ClearFlag(ZeroFlag);
    ClearFlag(SubtractFlag);

    m_HL = result;

    return 12;
}

/*
    LD A, (nn) - 0xFA

    The contents of the address specified by the operand nn are loaded into the accumulator.

    16 Cycles

    Flags affected(znhc): ----
*/
ulong InstructionSet::LDA_nn_(const byte& opCode)
{
    ushort nn = ReadUShortPC();
    SetHighByte(&m_AF, m_MMU->Read(nn));

    return 16;
}

/*
    EI - 0xFB

    Enable interrupts

    4 Cycles

    Flags affected(znhc): ----
*/
ulong InstructionSet::EI(const byte& opCode)
{
    m_IME = 0x01;

    return 4;
}

/*
    CP n - 0xFE

    The contents of 8-bit operand n are compared with the contents of the accumulator.
    If there is a true compare, the Z flag is set. The execution of this instruction
    does not affect the contents of the accumulator.

    8 Cycles

    Flags affected(znhc): z1hc
*/
ulong InstructionSet::CPn(const byte& opCode)
{
    byte n = ReadBytePC();
    byte A = GetHighByte(m_AF);
    byte result = A - n;

    (result == 0x00) ? SetFlag(ZeroFlag) : ClearFlag(ZeroFlag);
    ((A & 0xFF) < (n & 0xFF)) ? SetFlag(CarryFlag) : ClearFlag(CarryFlag);
    ((A & 0x0F) < (n & 0x0F)) ? SetFlag(HalfCarryFlag) : ClearFlag(HalfCarryFlag);
    SetFlag(SubtractFlag);

    return 8;
}

/*
    CPU 0xCB INSTRUCTION MAP
*/

/*
    RLC r
    11001011(CB) 00000rrr

    The contents of 8-bit register r are rotated left 1-bit position. The content of bit 7
    is copied to the carry flag and also to bit 0. Operand r identifies register
    B, C, D, E, H, L, or A.

    8 Cycles

    Flags affected(znhc): z00c
*/
ulong InstructionSet::RLCr(const byte& opCode)
{
    byte* r = GetByteRegister(opCode);

    // Grab bit 7 and store it in the carryflag
    ISBITSET(*r, 7) ? SetFlag(CarryFlag) : ClearFlag(CarryFlag);

    // Shift r left
    (*r) = *r << 1;

    // Set bit 0 of r to the old CarryFlag
    (*r) = IsFlagSet(CarryFlag) ? SETBIT((*r), 0) : CLEARBIT((*r), 0);

    // Affects Z, clears N, clears H, affects C
    (*r == 0x00) ? SetFlag(ZeroFlag) : ClearFlag(ZeroFlag);
    ClearFlag(SubtractFlag);
    ClearFlag(HalfCarryFlag);

    return 8;
}

/*
    RLC (HL)

    16 Cycles

    Flags affected(znhc): z00c
*/
ulong InstructionSet::RLC_HL_(const byte& opCode)
{
    byte r = m_MMU->Read(m_HL);

    // Grab bit 7 and store it in the carryflag
    ISBITSET(r, 7) ? SetFlag(CarryFlag) : ClearFlag(CarryFlag);

    // Shift r left
    r <<= 1;

    // Set bit 0 of r to the old CarryFlag
    r = IsFlagSet(CarryFlag) ? SETBIT((r), 0) : CLEARBIT((r), 0);

    m_MMU->Write(m_HL, r);

    // Affects Z, clears N, clears H, affects C
    (r == 0x00) ? SetFlag(ZeroFlag) : ClearFlag(ZeroFlag);
    ClearFlag(SubtractFlag);
    ClearFlag(HalfCarryFlag);

    return 16;
}

/*
    RRC r
    11001011(CB) 00001rrr

    The contents of 8-bit register r are rotated right 1-bit position. The content of bit 0
    is copied to the carry flag and also to bit 7. Operand r identifies register
    B, C, D, E, H, L, or A.

    8 Cycles

    Flags affected(znhc): z00c
*/
ulong InstructionSet::RRCr(const byte& opCode)
{
    byte* r = GetByteRegister(opCode);

    // Grab bit 0 and store it in the carryflag
    ISBITSET(*r, 0) ? SetFlag(CarryFlag) : ClearFlag(CarryFlag);

    // Shift r right
    (*r) = *r >> 1;

    // Set bit 0 of r to the old CarryFlag
    (*r) = IsFlagSet(CarryFlag) ? SETBIT((*r), 7) : CLEARBIT((*r), 7);

    // Affects Z, clears N, clears H, affects C
    (*r == 0x00) ? SetFlag(ZeroFlag) : ClearFlag(ZeroFlag);
    ClearFlag(SubtractFlag);
    ClearFlag(HalfCarryFlag);

    return 8;
}

/*
    RRC (HL)

    16 Cycles

    Flags affected(znhc): z00c
*/
ulong InstructionSet::RRC_HL_(const byte& opCode)
{
    byte r = m_MMU->Read(m_HL);

    // Grab bit 0 and store it in the carryflag
    ISBITSET(r, 0) ? SetFlag(CarryFlag) : ClearFlag(CarryFlag);

    // Shift r right
    r >>= 1;

    // Set bit 0 of r to the old CarryFlag
    r = IsFlagSet(CarryFlag) ? SETBIT((r), 7) : CLEARBIT((r), 7);

    m_MMU->Write(m_HL, r);

    // Affects Z, clears N, clears H, affects C
    (r == 0x00) ? SetFlag(ZeroFlag) : ClearFlag(ZeroFlag);
    ClearFlag(SubtractFlag);
    ClearFlag(HalfCarryFlag);

    return 16;
}

/*
    RL r
    11001011(CB) 00010rrr

    The contents of 8-bit register r are rotated left 1-bit position. The content of bit 7
    is copied to the carry flag and the previous content of the carry flag is copied to bit 0.
    Operand r identifies register: B, C, D, E, H, L, or A.

    8 Cycles

    Flags affected(znhc): z00c
*/
ulong InstructionSet::RLr(const byte& opCode)
{
    byte* r = GetByteRegister(opCode);

    // Grab the current CarryFlag val
    bool carry = IsFlagSet(CarryFlag);

    // Grab bit 7 and store it in the carryflag
    ISBITSET(*r, 7) ? SetFlag(CarryFlag) : ClearFlag(CarryFlag);

    // Shift r left
    (*r) = *r << 1;

    // Set bit 0 of r to the old CarryFlag
    (*r) = carry ? SETBIT((*r), 0) : CLEARBIT((*r), 0);

    // Affects Z, clears N, clears H, affects C
    (*r == 0x00) ? SetFlag(ZeroFlag) : ClearFlag(ZeroFlag);
    ClearFlag(SubtractFlag);
    ClearFlag(HalfCarryFlag);

    return 8;
}

/*
    RL (HL)

    16 Cycles

    Flags affected(znhc): z00c
*/
ulong InstructionSet::RL_HL_(const byte& opCode)
{
    byte r = m_MMU->Read(m_HL);

    // Grab the current CarryFlag val
    bool carry = IsFlagSet(CarryFlag);

    // Grab bit 7 and store it in the carryflag
    ISBITSET(r, 7) ? SetFlag(CarryFlag) : ClearFlag(CarryFlag);

    // Shift r left
    r <<= 1;

    // Set bit 0 of r to the old CarryFlag
    r = carry ? SETBIT((r), 0) : CLEARBIT((r), 0);

    m_MMU->Write(m_HL, r);

    // Affects Z, clears N, clears H, affects C
    (r == 0x00) ? SetFlag(ZeroFlag) : ClearFlag(ZeroFlag);
    ClearFlag(SubtractFlag);
    ClearFlag(HalfCarryFlag);

    return 16;
}

/*
    RR r
    11001011(CB) 00001rrr

    The contents of 8-bit register r are rotated right 1-bit position. The content of bit 0
    is copied to the carry flag and the previous content of the carry flag is copied to bit 7.
    Operand r identifies register: B, C, D, E, H, L, or A.

    8 Cycles

    Flags affected(znhc): z00c
*/
ulong InstructionSet::RRr(const byte& opCode)
{
    byte* r = GetByteRegister(opCode);

    // Grab the current CarryFlag val
    bool carry = IsFlagSet(CarryFlag);

    // Grab bit 0 and store it in the carryflag
    ISBITSET(*r, 0) ? SetFlag(CarryFlag) : ClearFlag(CarryFlag);

    // Shift r right
    (*r) = *r >> 1;

    // Set bit 7 of r to the old CarryFlag
    (*r) = carry ? SETBIT((*r), 7) : CLEARBIT((*r), 7);

    // Affects Z, clears N, clears H, affects C
    (*r == 0x00) ? SetFlag(ZeroFlag) : ClearFlag(ZeroFlag);
    ClearFlag(SubtractFlag);
    ClearFlag(HalfCarryFlag);

    return 8;
}

/*
    RR (HL)

    16 Cycles

    Flags affected(znhc): z00c
*/
ulong InstructionSet::RR_HL_(const byte& opCode)
{
    byte r = m_MMU->Read(m_HL);

    // Grab the current CarryFlag val
    bool carry = IsFlagSet(CarryFlag);

    // Grab bit 0 and store it in the carryflag
    ISBITSET(r, 0) ? SetFlag(CarryFlag) : ClearFlag(CarryFlag);

    // Shift r right
    r >>= 1;

    // Set bit 7 of r to the old CarryFlag
    r = carry ? SETBIT((r), 7) : CLEARBIT((r), 7);

    m_MMU->Write(m_HL, r);

    // Affects Z, clears N, clears H, affects C
    (r == 0x00) ? SetFlag(ZeroFlag) : ClearFlag(ZeroFlag);
    ClearFlag(SubtractFlag);
    ClearFlag(HalfCarryFlag);

    return 16;
}

/*
    SLA r
    11001011 00100rrrr

    An arithmetic shift left 1-bit position is performed on the contents of the operand r. The content
    of bit 7 is copied to the carry flag.

    8 Cycles

    Flags affected(znhc): z00c
*/
ulong InstructionSet::SLAr(const byte& opCode)
{
    byte* r = GetByteRegister(opCode);

    // Grab bit 7 and store it in the carryflag
    ISBITSET(*r, 7) ? SetFlag(CarryFlag) : ClearFlag(CarryFlag);

    // Shift r left
    (*r) = *r << 1;

    // Affects Z, clears N, clears H, affects C
    (*r == 0x00) ? SetFlag(ZeroFlag) : ClearFlag(ZeroFlag);
    ClearFlag(SubtractFlag);
    ClearFlag(HalfCarryFlag);

    return 8;
}

/*
    SLA (HL)

    16 Cycles

    Flags affected(znhc): z00c
*/
ulong InstructionSet::SLA_HL_(const byte& opCode)
{
    byte r = m_MMU->Read(m_HL);

    // Grab bit 7 and store it in the carryflag
    ISBITSET(r, 7) ? SetFlag(CarryFlag) : ClearFlag(CarryFlag);

    // Shift r left
    r = r << 1;
    m_MMU->Write(m_HL, r);

    // Affects Z, clears N, clears H, affects C
    (r == 0x00) ? SetFlag(ZeroFlag) : ClearFlag(ZeroFlag);
    ClearFlag(SubtractFlag);
    ClearFlag(HalfCarryFlag);

    return 16;
}

/*
    SRA r
    11001011 00101rrr

    An arithmetic shift right 1-bit position is performed on the contents of the operand r. The content
    of bit 0 is copied to the carry flag.

    8 Cycles

    Flags affected(znhc): z00c
*/
ulong InstructionSet::SRAr(const byte& opCode)
{
    byte* r = GetByteRegister(opCode);

    // Grab bit 0 and store it in the carryflag
    ISBITSET(*r, 0) ? SetFlag(CarryFlag) : ClearFlag(CarryFlag);

    // Shift r right
    (*r) = (*r >> 1) | (*r & 0x80);

    // Affects Z, clears N, clears H, affects C
    (*r == 0x00) ? SetFlag(ZeroFlag) : ClearFlag(ZeroFlag);
    ClearFlag(SubtractFlag);
    ClearFlag(HalfCarryFlag);

    return 8;
}

/*
    SRA (HL)

    16 Cycles

    Flags affected(znhc): z00c
*/
ulong InstructionSet::SRA_HL_(const byte& opCode)
{
    byte r = m_MMU->Read(m_HL);

    // Grab bit 0 and store it in the carryflag
    ISBITSET(r, 0) ? SetFlag(CarryFlag) : ClearFlag(CarryFlag);

    // Shift r right
    r = (r >> 1) | (r & 0x80);
    m_MMU->Write(m_HL, r);

    // Affects Z, clears N, clears H, affects C
    (r == 0x00) ? SetFlag(ZeroFlag) : ClearFlag(ZeroFlag);
    ClearFlag(SubtractFlag);
    ClearFlag(HalfCarryFlag);

    return 16;
}

/*
    SRL r
    11001011 00011rrr

    The contents of the operand "r" are shifted right 1 -bit.  The content of
    bit 0 is copied to the carry flag and bit 7 is reset.

    8 Cycles

    Flags affected(znhc): z00c
*/
ulong InstructionSet::SRLr(const byte& opCode)
{
    byte* r = GetByteRegister(opCode);

    // Grab bit 0 and store it in the carryflag
    ISBITSET(*r, 0) ? SetFlag(CarryFlag) : ClearFlag(CarryFlag);

    // Shift r right
    (*r) = *r >> 1;
    (*r) = CLEARBIT(*r, 7);

    // Affects Z, clears N, clears H, affects C
    (*r == 0x00) ? SetFlag(ZeroFlag) : ClearFlag(ZeroFlag);
    ClearFlag(SubtractFlag);
    ClearFlag(HalfCarryFlag);

    return 8;
}

/*
    SRL (HL)

    16 Cycles

    Flags affected(znhc): z00c
*/
ulong InstructionSet::SRL_HL_(const byte& opCode)
{
    byte r = m_MMU->Read(m_HL);

    // Grab bit 0 and store it in the carryflag
    ISBITSET(r, 0) ? SetFlag(CarryFlag) : ClearFlag(CarryFlag);

    // Shift r right
    r = r >> 1;
    r = CLEARBIT(r, 7);
    m_MMU->Write(m_HL, r);

    // Affects Z, clears N, clears H, affects C
    (r == 0x00) ? SetFlag(ZeroFlag) : ClearFlag(ZeroFlag);
    ClearFlag(SubtractFlag);
    ClearFlag(HalfCarryFlag);

    return 16;
}

/*
    BIT b, r
    11001011 01bbbrrr

    This instruction tests bit b in register r and sets the Z flag accordingly.

    8 Cycles

    Flags affected(znhc): z01-
*/
ulong InstructionSet::BITbr(const byte& opCode)
{
    byte bit = (opCode >> 3) & 0x07;
    byte* r = GetByteRegister(opCode);

    // Test bit b in r
    (!ISBITSET(*r, bit)) ? SetFlag(ZeroFlag) : ClearFlag(ZeroFlag);

    SetFlag(HalfCarryFlag); // H is set
    ClearFlag(SubtractFlag); // N is reset

    return 8;
}

/*
    BIT b, (HL)

    12 Cycles

    Flags affected(znhc): z01-
*/
ulong InstructionSet::BITb_HL_(const byte& opCode)
{
    byte bit = (opCode >> 3) & 0x07;
    byte r = m_MMU->Read(m_HL);

    // Test bit b in r
    (!ISBITSET(r, bit)) ? SetFlag(ZeroFlag) : ClearFlag(ZeroFlag);

    SetFlag(HalfCarryFlag); // H is set
    ClearFlag(SubtractFlag); // N is reset

    return 12;
}

/*
    RES b, r
    11001011 10bbbrrr

    Bit b in operand r is reset.

    8 Cycles

    Flags affected(znhc): ----
*/
ulong InstructionSet::RESbr(const byte& opCode)
{
    byte bit = (opCode >> 3) & 0x07;
    byte* r = GetByteRegister(opCode);
    *r = CLEARBIT(*r, bit);

    return 8;
}

/*
    RES b, (HL)

    16 Cycles

    Flags affected(znhc): ----
*/
ulong InstructionSet::RESb_HL_(const byte& opCode)
{
    byte bit = (opCode >> 3) & 0x07;
    byte r = m_MMU->Read(m_HL);
    m_MMU->Write(m_HL, CLEARBIT(r, bit));

    return 16;
}

/*
    SET b, r
    11001011 11bbbrrr

    Bit b in operand r is set.

    8 Cycles

    No flags affected.
*/
ulong InstructionSet::SETbr(const byte& opCode)
{
    byte bit = (opCode >> 3) & 0x07;
    byte* r = GetByteRegister(opCode);
    *r = SETBIT(*r, bit);

    return 8;
}

/*
    SET b, (HL)

    16 Cycles

    Flags affected(znhc): ----
*/
ulong InstructionSet::SETb_HL_(const byte& opCode)
{
    byte bit = (opCode >> 3) & 0x07;
    byte r = m_MMU->Read(m_HL);
    m_MMU->Write(m_HL, SETBIT(r, bit));

    return 16;
}

/*
    SWAP r
    11001011 00111rrr
    swap r         CB 3x        8 z000 exchange low/hi-nibble
    swap (HL)      CB 36       16 z000 exchange low/hi-nibble

    Exchange the low and hi nibble (a nibble is 4 bits)

    8 Cycles

    Flags affected(znhc): z000
*/
ulong InstructionSet::SWAPr(const byte& opCode)
{
    byte* r = GetByteRegister(opCode);
    byte lowNibble = (*r & 0x0F);
    byte highNibble = (*r & 0xF0);

    *r = (lowNibble << 4) | (highNibble >> 4);

    ((*r) == 0x00) ? SetFlag(ZeroFlag) : ClearFlag(ZeroFlag);
    ClearFlag(SubtractFlag);
    ClearFlag(HalfCarryFlag);
    ClearFlag(CarryFlag);

    return 8;
}

/*
    SWAP (HL)

    16 Cycles

    Flags affected(znhc) : z000
*/
ulong InstructionSet::SWAP_HL_(const byte& opCode)
{
    byte r = m_MMU->Read(m_HL);
    byte lowNibble = (r & 0x0F);
    byte highNibble = (r & 0xF0);

    m_MMU->Write(m_HL, (lowNibble << 4) | (highNibble >> 4));

    (r == 0x00) ? SetFlag(ZeroFlag) : ClearFlag(ZeroFlag);
    ClearFlag(SubtractFlag);
    ClearFlag(HalfCarryFlag);
    ClearFlag(CarryFlag);

    return 16;
}