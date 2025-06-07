/**
 * @file MainWindow.h
 * @brief MainWindow is the controller class for the GUI. It manages the scene, handles user interaction, and connects UI controls.
 * @author Denis Milistenfer <xmilis00@stud.fit.vutbr.cz>
 * @date 11.05.2025
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include "GraphicsScene.h"
#include "EditorMode.h"
#include "AutomatLib.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

/**
 * @class MainWindow
 * @brief Main controller for the FSM editor GUI.
 *
 * Responsible for setting up the interface, responding to user actions, managing FSM state, and handling file I/O.
 */
class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  /**
   * @brief Constructs the main window and sets up the UI.
   * @param parent Optional parent widget.
   */
  explicit MainWindow(QWidget *parent = nullptr);
  /**
   * @brief Destructor.
   */
  ~MainWindow();

  /**
   * @brief Provides access to the UI definition (for AutoModel usage).
   * @return Pointer to the MainWindow UI.
   */
  Ui::MainWindow* getUi() const;

  /**
   * @brief Provides access to the FSM graphics scene.
   * @return Pointer to the GraphicsScene instance.
   */
  GraphicsScene* getScene() const { return scene; }

private slots:
  /// Displays help information dialog.
  void on_actionShowHelp_triggered();

  /// Handles the AddState mode activation.
  void onAddStateClicked();

  /// Handles the AddTransition mode activation.
  void onAddTransitionClicked();

  /// Handles the Move mode activation.
  void onMoveModeClicked();

  /// Handles the Delete mode activation.
  void onDeleteModeClicked();

  /// Adds a new variable to the list.
  void onAddVariableClicked();

  /// Removes the selected variable from the list.
  void onRemoveVariableClicked();

  /// Adds a new input to the list.
  void onAddInputClicked();

  /// Removes the selected input from the list.
  void onRemoveInputClicked();

  /// Adds a new output to the list.
  void onAddOutputClicked();

  /// Removes the selected output from the list.
  void onRemoveOutputClicked();

  /**
   * @brief Updates action table when a state is added.
   * @param stateName Name of the newly added state.
   */
  void onStateAddedToTable(const QString& stateName);

  /**
   * @brief Updates action table when a state is renamed.
   * @param oldName Old state name.
   * @param newName New state name.
   */
  void onStateRenamedInTable(const QString& oldName, const QString& newName);

  /**
   * @brief Updates action table when a state is deleted.
   * @param stateName Name of the deleted state.
   */
  void onStateDeleted(const QString& stateName);

  /// Handles export operation and file saving.
  void onExportClicked();

  /// Handles import operation and populates GUI with FSM data.
  void onImportClicked();

  /// Starts the FSM process with the current model.
  void onStartClicked();

  /// Stops the FSM process.
  void onStopClicked();

private:
  Ui::MainWindow* ui;              ///< UI definition generated from Qt Designer
  GraphicsScene* scene;           ///< Scene handling the FSM editor canvas
  EditorMode currentMode = EditorMode::Move; ///< Default editor mode

  QProcess* fsmProcess = nullptr; ///< Holds the running FSM process
  bool waitingForInput = false;   ///< True if FSM is waiting for user input via terminal
  std::shared_ptr<AutomatLib::Automat> model; ///< Stores currently loaded automat model from parser

  /**
   * @brief Connects GUI buttons to their respective slots.
   */
  void setupConnections();

  /**
   * @brief Initializes and configures the graphics scene.
   */
  void setupScene();

  /**
   * @brief Visually highlights the currently active editor mode in the UI.
   * @param mode The active editor mode.
   */
  void updateModeUI(EditorMode mode);

  /**
   * @brief Highlights the currently active FSM state in the scene.
   * @param stateName Name of the state to highlight.
   */
  void updateCurrentState(const QString& stateName);

  /**
   * @brief Appends a line to the output terminal.
   * @param line Text to be appended.
   */
  void appendToTerminal(const QString& line);

  /**
   * @brief Sends user input string to the FSM process via stdin.
   * @param input The input to be sent.
   */
  void sendInputToFSM(const QString& input);

  /**
   * @brief Handles new output lines received from FSM stdout.
   */
  void handleFSMStdout();

  /**
   * @brief Handles error outputs received from FSM stderr.
   */
  void handleFSMStderr();
  
  /**
   * @brief Called when the FSM process terminates.
   * @param exitCode Process exit code.
   * @param status Process exit status.
   */
  void onFSMFinished(int exitCode, QProcess::ExitStatus status);

  /**
   * @brief Event filter used to capture ENTER presses inside the terminal.
   * @param obj The object being monitored.
   * @param event The event to filter.
   * @return True if the event was handled.
   */
  bool eventFilter(QObject* obj, QEvent* event) override;

protected:
  /**
   * @brief Intercepts and handles key presses in the main window.
   * @param event The key press event.
   */
  void keyPressEvent(QKeyEvent* event) override;
};

#endif // MAINWINDOW_H
