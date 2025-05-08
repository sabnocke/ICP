#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "EditorMode.h"
#include "GraphicsScene.h"
#include <QGraphicsDropShadowEffect>
#include <QFrame>
#include <QInputDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), scene(new GraphicsScene(this)) {
  ui->setupUi(this);
  setupScene();
  setupConnections();
  onAddStateClicked(); // Init with state AddState
  // Create a blue frame border around the graphics view manually
  ui->graphicsView->setStyleSheet("border: 1px solid #2196f3;");
}

MainWindow::~MainWindow() {
  delete ui;
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
