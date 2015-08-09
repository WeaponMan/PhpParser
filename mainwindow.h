#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include "wdirmodel.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
public slots:
    void openDialogDir();
    void readDir();
    void beginParse();
signals:
    void readReady();
private:
    bool cpDir(QString srcPath, QString dstPath);
    bool rmDir(QString dirPath);
    void readDirs(QString r_dirname);
    void parseCss(QString *fileContent);
    void parseHTML(QString *fileContent);
    void parseJS(QString *fileContent);
    void parsePHP(QString *fileContent);
    QString dirname;
    Ui::MainWindow *ui;
    WDirModel *model;
};

#endif // MAINWINDOW_H
