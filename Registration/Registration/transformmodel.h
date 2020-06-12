#pragma once

#include <QAbstractTableModel>
#include <QList>
#include <QMap>
#include <QStringList>
#include <QStandardItem>

class TransformModel : public QAbstractTableModel {
  Q_OBJECT
 public:
  explicit TransformModel(QObject* parent);

  explicit TransformModel(QList<QList<QVariant> > data,
                          QStringList horzHeader,
                          QStringList vertHeader,
                          QObject *parent);

  int rowCount(const QModelIndex &parent) const;

  int columnCount(const QModelIndex &parent) const;

  Qt::ItemFlags flags(const QModelIndex &index) const;

  QVariant data(const QModelIndex &index, int role) const;

  bool setData(const QModelIndex &index, const QVariant &value, int role);

  QVariant headerData(int section, Qt::Orientation orientation, int role) const;

  QMap<QString,QVariant> getData() const;


  Q_SLOT
  void onSelectedItemsChanged(QStandardItem *item);

  Q_SLOT
  void onMyItemsChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight);

 private:
  QStringList horzHeader;
  QStringList vertHeader;
  QList<QList<QVariant> > gridData;
};

