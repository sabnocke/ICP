#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "GraphicsScene.h"
#include "EditorMode.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// MainWindow is the controller class for the GUI
// It manages the scene, handles user interaction, and connects UI controls
class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

private slots:
  void onAddStateClicked();       // For handling AddState button
  void onAddTransitionClicked();  // For handling AddTransition button
  void onMoveModeClicked();       // For handling Move button

  void onAddVariableClicked();    // For handling AddVariable button
  void onRemoveVariableClicked(); // For handling RemoveVariable button

  void onAddInputClicked();       // For handling AddInput button
  void onRemoveInputClicked();    // For handling RemoveInput button

  void onAddOutputClicked();      // For handling AddOutput button
  void onRemoveOutputClicked();   // For handling RemoveOutput button

private:
  Ui::MainWindow *ui;             // UI definition from Designer
  GraphicsScene* scene;           // Scene handling the FSM editor canvas

  void setupConnections();       // Connect buttons to slots
  void setupScene();             // Set up the scene and view
  void updateModeUI(EditorMode); // Visually indicate the current mode

  EditorMode currentMode = EditorMode::Move; // Default to Move mode
};

#endif // MAINWINDOW_H
