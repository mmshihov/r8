#ifndef R8ENGINE_H
#define R8ENGINE_H

#include <QObject>
#include <QString>
#include <QVector>

class R8Exception {
public:
    R8Exception(QString message) : mMessage(message) {}
    const QString& Message() const {return mMessage;}
private:
    QString mMessage;
};

class R8Reference {
public:
    enum EAccessType {
        CONSTANT,
        REGISTER,
        MEMORY_BY_CONSTANT,
        MEMORY_BY_REGISTER,
        INSTRUCTION_INDEX
    };

    R8Reference() : mAccessType(CONSTANT), mValue(0) {}
    R8Reference(const R8Reference& Copy) : mAccessType(Copy.mAccessType), mValue(Copy.mValue) {}
    R8Reference(EAccessType  AccessType, unsigned int Value) : mAccessType(AccessType),mValue(Value) {}

    EAccessType  AccessType() const {return mAccessType;}
    unsigned int Value() const {return mValue;}

private:
    EAccessType  mAccessType;
    unsigned int mValue; //it's derived from mAccessType field how to interpret
};


class R8Instruction {
public:
    enum EOpcode {
        HALT_OPCODE,//end of program... not available for user...
        IN_OPCODE,  //in dst
        OUT_OPCODE, //out src
        ROR_OPCODE, //ror src, n, dst
        ROL_OPCODE, //rol src, n, dst
        NOT_OPCODE, //not stc, dst
        OR_OPCODE,  //or src1, src2, dst
        AND_OPCODE, //and src1, src2, dst
        NOR_OPCODE, //nor src1, src2, dst
        NAND_OPCODE,//nand src1, src2, dst
        XOR_OPCODE, //xor src1, src2, dst
        ADD_OPCODE, //add src1, src2, dst
        SUB_OPCODE, //sub src1, src2, dst
        JZ_OPCODE,  //jz src, dst(label)
        JO_OPCODE   //jo src, dst(label)
    };

    R8Instruction():mOpcode(HALT_OPCODE) {}
    R8Instruction(EOpcode opcode):mOpcode(opcode) {}
    R8Instruction(EOpcode opcode, const R8Reference& o1, const R8Reference& o2, const R8Reference& r):
        mOpcode(opcode),mOperand1(o1),mOperand2(o2),mResult(r) {}

    EOpcode Opcode() {return mOpcode;}
    void SetOpcode(EOpcode Opcode) {mOpcode = Opcode;}

    R8Reference Operand1() const {return mOperand1;}
    R8Reference Operand2() const {return mOperand2;}
    R8Reference Result()   const {return mResult;}

    void SetOperand1(const R8Reference& Value) {mOperand1 = Value;}
    void SetOperand2(const R8Reference& Value) {mOperand2 = Value;}
    void SetResult(const R8Reference& Value)   {mResult = Value;}

private:
    EOpcode      mOpcode;    //most common form of command: "cmd op1, op2, r"
    R8Reference  mOperand1;
    R8Reference  mOperand2;
    R8Reference  mResult;
};


class R8Program {
public:
    void Clear();
    R8Instruction Instruction(unsigned int Index) const;
    int Length() const {return mInstructions.size();}
    void UpdateInstruction(unsigned int Index, const R8Instruction& Instruction); //compiler need it...
    unsigned int AddInstruction(const R8Instruction& instruction);
private:
    QVector<R8Instruction> mInstructions;
};


class R8InputPort {
public:
    unsigned char Input() {return DoInput();}
    virtual ~R8InputPort() {}

    bool IsFailure() const {return mIsFailure;}
    void SetFailure(bool value) {mIsFailure = value;}
protected:
    virtual unsigned char DoInput() {return 0;}
private:
    bool mIsFailure;
};


class R8Engine : public QObject {
    Q_OBJECT

public:
    R8Engine();

    static const unsigned int REGISTERS_COUNT = 8;
    static const unsigned int MEMORY_SIZE = (1 << REGISTERS_COUNT);

    void Reset();
    void SetProgram(const R8Program& program) {mProgram = program; Reset();}
    void SetInputPort(R8InputPort *port) {mInputPort = port;}
    void Step();

    unsigned int IP() const {return mIP;}
    unsigned int ExecutionTime() const {return mExecutionTime;}

    unsigned char Register(unsigned int index);
    void SetRegister(unsigned int index, unsigned char value);

    unsigned char MemoryCell(unsigned int index);
    void SetMemoryCell(unsigned int index, unsigned char value);

    void Halt();

private:
    static const unsigned int REGISTER_ACCESS_TIME  = 1;
    static const unsigned int MEMORY_ACCESS_TIME    = 8;
    static const unsigned int CONSTANT_ACCESS_TIME  = 0;
    static const unsigned int OPERATION_TIME        = 1;
    static const unsigned int JUMP_TIME             = 8;

    unsigned char mRegisters[REGISTERS_COUNT];
    unsigned char mMemoryCells[MEMORY_SIZE];
    R8Program     mProgram;
    R8InputPort  *mInputPort;

    unsigned int  mIP;

    unsigned int  mExecutionTime;

    unsigned char GetOperand(const R8Reference& ref);
    void SetResult(const R8Reference& ref, unsigned char result);
    unsigned int GetIP(const R8Reference& ref);
    void GoToNextInstruction();
    void GoToInstruction(unsigned int index);
    void GotoInHaltState();

    void In(const R8Instruction& I);
    void Out(const R8Instruction& I);
    void Ror(const R8Instruction& I);
    void Rol(const R8Instruction& I);
    void Not(const R8Instruction& I);
    void Or(const R8Instruction& I);
    void And(const R8Instruction& I);
    void Nor(const R8Instruction& I);
    void Nand(const R8Instruction& I);
    void Xor(const R8Instruction& I);
    void Add(const R8Instruction& I);
    void Sub(const R8Instruction& I);
    void Jz(const R8Instruction& I);
    void Jo(const R8Instruction& I);

    void UpdateExecutionTime(unsigned int time) {mExecutionTime += time;}
signals:
    void SignalReset();
    void SignalHalt();
    void SignalOutput(unsigned char value);
    void SignalWriteRegister(unsigned int index);
    void SignalWriteMemory(unsigned int index);
};


#endif // R8ENGINE_H
