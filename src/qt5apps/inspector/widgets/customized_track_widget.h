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

#include <qwt_plot.h>
#include <qwt_series_data.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_spectrogram.h>
#include <qwt_color_map.h>

#include <QWidget>
#include <QTableView>
#include <QAbstractTableModel>
#include <QLineEdit>
#include <QStackedLayout>
#include <QComboBox>
#include <QToolButton>
#include <QSignalMapper>

#include <QPainter>

#include "qcustomplot.h"

namespace MarsyasQt {
class RealvecTableWidget;
}

namespace track {

/*
class CustomizedPlot : public QCustomPlot{
public:
    CustomizedPlot() : QCustomPlot(){}

    void setData(const Marsyas::realvec * new_data){
        data = new_data;
    }

    void plotData(){

        this->clearGraphs();
        if(data){
            Marsyas::mrs_natural row_count = data->getRows();
            Marsyas::mrs_natural column_count = data->getCols();

            Marsyas::mrs_real max = data->maxval();
            Marsyas::mrs_real min = data->minval();

            if(max==min){
                max++;
                min--;
            }

            for(Marsyas::mrs_natural r = 0; r<row_count;r++){
                QPen pen;
                pen.setColor(QColor::fromHsvF( (qreal)r / row_count, 1, 1));
                this->addGraph();
                this->graph(r)->setPen(pen);
                QVector<Marsyas::mrs_real> y=QVector<Marsyas::mrs_real>(column_count);
                QVector<Marsyas::mrs_real> x=QVector<Marsyas::mrs_real>(column_count);
                for(Marsyas::mrs_natural c = 0; c<column_count;c++){
                    x[c] = c;
                    y[c] = (*data)(r,c);
                    //std::cout<<x<<" "<<y<<" "<<" real values: "<<(*data)(r,c)<<" "<<c<<"\n";
                    //painter.drawPoint((int)(x),(int)(y));
                }
                this->graph(r)->setData(x,y);
                this->xAxis->setRange(0,column_count);
                this->yAxis->setRange(min,max);
                this->replot();
            }

        }
        else{
            return;
        }
    }

private:
    const Marsyas::realvec * data;
};
*/

class CustomizedPlot : public QWidget{

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

    CustomizedPlot(QWidget* parent = 0)
        : QWidget(parent)
    {
        QWidget::setBackgroundRole(QPalette::Window);
        data = 0;
        type = 1;
    }

    void setData(const Marsyas::realvec * new_data){
        data = new_data;
    }

    void setType(int new_type){
        type = new_type;
    }

protected:
    void paintEvent(QPaintEvent *p){
        plotWidgetOutline();

        if(data){
           if(type == Points){
                plotPoint();
           }
           else if(type == Line){
               plotLine();
           }
           else if(type == Sticks){
               plotStick();
           }
           else{
               plotLine();
           }
        }
        else{
            return;
        }
    }

    void plotPoint(){

        QPainter painter(this);
        Marsyas::mrs_real plot_width = this->width()-10*2;

        Marsyas::mrs_natural row_count = data->getRows();
        Marsyas::mrs_natural column_count = data->getCols();

        Marsyas::mrs_real max = data->maxval();
        Marsyas::mrs_real min = data->minval();

        if(max==min){
            max++;
            min--;
        }

        for(Marsyas::mrs_natural r = 0; r<row_count;r++){
            QPen pen;
            if(row_count<plot_width){
                std::cout<<static_cast<int>(std::ceil(plot_width/row_count))<<"\n";
                pen.setWidth(static_cast<int>(std::ceil(plot_width/column_count)));
            }
            else{
                pen.setWidth(1);
            }
            pen.setColor(QColor::fromHsvF( (qreal)r / row_count, 1, 1));
            painter.setPen(pen);
            int minY = INT_MAX;
            int maxY = INT_MIN;
            int currX = 10;
            for(Marsyas::mrs_natural c = 0; c<column_count;c++){
                Marsyas::mrs_real x = 10 + (width()-20.0)/column_count*c;
                Marsyas::mrs_real y = 10 + (height()-20.0)/(max-min)*(max-(*data)(r,c));

                if(y>maxY){
                    maxY = y;
                }

                if(y<minY){
                    minY = y;
                }

                if(x!=currX){
                    if(minY!=INT_MAX && maxY!=INT_MIN){
                        painter.drawLine(currX,minY,currX,maxY);
                    }
                    currX = x;
                    minY = INT_MAX;
                    maxY = INT_MIN;
                }

                //std::cout<<x<<" "<<y<<" "<<" real values: "<<(*data)(r,c)<<" "<<c<<"\n";
                //painter.drawPoint((int)(x),(int)(y));
            }
            if(minY!=INT_MAX && maxY!=INT_MIN){
                painter.drawLine(currX,minY,currX,maxY);
            }
        }
    }

    void plotLine(){

        QPainter painter(this);
        Marsyas::mrs_real plot_width = this->width()-10*2;

        Marsyas::mrs_natural row_count = data->getRows();
        Marsyas::mrs_natural column_count = data->getCols();

        Marsyas::mrs_real max = data->maxval();
        Marsyas::mrs_real min = data->minval();

        if(max==min){
            max++;
            min--;
        }

        for(Marsyas::mrs_natural r = 0; r<row_count;r++){
            QPen pen;
            if(row_count<plot_width){
                std::cout<<static_cast<int>(std::ceil(plot_width/row_count))<<"\n";
                pen.setWidth(static_cast<int>(std::ceil(plot_width/column_count)));
            }
            else{
                pen.setWidth(1);
            }
            pen.setColor(QColor::fromHsvF( (qreal)r / row_count, 1, 1));
            painter.setPen(pen);
            int prevMinY = INT_MAX;
            int prevMaxY = INT_MIN;

            int currMinY = INT_MAX;
            int currMaxY = INT_MIN;

            int currX = 10;
            int prevX = 10;

            for(Marsyas::mrs_natural c = 0; c<column_count;c++){
                Marsyas::mrs_real x = 10 + (width()-20.0)/column_count*c;
                Marsyas::mrs_real y = 10 + (height()-20.0)/(max-min)*(max-(*data)(r,c));

                if(y>currMaxY){
                    currMaxY = y;
                }

                if(y<currMinY){
                    currMinY = y;
                }

                if(x!=currX){

                    if(prevMaxY != INT_MIN){
                        painter.drawLine(prevX,prevMaxY,currX,currMinY);
                    }
                    if(currMinY!=INT_MAX && currMaxY!=INT_MIN){
                        painter.drawLine(currX,currMinY,currX,currMaxY);
                    }

                    prevMaxY = currMaxY;
                    prevMinY = currMinY;
                    prevX = currX;


                    currX = x;
                    currMinY = INT_MAX;
                    currMaxY = INT_MIN;
                }

                //std::cout<<x<<" "<<y<<" "<<" real values: "<<(*data)(r,c)<<" "<<c<<"\n";
                //painter.drawPoint((int)(x),(int)(y));
            }
            if(currMinY!=INT_MAX && currMaxY!=INT_MIN){
                painter.drawLine(currX,currMinY,currX,currMaxY);
            }
        }
    }

    void plotStick(){

        QPainter painter(this);
        Marsyas::mrs_real plot_width = this->width()-10*2;

        Marsyas::mrs_natural row_count = data->getRows();
        Marsyas::mrs_natural column_count = data->getCols();

        Marsyas::mrs_real max = data->maxval();
        Marsyas::mrs_real min = data->minval();

        if(max==min){
            max++;
            min--;
        }

        for(Marsyas::mrs_natural r = 0; r<row_count;r++){
            QPen pen;
            if(row_count<plot_width){
                std::cout<<static_cast<int>(std::ceil(plot_width/row_count))<<"\n";
                pen.setWidth(static_cast<int>(std::ceil(plot_width/column_count)));
            }
            else{
                pen.setWidth(1);
            }
            pen.setColor(QColor::fromHsvF( (qreal)r / row_count, 1, 1));
            painter.setPen(pen);
            int minY = INT_MAX;
            int maxY = INT_MIN;
            int currX = 10;
            for(Marsyas::mrs_natural c = 0; c<column_count;c++){
                Marsyas::mrs_real x = 10 + (width()-20.0)/column_count*c;
                Marsyas::mrs_real y = 10 +  (height()-20.0)/(max-min)*(max-(*data)(r,c));

                if(y>maxY){
                    maxY = y;
                }

                if(y<minY){
                    minY = y;
                }

                if(x!=currX){
                    if(minY!=INT_MAX && maxY!=INT_MIN){
                        if(maxY>height()/2){
                            painter.drawLine(currX,height()-10,currX,maxY);
                        }
                        if(minY<height()/2){
                            painter.drawLine(currX,minY,currX,height()-10);
                        }
                    }
                    currX = x;
                    minY = INT_MAX;
                    maxY = INT_MIN;
                }

                //std::cout<<x<<" "<<y<<" "<<" real values: "<<(*data)(r,c)<<" "<<c<<"\n";
                //painter.drawPoint((int)(x),(int)(y));
            }
            if(minY!=INT_MAX && maxY!=INT_MIN){
                if(maxY>0){
                    painter.drawLine(currX,0,currX,maxY);
                }
                if(minY<0){
                    painter.drawLine(currX,minY,currX,0);
                }
            }
        }
    }

    void plotWidgetOutline(){
        QPainter painter(this);

        QPen pen;
        pen.setColor(QColor(0, 0, 255));
        pen.setWidth(1);
        painter.setPen(pen);

        painter.drawRect(10, 10, width()-2*10, height()-2*10);
    }
private:
    const Marsyas::realvec * data;
    int type;
};


}


class DebugController;






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
  QSize sizeHint() const { return QSize(500,100); }
  QSize minimumSizeHint() const { return QSize(50,50); }

public slots:
  void setDisplayType( int type );
  void displayControl( Marsyas::MarSystem *system, const QString & path );
  void displayPort( Marsyas::MarSystem * system, FlowDirection port );
  void refresh(bool autoScale = false);
  void clear();
  void autoScale();

signals:
  void pathChanged(const QString &);

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
  track::CustomizedPlot *m_plotter;
  //track::RealvecPlot *m_plot;
  Marsyas::realvec m_data;

  QSignalMapper m_style_menu_mapper;
};



#endif // MARSYAS_INSPECTOR_CUSTOMIZED_TRACK_WIDGET_INCLUDED
