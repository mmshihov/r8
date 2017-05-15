#ifndef R8ASMWINDOW_H
#define R8ASMWINDOW_H

#include <QMainWindow>

#include <QLineEdit>

#include "r8compiler.h"
#include "r8syntaxhighlighter.h"

namespace Ui {
class R8AsmWindow;
}

class R8InputPort;
class R8SourceEditor;
class R8SourceEditorCharStream;

class R8AsmWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit R8AsmWindow(QWidget *parent = 0);
    ~R8AsmWindow();

private:
    Ui::R8AsmWindow *ui;

    typedef void (R8AsmWindow::*TVariantPtr)();
    enum EState {
        EDIT_STATE,
        STEP_STATE,
        RUN_STATE,
        HALT_STATE
    };

    enum EByteViewMode {
        BIN_MODE,
        OCT_MODE,
        HEX_MODE,
        DEC_MODE
    };

    static const int SHIFTS_COUNT      = 2; // rol, ror
    static const int LOGICS_COUNT      = 6; // {and,not},{or,not},{xor,or}, nand, nor, {xor,and}
    static const int ARITHMETICS_COUNT = 2; // add,sub
    static const int JUMPS_COUNT       = 2; // jz,jo

    static const int VARIANTS_COUNT    = SHIFTS_COUNT * LOGICS_COUNT * ARITHMETICS_COUNT * JUMPS_COUNT + 1;

    static const int MEMORY_TABLE_COLUMN_COUNT = 16;

    EState                    mCurrentState;

    R8Compiler                mCompiler;
    R8Engine                  mEngine;
    R8SyntaxHighlighter      *mSyntaxHighlighter;
    R8InputPort              *mInputPort;
    R8SourceEditor           *mSourceEditor;
    R8SourceEditorCharStream *mCharStream;

    QAction                  *mSaveSourceAction,
                             *mSaveSourceAsAction,
                             *mOpenSourceAction,
                             *mCompileAction,
                             *mStepAction,
                             *mRunAction,
                             *mResetAction,
                             *mStopAction,
                             *mSetBreakpointAction;

    QString                   mSourcePath;

    EByteViewMode mRegisterViewMode[R8Engine::REGISTERS_COUNT];
    QLineEdit *mRegisterView[R8Engine::REGISTERS_COUNT];

    void SetEngineCommandSetVariant(int variant);
    void SetZeroCommandSet();

    void ClearCommandSet();
    void RehighlightSource();

    void SetCommonCommands();

    void SetShiftsVariant(int variant);
    void SetLogicsVariant(int variant);
    void SetArithmeticsVariant(int variant);
    void SetJumpsVariant(int variant);

    void SetLogicAndNot();
    void SetLogicOrNot();
    void SetLogicXorOr();
    void SetLogicXorAnd();

    void SetInCommand();
    void SetOutCommand();

    void SetShiftROR();
    void SetShiftROL();

    void SetLogicNot();
    void SetLogicXor();
    void SetLogicAnd();
    void SetLogicOr();
    void SetLogicNand();
    void SetLogicNor();

    void SetArithmeticsAdd();
    void SetArithmeticsSub();

    void SetJumpJz();
    void SetJumpJo();

    void InitRegisterViewModes();
    void InitRegisterViews();
    void InitMemoryTable();
    void InitSourceEditor();
    void InitSyntaxHighlighter();

    void InitMenu();
    void InitToolBar();
    void InitToolButtons();

    void InitActions();
    void InitStatusbar();

    void InitCharStream();

    void ShiftRegisterViewFor(int index);
    void ViewRegister(int index);
    void ViewAllRegisters();
    void ViewMemory(int index);
    void ViewAllMemory();
    void ViewIP();
    void ViewExecutionTime();

    void ShowR8State();

    QString FormatCell(unsigned char value, EByteViewMode mode) const;

    void AttachInputDialog();
    void ConnectEngineSignals();
    void ConnectEditorSignals();

    void DescribeCompilerException(const R8CompilerException& ex);
    void DescribeLexerException(const R8LexerException& ex);
    void ErrorMessage(const QString& message, int line);

    bool IsBreakedIp(int ip) const;
    void HideIpMarkInEditor();

    EState CurrentState() const {return mCurrentState;}
    bool   IsCurrentStateIs(EState state) const {return (CurrentState() == state);}
    void   ShiftCurrentStateTo(EState state);
    void   SetActionsForState(EState state);
    void   SetEditorForState(EState state);

    void   Step();

private slots:
    void SlotEngineReset();
    void SlotEngineHalt();
    void SlotEngineOutput(unsigned char value);
    void SlotEngineWriteRegister(unsigned int index);
    void SlotEngineWriteMemory(unsigned int index);

    void SlotSourceChanged();

    void SlotSaveSource();
    void SlotSaveAsSource();
    void SlotOpenSource();
    void SlotExit();
    void SlotAbout();
    void SlotEnglish();
    void SlotRussian();
    void SlotCompile();
    void SlotStep();
    void SlotRun();
    void SlotReset();
    void SlotStop();
    void SlotSetBreakpoint();
    void SlotArchitectureVariant(int variant);

    void on_r0Title_clicked();
    void on_r1Title_clicked();
    void on_r2Title_clicked();
    void on_r3Title_clicked();
    void on_r4Title_clicked();
    void on_r5Title_clicked();
    void on_r6Title_clicked();
    void on_r7Title_clicked();
};

#endif // R8ASMWINDOW_H
