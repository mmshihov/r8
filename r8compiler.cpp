#include "r8compiler.h"

R8Compiler::R8Compiler() {
}

void R8Compiler::Compile() {
    InitCompile();

    NextToken();
    while (1) {
        if (CurrentToken().Type() == R8Token::END_OF_SOURCE) {
            break;
        } else if (CurrentToken().Type() == R8Token::IDENTIFIER) { //command or label
            int commandStartLine = CurrentLine();
            R8Token idToken = CurrentToken();
            NextToken();
            if (CurrentToken().Type() == R8Token::COLON) {
                CompileLabel(idToken);
            } else {
                mIps[CompiledInstructionIndex()] = commandStartLine;
                CompileCommand(idToken);
            }
        } else
            throw R8CompilerException(R8CompilerException::BAD_EXPRESSION, CurrentLine(), CurrentToken().TokenString()); //
    }

    ResolveReferences();
}

void R8Compiler::ClearAvailableCommands() { mCommands.clear(); }

void R8Compiler::SetAvailableCommand(const QString &name, const R8CommandDescriptor& descriptor) {
    mCommands[name] = descriptor;
}

int R8Compiler::SourceLineForIp(int ip) const {
    if (mIps.contains(ip))
        return mIps[ip];
    return -1;
}

void R8Compiler::ResolveReferences() {
    QList<QString> labelNames = mGoTos.keys();
    for (QList<QString>::iterator it = labelNames.begin(); it != labelNames.end(); ++it) {
        if (!mLabels.contains(*it)) {
            QList<unsigned int> ips = mGoTos.values(*it);

            unsigned int ip = 0;
            QListIterator<unsigned int> ipsIt(ips);
            while (ipsIt.hasNext()) {
                unsigned int currIp = ipsIt.next();
                ip = (ip < currIp) ? ip : currIp;
            }

            int line = SourceLineForIp(ip);
            if (line < 0)
                line = 0;

            throw R8CompilerException(R8CompilerException::UNRESOLVED_LABEL, line, *it);
        }

        unsigned int labelIndex = mLabels[*it];
        QList<unsigned int> gotoIndexes = mGoTos.values(*it);
        for (QList<unsigned int>::iterator gotoIt = gotoIndexes.begin(); gotoIt != gotoIndexes.end(); ++gotoIt) {
            unsigned int gotoIndex = *gotoIt;
            R8Instruction gotoInstruction = mProgram.Instruction(gotoIndex);
            R8Reference   labelReference(R8Reference::INSTRUCTION_INDEX, labelIndex);
            gotoInstruction.SetResult(labelReference);
            mProgram.UpdateInstruction(gotoIndex, gotoInstruction);
        }
    }
}

void R8Compiler::SkipCommaToken() {
    if (CurrentToken().Type() == R8Token::COMMA) {
        NextToken();
    } else
        throw R8CompilerException(R8CompilerException::COMMA_EXPECTED, CurrentLine());
}

void R8Compiler::InitCompile() {
    mProgram.Clear();
    mLabels.clear();
    mGoTos.clear();
    mIps.clear();
}

void R8Compiler::CompileLabel(const R8Token &token) {
    if (mLabels.contains(token.TokenString().toUpper()))
        throw R8CompilerException(R8CompilerException::LABEL_REDEFINITION, CurrentLine(), token.TokenString());

    mLabels[token.TokenString().toUpper()] = CompiledInstructionIndex(); //point next command
    NextToken();
}

void R8Compiler::CompileCommand(const R8Token &opcodeToken) {
    if (!mCommands.contains(opcodeToken.TokenString().toUpper()))
        throw R8CompilerException(R8CompilerException::UNDEFINED_COMMAND, CurrentLine(), opcodeToken.TokenString());

    R8CommandDescriptor descriptor = mCommands[opcodeToken.TokenString().toUpper()];
    switch (descriptor.Type()) {
    case R8CommandDescriptor::ARGS_NO:          CompileArgsNoCommand(descriptor.Opcode());         break;
    case R8CommandDescriptor::ARGS_SRC:         CompileArgsSrcCommand(descriptor.Opcode());        break;
    case R8CommandDescriptor::ARGS_DST:         CompileArgsDstCommand(descriptor.Opcode());        break;
    case R8CommandDescriptor::ARGS_SRC_DST:     CompileArgsSrcDstCommand(descriptor.Opcode());     break;
    case R8CommandDescriptor::ARGS_SRC_SRC_DST: CompileArgsSrcSrcDstCommand(descriptor.Opcode());  break;
    case R8CommandDescriptor::ARGS_SRC_LABEL:   CompileArgsSrcLabelCommand(descriptor.Opcode());   break;
    default:
        Q_ASSERT(false);
    }
}

void R8Compiler::CompileArgsNoCommand(R8Instruction::EOpcode opcode) {
    R8Instruction instr(opcode);
    mProgram.AddInstruction(instr);
}

void R8Compiler::CompileArgsSrcCommand(R8Instruction::EOpcode opcode) {
    R8Instruction instr(opcode);
    R8Reference ref = CompileSrcReference();
    instr.SetOperand1(ref);
    instr.SetOperand2(ref);
    mProgram.AddInstruction(instr);
}

void R8Compiler::CompileArgsDstCommand(R8Instruction::EOpcode opcode) {
    R8Instruction instr(opcode);
    R8Reference ref = CompileDstReference();
    instr.SetResult(ref);
    mProgram.AddInstruction(instr);
}

void R8Compiler::CompileArgsSrcDstCommand(R8Instruction::EOpcode opcode) {
    R8Instruction instr(opcode);
    R8Reference srcRef = CompileSrcReference();
    instr.SetOperand1(srcRef);
    instr.SetOperand2(srcRef);
    SkipCommaToken();
    R8Reference resultRef = CompileDstReference();
    instr.SetResult(resultRef);
    mProgram.AddInstruction(instr);
}

void R8Compiler::CompileArgsSrcSrcDstCommand(R8Instruction::EOpcode opcode) {
    R8Instruction instr(opcode);
    R8Reference src1Ref = CompileSrcReference();
    instr.SetOperand1(src1Ref);
    SkipCommaToken();
    R8Reference src2Ref = CompileSrcReference();
    instr.SetOperand2(src2Ref);
    SkipCommaToken();
    R8Reference resultRef = CompileDstReference();
    instr.SetResult(resultRef);
    mProgram.AddInstruction(instr);
}

void R8Compiler::CompileArgsSrcLabelCommand(R8Instruction::EOpcode opcode) {
    R8Instruction instr(opcode);
    R8Reference srcRef = CompileSrcReference();
    instr.SetOperand1(srcRef);
    instr.SetOperand2(srcRef);
    SkipCommaToken();
    R8Reference labelRef = CompileLabelReference();
    instr.SetResult(labelRef);
    mProgram.AddInstruction(instr);
}

R8Reference R8Compiler::CompileSrcReference(){
    R8Reference ref;
    if (CurrentToken().Type() == R8Token::IDENTIFIER) { //must be a register r0,..,r7
        ref = R8Reference(RegisterReferenceBy(CurrentToken().TokenString()));
        NextToken();
    } else if (CurrentToken().Type() == R8Token::NUMBER) { //constant
        ref = R8Reference(R8Reference::CONSTANT, CurrentToken().Value());
        NextToken();
    } else if (CurrentToken().Type() == R8Token::LEFT_SBRACE) { //start of [12] or [r1]
        NextToken();
        if (CurrentToken().Type() == R8Token::IDENTIFIER) {
            ref = MemoryByRegisterReferenceBy(CurrentToken().TokenString());
        } else if (CurrentToken().Type() == R8Token::NUMBER) {
            ref = R8Reference(R8Reference::MEMORY_BY_CONSTANT, CurrentToken().Value());
        } else
            throw R8CompilerException(R8CompilerException::BAD_REFERENCE, CurrentLine(), CurrentToken().TokenString());

        NextToken();
        if (CurrentToken().Type() != R8Token::RIGHT_SBRACE)
            throw R8CompilerException(R8CompilerException::RBRACE_EXPECTED, CurrentLine(), CurrentToken().TokenString());
        NextToken();
    } else
        throw R8CompilerException(R8CompilerException::REFERENCE_EXPECTED, CurrentLine(), CurrentToken().TokenString());
    return ref;
}

R8Reference R8Compiler::MemoryByRegisterReferenceBy(QString str) {
    return R8Reference(R8Reference::MEMORY_BY_REGISTER, RegisterIndexFrom(str));
}

R8Reference R8Compiler::RegisterReferenceBy(QString str) {
    return R8Reference(R8Reference::REGISTER, RegisterIndexFrom(str));
}

unsigned char R8Compiler::RegisterIndexFrom(QString str) {
    if (str.length() == 2) {
        QChar rch   = str[0];
        QChar idxch = str[1];
        if (((rch == QChar('r')) || (rch == QChar('R'))) && ((QChar('0') <= idxch) && (idxch <= QChar('7'))))
            return (unsigned char)(idxch.unicode() - QChar('0').unicode());
    }
    throw R8CompilerException(R8CompilerException::REGISTER_EXPECTED, CurrentLine(), str);
}

R8Reference R8Compiler::CompileDstReference() {
    R8Reference ref;
    if (CurrentToken().Type() == R8Token::IDENTIFIER) { //must be a register r0,..,r7
        ref = R8Reference(RegisterReferenceBy(CurrentToken().TokenString()));
        NextToken();
    } else if (CurrentToken().Type() == R8Token::LEFT_SBRACE) { //start of [12] or [r1]
        NextToken();
        if (CurrentToken().Type() == R8Token::IDENTIFIER) {
            ref = MemoryByRegisterReferenceBy(CurrentToken().TokenString());
        } else if (CurrentToken().Type() == R8Token::NUMBER) {
            ref = R8Reference(R8Reference::MEMORY_BY_CONSTANT, CurrentToken().Value());
        } else
            throw R8CompilerException(R8CompilerException::BAD_REFERENCE, CurrentLine(), CurrentToken().TokenString());

        NextToken();
        if (CurrentToken().Type() != R8Token::RIGHT_SBRACE)
            throw R8CompilerException(R8CompilerException::RBRACE_EXPECTED, CurrentLine(), CurrentToken().TokenString());
        NextToken();
    } else
        throw R8CompilerException(R8CompilerException::REFERENCE_EXPECTED, CurrentLine(), CurrentToken().TokenString());
    return ref;
}

R8Reference R8Compiler::CompileLabelReference() {
    if (CurrentToken().Type() == R8Token::IDENTIFIER) {
        mGoTos.insert(CurrentToken().TokenString().toUpper(), CompiledInstructionIndex());
        NextToken();
        return R8Reference(R8Reference::INSTRUCTION_INDEX, 0);
    } else
        throw R8CompilerException(R8CompilerException::LABEL_EXPECTED, CurrentLine(), CurrentToken().TokenString());
}

