#ifndef AUTOMATMODEL_H
#define AUTOMATMODEL_H

#include <QString>
#include <QStringList>
#include <QMap>
#include <QList>

// Represents a transition between two states
struct TransitionRecord {
  QString from, to, label;
};

class MainWindow;  // Forward declaration

// Holds the internal representation of the automat (name, states, transitions, ...)
// Provides methods to gather data from the GUI and export it to a text file
class AutomatModel {
public:
  void gatherInfo(MainWindow* window); // Populates the model from GUI fields (called before exporting)
  bool exportInfo(const QString& filename) const; // Exports the current model to a text file in parser compatible format
  bool importInfo(const QString& filename) const; // TODO

private:
  QString name;                         // Name of the automat
  QString comment;                      // Automat comment
  QStringList inputs;                   // List of inputs
  QStringList outputs;                  // List of outputs
  QStringList variableLines;            // Raw lines representing variables ("int x = 1")
  QMap<QString, QString> stateActions;  // State name, state action
  QString initialStateName;             // Name of the initial state
  QList<TransitionRecord> transitions;  // All transition records
};

#endif // AUTOMATMODEL_H
