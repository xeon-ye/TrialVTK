#pragma once

#include <QStandardItemModel>
#include <QList>
#include <QStringList>

class TransformModel : public QStandardItemModel {
 Q_OBJECT:
 public:
  explicit TransformModel(QObject *parent = 0);
  explicit TransformModel(QList<QList<QVariant> > data,
                          QObject *parent = 0);

  virtual int rowCount(const QModelIndex &parent) const;

  virtual int columnCount(const QModelIndex &parent) const;

  virtual Qt::ItemFlags flags(const QModelIndex &index) const;

  virtual QVariant data(const QModelIndex &index, int role) const;

 signals:

 public slots:
  void onSelectedItemsChanged(QStandardItem *item);

 private:
  QStringList horzHeader;
  QStringList vertHeader;
  QList<QList<QVariant> > data;
};

