#include "r8asmwindow.h"

#include <QComboBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextDocumentWriter>
#include <QFile>
#include <QTextStream>
#include <QSettings>

#include "ui_r8asmwindow.h"

#include "r8charstream.h"
#include "r8inputdialog.h"
#include "r8sourceeditor.h"

R8AsmWindow::R8AsmWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::R8AsmWindow) {
    ui->setupUi(this);

    InitRegisterViewModes();
    InitRegisterViews();
    InitMemoryTable();

    InitSourceEditor();
    InitSyntaxHighlighter();

    ViewAllRegisters();
    ViewAllMemory();

    AttachInputDialog();
    ConnectEngineSignals();
    ConnectEditorSignals();

    InitActions();

    InitMenu();
    InitToolBar();
    InitToolButtons();

    InitStatusbar();

    SetEngineCommandSetVariant(0);
    InitCharStream();
}

R8AsmWindow::~R8AsmWindow() {
    delete ui;
    delete mInputPort;
    delete mCharStream ;
}

void R8AsmWindow::on_r0Title_clicked() { ShiftRegisterViewFor(0); }
void R8AsmWindow::on_r1Title_clicked() { ShiftRegisterViewFor(1); }
void R8AsmWindow::on_r2Title_clicked() { ShiftRegisterViewFor(2); }
void R8AsmWindow::on_r3Title_clicked() { ShiftRegisterViewFor(3); }
void R8AsmWindow::on_r4Title_clicked() { ShiftRegisterViewFor(4); }
void R8AsmWindow::on_r5Title_clicked() { ShiftRegisterViewFor(5); }
void R8AsmWindow::on_r6Title_clicked() { ShiftRegisterViewFor(6); }
void R8AsmWindow::on_r7Title_clicked() { ShiftRegisterViewFor(7); }

void R8AsmWindow::SetEngineCommandSetVariant(int variant) {
    ClearCommandSet();

    if (variant <= 0) {
        SetZeroCommandSet();
        return;
    }

    --variant;

    SetCommonCommands();

    SetShiftsVariant(variant % SHIFTS_COUNT);
    variant = variant / SHIFTS_COUNT;

    SetLogicsVariant(variant % LOGICS_COUNT);
    variant = variant / LOGICS_COUNT;

    SetArithmeticsVariant(variant % ARITHMETICS_COUNT);
    variant = variant / ARITHMETICS_COUNT;

    SetJumpsVariant(variant % JUMPS_COUNT);

    ShiftCurrentStateTo(EDIT_STATE);

    RehighlightSource();
}

void R8AsmWindow::SetZeroCommandSet() {
    SetCommonCommands();

    SetLogicAnd();
    SetLogicNot();
    SetLogicOr();
    SetLogicXor();

    SetShiftROL();
    SetShiftROR();

    SetArithmeticsAdd();
    SetArithmeticsSub();

    SetJumpJo();
    SetJumpJz();

    ShiftCurrentStateTo(EDIT_STATE);

    RehighlightSource();
}

void R8AsmWindow::ClearCommandSet() {
    ui->commandsListWidget->clear();
    mCompiler.ClearAvailableCommands();
    mSyntaxHighlighter->ClearAvailableCommands();
}

void R8AsmWindow::RehighlightSource() { mSyntaxHighlighter->rehighlight(); }

void R8AsmWindow::SetCommonCommands() {
    SetInCommand();
    SetOutCommand();
}

void R8AsmWindow::SetShiftsVariant(int variant) {
    Q_ASSERT(variant < SHIFTS_COUNT);

    static const TVariantPtr sShifts[SHIFTS_COUNT] = {
        &R8AsmWindow::SetShiftROR,
        &R8AsmWindow::SetShiftROL};
    (this->*(sShifts[variant]))();
}

void R8AsmWindow::SetLogicsVariant(int variant) {
    Q_ASSERT(variant < LOGICS_COUNT);

    static const TVariantPtr sLogics[LOGICS_COUNT] = {
        &R8AsmWindow::SetLogicAndNot,
        &R8AsmWindow::SetLogicOrNot,
        &R8AsmWindow::SetLogicXorOr,
        &R8AsmWindow::SetLogicXorAnd,
        &R8AsmWindow::SetLogicNand,
        &R8AsmWindow::SetLogicNor
    };
    (this->*(sLogics[variant]))();
}

void R8AsmWindow::SetArithmeticsVariant(int variant) {
    Q_ASSERT(variant < ARITHMETICS_COUNT);

    static const TVariantPtr sArithmetics[ARITHMETICS_COUNT] = {
        &R8AsmWindow::SetArithmeticsAdd,
        &R8AsmWindow::SetArithmeticsSub
    };
    (this->*(sArithmetics[variant]))();
}

void R8AsmWindow::SetJumpsVariant(int variant) {
    Q_ASSERT(variant < JUMPS_COUNT);

    static const TVariantPtr sJumps[JUMPS_COUNT] = {
        &R8AsmWindow::SetJumpJz,
        &R8AsmWindow::SetJumpJo
    };
    (this->*(sJumps[variant]))();
}

void R8AsmWindow::SetLogicAndNot() {
    SetLogicAnd();
    SetLogicNot();
}

void R8AsmWindow::SetLogicOrNot() {
    SetLogicOr();
    SetLogicNot();
}

void R8AsmWindow::SetLogicXorOr() {
    SetLogicXor();
    SetLogicOr();
}

void R8AsmWindow::SetLogicXorAnd() {
    SetLogicXor();
    SetLogicAnd();
}

void R8AsmWindow::SetInCommand() {
    mCompiler.SetAvailableCommand(
                QString("IN"),
                R8CommandDescriptor(R8CommandDescriptor::ARGS_DST, R8Instruction::IN_OPCODE));
    ui->commandsListWidget->addItem(tr("in <dst>   ;dst := input"));
    mSyntaxHighlighter->SetAvailableCommand(
                QString("IN"),
                R8CommandDescriptor(R8CommandDescriptor::ARGS_DST, R8Instruction::IN_OPCODE));
}

void R8AsmWindow::SetOutCommand() {
    mCompiler.SetAvailableCommand(
                QString("OUT"),
                R8CommandDescriptor(R8CommandDescriptor::ARGS_SRC, R8Instruction::OUT_OPCODE));
    ui->commandsListWidget->addItem(tr("out <src>   ;output := src"));
    mSyntaxHighlighter->SetAvailableCommand(
                QString("OUT"),
                R8CommandDescriptor(R8CommandDescriptor::ARGS_SRC, R8Instruction::OUT_OPCODE));
}

void R8AsmWindow::SetShiftROR() {
    mCompiler.SetAvailableCommand(
                QString("ROR"),
                R8CommandDescriptor(R8CommandDescriptor::ARGS_SRC_SRC_DST, R8Instruction::ROR_OPCODE));
    ui->commandsListWidget->addItem(tr("ror <src1>, <src2>, <dst>   ;dst := src1 >>> src2"));
    mSyntaxHighlighter->SetAvailableCommand(
                QString("ROR"),
                R8CommandDescriptor(R8CommandDescriptor::ARGS_SRC_SRC_DST, R8Instruction::ROR_OPCODE));
}

void R8AsmWindow::SetShiftROL() {
    mCompiler.SetAvailableCommand(
                QString("ROL"),
                R8CommandDescriptor(R8CommandDescriptor::ARGS_SRC_SRC_DST, R8Instruction::ROL_OPCODE));
    ui->commandsListWidget->addItem(tr("rol <src1>, <src2>, <dst>   ;dst := src1 <<< src2"));
    mSyntaxHighlighter->SetAvailableCommand(
                QString("ROL"),
                R8CommandDescriptor(R8CommandDescriptor::ARGS_SRC_SRC_DST, R8Instruction::ROL_OPCODE));
}

void R8AsmWindow::SetLogicNot() {
    mCompiler.SetAvailableCommand(
                QString("NOT"),
                R8CommandDescriptor(R8CommandDescriptor::ARGS_SRC_DST, R8Instruction::NOT_OPCODE));
    ui->commandsListWidget->addItem(tr("not <src>, <dst>   ;dst := NOT(src)"));
    mSyntaxHighlighter->SetAvailableCommand(
                QString("NOT"),
                R8CommandDescriptor(R8CommandDescriptor::ARGS_SRC_DST, R8Instruction::NOT_OPCODE));
}

void R8AsmWindow::SetLogicXor() {
    mCompiler.SetAvailableCommand(
                QString("XOR"),
                R8CommandDescriptor(R8CommandDescriptor::ARGS_SRC_SRC_DST, R8Instruction::XOR_OPCODE));
    ui->commandsListWidget->addItem(tr("xor <src1>, <src2>, <dst>   ;dst := XOR(src1,src2)"));
    mSyntaxHighlighter->SetAvailableCommand(
                QString("XOR"),
                R8CommandDescriptor(R8CommandDescriptor::ARGS_SRC_SRC_DST, R8Instruction::XOR_OPCODE));
}

void R8AsmWindow::SetLogicAnd() {
    mCompiler.SetAvailableCommand(
                QString("AND"),
                R8CommandDescriptor(R8CommandDescriptor::ARGS_SRC_SRC_DST, R8Instruction::AND_OPCODE));
    ui->commandsListWidget->addItem(tr("and <src1>, <src2>, <dst>   ;dst := AND(src1,src2)"));
    mSyntaxHighlighter->SetAvailableCommand(
                QString("AND"),
                R8CommandDescriptor(R8CommandDescriptor::ARGS_SRC_SRC_DST, R8Instruction::AND_OPCODE));
}

void R8AsmWindow::SetLogicOr() {
    mCompiler.SetAvailableCommand(
                QString("OR"),
                R8CommandDescriptor(R8CommandDescriptor::ARGS_SRC_SRC_DST, R8Instruction::OR_OPCODE));
    ui->commandsListWidget->addItem(tr("or <src1>, <src2>, <dst>   ;dst := OR(src1,src2)"));
    mSyntaxHighlighter->SetAvailableCommand(
                QString("OR"),
                R8CommandDescriptor(R8CommandDescriptor::ARGS_SRC_SRC_DST, R8Instruction::OR_OPCODE));
}

void R8AsmWindow::SetLogicNand() {
    mCompiler.SetAvailableCommand(
                QString("NAND"),
                R8CommandDescriptor(R8CommandDescriptor::ARGS_SRC_SRC_DST, R8Instruction::NAND_OPCODE));
    ui->commandsListWidget->addItem(tr("nand <src1>, <src2>, <dst>   ;dst := NOT(AND(src1,src2))"));
}

void R8AsmWindow::SetLogicNor() {
    mCompiler.SetAvailableCommand(
                QString("NOR"),
                R8CommandDescriptor(R8CommandDescriptor::ARGS_SRC_SRC_DST, R8Instruction::NOR_OPCODE));
    ui->commandsListWidget->addItem(tr("nor <src1>, <src1>, <dst>   ;dst := NOT(OR(src1,src2))"));
    mSyntaxHighlighter->SetAvailableCommand(
                QString("NOR"),
                R8CommandDescriptor(R8CommandDescriptor::ARGS_SRC_SRC_DST, R8Instruction::NOR_OPCODE));
}

void R8AsmWindow::SetArithmeticsAdd() {
    mCompiler.SetAvailableCommand(
                QString("ADD"),
                R8CommandDescriptor(R8CommandDescriptor::ARGS_SRC_SRC_DST, R8Instruction::ADD_OPCODE));
    ui->commandsListWidget->addItem(tr("add <src1>, <src2>, <dst>   ;dst := src1 + src2"));
    mSyntaxHighlighter->SetAvailableCommand(
                QString("ADD"),
                R8CommandDescriptor(R8CommandDescriptor::ARGS_SRC_SRC_DST, R8Instruction::ADD_OPCODE));
}

void R8AsmWindow::SetArithmeticsSub() {
    mCompiler.SetAvailableCommand(
                QString("SUB"),
                R8CommandDescriptor(R8CommandDescriptor::ARGS_SRC_SRC_DST, R8Instruction::SUB_OPCODE));
    ui->commandsListWidget->addItem(tr("sub <src1>, <src2>, <dst>  ;dst := src1 - src2"));
    mSyntaxHighlighter->SetAvailableCommand(
                QString("SUB"),
                R8CommandDescriptor(R8CommandDescriptor::ARGS_SRC_SRC_DST, R8Instruction::SUB_OPCODE));
}

void R8AsmWindow::SetJumpJz() {
    mCompiler.SetAvailableCommand(
                QString("JZ"),
                R8CommandDescriptor(R8CommandDescriptor::ARGS_SRC_LABEL, R8Instruction::JZ_OPCODE));
    ui->commandsListWidget->addItem(tr("jz <src>, <lbl>   ;if (src=0x00) goto lbl"));
    mSyntaxHighlighter->SetAvailableCommand(
                QString("JZ"),
                R8CommandDescriptor(R8CommandDescriptor::ARGS_SRC_LABEL, R8Instruction::JZ_OPCODE));
}

void R8AsmWindow::SetJumpJo() {
    mCompiler.SetAvailableCommand(
                QString("JO"),
                R8CommandDescriptor(R8CommandDescriptor::ARGS_SRC_LABEL, R8Instruction::JO_OPCODE));
    ui->commandsListWidget->addItem(tr("jo <src>, <lbl>   ;if (src=0xFF) goto lbl"));
    mSyntaxHighlighter->SetAvailableCommand(
                QString("JO"),
                R8CommandDescriptor(R8CommandDescriptor::ARGS_SRC_LABEL, R8Instruction::JO_OPCODE));
}

void R8AsmWindow::InitRegisterViewModes() {
    for (unsigned int i = 0; i<R8Engine::REGISTERS_COUNT; ++i)
        mRegisterViewMode[i] = HEX_MODE;
}

void R8AsmWindow::InitRegisterViews() {
    mRegisterView[0] = ui->r0LineEdit;
    mRegisterView[1] = ui->r1LineEdit;
    mRegisterView[2] = ui->r2LineEdit;
    mRegisterView[3] = ui->r3LineEdit;
    mRegisterView[4] = ui->r4LineEdit;
    mRegisterView[5] = ui->r5LineEdit;
    mRegisterView[6] = ui->r6LineEdit;
    mRegisterView[7] = ui->r7LineEdit;
}

void R8AsmWindow::InitMemoryTable() {
    int rowCount = (R8Engine::MEMORY_SIZE + MEMORY_TABLE_COLUMN_COUNT - 1) / MEMORY_TABLE_COLUMN_COUNT ;

    ui->memoryTable->setColumnCount(MEMORY_TABLE_COLUMN_COUNT);
    ui->memoryTable->setRowCount(rowCount);

    QStringList columnLabels;
    for (int i=0; i<MEMORY_TABLE_COLUMN_COUNT ; ++i)
        columnLabels.append(QString("%1").arg(i, 0, MEMORY_TABLE_COLUMN_COUNT ).toUpper());
    ui->memoryTable->setHorizontalHeaderLabels(columnLabels);

    QStringList rowLabels;
    for (int i=0; i<rowCount; ++i)
        rowLabels.append(QString("0x%1").arg(QString("%1").arg(i, 0, MEMORY_TABLE_COLUMN_COUNT).toUpper()));
    ui->memoryTable->setVerticalHeaderLabels(rowLabels);

    for (int ri=0; ri<rowCount; ++ri)
        for (int ci=0; ci<MEMORY_TABLE_COLUMN_COUNT ; ++ci) {
            QTableWidgetItem *item = new QTableWidgetItem();
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            ui->memoryTable->setItem(ri, ci, item);
        }
}

void R8AsmWindow::InitSourceEditor() {
    mSourceEditor = (R8SourceEditor*)ui->sourcePlainText;
}

void R8AsmWindow::InitSyntaxHighlighter() {
    mSyntaxHighlighter = new R8SyntaxHighlighter(ui->sourcePlainText->document());
}

void R8AsmWindow::InitMenu() {
    QMenu *fileMenu = new QMenu(tr("&File"));
    fileMenu->addAction(mOpenSourceAction);
    fileMenu->addSeparator();
    fileMenu->addAction(mSaveSourceAction);
    fileMenu->addAction(mSaveSourceAsAction);
    fileMenu->addSeparator();
    QAction *exitAction = fileMenu->addAction(tr("&Exit"));
    connect(exitAction, SIGNAL(triggered()), SLOT(SlotExit()));

    QMenu *programMenu = new QMenu(tr("&Program"));
    programMenu->addAction(mCompileAction);
    programMenu->addSeparator();
    programMenu->addAction(mStepAction);
    programMenu->addAction(mRunAction);
    programMenu->addAction(mStopAction);
    programMenu->addAction(mSetBreakpointAction);
    programMenu->addAction(mResetAction);

    QMenu *helpMenu = new QMenu(tr("&Help"));
    QAction *aboutAction = helpMenu->addAction(tr("About R8"));
    connect(aboutAction, SIGNAL(triggered()), SLOT(SlotAbout()));
    helpMenu->addSeparator();
    QMenu *langSubMenu = helpMenu->addMenu(tr("UI language"));
    QAction *englishUIAction = langSubMenu->addAction(QString("English"));
    connect(englishUIAction, SIGNAL(triggered()), SLOT(SlotEnglish()));
    QAction *russianUIAction = langSubMenu->addAction(QString("Русский"));
    connect(russianUIAction, SIGNAL(triggered()), SLOT(SlotRussian()));

    menuBar()->addMenu(fileMenu);
    menuBar()->addMenu(programMenu);
    menuBar()->addMenu(helpMenu);
}

void R8AsmWindow::InitToolBar() {
    QToolBar *toolBar = ui->mainToolBar;

    toolBar->addAction(mOpenSourceAction);
    toolBar->addSeparator();
    toolBar->addAction(mSaveSourceAction);
    toolBar->addAction(mSaveSourceAsAction);
    toolBar->addSeparator();
    toolBar->addAction(mCompileAction);
    toolBar->addSeparator();
    toolBar->addAction(mStepAction);
    toolBar->addAction(mRunAction);
    toolBar->addAction(mStopAction);
    toolBar->addAction(mSetBreakpointAction);
    toolBar->addAction(mResetAction);

    addToolBar(toolBar);
}

void R8AsmWindow::InitToolButtons() {
    ui->compileToolButton->setDefaultAction(mCompileAction);
    ui->compileToolButton->setDefaultAction(mCompileAction);
    ui->stepToolButton->setDefaultAction(mStepAction);
    ui->runToolButton->setDefaultAction(mRunAction);
    ui->resetToolButton->setDefaultAction(mResetAction);
    ui->stopToolButton->setDefaultAction(mStopAction);
    ui->setBreakpointToolButton->setDefaultAction(mSetBreakpointAction);
}

void R8AsmWindow::InitActions() {
    mSaveSourceAction = new QAction(QIcon(":/images/save"), tr("&Save"), this);
    mSaveSourceAction->setShortcut(QKeySequence::Save);
    mSaveSourceAction->setToolTip(tr("Save source"));
    mSaveSourceAction->setStatusTip(tr("Save program source to file"));
    mSaveSourceAction->setWhatsThis(tr("Save program source to file"));
    connect(mSaveSourceAction, SIGNAL(triggered()), SLOT(SlotSaveSource()));

    mSaveSourceAsAction = new QAction(QIcon(":/images/saveas"), tr("&Save as..."), this);
    mSaveSourceAsAction->setShortcut(QKeySequence::SaveAs);
    mSaveSourceAsAction->setToolTip(tr("Save source as..."));
    mSaveSourceAsAction->setStatusTip(tr("Save program source to file"));
    mSaveSourceAsAction->setWhatsThis(tr("Save program source to file"));
    connect(mSaveSourceAsAction, SIGNAL(triggered()), SLOT(SlotSaveAsSource()));

    mOpenSourceAction = new QAction(QIcon(":/images/open"), tr("&Open"), this);
    mOpenSourceAction->setShortcut(QKeySequence::Open);
    mOpenSourceAction->setToolTip(tr("Open program source"));
    mOpenSourceAction->setStatusTip(tr("Save program source to file"));
    mOpenSourceAction->setWhatsThis(tr("Save program source to file"));
    connect(mOpenSourceAction, SIGNAL(triggered()), SLOT(SlotOpenSource()));

    mCompileAction = new QAction(QIcon(":/images/compile"), tr("&Compile"), this);
    mCompileAction->setShortcut(QKeySequence(tr("F8")));
    mCompileAction->setToolTip(tr("Compile source (F8)"));
    mCompileAction->setStatusTip(tr("Compile program source to machine code (F8)"));
    mCompileAction->setWhatsThis(tr("Compile program source to machine code (F8)"));
    connect(mCompileAction, SIGNAL(triggered()), SLOT(SlotCompile()));

    mStepAction = new QAction(QIcon(":/images/step"), tr("Step"), this);
    mStepAction->setShortcut(QKeySequence(tr("F10")));
    mStepAction->setToolTip(tr("Step (F10)"));
    mStepAction->setStatusTip(tr("Execute one R8 command (F10)"));
    mStepAction->setWhatsThis(tr("Execute one R8 command (F10)"));
    connect(mStepAction, SIGNAL(triggered()), SLOT(SlotStep()));

    mRunAction = new QAction(QIcon(":/images/run"), tr("Run"), this);
    mRunAction->setShortcut(QKeySequence(tr("F5")));
    mRunAction->setToolTip(tr("Run (F5)"));
    mRunAction->setStatusTip(tr("Execute R8 program (F5)"));
    mRunAction->setWhatsThis(tr("Execute R8 program (F5)"));
    connect(mRunAction, SIGNAL(triggered()), SLOT(SlotRun()));

    mResetAction = new QAction(QIcon(":/images/restart"), tr("&Restart"), this);
    mResetAction->setShortcut(QKeySequence(tr("F4")));
    mResetAction->setToolTip(tr("Reset/Restart (F4)"));
    mResetAction->setStatusTip(tr("Reset R8 processor and restart program (F4)"));
    mResetAction->setWhatsThis(tr("Reset R8 processor and restart program (F4)"));
    connect(mResetAction, SIGNAL(triggered()), SLOT(SlotReset()));

    mStopAction = new QAction(QIcon(":/images/stop"), tr("Stop"), this);
    mStopAction->setShortcut(QKeySequence(tr("F2")));
    mStopAction->setToolTip(tr("Stop (F2)"));
    mStopAction->setStatusTip(tr("Stop R8 program execution (F2)"));
    mStopAction->setWhatsThis(tr("Stop R8 program execution (F2)"));
    connect(mStopAction, SIGNAL(triggered()), SLOT(SlotStop()));

    mSetBreakpointAction = new QAction(QIcon(":/images/breakpoint"), tr("Set/clear &breakpoint"), this);
    mSetBreakpointAction->setShortcut(QKeySequence(tr("F9")));
    mSetBreakpointAction->setToolTip(tr("Set/clear breakpoint (F9)"));
    mSetBreakpointAction->setStatusTip(tr("Set/clear breakpoint at current source line (F9)"));
    mSetBreakpointAction->setWhatsThis(tr("Set/clear breakpoint at current source line (F9)"));
    connect(mSetBreakpointAction, SIGNAL(triggered()), SLOT(SlotSetBreakpoint()));
}

void R8AsmWindow::InitStatusbar() {
    QComboBox *combo = new QComboBox();
    for (int i=0; i<VARIANTS_COUNT; ++i) {
        combo->addItem(QString(tr("Command set #%1").arg(i)));
    }
    connect(combo, SIGNAL(currentIndexChanged(int)), this, SLOT(SlotArchitectureVariant(int)));
    statusBar()->addWidget(combo, 100);
}

void R8AsmWindow::InitCharStream() {
    mCharStream = new R8SourceEditorCharStream();
}

void R8AsmWindow::ShiftRegisterViewFor(int index) {
    Q_ASSERT((unsigned int)index < R8Engine::REGISTERS_COUNT);

    switch (mRegisterViewMode[index]) {
    case BIN_MODE:
        mRegisterViewMode[index] = OCT_MODE; break;
    case OCT_MODE:
        mRegisterViewMode[index] = HEX_MODE; break;
    case HEX_MODE:
        mRegisterViewMode[index] = DEC_MODE; break;
    case DEC_MODE:
    default:
        mRegisterViewMode[index] = BIN_MODE;
    }

    ViewRegister(index);
}

void R8AsmWindow::ViewRegister(int index) {
    Q_ASSERT((unsigned int)index < R8Engine::REGISTERS_COUNT);

    mRegisterView[index]->setText(FormatCell(mEngine.Register(index), mRegisterViewMode[index]));
}

void R8AsmWindow::ViewAllRegisters() {
    for (unsigned int i=0; i<R8Engine::REGISTERS_COUNT; ++i)
        ViewRegister(i);
}

void R8AsmWindow::ViewMemory(int index) {
    int row = index / MEMORY_TABLE_COLUMN_COUNT;
    int col = index % MEMORY_TABLE_COLUMN_COUNT;

    QTableWidgetItem *item = ui->memoryTable->item(row, col);
    item->setText(QString("%1").arg(mEngine.MemoryCell(index), 2, 16, QLatin1Char('0')));
}

void R8AsmWindow::ViewAllMemory() {
    for (unsigned int i=0; i<R8Engine::MEMORY_SIZE; ++i)
        ViewMemory(i);
    ui->memoryTable->resizeColumnsToContents();
}

void R8AsmWindow::ViewIP() {
    if (IsCurrentStateIs(EDIT_STATE)) {
        HideIpMarkInEditor();
        return;
    }
    mSourceEditor->SetIpAtLine(mCompiler.SourceLineForIp(mEngine.IP()));
}

void R8AsmWindow::ViewExecutionTime() {
    unsigned int time = mEngine.ExecutionTime();
    unsigned int timeMod100 = time % 100;

    if ((11 <= timeMod100) && (timeMod100 <= 14)) {
        ui->timeLabel->setText(QString(tr("%1 clocks", "11")).arg(time));
        return;
    }

    unsigned int timeMod10 = time % 10;
    if (timeMod10 == 0) {
        ui->timeLabel->setText(QString(tr("%1 clocks", "0")).arg(time));
    } else if (timeMod10 == 1) {
        ui->timeLabel->setText(QString(tr("%1 clock", "1")).arg(time));
    } else if (timeMod10 >= 5) {
        ui->timeLabel->setText(QString(tr("%1 clocks", "5")).arg(time));
    } else {
        ui->timeLabel->setText(QString(tr("%1 clocks", "2")).arg(time));
    }
}

void R8AsmWindow::ShowR8State() {
    ViewAllRegisters();
    ViewAllMemory();
    ViewIP();
    ViewExecutionTime();
}

QString R8AsmWindow::FormatCell(unsigned char value, R8AsmWindow::EByteViewMode mode) const {
    switch (mode) {
    case BIN_MODE:
        return QString("0b%1").arg((unsigned int)value, 8, 2, QLatin1Char('0'));
    case OCT_MODE:
        return QString("0%1").arg((unsigned int)value, 0, 8);
    case HEX_MODE:
        return QString("0x%1").arg((unsigned int)value, 2, 16, QLatin1Char('0'));
    case DEC_MODE:
    default:
        return QString("%1").arg((unsigned int)value);
    }
}

void R8AsmWindow::AttachInputDialog() {
    mInputPort = new R8UiInputPort();
    mEngine.SetInputPort(mInputPort);
}

void R8AsmWindow::ConnectEngineSignals() {
    connect(&mEngine, SIGNAL(SignalReset()),                     this, SLOT(SlotEngineReset()));
    connect(&mEngine, SIGNAL(SignalHalt()),                      this, SLOT(SlotEngineHalt()));
    connect(&mEngine, SIGNAL(SignalOutput(unsigned char)),       this, SLOT(SlotEngineOutput(unsigned char)));
    connect(&mEngine, SIGNAL(SignalWriteRegister(unsigned int)), this, SLOT(SlotEngineWriteRegister(unsigned int)));
    connect(&mEngine, SIGNAL(SignalWriteMemory(unsigned int)),   this, SLOT(SlotEngineWriteMemory(unsigned int)));
}

void R8AsmWindow::ConnectEditorSignals() {
    connect(mSourceEditor, SIGNAL(textChanged()), this, SLOT(SlotSourceChanged()));
}

void R8AsmWindow::DescribeCompilerException(const R8CompilerException &ex) {
    switch (ex.Type()) {
    case R8CompilerException::BAD_EXPRESSION:
        ErrorMessage(tr("Bad expression"), ex.LineNumber());
        break;
    case R8CompilerException::UNRESOLVED_LABEL:
        ErrorMessage(tr("Unresolved label \"%1\"").arg(ex.Info()), ex.LineNumber());
        break;
    case R8CompilerException::COMMA_EXPECTED:
        ErrorMessage(tr("Comma expected"), ex.LineNumber());
        break;
    case R8CompilerException::LABEL_REDEFINITION:
        ErrorMessage(tr("Label \"%1\" redefinition").arg(ex.Info()), ex.LineNumber());
        break;
    case R8CompilerException::UNDEFINED_COMMAND:
        ErrorMessage(tr("Undefined \"%1\" command").arg(ex.Info()), ex.LineNumber());
        break;
    case R8CompilerException::BAD_REFERENCE:
        ErrorMessage(tr("Bad memory reference"), ex.LineNumber());
        break;
    case R8CompilerException::REFERENCE_EXPECTED:
        ErrorMessage(tr("Reference expected"), ex.LineNumber());
        break;
    case R8CompilerException::RBRACE_EXPECTED:
        ErrorMessage(tr("Rbrace \"]\" expected"), ex.LineNumber());
        break;
    case R8CompilerException::LABEL_EXPECTED:
        ErrorMessage(tr("Rbrace \"]\" expected"), ex.LineNumber());
        break;
    case R8CompilerException::REGISTER_EXPECTED:
        ErrorMessage(tr("Register name expected instead \"%1\"").arg(ex.Info()), ex.LineNumber());
        break;
    default:
        ErrorMessage(tr("Error of unknown type"), ex.LineNumber());
    }
}

void R8AsmWindow::DescribeLexerException(const R8LexerException &ex) {
    switch (ex.Type()) {
    case R8LexerException::UNKNOWN_TOKEN:
        ErrorMessage(tr("Unlnown token \"%1\" lexical error").arg(ex.Info()), ex.LineNumber());
        break;
    default:
        ErrorMessage(tr("Lexical error of unknown type"), ex.LineNumber());
    }
}

void R8AsmWindow::ErrorMessage(const QString &message, int line) {
    ui->outputListWidget->clear();
    ui->outputListWidget->insertItem(0, QString(tr("%1 at %2")).arg(message).arg(line + 1));
    mSourceEditor->SetIpAtLine(line);
}

bool R8AsmWindow::IsBreakedIp(int ip) const {
    int currLine = mCompiler.SourceLineForIp(ip);
    int nextLine = mCompiler.SourceLineForIp(ip + 1);

    if (currLine >= 0) {
        if (nextLine >= 0) {
            if (nextLine > currLine)
                return mSourceEditor->IsBreakointBetweenLines(currLine, nextLine - 1);
        }
        return mSourceEditor->IsBreakointBetweenLines(currLine, currLine);
    } else
        return false;
}

void R8AsmWindow::HideIpMarkInEditor() {
    mSourceEditor->SetIpAtLine(-1);
}

void R8AsmWindow::ShiftCurrentStateTo(R8AsmWindow::EState state) {
    mCurrentState = state;

    SetActionsForState(state);
    SetEditorForState(state);
}

void R8AsmWindow::SetActionsForState(EState state) {
    switch (state) {
    case EDIT_STATE:
        mStepAction->setEnabled(false);
        mRunAction->setEnabled(false);
        mResetAction->setEnabled(false);
        mStopAction->setEnabled(false);
        mSetBreakpointAction->setEnabled(false);

        mOpenSourceAction->setEnabled(true);
        mSaveSourceAction->setEnabled(true);
        mSaveSourceAsAction->setEnabled(true);
        break;
    case STEP_STATE:
        mStepAction->setEnabled(true);
        mRunAction->setEnabled(true);
        mResetAction->setEnabled(true);
        mStopAction->setEnabled(false);
        mSetBreakpointAction->setEnabled(true);

        mOpenSourceAction->setEnabled(true);
        mSaveSourceAction->setEnabled(true);
        mSaveSourceAsAction->setEnabled(true);
        break;
    case RUN_STATE:
        mStepAction->setEnabled(false);
        mRunAction->setEnabled(false);
        mResetAction->setEnabled(true);
        mStopAction->setEnabled(true);
        mSetBreakpointAction->setEnabled(true);

        mOpenSourceAction->setEnabled(false);
        mSaveSourceAction->setEnabled(false);
        mSaveSourceAsAction->setEnabled(false);
        break;
    case HALT_STATE:
        mStepAction->setEnabled(false);
        mRunAction->setEnabled(false);
        mResetAction->setEnabled(true);
        mStopAction->setEnabled(false);
        mSetBreakpointAction->setEnabled(true);

        mOpenSourceAction->setEnabled(false);
        mSaveSourceAction->setEnabled(false);
        mSaveSourceAsAction->setEnabled(false);
        break;
    default:
        Q_ASSERT(false);
    }
}

void R8AsmWindow::SetEditorForState(R8AsmWindow::EState state) {
    switch (state) {
    case EDIT_STATE:
        mSourceEditor->ShiftStateTo(R8SourceEditor::EDIT_STATE);
        break;
    default:
        mSourceEditor->ShiftStateTo(R8SourceEditor::DEBUG_STATE);
    }
}

void R8AsmWindow::Step() {
    unsigned int line = mCompiler.SourceLineForIp(mEngine.IP());

    try {
        mEngine.Step();
    } catch (R8Exception& ex) {
        ui->outputListWidget->insertItem(0, QString(tr("Execution error: \"%1\" at %2")).arg(ex.Message()).arg(line));
        mEngine.Halt();
    }
}

void R8AsmWindow::SlotEngineReset() {
    ui->outputListWidget->clear();
    ShowR8State();
}

void R8AsmWindow::SlotEngineHalt() {
    ShiftCurrentStateTo(HALT_STATE);

    ShowR8State();
}

void R8AsmWindow::SlotEngineOutput(unsigned char value) {
    value = value;
    ui->outputListWidget->insertItem(
                0,
                QString("0x%1 (0b%2, %3)")
                    .arg((unsigned int)value, 2, 16, QLatin1Char('0'))
                         .arg((unsigned int)value, 8, 2,  QLatin1Char('0'))
                         .arg((unsigned int)value));
}

void R8AsmWindow::SlotEngineWriteRegister(unsigned int index) {
    ViewRegister(index);
}

void R8AsmWindow::SlotEngineWriteMemory(unsigned int index) {
    ViewMemory(index);
}

void R8AsmWindow::SlotSourceChanged() {
    if (IsCurrentStateIs(EDIT_STATE))
        return;
    ShiftCurrentStateTo(EDIT_STATE);
    ViewIP();
}

void R8AsmWindow::SlotSaveSource() {
    if (mSourcePath.isEmpty()) {
        QString fileName = QFileDialog::getSaveFileName(
                    this,
                    tr("Save R8 source file"),
                    QString(),
                    tr("R8 sources(*.r8);;Text files (*.txt)"));
        if (fileName.isNull())
            return;
        mSourcePath = fileName;
    }

    if (mSourcePath.isEmpty() || mSourcePath.isNull())
        return;

    QTextDocumentWriter writer;
    writer.setFormat("plaintext");
    writer.setFileName(mSourcePath);

    writer.write(mSourceEditor->document());
}

void R8AsmWindow::SlotSaveAsSource() {
    QString fileName = QFileDialog::getSaveFileName(
                this,
                tr("Save R8 source file"),
                QString(),
                tr("R8 sources(*.r8);;Text files (*.txt)"));
    if (fileName.isNull())
        return;

    mSourcePath = fileName;

    if (mSourcePath.isEmpty() || mSourcePath.isNull())
        return;

    QTextDocumentWriter writer;
    writer.setFormat("plaintext");
    writer.setFileName(mSourcePath);

    writer.write(mSourceEditor->document());
}

void R8AsmWindow::SlotOpenSource() {
    QString fileName = QFileDialog::getOpenFileName(
                this,
                tr("Open R8 source file"),
                QString(),
                tr("R8 sources(*.r8);;Text files (*.txt)"));
    if (fileName.isNull())
        return;

    mSourcePath = fileName;

    QFile file(mSourcePath);
    file.open(QFile::ReadOnly | QFile::Text);
    QTextStream fileStream(&file);
    mSourceEditor->setPlainText(fileStream.readAll());
    mSourceEditor->ClearAllBreakpoints();

    ShiftCurrentStateTo(EDIT_STATE);
}

void R8AsmWindow::SlotExit() {
    if (QMessageBox::question(
                this,
                tr("Save before exit"),
                tr("Save R8 source file?"),
                QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        SlotSaveSource();
    }
    qApp->exit();
}

void R8AsmWindow::SlotAbout() {
    QMessageBox::information(
                this,
                tr("About R8"),
                tr( "RISC R8 Engine (selectable command set)\n"
                    "Author: M.M.Shihov, m.m.shihov@gmail.com"));
}

void R8AsmWindow::SlotEnglish() {
    QSettings settings;
    settings.setValue("ui/language", "en_US");
}

void R8AsmWindow::SlotRussian() {
    QSettings settings;
    settings.setValue("ui/language", "ru_RU");
}

void R8AsmWindow::SlotCompile() {
    try {
        mCharStream->SetSourceTextBlock(mSourceEditor->document()->begin());
        mCompiler.SetSource(mCharStream);
        mCompiler.Compile();

        ShiftCurrentStateTo(STEP_STATE);

        mEngine.SetProgram(mCompiler.CompiledCode());
    } catch (const R8CompilerException& compilerEx) {
        DescribeCompilerException(compilerEx);

        ShiftCurrentStateTo(EDIT_STATE);
    } catch (const R8LexerException& lexerEx) {
        DescribeLexerException(lexerEx);

        ShiftCurrentStateTo(EDIT_STATE);
    }
}

void R8AsmWindow::SlotStep() {
    mSourceEditor->SetIpAtLine(mCompiler.SourceLineForIp(mEngine.IP()));
    Step();
    mSourceEditor->SetIpAtLine(mCompiler.SourceLineForIp(mEngine.IP()));
    ViewExecutionTime();
}

void R8AsmWindow::SlotRun() {
    ShiftCurrentStateTo(RUN_STATE);

    HideIpMarkInEditor();

    while (IsCurrentStateIs(RUN_STATE)) {
        Step();
        if (IsBreakedIp(mEngine.IP())) {
            ShiftCurrentStateTo(STEP_STATE);
            break;
        }
        qApp->processEvents();
    }

    ShowR8State();
}

void R8AsmWindow::SlotReset() {
    ShiftCurrentStateTo(STEP_STATE);

    mEngine.Reset();
}

void R8AsmWindow::SlotStop() {
    ShiftCurrentStateTo(STEP_STATE);
}

void R8AsmWindow::SlotSetBreakpoint() {
    mSourceEditor->AddOrRemoveBreakpointAt(mSourceEditor->textCursor().blockNumber());
}

void R8AsmWindow::SlotArchitectureVariant(int variant) {
    SetEngineCommandSetVariant(variant);
}
