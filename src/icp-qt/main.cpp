/**
 * @file main.cpp
 * @brief Entry point of the FSM editor application. Initializes the Qt application and displays the main window interface for user interaction with the FSM canvas and controls.
 * @author Denis Milistenfer <xmilis00@stud.fit.vutbr.cz>
 * @date 11.05.2025
 */

#include "mainwindow.h"
#include <QApplication>

// Main function initializes the Qt application and shows the main window.
int main(int argc, char *argv[])
{
  QApplication app(argc, argv); // Initialize Qt app
  MainWindow window;             // Create the main application window
  window.show();                 // Show it
  return app.exec();            // Enter Qt event loop
}
