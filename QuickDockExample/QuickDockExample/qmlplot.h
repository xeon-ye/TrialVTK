#pragma once

#include <QtQuick>
class DataManager;

class CustomPlotItem : public QQuickPaintedItem {
  Q_OBJECT

 public:
  explicit CustomPlotItem( QQuickItem* parent = 0 );
  virtual ~CustomPlotItem();

  void paint( QPainter* painter ) override;

  Q_INVOKABLE void initCustomPlot();

 protected:
  void mousePressEvent( QMouseEvent* event ) override;
  void mouseReleaseEvent( QMouseEvent* event ) override;
  void mouseMoveEvent( QMouseEvent* event ) override;
  void mouseDoubleClickEvent( QMouseEvent* event ) override;
  void wheelEvent( QWheelEvent *event ) override;

 private:
  DataManager*         m_CustomPlot;
  void routeMouseEvents( QMouseEvent* event );
  void routeWheelEvents( QWheelEvent* event );

 private slots:
  void onCustomReplot();
  void updateCustomPlotSize();
};


