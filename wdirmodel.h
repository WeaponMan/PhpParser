#ifndef WDIRMODEL_H
#define WDIRMODEL_H

#include <QFileSystemModel>
#include <QSet>
#include <QVariant>
#include <QModelIndex>

class WDirModel : public QFileSystemModel
{
    Q_OBJECT
public:
    QVariant data(const QModelIndex& index, int role) const;
    bool setData(const QModelIndex& index, const QVariant& value, int role);
    Qt::ItemFlags flags(const QModelIndex& index) const;
    void appendFile(QString a)
    {
        files.insert(a);
        QRegExp ext(".+\\.(?:php|js|css|tpl|html)$");
        if(ext.exactMatch(a))
         states.insert(a, Qt::Unchecked);
        else
         states.insert(a, 4);
    }
    void appendDir(QString a){ dirs.insert(a); states.insert(a, Qt::Unchecked); }
    bool dirContain(QString key){ return dirs.contains(key); }
    void setState(QString key, int value) { states[key] = value; }
    QMap<QString,int> getStates(){ return states; }
    void setPathRootDir(QString path) { pathRootDir = path; }
    void parents(QString path);
    void childrens(QString path);
private:
    QString pathRootDir;
    QSet<QString> dirs, files;
    QMap<QString,int> states;
};

#endif // WDIRMODEL_H
