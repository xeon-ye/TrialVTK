/**
 * @file   FileDialog.cxx
 * @author Jens Munk Hansen <jmh@debian9laptop.parknet.dk>
 * @date   Mon Feb 17 20:03:41 2020
 *
 * @brief
 *
 * Copyright 2020 Jens Munk Hansen
 *
 */

#include <App/filedialog.hpp>

FileDialog::FileDialog(QWidget *parent) : QFileDialog(parent) {
  setOption(QFileDialog::DontUseNativeDialog);
  setFileMode(QFileDialog::Directory);

  for (auto *pushButton : findChildren<QPushButton*>()) {
    qDebug() << pushButton->text();
    if (pushButton->text() == "&Open" || pushButton->text() == "&Choose") {
      openButton = pushButton;
      break;
    }
  }
  openButton->installEventFilter(this);
  disconnect(openButton, SIGNAL(clicked(bool)));
  connect(openButton, &QPushButton::clicked, this, &FileDialog::chooseClicked);

  treeView = findChild<QTreeView*>();
  if (treeView) {
    treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
  }

  listView = findChild<QListView*>("listView");
  if (listView) {
    listView->setSelectionMode(QAbstractItemView::ExtendedSelection);
  }
}

void FileDialog::chooseClicked() {
  selectedFilePaths.clear();
  qDebug() << treeView->selectionModel()->selection();
  for (const auto& modelIndex : listView->selectionModel()->selectedIndexes()) {
    qDebug() << modelIndex.column();
    if (modelIndex.column() == 0)
      selectedFilePaths.append(directory().absolutePath() + "/" + modelIndex.data().toString());
  }
  emit filesSelected(selectedFilePaths);
  hide();
  qDebug() << selectedFilePaths;
  QDialog::accept();
}

QStringList FileDialog::selectedFiles() const {
  return selectedFilePaths;
}

bool FileDialog::eventFilter(QObject* watched, QEvent* event) {
  QPushButton *btn = qobject_cast<QPushButton*>(watched);
  if (btn) {
    if (event->type() == QEvent::EnabledChange) {
      if (!btn->isEnabled()) {
        btn->setEnabled(true);
      }
    }
  }

  return QWidget::eventFilter(watched, event);
}

/* Local variables: */
/* indent-tabs-mode: nil */
/* tab-width: 2 */
/* c-basic-offset: 2 */
/* End: */
