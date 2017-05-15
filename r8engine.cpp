#include "r8engine.h"

R8Engine::R8Engine() {
    Reset();
}

void R8Engine::Reset() {
    mIP = 0;
    mExecutionTime = 0;
    for (unsigned int i=0; i<REGISTERS_COUNT; ++i)
        mRegisters[i] = 0;
    for (unsigned int i=0; i<MEMORY_SIZE; ++i)
        mMemoryCells[i] = 0;

    emit SignalReset();
}

void R8Engine::Step() {
    int ip = mIP;

    R8Instruction Instr = mProgram.Instruction(ip);
    switch (Instr.Opcode()) {
    case R8Instruction::HALT_OPCODE: Halt();      break;
    case R8Instruction::IN_OPCODE:   In(Instr);   break;
    case R8Instruction::OUT_OPCODE:  Out(Instr);  break;
    case R8Instruction::ROR_OPCODE:  Ror(Instr);  break;
    case R8Instruction::ROL_OPCODE:  Rol(Instr);  break;
    case R8Instruction::NOT_OPCODE:  Not(Instr);  break;
    case R8Instruction::OR_OPCODE:   Or(Instr);   break;
    case R8Instruction::AND_OPCODE:  And(Instr);  break;
    case R8Instruction::NOR_OPCODE:  Nor(Instr);  break;
    case R8Instruction::NAND_OPCODE: Nand(Instr); break;
    case R8Instruction::XOR_OPCODE:  Xor(Instr);  break;
    case R8Instruction::ADD_OPCODE:  Add(Instr);  break;
    case R8Instruction::SUB_OPCODE:  Sub(Instr);  break;
    case R8Instruction::JZ_OPCODE:   Jz(Instr);   break;
    case R8Instruction::JO_OPCODE:   Jo(Instr);   break;
    default:
        throw R8Exception(tr("Uncnown opcode"));
    }
}

unsigned char R8Engine::Register(unsigned int index) {
    if (index < REGISTERS_COUNT)
        return mRegisters[index];

    throw R8Exception(tr("Incorrect register index"));
}

void R8Engine::SetRegister(unsigned int index, unsigned char value) {
    if (index < REGISTERS_COUNT) {
        mRegisters[index] = value;
        emit SignalWriteRegister(index);
    } else
        throw R8Exception(tr("Incorrect register index"));
}

unsigned char R8Engine::MemoryCell(unsigned int index) {
    if (index < MEMORY_SIZE)
        return mMemoryCells[index];

    throw R8Exception(tr("Incorrect memory index"));
}

void R8Engine::SetMemoryCell(unsigned int index, unsigned char value) {
    if (index < MEMORY_SIZE) {
        mMemoryCells[index] = value;
        emit SignalWriteMemory(index);
    } else
        throw R8Exception(tr("Incorrect memory index"));
}

unsigned char R8Engine::GetOperand(const R8Reference &ref) {
    switch (ref.AccessType()) {
    case R8Reference::CONSTANT:
        UpdateExecutionTime(CONSTANT_ACCESS_TIME);
        return (unsigned char)ref.Value();
    case R8Reference::REGISTER:
        UpdateExecutionTime(REGISTER_ACCESS_TIME);
        return Register((unsigned char)ref.Value());
    case R8Reference::MEMORY_BY_CONSTANT:
        UpdateExecutionTime(MEMORY_ACCESS_TIME);
        return MemoryCell((unsigned char)ref.Value());
    case R8Reference::MEMORY_BY_REGISTER:
        UpdateExecutionTime(REGISTER_ACCESS_TIME);
        UpdateExecutionTime(MEMORY_ACCESS_TIME);
        return MemoryCell((unsigned int)Register((unsigned char)ref.Value()));
    default:
        throw R8Exception(tr("Bad reference for operand"));
    }
}

void R8Engine::SetResult(const R8Reference &ref, unsigned char result) {
    switch (ref.AccessType()) {
    case R8Reference::REGISTER:
        UpdateExecutionTime(REGISTER_ACCESS_TIME);
        SetRegister((unsigned char)ref.Value(), result);
        break;
    case R8Reference::MEMORY_BY_CONSTANT:
        UpdateExecutionTime(MEMORY_ACCESS_TIME);
        SetMemoryCell((unsigned char)ref.Value(), result);
        break;
    case R8Reference::MEMORY_BY_REGISTER:
        UpdateExecutionTime(REGISTER_ACCESS_TIME);
        UpdateExecutionTime(MEMORY_ACCESS_TIME);
        SetMemoryCell((unsigned int)Register((unsigned char)ref.Value()), result);
        break;
    default:
        throw R8Exception(tr("Bad reference for result"));
    }
}

unsigned int R8Engine::GetIP(const R8Reference &ref) {
    if (ref.AccessType() == R8Reference::INSTRUCTION_INDEX)
        return ref.Value();
    throw R8Exception(tr("It is not an LABEL reference"));
}

void R8Engine::GoToNextInstruction() {
    mIP++;
}

void R8Engine::GoToInstruction(unsigned int index) {
    mIP = index;
}

void R8Engine::GotoInHaltState() {mIP = mProgram.Length();}

void R8Engine::Halt() {
    GotoInHaltState();
    emit SignalHalt();
}

void R8Engine::In(const R8Instruction &I) {
    Q_ASSERT(mInputPort != 0);

    unsigned char x = mInputPort->Input();
    if (!mInputPort->IsFailure()) {
        SetResult(I.Result(), x);
        GoToNextInstruction();

        UpdateExecutionTime(OPERATION_TIME);
    } else {
        Halt();
    }
}

void R8Engine::Out(const R8Instruction &I) {
    //todo: вывод (через метод контроллеров?)
    unsigned char o = GetOperand(I.Operand1());
    emit SignalOutput(o);
    GoToNextInstruction();

    UpdateExecutionTime(OPERATION_TIME);
}

void R8Engine::Ror(const R8Instruction &I) {
    unsigned char x = GetOperand(I.Operand1());
    unsigned char n = GetOperand(I.Operand2());
    n = (n % 8);
    unsigned char r = (x >> n) | (x << (8-n));
    SetResult(I.Result(), r);
    GoToNextInstruction();

    UpdateExecutionTime(OPERATION_TIME);
}

void R8Engine::Rol(const R8Instruction &I) {
    unsigned char x = GetOperand(I.Operand1());
    unsigned char n = GetOperand(I.Operand2());
    n = (n % 8);
    unsigned char r = (x << n) | (x >> (8-n));
    SetResult(I.Result(), r);
    GoToNextInstruction();

    UpdateExecutionTime(OPERATION_TIME);
}

void R8Engine::Not(const R8Instruction &I) {
    unsigned char x = GetOperand(I.Operand1());
    SetResult(I.Result(), ~x);
    GoToNextInstruction();

    UpdateExecutionTime(OPERATION_TIME);
}

void R8Engine::Or(const R8Instruction &I) {
    unsigned char x = GetOperand(I.Operand1());
    unsigned char y = GetOperand(I.Operand2());
    unsigned char r = (x | y);
    SetResult(I.Result(), r);
    GoToNextInstruction();

    UpdateExecutionTime(OPERATION_TIME);
}

void R8Engine::And(const R8Instruction &I) {
    unsigned char x = GetOperand(I.Operand1());
    unsigned char y = GetOperand(I.Operand2());
    unsigned char r = (x & y);
    SetResult(I.Result(), r);
    GoToNextInstruction();

    UpdateExecutionTime(OPERATION_TIME);
}

void R8Engine::Nor(const R8Instruction &I) {
    unsigned char x = GetOperand(I.Operand1());
    unsigned char y = GetOperand(I.Operand2());
    unsigned char r = ~(x | y);
    SetResult(I.Result(), r);
    GoToNextInstruction();

    UpdateExecutionTime(OPERATION_TIME);
}

void R8Engine::Nand(const R8Instruction &I) {
    unsigned char x = GetOperand(I.Operand1());
    unsigned char y = GetOperand(I.Operand2());
    unsigned char r = ~(x & y);
    SetResult(I.Result(), r);
    GoToNextInstruction();

    UpdateExecutionTime(OPERATION_TIME);
}

void R8Engine::Xor(const R8Instruction &I) {
    unsigned char x = GetOperand(I.Operand1());
    unsigned char y = GetOperand(I.Operand2());
    unsigned char r = (x ^ y);
    SetResult(I.Result(), r);
    GoToNextInstruction();

    UpdateExecutionTime(OPERATION_TIME);
}

void R8Engine::Add(const R8Instruction &I) {
    unsigned char x = GetOperand(I.Operand1());
    unsigned char y = GetOperand(I.Operand2());
    unsigned char r = x + y;
    SetResult(I.Result(), r);
    GoToNextInstruction();

    UpdateExecutionTime(OPERATION_TIME);
}

void R8Engine::Sub(const R8Instruction &I) {
    unsigned char x = GetOperand(I.Operand1());
    unsigned char y = GetOperand(I.Operand2());
    unsigned char r = x + (~y) + 1;
    SetResult(I.Result(), r);
    GoToNextInstruction();

    UpdateExecutionTime(OPERATION_TIME);
}

void R8Engine::Jz(const R8Instruction &I) {
    unsigned char x = GetOperand(I.Operand1());
    if (x==0) {
        GoToInstruction( GetIP(I.Result()) );

        UpdateExecutionTime(JUMP_TIME);
    } else
        GoToNextInstruction();

    UpdateExecutionTime(OPERATION_TIME);
}

void R8Engine::Jo(const R8Instruction &I) {
    unsigned char x = GetOperand(I.Operand1());
    if (x==0xFF) {
        GoToInstruction( GetIP(I.Result()) );

        UpdateExecutionTime(JUMP_TIME);
    } else
        GoToNextInstruction();

    UpdateExecutionTime(OPERATION_TIME);
}

void R8Program::Clear() {
    mInstructions.clear();
}

R8Instruction R8Program::Instruction(unsigned int Index) const {
    if (Index < (unsigned int)mInstructions.size())
        return mInstructions[Index];
    return R8Instruction(R8Instruction::HALT_OPCODE); //останов!
}

void R8Program::UpdateInstruction(unsigned int Index, const R8Instruction &Instruction) {
    if (Index < (unsigned int)mInstructions.size()) {
        mInstructions[Index] = Instruction;
    } else
        throw R8Exception("Instruction index outside of program!");
}

unsigned int R8Program::AddInstruction(const R8Instruction &instruction) {
    mInstructions.append(instruction);
    return (mInstructions.size() - 1);
}
