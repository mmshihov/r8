#ifndef R8COMPILER_H
#define R8COMPILER_H

#include <QString>
#include <QMap>
#include <QMultiMap>

#include "r8engine.h"
#include "r8lexer.h"

class R8CompilerException {
public:
    enum EType {
        BAD_EXPRESSION,
        UNRESOLVED_LABEL,
        COMMA_EXPECTED,
        LABEL_REDEFINITION,
        UNDEFINED_COMMAND,
        BAD_REFERENCE,
        REFERENCE_EXPECTED,
        RBRACE_EXPECTED,
        LABEL_EXPECTED,
        REGISTER_EXPECTED
    };

    R8CompilerException(EType type, unsigned int lineNumber, const QString& info = QString()) :
        mType(type),mLineNumber(lineNumber),mInfo(info) {}

    EType Type() const {return mType;}
    unsigned int LineNumber() const {return mLineNumber;}
    QString Info() const {return mInfo;}
private:
    EType mType;
    unsigned int mLineNumber;
    QString      mInfo;
};

class R8CommandDescriptor {
public:
    enum EType {            // команда определяет правила разбора её аргументов
        ARGS_NO,            // hlt
        ARGS_SRC,           // out r1
        ARGS_DST,           // int r2
        ARGS_SRC_DST,       // not r1,r2
        ARGS_SRC_SRC_DST,   // add r1,r2,r3
        ARGS_SRC_LABEL      // jz  r1,label_for
    };

    R8CommandDescriptor():mType(ARGS_NO),mOpcode(R8Instruction::HALT_OPCODE) {}
    R8CommandDescriptor(EType type, R8Instruction::EOpcode opcode):mType(type),mOpcode(opcode) {}

    EType                   Type()   const {return mType;}
    R8Instruction::EOpcode  Opcode() const {return mOpcode;}

private:
    EType                   mType;
    R8Instruction::EOpcode  mOpcode;
};

class R8Compiler {
public:
    R8Compiler();
    void SetSource(R8CharStream *charStream) {mLexer.SetSource(charStream);}
    void Compile();
    void ClearAvailableCommands();
    void SetAvailableCommand(const QString& name, const R8CommandDescriptor& descriptor);
    int  SourceLineForIp(int ip) const;
    const R8Program& CompiledCode() const {return mProgram;}

private:
    typedef QMap<QString, unsigned int> TLabelsMapng;
    typedef QMultiMap<QString, unsigned int> TGoToMapping;
    typedef QMap<QString, R8CommandDescriptor>  TCommandNameMapping;
    typedef QMap<int, int>  TIpMapping;

    R8Program     mProgram;
    R8Lexer       mLexer;

    TLabelsMapng         mLabels;   // f: label_name -> command_index_label_points_to
    TGoToMapping         mGoTos;    // f: label_name -> { goto_command_index }
    TCommandNameMapping  mCommands; // f: command_name -> command_descriptor_for_compile
    TIpMapping           mIps;      // f: opcode_index -> source_line

    void ResolveReferences();

    int CurrentLine() {return mLexer.CurrentLine();}
    const R8Token& CurrentToken() {return mLexer.CurrentToken();}
    int CompiledInstructionIndex() const {return mProgram.Length();}

    void NextToken() {mLexer.NextToken();}
    void SkipCommaToken();

    void InitCompile();
    void CompileLabel(const R8Token& token);
    void CompileCommand(const R8Token& token);

    void CompileArgsNoCommand(R8Instruction::EOpcode opcode);
    void CompileArgsSrcCommand(R8Instruction::EOpcode opcode);
    void CompileArgsDstCommand(R8Instruction::EOpcode opcode);
    void CompileArgsSrcDstCommand(R8Instruction::EOpcode opcode);
    void CompileArgsSrcSrcDstCommand(R8Instruction::EOpcode opcode);
    void CompileArgsSrcLabelCommand(R8Instruction::EOpcode opcode);

    R8Reference CompileSrcReference();
    R8Reference MemoryByRegisterReferenceBy(QString str);
    R8Reference RegisterReferenceBy(QString str);
    unsigned char RegisterIndexFrom(QString str);
    R8Reference CompileDstReference();
    R8Reference CompileLabelReference();
};

#endif // R8COMPILER_H
