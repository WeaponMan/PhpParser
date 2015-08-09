#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QDir>

bool MainWindow::rmDir(QString dirPath)
{
    QDir dir(dirPath);
    if (!dir.exists())
        return true;
    foreach(const QFileInfo &info, dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot))
    {
        if (info.isDir())
        {
            if (!rmDir(info.filePath()))
                return false;
        }
        else
        {
            if (!dir.remove(info.fileName()))
                return false;
        }
    }
    QDir parentDir(QFileInfo(dirPath).path());
    return parentDir.rmdir(QFileInfo(dirPath).fileName());
}

bool MainWindow::cpDir(QString srcPath,QString dstPath)
{
    rmDir(dstPath);
    QDir parentDstDir(QFileInfo(dstPath).path());
    if (!parentDstDir.mkdir(QFileInfo(dstPath).fileName()))
        return false;

    QDir srcDir(srcPath);
    foreach(const QFileInfo &info, srcDir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot))
    {
        QString srcItemPath = srcPath + "/" + info.fileName();
        QString dstItemPath = dstPath + "/" + info.fileName();
        if (info.isDir())
        {
            if (!cpDir(srcItemPath, dstItemPath))
            {
                return false;
            }
        }
        else if (info.isFile())
        {
            if (!QFile::copy(srcItemPath, dstItemPath))
            {
                return false;
            }
        }
    }
    return true;
}


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->otevritSlozku,SIGNAL(clicked()),this,SLOT(openDialogDir()));
    model = new WDirModel();
    model->setReadOnly(false);
    connect(this,SIGNAL(readReady()),this,SLOT(readDir()));
    connect(ui->startParse,SIGNAL(clicked()),this,SLOT(beginParse()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::openDialogDir()
{
    ui->startParse->setDisabled(true);
    QString dirname_user = QFileDialog::getExistingDirectory(this,"Vyber koøenovou složku projektu",QDir::homePath());
    if(!dirname_user.isNull())
    {
       #ifdef Q_WS_WIN
        dirname = dirname_user.replace(QRegExp("\\\\"),"/");
       #else
        dirname = dirname_user;
       #endif
        model->setPathRootDir(dirname);
        emit readReady();
    }
}

void MainWindow::readDirs(QString r_dirname)
{
    QDir dir(r_dirname);
    bool contains = false;
    QRegExp exp(".+\\.(?:php|js|css|tpl|html)$");
    dir.setFilter(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs);
    for(unsigned int i = 0; i < dir.count() ; i++)
    {
        QString dir_name = r_dirname + "/" + dir[i];
        QFileInfo test(dir_name);
        if(test.isDir())
        {
#ifdef Q_WS_WIN
            model->appendDir(dir_name.replace(QRegExp("\\\\"),"/"));
#else
            model->appendDir(dir_name);
#endif
            readDirs(dir_name);
        }
        if(test.isFile())
        {
            if(!contains && exp.exactMatch(dir_name)) contains = true;
#ifdef Q_WS_WIN
            model->appendFile(dir_name.replace(QRegExp("\\\\"),"/"));
#else
            model->appendFile(dir_name);
#endif
        }
    }
#ifdef Q_WS_WIN
    if(!contains && model->dirContain(r_dirname.replace(QRegExp("\\\\"),"/")))
    {
        model->setState(r_dirname.replace(QRegExp("\\\\"),"/"),4);
    }
#else
    if(!contains && model->dirContain(r_dirname))
    {
        model->setState(r_dirname,4);
    }
#endif
}

void MainWindow::readDir()
{
    readDirs(dirname);
    model->setRootPath(dirname);
    ui->dirTree->setModel(model);
    ui->dirTree->setRootIndex(model->index(dirname));
    ui->startParse->setDisabled(false);
}

void MainWindow::beginParse()
{
     QString ParseDir(dirname + ".min");
     QRegExp css(".+\\.css$");
     QRegExp js(".+\\.js$");
     QRegExp html(".+\\.(?:tpl|html)$");
     QRegExp php(".+\\.php$");
     cpDir(this->dirname,ParseDir);
     QMapIterator<QString, int> i(model->getStates());
     while(i.hasNext())
     {
         i.next();
         if(i.value() == 4 || i.value() == Qt::PartiallyChecked || i.value() == Qt::Unchecked || model->dirContain(i.key())) continue;
         if(i.value() == Qt::Checked && !model->dirContain(i.key()))
         {
             QFile file(ParseDir + i.key().right(i.key().length() - dirname.length()));
             if(file.open(QIODevice::ReadOnly | QIODevice::Text))
             {
                 QTextStream in(&file);
                 QString fileContent = in.readAll();
                 qDebug() << fileContent;
                 qDebug() << "----------------------------------------------------";
                 qDebug() << file.fileName();
                 if(css.exactMatch(file.fileName()))
                 {
                     parseCss(&fileContent);
                 }
                 else if(js.exactMatch(file.fileName()))
                 {
                    parseJS(&fileContent);
                 }
                 else if(html.exactMatch(file.fileName()))
                 {
                    parseHTML(&fileContent);
                 }
                 else if(php.exactMatch(file.fileName()))
                 {
                    parsePHP(&fileContent);
                    "<\\?[php]*([^\\?>]*)\\?>"; // matching php code

                 }
                 qDebug() << "----------------------------------------------------";
                 file.close();
                 if(file.open(QIODevice::WriteOnly | QIODevice::Truncate))
                 {
                     QTextStream out(&file);
                     out << fileContent;
                     file.close();
                 }
             }
         }
     }
}

void MainWindow::parseCss(QString *fileContent)
{
    *fileContent = fileContent->replace(QRegExp("(\n|(\r\n)|\t)"),"");
    *fileContent = fileContent->replace(QRegExp("/\*.*?\*/"),"");
    *fileContent = fileContent->replace(";}","}");
    *fileContent = fileContent->replace(QRegExp("\\{\\s{1,}"),"{");
    *fileContent = fileContent->replace(QRegExp(";\\s{1,}"),";");
    *fileContent = fileContent->replace(QRegExp("\\}\\s{1,}"),"}");
}

void MainWindow::parseHTML(QString *fileContent)
{
    *fileContent = fileContent->replace(QRegExp("(?:(>)\\s+(<))|(?:\\s{2,})|\n"),"\\1\\2");
    *fileContent = fileContent->replace(QRegExp("<!--(.*?)-->"),"");

}

void MainWindow::parseJS(QString *fileContent)
{
    qPrintable(*fileContent);
    qDebug() << "JavaScript";
    *fileContent = fileContent->replace(QRegExp("(/\\*([^*]|[\\r\\n]|(\\*+([^*/]|[\\r\\n])))*\\*+/)"),"");
    qDebug() << *fileContent;
    *fileContent = fileContent->replace(QRegExp("//[^\\r\\n]*"),"");
    qDebug() << *fileContent;
    //*fileContent = fileContent->replace(QRegExp("(\n|(\r\n)|\t)"),"");
    //*fileContent = fileContent->replace(QRegExp("/\\*(.*?)\\*/"),"");
    //*fileContent = fileContent->replace(QRegExp("\\{\\s{1,}"),"{");
    //*fileContent = fileContent->replace(QRegExp("\\}\\s{1,}"),"}");
    //*fileContent = fileContent->replace(QRegExp(";\\s{1,}"),";");
}

void MainWindow::parsePHP(QString *fileContent)
{
    if(fileContent->indexOf("<?php") == -1)
    {
        return parseHTML(fileContent);
    }
   // *fileContent = fileContent->replace(QRegExp("\\/\\/.*(?=[\n\r])"),"");
    *fileContent = fileContent->replace(QRegExp("(\n|(\r\n)|\t)"),"");
    *fileContent = fileContent->replace(QRegExp("/\*.*?\*/"),"");
    *fileContent = fileContent->replace("<?php","<?php ");
}
