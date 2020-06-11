#include <Registration/transformmodel.h>

class TransformModel(QtGui.QStandardItemModel):
    def __init__(self, data, horzHeader=None, vertHeader=None, parent=None):
    """
    Args:
    datain:    a list of lists\n
    horHeader: a list of strings
    verHeader: a list of strings
    """
    QtGui.QStandardItemModel.__init__(self, len(data), len(data[0]), parent)

    self.arraydata = data

                     for row in range(self.rowCount()):
                       for col in range(self.columnCount()):
                         index = self.index(row,col)
                                 self.setData(index, QVariant('%5.2f' % (float(data[row][col]))))
                                 self.horzHeader = horzHeader
                                     self.vertHeader = vertHeader

                                         def flags(self, index):
                                         f = Qt.ItemIsSelectable | Qt.ItemIsEditable | Qt.ItemIsEnabled
                                             return f

                                                 def on_itemChanged(self, item):
                                                 return

                                                     def setAllData(self, aff):
                                                     for row in range(self.rowCount()):
                                                       for col in range(self.columnCount()):
                                                         index = self.index(row,col)
                                                             self.setData(index, QVariant('%5.2f' % (float(aff[row][col]))))

                                                             def getData(self):
                                                             output = np.zeros((4,4))
                                                                 for i in range(self.rowCount()):
                                                                   for j in range(self.columnCount()):
                                                                     output[i,j] = self.data(self.index(i,j), Qt.DisplayRole)
                                                                         return output
