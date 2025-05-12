/**
 * @file AutomatModel.h
 * @brief Holds the internal representation of the automat (name, states, transitions, ...)
 * @author Denis Milistenfer <xmilis00@stud.fit.vutbr.cz>
 * @date 11.05.2025
 */

#ifndef AUTOMATMODEL_H
#define AUTOMATMODEL_H

#include <QString>
#include <QStringList>
#include <QMap>
#include <QList>

/// Represents a transition between two states
struct TransitionRecord {
  QString from;  ///< Source state name
  QString to;    ///< Destination state name
  QString label; ///< Label or condition on the transition
};

class MainWindow;  // Forward declaration

/**
 * @class AutomatModel
 * @brief Holds the internal representation of the automat (name, states, transitions, ...)
 *
 * Provides methods to gather data from the GUI and export it to a text file.
 */
class AutomatModel {
public:
/**
   * @brief Populates the model from GUI fields.
   * @param window Pointer to the MainWindow containing user input.
   */
  void gatherInfo(MainWindow* window);
  /**
   * @brief Exports the current model to a text file in parser-compatible format.
   * @param filename Destination path to save the file.
   * @return True if export succeeds, false otherwise.
   */
  bool exportInfo(const QString& filename) const;
  /**
   * @brief Imports model data from a text file.
   * @param filename Path to the file to import.
   * @return True if import is successful, false otherwise.
   */
  bool importInfo(const QString& filename) const;

private:
  QString name;                         ///< Name of the automat
  QString comment;                      ///< Automat comment
  QStringList inputs;                   ///< List of inputs
  QStringList outputs;                  ///< List of outputs
  QStringList variableLines;            ///< Raw lines representing variables (e.g., "int x = 1")
  QMap<QString, QString> stateActions;  ///< Map of state name to its action
  QString initialStateName;             ///< Name of the initial state
  QList<TransitionRecord> transitions;  ///< List of all transition records
};

#endif // AUTOMATMODEL_H
