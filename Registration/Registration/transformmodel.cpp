#include <Registration/transformmodel.h>
#include <math.h>

TransformModel::TransformModel(QObject *parent) :
  QAbstractTableModel(parent) {
  QStringList _horzHeader;
  _horzHeader << "X" << "Y" << "Z" << "W";
  QStringList _vertHeader;
  _vertHeader << "X" << "Y" << "Z" << "W" << "O";

  QList<QList<QVariant> > data;

  QList<QVariant> row0;
  row0.append(1);
  row0.append(0);
  row0.append(0);
  row0.append(0);

  QList<QVariant> row1;
  row1.append(0);
  row1.append(1);
  row1.append(0);
  row1.append(0);

  QList<QVariant> row2;
  row2.append(0);
  row2.append(0);
  row2.append(1);
  row2.append(0);

  QList<QVariant> row3;
  row3.append(0);
  row3.append(0);
  row3.append(0);
  row3.append(1);

  QList<QVariant> row4;
  row4.append(0);
  row4.append(0);
  row4.append(0);
  row4.append(0);

  data.append(row0);
  data.append(row1);
  data.append(row2);
  data.append(row3);
  data.append(row4);

  this->gridData = data;
  this->vertHeader = _vertHeader;
  this->horzHeader = _horzHeader;

  bool ok = connect(this,
                    SIGNAL(dataChanged(QModelIndex, QModelIndex)),
                    this, SLOT(onMyItemsChanged(QModelIndex, QModelIndex)));

  Q_ASSERT(ok);
}

TransformModel::TransformModel(QList<QList<QVariant> > data, QStringList horzHeader, QStringList vertHeader, QObject *parent) :
  QAbstractTableModel(parent) {
  // Consider using copy-ctor
  this->horzHeader = horzHeader;
  this->vertHeader = vertHeader;
  this->gridData = data;

  bool ok = connect(this,
                    SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                    this, SLOT(onMyItemsChanged(QModelIndex, QModelIndex)));

  Q_ASSERT(ok);
}

int TransformModel::rowCount(const QModelIndex &parent) const {
  return this->gridData.size();
}

int TransformModel::columnCount(const QModelIndex &parent) const {
  int columnCount = 0;

  if (this->gridData.size() > 0) {
    columnCount = this->gridData[0].size();
  } else {
    columnCount = 0;
  }
  return columnCount;
}

// Editable models must implement this
Qt::ItemFlags TransformModel::flags(const QModelIndex &index) const {
  return QAbstractTableModel::flags(index);// | Qt::ItemIsEditable;
#if 0
  Qt::ItemFlags f = Qt::NoItemFlags;
  if (index.row() < 3) {
    f = f | Qt::ItemIsEditable | Qt::ItemIsEnabled;
  }
  return f;
#endif
}

QVariant TransformModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  } else {
    if (role == Qt::DisplayRole) {
      return QVariant(gridData[index.row()][index.column()]);
    } else if (role == Qt::EditRole) {
      return QVariant(gridData[index.row()][index.column()]);
    } else {
      return QVariant();
    }
  }
}

bool TransformModel::setData(const QModelIndex &index, const QVariant &value, int role) {
  if (role == Qt::EditRole) {
    gridData[index.row()][index.column()] = value;

    Q_EMIT(dataChanged(index,index));

    return true;
  } else {
    return false;
  }
}

QVariant TransformModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (role == Qt::DisplayRole) {
    if (orientation == Qt::Horizontal) {
      if (section < this->horzHeader.size()) {
        return QVariant(this->horzHeader[section]);
      }
    } else {
      if (section < this->vertHeader.size()) {
        return QVariant(this->vertHeader[section]);
      }
    }
  }

  return QVariant();
}

void TransformModel::onMyItemsChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight) {
}


void TransformModel::onSelectedItemsChanged(QStandardItem *item) {
  int col = item->index().column();
  int row = item->index().row();

  int N        = data(index(0,col),Qt::DisplayRole).toInt();
  float offset = data(index(1,col),Qt::DisplayRole).toFloat();
  float delta  = data(index(2,col),Qt::DisplayRole).toFloat();

  float _max = data(index(4,col),Qt::DisplayRole).toFloat();
  float _min = data(index(3,col),Qt::DisplayRole).toFloat();
  float rng = _max - _min;

  if (row == 0) {
    // Change delta to simply change resolution
    if ((rng > std::numeric_limits<float>::epsilon()) && (N>1)) {
      delta = rng / N;
      delta = float(int(10000 * delta)) / 10000.0f;
      offset = round(0.5f*(_max + _min) / delta);

      setData(index(2,col),QVariant(delta),Qt::EditRole);
      setData(index(1,col),QVariant(offset), Qt::EditRole);
    }
  }

  if (row < 3) {
    float colmin = (-(float(N)-1.0f)/2 + offset)*delta;
    colmin = float(int(1000*colmin)) / 1000.0f;
    float colmax = ((float(N)-1.0f) - (float(N)-1.0f)/2 + offset)*delta;
    colmax = float(int(1000*colmax)) / 1000.0f;

    setData(index(3,col),QVariant(colmin),Qt::EditRole);
    setData(index(4,col),QVariant(colmax),Qt::EditRole);
  }
}

QMap<QString,QVariant> TransformModel::getData() const {
  QMap<QString,QVariant> _gridData;

  _gridData.insert("nx",      data(index(0,0), Qt::DisplayRole).toInt());
  _gridData.insert("offset_x",data(index(1,0), Qt::DisplayRole).toFloat());
  _gridData.insert("dx",      data(index(2,0), Qt::DisplayRole).toFloat());

  _gridData.insert("ny",      data(index(0,1), Qt::DisplayRole).toInt());
  _gridData.insert("offset_y",data(index(1,1), Qt::DisplayRole).toFloat());
  _gridData.insert("dy",      data(index(2,1), Qt::DisplayRole).toFloat());

  _gridData.insert("nz",      data(index(0,2), Qt::DisplayRole).toInt());
  _gridData.insert("offset_z",data(index(1,2), Qt::DisplayRole).toFloat());
  _gridData.insert("dz",      data(index(2,2), Qt::DisplayRole).toFloat());

  return _gridData;
}
