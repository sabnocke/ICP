/**
 * @file mainwindow.cpp
 * @brief Implements the GUI logic for the FSM editor, including event handling, user interaction, import/export functionality, and integration with the FSM runtime process. This is the core controller of the application window and its behavior.
 * @author Denis Milistenfer <xmilis00@stud.fit.vutbr.cz>
 * @date 11.05.2025
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "EditorMode.h"
#include "GraphicsScene.h"
#include "AutomatModel.h"
#include "TransitionItem.h"

#include "ParserLib.h"

#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include <QDir>
#include <QDebug>
#include <QRandomGenerator>
#include <QTimer>

// Constructor: Initializes the main window, sets up the scene, connects UI signals, and defaults to AddState mode
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), scene(new GraphicsScene(this)) {
  ui->setupUi(this);
  setupScene();
  setupConnections();
  onAddStateClicked(); // Init with state AddState
  // Create a blue frame border around the graphics view manually
  ui->graphicsView->setStyleSheet("border: 1px solid #2196f3;");
  ui->graphicsView->setScene(scene);
  // Initialize actions table
  ui->stateActionsTable->setColumnCount(2);
  ui->stateActionsTable->setHorizontalHeaderLabels({"State", "Action"});
  ui->stateActionsTable->horizontalHeader()->setStretchLastSection(true);
  // Initialize terminal as readonly
  ui->outputTerminal->setReadOnly(true);
  ui->outputTerminal->installEventFilter(this);
}

// Destructor: Cleans up the UI resources
MainWindow::~MainWindow() {
  delete ui;
}

// Help message
void MainWindow::on_actionShowHelp_triggered() {
  QString helpText = R"(
<b>Finite State Machine Editor</b><br><br>

<b>Edit Modes:</b><br>
- <b> + :</b> Add states - Click on the canvas to create a new state.<br>
- <b> -> :</b> Add transitions - Click on two states to create a transition between them.<br>
- <b> Move :</b> Drag states around canvas, double-click on state to rename it,
  double click on transition label to change it or right-click on state to mark it as initial.<br>
- <b> X :</b> Click on state or transition to remove it.<br><br>

<b>Inputs, Outputs, Variables:</b><br>
- Fill them in on left.<br><br>

<b>Actions:</b><br>
- For each state, define actions in the action table below canvas.<br><br>

<b>File->Export:</b> Save FSM to a file.<br>
<b>File->Import:</b> Load FSM from a file.<br><br>
)";

  QMessageBox::information(this, "Help", helpText);
}

// Provides access to the UI object for external components (AutomatModel)
Ui::MainWindow* MainWindow::getUi() const {
  return ui;
}

// Initializes the FSM graphics scene and sets it into the view
void MainWindow::setupScene() {
  ui->graphicsView->setScene(scene);
  scene->setSceneRect(0, 0, 500, 250); // Large canvas
}

// Connects GUI buttons to corresponding functionality
void MainWindow::setupConnections() {
  connect(ui->addStateButton, &QPushButton::clicked, this, &MainWindow::onAddStateClicked);
  connect(ui->addTransitionButton, &QPushButton::clicked, this, &MainWindow::onAddTransitionClicked);
  connect(ui->moveButton, &QPushButton::clicked, this, &MainWindow::onMoveModeClicked);
  connect(ui->deleteButton, &QPushButton::clicked, this, &MainWindow::onDeleteModeClicked);

  connect(ui->addVariableButton, &QPushButton::clicked, this, &MainWindow::onAddVariableClicked);
  connect(ui->removeVariableButton, &QPushButton::clicked, this, &MainWindow::onRemoveVariableClicked);

  connect(ui->addInputButton, &QPushButton::clicked, this, &MainWindow::onAddInputClicked);
  connect(ui->removeInputButton, &QPushButton::clicked, this, &MainWindow::onRemoveInputClicked);

  connect(ui->addOutputButton, &QPushButton::clicked, this, &MainWindow::onAddOutputClicked);
  connect(ui->removeOutputButton, &QPushButton::clicked, this, &MainWindow::onRemoveOutputClicked);

  connect(scene, &GraphicsScene::stateAdded, this, &MainWindow::onStateAddedToTable);
  connect(scene, &GraphicsScene::stateRenamed, this, &MainWindow::onStateRenamedInTable);
  connect(scene, &GraphicsScene::stateDeleted, this, &MainWindow::onStateDeleted);

  connect(ui->actionExport, &QAction::triggered, this, &MainWindow::onExportClicked);
  connect(ui->actionImport, &QAction::triggered, this, &MainWindow::onImportClicked);

  connect(ui->startButton, &QPushButton::clicked, this, &MainWindow::onStartClicked);

}

// Switches scene mode to AddState
void MainWindow::onAddStateClicked() {
  currentMode = EditorMode::AddState;
  scene->setMode(currentMode);
  updateModeUI(currentMode);
}

// Switches scene mode to AddTransition
void MainWindow::onAddTransitionClicked() {
  currentMode = EditorMode::AddTransition;
  scene->setMode(currentMode);
  updateModeUI(currentMode);
}

// Switches scene mode to Move
void MainWindow::onMoveModeClicked() {
  currentMode = EditorMode::Move;
  scene->setMode(currentMode);
  updateModeUI(currentMode);
}

// Switches scene mode to Delete
void MainWindow::onDeleteModeClicked() {
  currentMode = EditorMode::Delete;
  scene->setMode(currentMode);
  updateModeUI(currentMode);
}

// Highlights the currently active mode button by setting its style
void MainWindow::updateModeUI(EditorMode mode) {
  // Clear all styles first
  ui->addStateButton->setStyleSheet("");
  ui->addTransitionButton->setStyleSheet("");
  ui->moveButton->setStyleSheet("");
  ui->deleteButton->setStyleSheet("");

  // Apply blue style to the active one
  switch (mode) {
    case EditorMode::AddState:
      ui->addStateButton->setStyleSheet("background-color: #2196f3");
      break;
    case EditorMode::AddTransition:
      ui->addTransitionButton->setStyleSheet("background-color: #2196f3");
      break;
    case EditorMode::Move:
      ui->moveButton->setStyleSheet("background-color: #2196f3");
      break;
    case EditorMode::Delete:
      ui->deleteButton->setStyleSheet("background-color: #2196f3");
      break;
    default:
      break;
  }
}

// Adds a new variable to the list
void MainWindow::onAddVariableClicked() {
  bool ok;
  QString var = QInputDialog::getText(this, "Add Variable", "Enter variable name:", QLineEdit::Normal, "", &ok);
  if (ok && !var.isEmpty()) {
    ui->automatVariables->addItem(var);
  }
}

// Removes the selected variable from the list
void MainWindow::onRemoveVariableClicked() {
  QListWidgetItem* item = ui->automatVariables->currentItem();
  if (item) {
    delete item;
  }
}

// Adds a new input to the list
void MainWindow::onAddInputClicked() {
  bool ok;
  QString input = QInputDialog::getText(this, "Add Input", "Enter input name:", QLineEdit::Normal, "", &ok);
  if (ok && !input.isEmpty()) {
    ui->automatInputs->addItem(input);
  }
}

// Removes the selected input from the list
void MainWindow::onRemoveInputClicked() {
  QListWidgetItem* item = ui->automatInputs->currentItem();
  if (item) {
    delete item;
  }
}

// Adds a new output to the list
void MainWindow::onAddOutputClicked() {
  bool ok;
  QString output = QInputDialog::getText(this, "Add Output", "Enter output name:", QLineEdit::Normal, "", &ok);
  if (ok && !output.isEmpty()) {
    ui->automatOutputs->addItem(output);
  }
}

// Removes the selected output from the list
void MainWindow::onRemoveOutputClicked() {
  QListWidgetItem* item = ui->automatOutputs->currentItem();
  if (item) {
    delete item;
  }
}

// Handles state addition by appending a new row into the state actions table
// The first column shows the state name (non-editable), second is editable action text
void MainWindow::onStateAddedToTable(const QString& stateName) {
  int row = ui->stateActionsTable->rowCount();
  ui->stateActionsTable->insertRow(row);

  QTableWidgetItem* nameItem = new QTableWidgetItem(stateName);
  nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);  // Non-editable
  ui->stateActionsTable->setItem(row, 0, nameItem);

  QTableWidgetItem* actionItem = new QTableWidgetItem();
  ui->stateActionsTable->setItem(row, 1, actionItem);
}

// Removes the corresponding row from the state actions table
// when a state is deleted from the scene
void MainWindow::onStateDeleted(const QString& stateName) {
  for (int row = 0; row < ui->stateActionsTable->rowCount(); ++row) {
    QTableWidgetItem* item = ui->stateActionsTable->item(row, 0); // 1st column = state name
    if (item && item->text() == stateName) {
      ui->stateActionsTable->removeRow(row);
      break;
    }
  }
}

// Updates the state name in the actions table when a state is renamed in the scene
void MainWindow::onStateRenamedInTable(const QString& oldName, const QString& newName) {
  for (int i = 0; i < ui->stateActionsTable->rowCount(); ++i) {
    if (ui->stateActionsTable->item(i, 0)->text() == oldName) {
      ui->stateActionsTable->item(i, 0)->setText(newName);
      break;
    }
  }
}

// Handles export logic
// Opens file dialog, gathers data from GUI into model, then attempts to export it
// Displays a warning dialog if saving fails
void MainWindow::onExportClicked() {
  // Gather data from GUI
  AutomatModel model;
  model.gatherInfo(this);

  // Export to temporary file to test validity first
  const QString tempPath = QDir::temp().filePath("fsm_export_validation.txt");
  if (!model.exportInfo(tempPath)) {
    QMessageBox::warning(this, "Export Error", "Could not write to temporary file for validation.");
    return;
  }

  // Validate with parser
  ParserLib::Parser parser;
  auto result = parser.parseAutomat(tempPath.toStdString());
  if (result.states.Size() == 0) {
    QMessageBox::critical(this, "Validation Failed", "Cannot export: Your automat is invalid.");
    return;
  }

  // If valid, export to chosen file
  QString fileName = QFileDialog::getSaveFileName(this, "Export Automat", "automat_model", "Text Files (*.txt)");
  if (fileName.isEmpty()) return;

  if (!model.exportInfo(fileName)) {
    QMessageBox::warning(this, "Export Error", "Could not write to the file.");
    return;
  }
}

// Handles import logic
// Opens a file dialog, parses automat model from the selected file using the parser,
// and populates the GUI with the imported model
void MainWindow::onImportClicked() {
  QString fileName = QFileDialog::getOpenFileName(this, "Open FSM File", "", "FSM Files (*.txt)");
  if (fileName.isEmpty()) return;

  // Parse the FSM file
  ParserLib::Parser parser;
  auto result = parser.parseAutomat(fileName.toStdString());


  auto& parsed = result;

  // Name & Comment
  ui->automatName->clear();
  ui->automatName->setText(QString::fromStdString(parsed.Name));
  ui->automatComment->clear();
  ui->automatComment->setPlainText(QString::fromStdString(parsed.Comment));

  // Variables
  ui->automatVariables->clear();
for (const auto& var : parsed.variables) {
    std::string line = var.Type + " " + var.Name + " = " + var.Value;
    ui->automatVariables->addItem(QString::fromStdString(line));
}


  // Inputs
  ui->automatInputs->clear();
  for (const auto& inName : parsed.inputs) {
    ui->automatInputs->addItem(QString::fromStdString(inName));
  }

  // Outputs
  ui->automatOutputs->clear();
  for (const auto& outName : parsed.outputs) {
    ui->automatOutputs->addItem(QString::fromStdString(outName));
  }

  // States
  scene->clearScene();
  ui->graphicsView->scene()->clear();
  std::map<std::string, StateItem*> stateItems;
  int x = 0;
  for (const auto& state : parsed.states) {
    const auto& name = state.Name;
    auto* s = scene->createState(QPointF(x, 0), QString::fromStdString(name));
    stateItems[name] = s;
    x += 150;
  }

  // Transitions
  for (const auto& [id, t] : parsed.transitions) {
    auto from = stateItems[t.from];
    auto to   = stateItems[t.to];
    if (from && to) {
      auto* tr = scene->createTransitionByNames(from, to);
      tr->setLabel(QString::fromStdString(t.input));
    }
  }

  // Highlight the first state
  if (!parsed.states.empty()) {
    auto firstState = parsed.states.First();  // Call your custom method
    auto* first = stateItems[firstState.Name];
    if (first) scene->setInitialState(first);
  }

  // Actions
  ui->stateActionsTable->setRowCount(static_cast<int>(parsed.states.Size()));
  int row = 0;
  for (const auto& state : parsed.states) {
    // Name column, non-editable
    auto* nameItem = new QTableWidgetItem(QString::fromStdString(state.Name));
    nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
    ui->stateActionsTable->setItem(row, 0, nameItem);

    // Action column
    auto* actionItem = new QTableWidgetItem(QString::fromStdString(state.Action));
    ui->stateActionsTable->setItem(row, 1, actionItem);

    ++row;
  }

  model = std::make_shared<AutomatLib::Automat>(std::move(result));
  scene->setMode(EditorMode::Move);
  QMessageBox::information(this, "Import", "FSM imported successfully.");
}

// Starts the FSM runtime process with the currently defined model
// Validates the model, exports it to a temporary file, and runs the backend FSM process
void MainWindow::onStartClicked() {
  // Gather current info from GUI into AutomatModel
  AutomatModel model;
  model.gatherInfo(this);

  // Export model to a temporary file
  const QString tempFilePath = QDir::temp().filePath("fsm_runtime_definition.txt");
  if (!model.exportInfo(tempFilePath)) {
    QMessageBox::critical(this, "Export Error", "Failed to export automat to file.");
    return;
  }

  // Clean up old process if it exists
  if (fsmProcess) {
    fsmProcess->kill();
    fsmProcess->deleteLater();
  }

  // Send to parser for verification
  ParserLib::Parser parser;
  AutomatLib::Automat parsed = parser.parseAutomat(tempFilePath.toStdString());
  appendToTerminal("Validating your automat ...");
  if (parsed.states.Size() == 0) {
    appendToTerminal("BAD");
    QMessageBox::critical(this, "Validation Error", "Your automat is invalid.");
    return;
  }
  appendToTerminal("OK");

  // Start the FSM process with the exported file as argument
  appendToTerminal("Starting FSM process ...");
  fsmProcess = new QProcess(this);
  connect(fsmProcess, &QProcess::readyReadStandardOutput, this, &MainWindow::handleFSMStdout);
  connect(fsmProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
          this, &MainWindow::onFSMFinished);

  QString fsmPath = QCoreApplication::applicationDirPath() + "/../../../Release/fsm.exe";
  fsmPath = QDir::cleanPath(fsmPath);
  fsmProcess->start(fsmPath, QStringList() << tempFilePath);

  if (!fsmProcess->waitForStarted()) {
    appendToTerminal("BAD");
    QMessageBox::critical(this, "Error", "Failed to start FSM process.");
    return;
  }

  appendToTerminal("OK");
}

// Handles new output from FSM runtime
// Parses FSM messages (STATE, INPUT_REQUEST) and updates GUI or logs
void MainWindow::handleFSMStdout() {
  const QString output = fsmProcess->readAllStandardOutput();
  const QStringList lines = output.split('\n', Qt::SkipEmptyParts);
  for (const QString& line : lines) {
    if (line.startsWith("STATE:")) {
      updateCurrentState(line.mid(6).trimmed());
    } else if (line.startsWith("INPUT_REQUEST:")) {
      QString prompt = line.mid(QString("INPUT_REQUEST:").length()).trimmed();
      appendToTerminal("Input requested: " + prompt);

      // Allow user to enter response
      ui->outputTerminal->setReadOnly(false);
      ui->outputTerminal->appendPlainText(">> ");  // show prompt
      waitingForInput = true;  // Set flag
    } else {
      appendToTerminal(line.trimmed());
    }
  }
}

// Called when the FSM runtime process exits
// Logs the exit code and optionally provides feedback
void MainWindow::onFSMFinished(int exitCode, QProcess::ExitStatus status) {
  appendToTerminal(QString("FSM process exited with code %1").arg(exitCode));
}

// Appends a line to the output terminal
// Used to display log messages or outputs from the running FSM
void MainWindow::appendToTerminal(const QString& line) {
  ui->outputTerminal->appendPlainText(line);
}

// Highlights the state in the scene whose name matches stateName
// Called when the FSM sends a "STATE:" message via ZeroMQ
void MainWindow::updateCurrentState(const QString& stateName) {
  // Loop over all items in the scene to find states
  for (QGraphicsItem* item : scene->items()) {
    if (auto* state = qgraphicsitem_cast<StateItem*>(item)) {
      bool isCurrent = (state->getName() == stateName);
      state->setHighlighted(isCurrent);
    }
  }
}

// Sends a line of user input to the running FSM via stdin
// Triggered after input is entered in the terminal
void MainWindow::sendInputToFSM(const QString& input) {
  if (fsmProcess && fsmProcess->state() == QProcess::Running) {
    fsmProcess->write(QString("INPUT: %1\n").arg(input).toUtf8()); // Send user input
    appendToTerminal("Sent input: " + input);
  }
}

// Handles ENTER key during user input
// Captures typed input and sends it to the FSM
void MainWindow::keyPressEvent(QKeyEvent* event) {
  if (waitingForInput && event->key() == Qt::Key_Return) {
    QString allText = ui->outputTerminal->toPlainText();
    QString lastLine = allText.section('\n', -1).trimmed();

    if (lastLine.startsWith(">>")) {
      QString userInput = lastLine.mid(2).trimmed(); // Skip >>
      sendInputToFSM(userInput);
      ui->outputTerminal->setReadOnly(true);
      waitingForInput = false;
    }
  }
}

// Captures key presses inside output terminal
// Prevents new lines from being added and routes input to FSM
bool MainWindow::eventFilter(QObject* obj, QEvent* event) {
  if (obj == ui->outputTerminal && event->type() == QEvent::KeyPress) {
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
    if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
      QString text = ui->outputTerminal->toPlainText().trimmed();

      // Extract only the last line if terminal has logs + input
      QStringList lines = text.split("\n");
      QString lastLine = lines.last();

      // Send to FSM
      if (fsmProcess) {
        fsmProcess->write((lastLine + "\n").toUtf8());
      }

      // Make terminal read-only again
      ui->outputTerminal->setReadOnly(true);

      return true;
    }
  }
  return QMainWindow::eventFilter(obj, event);
}
