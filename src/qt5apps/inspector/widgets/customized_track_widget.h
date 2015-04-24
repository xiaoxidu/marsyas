/*
** Copyright (C) 1998-2013 George Tzanetakis <gtzan@cs.uvic.ca>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef MARSYAS_INSPECTOR_CUSTOMIZED_TRACK_WIDGET_INCLUDED
#define MARSYAS_INSPECTOR_CUSTOMIZED_TRACK_WIDGET_INCLUDED

#include <marsyas/system/MarSystem.h>
#include <marsyas/realvec.h>

#include <QWidget>
#include <QTableView>
#include <QAbstractTableModel>
#include <QLineEdit>
#include <QStackedLayout>
#include <QComboBox>
#include <QToolButton>
#include <QSignalMapper>
#include <QPushButton>

#include <QPainter>

#include "qcustomplot.h"

namespace MarsyasQt {
class RealvecTableWidget;
}

class DebugController;

class CustomizedPlot : public QWidget
{

  Q_OBJECT
public:

    enum DisplayType {
      Table,
      Points,
      Sticks,
      Line,
      Curve//,
      //Image
    };

    CustomizedPlot( DebugController* debugger, QWidget* parent = 0)
        : m_debugger(debugger), QWidget(parent)
    {
        QWidget::setBackgroundRole(QPalette::Window);
        data = 0;
        type = 1;
        data_size = 1;
        plot_margin = 0;
    }

    void setData(const Marsyas::realvec * new_data)
    {
        data = new_data;
    }

    void setType(int new_type)
    {
        type = new_type;
    }

    void setDataSize(int size)
    {
        data_size = size;
    }

    void setMargin(int margin)
    {
        plot_margin = margin;
    }

    int getMargin() const {return plot_margin;}

public slots:


protected:

    void paintEvent(QPaintEvent *p);
    void plotPoint();
    void plotLine();
    void plotStick();
    void plotWidgetOutline();

private:
    const Marsyas::realvec * data;
    int type;
    int data_size;
    DebugController *m_debugger;
    int plot_margin;
};


class CustomizedTrackWidget : public QWidget
{
  Q_OBJECT
public:
  enum FlowDirection {
    UndefinedFlowDirection,
    Input,
    Output
  };
  enum DisplayType {
    Table,
    Points,
    Sticks,
    Line,
    Curve//,
    //Image
  };

  CustomizedTrackWidget( DebugController *debugger, QWidget * parent = 0 );
  ~CustomizedTrackWidget();
  QSize sizeHint() const { return QSize(500,120); }
  QSize minimumSizeHint() const { return QSize(50,50); }

  int getPlotterLeft() const { return m_plotter->geometry().left();}
  int getPlotterRight() const { return m_plotter->geometry().right();}

  CustomizedPlot* plotWidget() { return this->m_plotter;}

public slots:
  void setDisplayType( int type );
  void displayControl( Marsyas::MarSystem *system, const QString & path );
  void displayPort( Marsyas::MarSystem * system, FlowDirection port );
  void refresh(bool autoScale = false);
  void clear();
  void autoScale();
  void emitCurrTrackWidget();

signals:
  void pathChanged(const QString &);
  void currTrackWidget(CustomizedTrackWidget *);

private:
  void refreshFromControl();
  void refreshFromPort();
  void clearPlot();

  DebugController *m_debugger;

  bool m_display_port;
  FlowDirection m_port_type;
  QString m_path;
  Marsyas::MarSystem * m_system;

  QStackedLayout * m_stack;
  QComboBox * m_display_type_selector;
  QToolButton * m_auto_scale_btn;

  MarsyasQt::RealvecTableWidget *m_table;
  CustomizedPlot *m_plotter;
  Marsyas::realvec m_data;

  QSignalMapper m_style_menu_mapper;

  QPushButton * m_select_button;
};

#endif // MARSYAS_INSPECTOR_CUSTOMIZED_TRACK_WIDGET_INCLUDED
