#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "EditorMode.h"
#include "GraphicsScene.h"
#include "AutomatModel.h"

//#include "ParserLib.h"
//#include "AutomatLib.h"
//#include "messages/p2p.h"

#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include <QDir>
#include <QDebug>

// Constructor: Initializes the main window, sets up the scene, connects UI signals, and defaults to AddState mode
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), scene(new GraphicsScene(this)) {
  ui->setupUi(this);
  setupScene();
  setupConnections();
  onAddStateClicked(); // Init with state AddState
  // Create a blue frame border around the graphics view manually
  ui->graphicsView->setStyleSheet("border: 1px solid #2196f3;");
  // Initialize actions table
  ui->stateActionsTable->setColumnCount(2);
  ui->stateActionsTable->setHorizontalHeaderLabels({"State", "Action"});
  ui->stateActionsTable->horizontalHeader()->setStretchLastSection(true);
}

// Destructor: Cleans up the UI resources
MainWindow::~MainWindow() {
  delete ui;
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

  connect(ui->addVariableButton, &QPushButton::clicked, this, &MainWindow::onAddVariableClicked);
  connect(ui->removeVariableButton, &QPushButton::clicked, this, &MainWindow::onRemoveVariableClicked);

  connect(ui->addInputButton, &QPushButton::clicked, this, &MainWindow::onAddInputClicked);
  connect(ui->removeInputButton, &QPushButton::clicked, this, &MainWindow::onRemoveInputClicked);

  connect(ui->addOutputButton, &QPushButton::clicked, this, &MainWindow::onAddOutputClicked);
  connect(ui->removeOutputButton, &QPushButton::clicked, this, &MainWindow::onRemoveOutputClicked);

  connect(scene, &GraphicsScene::stateAdded, this, &MainWindow::onStateAddedToTable);
  connect(scene, &GraphicsScene::stateRenamed, this, &MainWindow::onStateRenamedInTable);

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

// Highlights the currently active mode button by setting its style
void MainWindow::updateModeUI(EditorMode mode) {
  // Clear all styles first
  ui->addStateButton->setStyleSheet("");
  ui->addTransitionButton->setStyleSheet("");
  ui->moveButton->setStyleSheet("");

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
  QString fileName = QFileDialog::getSaveFileName(this, "Export Automat", "automat_model", "Text Files (*.txt)");
  if (fileName.isEmpty()) return;

  AutomatModel model;
  model.gatherInfo(this);
  if (!model.exportInfo(fileName)) {
    QMessageBox::warning(this, "Export Error", "Could not write to the file.");
  }
}

// Handles import logic
// Opens a file dialog, parses automat model from the selected file using the parser,
// and populates the GUI with the imported model
void MainWindow::onImportClicked() {
  QString filePath = QFileDialog::getOpenFileName(this, "Import FSM", "", "FSM files (*.txt);;All files (*.*)");
  if (filePath.isEmpty()) return;

  // TODO: Use Parser::parseAutomat() to parse
}

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

  // Start the FSM process with the exported file as argument
  QProcess* fsmProcess = new QProcess(this);
  fsmProcess->start("fsmExecutable", QStringList() << tempFilePath);
  if (!fsmProcess->waitForStarted()) {
    QMessageBox::critical(this, "Error", "Failed to start FSM process.");
    return;
  }

  /*
  // TODO:
  // Set up ZeroMQ communication from GUI side (connect)

  static zmq::context_t context;
  communicator = std::make_unique<PairCommunicator>(
      &context,
      "ipc://pair.ipc",  // endpoint
      false,             // GUI is the client (connects)
      [this](std::string_view msg) {
        QString message = QString::fromStdString(std::string(msg));
        qDebug() << "[FSM MSG]" << message;

        if (message.startsWith("STATE:")) {
          QString state = message.mid(6).trimmed();
          updateCurrentStateInGUI(state);
        } else if (message.startsWith("OUTPUT:")) {
          QString output = message.mid(7).trimmed();
          appendToOutputLog(output);
        } else if (message.startsWith("LOG:")) {
          QString logEntry = message.mid(4).trimmed();
          appendToLogView(logEntry);
        } else {
          qWarning() << "[Unknown FSM message]" << message;
        }
      });
  */
}
