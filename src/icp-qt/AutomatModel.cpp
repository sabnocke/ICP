/**
 * @file AutomatModel.cpp
 * @brief Implements the logic for collecting FSM data from the GUI and exporting it into a parser-compatible format.
 * @author Denis Milistenfer <xmilis00@stud.fit.vutbr.cz>
 * @date 11.05.2025
 */

#include "AutomatModel.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "GraphicsScene.h"
#include "StateItem.h"
#include "TransitionItem.h"

#include <QFile>
#include <QTextStream>
#include <QTableWidget>

// Gathers information from the UI and scene into the internal AutomatModel structure
void AutomatModel::gatherInfo(MainWindow* window) {
  auto* ui = window->getUi();
  auto* scene = window->getScene();

  // Get name and comment
  name = ui->automatName->text();
  comment = ui->automatComment->toPlainText();

  // Collect inputs
  inputs.clear();
  for (int i = 0; i < ui->automatInputs->count(); ++i) {
    inputs.append(ui->automatInputs->item(i)->text());
  }

  // Collect outputs
  outputs.clear();
  for (int i = 0; i < ui->automatOutputs->count(); ++i) {
    outputs.append(ui->automatOutputs->item(i)->text());
  }

  // Collect variables
  variableLines.clear();
  for (int i = 0; i < ui->automatVariables->count(); ++i) {
    QString line = ui->automatVariables->item(i)->text();
    variableLines.append(line);
  }

  // Collect state actions from the table
  stateActions.clear();
  QTableWidget* table = ui->stateActionsTable;
  for (int i = 0; i < table->rowCount(); ++i) {
    QString stateName = table->item(i, 0)->text();
    QString action = table->item(i, 1)->text();
    stateActions[stateName] = action;
  }

  // Determine the initial state (if any)
  if (auto* init = scene->getInitialState())
    initialStateName = init->getName();
  else
    initialStateName.clear();

  // Collect all transitions from the scene
  transitions.clear();
  for (auto* item : scene->items()) {
    auto* tt = dynamic_cast<TransitionItem*>(item);
    if (!tt) continue; // Skip items that are not transitions

    TransitionRecord rec;
    rec.from  = tt->getFromState()->getName();
    rec.to    = tt->getToState()->getName();
    rec.label = tt->getLabel();

    transitions.append(rec);
  }
}

// Exports the contents of the AutomatModel into a file in human-readable format
bool AutomatModel::exportInfo(const QString& filename) const {
  QFile file(filename);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    return false;
  QTextStream out(&file);

  // Name
  out << "Name: " << name << "\n";

  // Comment
  out << "Comment: " << comment << "\n\n";

  // Inputs
  if (!inputs.empty()) {
    out << "Input: ";
    int count = 0;
    for (const auto& input : inputs) {
      if (count != 0){
        out << ", ";
      }
      out << input;
      count++;
    }
    out << "\n";
  }

  // Outputs
  if (!outputs.empty()){
    out << "Output: ";
    int count = 0;
    for (const auto& output : outputs) {
      if (count != 0){
        out << ", ";
      }
      out << output;
      count++;
    }
    out << "\n";
    out << "\n";
  }

  // Variables
  if (!variableLines.empty()){
    out << "Variables:\n";
    for (const QString& varLine : variableLines) {
      out << varLine << "\n";
    }
    out << "\n";
  }

  // States and actions
  out << "States:\n";
  QStringList names = stateActions.keys();
  // Ensure the initial state is listed first (if exists)
  if (!initialStateName.isEmpty() && names.removeOne(initialStateName))
    names.prepend(initialStateName);

  for (const QString& s : names) {
    out << "state " << s << " [" << stateActions.value(s).trimmed() << "]\n";
  }
  out << "\n";

  // Transitions
  if (!transitions.empty()){
    out << "Transitions:\n";
    for (auto& tr : transitions) {
      out << tr.from << " --> " << tr.to << " : " << tr.label << "\n";
    }
  }

  return true;
}
