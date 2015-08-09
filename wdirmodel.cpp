#include "wdirmodel.h"
#include <QDebug>

void WDirModel::parents(QString path)
{
    if(path == pathRootDir) return;
    QFileInfo info(path);
    if(info.absolutePath() != pathRootDir && dirs.contains(info.absolutePath()))
    {
        if(states[path] == Qt::Unchecked)
        {
                if(states[info.absolutePath()] != 4)
                {
                    QDir parentDir(info.absolutePath());
                    if(parentDir.count() == 1)
                    {
                        states[info.absolutePath()] = Qt::Unchecked;
                    }
                    else
                    {
                        bool checked = false;
                        for(unsigned int i = 0; i < parentDir.count(); i++)
                        {
                            QString a = info.absolutePath() + "/" + parentDir[i];
                            if(states.contains(a))
                            {
                                if(states[a] == Qt::Checked)
                                {
                                    checked = true;
                                    break;
                                }
                            }
                        }
                        if(checked)
                        {
                           states[info.absolutePath()] = Qt::PartiallyChecked;
                        }
                        else
                        {
                           states[info.absolutePath()] = Qt::Unchecked;
                        }
                    }
                    parents(info.absolutePath());
                }
        }
        else if(states[path] == Qt::Checked || states[path] == Qt::PartiallyChecked)
        {
            QDir parentDir(info.absolutePath());
            parentDir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
            if(states[info.absolutePath()] == Qt::PartiallyChecked)
            {
                bool all = true;
                for(unsigned int i = 0 ; i < parentDir.count(); i++)
                {
                    QString a = info.absolutePath() + "/" + parentDir[i];
                    if(states[a] == Qt::Unchecked)
                    {
                        all = false;
                        break;
                    }
                }
                if(all)
                    states[info.absolutePath()] = Qt::Checked;
            }
            else if(states[info.absolutePath()] == Qt::Unchecked)
            {
                if(parentDir.count() == 1)
                {
                    states[info.absolutePath()] = Qt::Checked;
                }
                else
                {
                    states[info.absolutePath()] = Qt::PartiallyChecked;
                }
            }
            parents(info.absolutePath());
        }
   }
}

void WDirModel::childrens(QString path)
{
        QDir root(path);
        root.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
        if(states[path] == Qt::Checked)
        {
            for(unsigned int i = 0; i < root.count();i++)
            {
                QString a = path + "/" + root[i];
                if(states[a] == Qt::Checked) continue;
                else if(states[a] == Qt::Unchecked && files.contains(a))
                {
                    states[a] = Qt::Checked;
                }
                else if ((states[a] == Qt::Unchecked) && dirs.contains(a))
                {
                    states[a] = Qt::Checked;
                    childrens(a);
                }
                else if(states[a] == 4 && dirs.contains(a))
                {
                    childrens(a);
                }
            }
        }
        else if(states[path] == Qt::Unchecked)
        {
            for(unsigned int i = 0; i < root.count();i++)
            {
                QString a = path + "/" + root[i];
                if(states[a] == Qt::Unchecked) continue;
                else if(states[a] == Qt::Checked && files.contains(a))
                {
                    states[a] = Qt::Unchecked;
                }
                else if ((states[a] == Qt::Checked || states[a] == Qt::PartiallyChecked) && dirs.contains(a))
                {
                    states[a] = Qt::Unchecked;
                    childrens(a);
                }
                else if(states[a] == 4 && dirs.contains(a))
                {
                    childrens(a);
                }
            }
        }
        else if(states[path] == 4)
        {
            QFileInfo info(path);
            if(states[info.absolutePath()] == Qt::Checked)
            {
                for(unsigned int i = 0; i < root.count(); i++)
                {
                    QString a = path + "/" + root[i];
                    if(states[a] == Qt::Checked) continue;
                    else if(states[a] == Qt::Unchecked && files.contains(a))
                    {
                        states[a] = Qt::Checked;
                    }
                    else if ((states[a] == Qt::Unchecked) && dirs.contains(a))
                    {
                        states[a] = Qt::Checked;
                        childrens(a);
                    }
                    else if(states[a] == 4 && dirs.contains(a))
                    {
                        childrens(a);
                    }
                }
            }
            else if(states[info.absolutePath()] == Qt::Unchecked)
            {
                for(unsigned int i = 0; i < root.count();i++)
                {
                    QString a = path + "/" + root[i];

                    if(states[a] == Qt::Unchecked) continue;
                    else if(states[a] == Qt::Checked && files.contains(a))
                    {
                        states[a] = Qt::Unchecked;
                    }
                    else if ((states[a] == Qt::Checked || states[a] == Qt::PartiallyChecked) && dirs.contains(a))
                    {
                        states[a] = Qt::Unchecked;
                        childrens(a);
                    }
                    else if(states[a] == 4 && dirs.contains(a))
                    {
                        childrens(a);
                    }
                }
            }
        }
}

QVariant WDirModel::data(const QModelIndex &index, int role) const
{
        if(index.isValid() && index.column() == 0 && role == Qt::CheckStateRole)
        {
            if(states.contains(filePath(index)))
            {
                switch(states[filePath(index)])
                {
                   case Qt::Checked:
                   {
                        return Qt::Checked;
                        break;
                   }
                   case Qt::Unchecked:
                   {
                        return Qt::Unchecked;
                        break;
                   }
                   case Qt::PartiallyChecked:
                   {
                        return Qt::PartiallyChecked;
                        break;
                   }
                }
            }
        }
        return QFileSystemModel::data(index, role);
}

bool WDirModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
        if(index.isValid() && index.column() == 0 && role == Qt::CheckStateRole)
        {
            QString path(filePath(index));
            QFileInfo info(path);
            if(states.contains(path))
            {
                switch(states[path])
                {
                   case Qt::Checked: // To Uncheck
                   {
                     states[path] = Qt::Unchecked;
                     if(info.isDir())
                     {
                         childrens(path);
                         if(path != pathRootDir)
                             parents(path);
                     }
                     else if(info.isFile())
                     {
                         if(info.absolutePath() != path)
                             parents(path);
                     }
                     break;
                   }

                   case Qt::Unchecked: // To Check
                   {
                     states[path] = Qt::Checked;
                     if(info.isDir())
                     {
                         childrens(path);
                         if(path != pathRootDir)
                             parents(path);
                     }
                     else if(info.isFile())
                     {
                         if(info.absolutePath() != path)
                             parents(path);
                     }
                     break;
                   }

                   case Qt::PartiallyChecked: // To Uncheck, Only dirs
                   {
                     states[path] = Qt::Unchecked;
                     childrens(path);
                     if(info.absolutePath() != path)
                        parents(path);
                     break;
                   }
                }
            }
            emit layoutChanged();
            return true;
        }
        return QFileSystemModel::setData(index, value, role);
}

Qt::ItemFlags WDirModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags f = QFileSystemModel::flags(index);
    if (index.column() == 0)
    {
        f |= Qt::ItemIsUserCheckable;
        f |= Qt::ItemIsTristate;
    }
    return f;
}
