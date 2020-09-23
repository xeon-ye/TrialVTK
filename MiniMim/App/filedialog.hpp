#pragma once

#include <QFileDialog>
#include <QWidget>
#include <QTreeView>
#include <QListView>
#include <QPushButton>
#include <QStringList>
#include <QModelIndex>
#include <QDir>
#include <QEvent>
#include <QDebug>

class FileDialog : public QFileDialog {
  Q_OBJECT
 public:
  explicit FileDialog(QWidget *parent = Q_NULLPTR);
  bool eventFilter(QObject* watched, QEvent* event);

  QStringList selectedFiles() const;

 public slots:
  void chooseClicked();

 private:
  QTreeView *treeView;
  QListView *listView;
  QPushButton *openButton;
  QStringList selectedFilePaths;
};

/* Local variables: */
/* indent-tabs-mode: nil */
/* tab-width: 2 */
/* c-basic-offset: 2 */
/* End: */
