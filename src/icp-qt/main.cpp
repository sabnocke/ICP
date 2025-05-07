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
