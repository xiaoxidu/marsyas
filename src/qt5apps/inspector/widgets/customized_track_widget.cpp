#include "customized_track_widget.h"
#include "../debug_controller.h"
#include "../../common/realvec_table_widget.h"

#include <marsyas/system/MarControl.h>

#include <QDebug>
#include <QStackedLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolButton>
#include <QMenu>
#include <QAction>

using namespace Marsyas;
using namespace MarsyasQt;

void CustomizedPlot::paintEvent(QPaintEvent *p)
{
    //plotWidgetOutline();
    if(data)
    {
       if(type == Points)
       {
            plotPoint();
       }
       else if(type == Line)
       {
           plotLine();
       }
       else if(type == Sticks)
       {
           plotStick();
       }
       else
       {
           plotLine();
       }

    }
    else
    {
        return;
    }

}

void CustomizedPlot::plotPoint()
{

    QPainter painter(this);

    mrs_natural row_count = data->getRows();
    mrs_natural column_count = data->getCols();

    mrs_real max = data->maxval();
    mrs_real min = data->minval();

    if(max==min)
    {
        max++;
        min--;
    }
    mrs_natural total_count = data_size*m_debugger->getTickCount();
    mrs_natural offset = total_count-column_count;

    for(mrs_natural r = 0; r<row_count;r++)
    {

        QPen pen;
        //if(row_count<plot_width && column_count!=0){
        //    pen.setWidth(static_cast<int>(std::ceil(plot_width/total_count)));
        //}
        //else{
            pen.setWidth(1);
        //}
        pen.setColor(QColor::fromHsvF( (qreal)r / row_count, 1, 1));
        painter.setPen(pen);
        mrs_natural minY = INT_MAX;
        mrs_natural maxY = INT_MIN;
        mrs_natural currX = -plot_margin;

        mrs_natural x, y;

        for(mrs_natural c = 0; c<offset;c++)
        {
            x = (total_count==1 ? plot_margin : plot_margin + (width()-plot_margin*2.0)/(total_count-1)*c);
            y = plot_margin + (height()-plot_margin*2.0)/(max-min)*(max);

            if(y>maxY)
            {
                maxY = y;
            }

            if(y<minY)
            {
                minY = y;
            }

            if(x!=currX)
            {
                if(minY!=INT_MAX && maxY!=INT_MIN)
                {
                    painter.drawLine(currX,minY,currX,maxY);
                }
                currX = x;
                minY = INT_MAX;
                maxY = INT_MIN;
            }
        }

        for(mrs_natural c = 0; c<column_count;c++)
        {
            x = (total_count==1 ? plot_margin : plot_margin + (width()-plot_margin*2.0)/(total_count-1)*(c+offset));
            y = plot_margin + (height()-plot_margin*2.0)/(max-min)*(max-(*data)(r,c));

            if(y>maxY)
            {
                maxY = y;
            }

            if(y<minY)
            {
                minY = y;
            }

            if(x!=currX)
            {
                if(minY!=INT_MAX && maxY!=INT_MIN)
                {
                    painter.drawLine(currX,minY,currX,maxY);
                }
                currX = x;
                if(c!=column_count-1)
                {
                    minY = INT_MAX;
                    maxY = INT_MIN;
                }
            }
        }
        if(minY!=INT_MAX && maxY!=INT_MIN)
        {
            painter.drawLine(currX,minY,currX,maxY);
        }
    }
}

void CustomizedPlot::plotLine()
{

    QPainter painter(this);

    mrs_natural row_count = data->getRows();
    mrs_natural column_count = data->getCols();

    mrs_real max = data->maxval();
    mrs_real min = data->minval();

    if(max==min)
    {
        max++;
        min--;
    }

    int total_count = data_size*m_debugger->getTickCount();
    int offset = total_count-column_count;

    for(mrs_natural r = 0; r<row_count;r++)
    {
        QPen pen;

        //if(row_count<plot_width && column_count!=0)
        //{
        //    pen.setWidth(static_cast<int>(std::ceil(plot_width/total_count)));
        //}
        //else
        //{
        pen.setWidth(1);
        //}
        pen.setColor(QColor::fromHsvF( (qreal)r / row_count, 1, 1));
        painter.setPen(pen);
        mrs_natural prevMinY = INT_MAX;
        mrs_natural prevMaxY = INT_MIN;

        mrs_natural currMinY = INT_MAX;
        mrs_natural currMaxY = INT_MIN;

        mrs_natural currX = plot_margin;
        mrs_natural prevX = plot_margin;

        mrs_natural x, y;

        for(mrs_natural c = 0; c<offset;c++)
        {
            x = (total_count==1 ? plot_margin : plot_margin + (width()-plot_margin*2.0)/(total_count-1)*c);
            y = plot_margin + (height()-plot_margin*2.0)/(max-min)*(max);

            if(y>currMaxY)
            {
                currMaxY = y;
            }

            if(y<currMinY)
            {
                currMinY = y;
            }

            if(x!=currX)
            {

                if(prevMaxY != INT_MIN)
                {
                    painter.drawLine(prevX,prevMaxY,currX,currMinY);
                }
                if(currMinY!=INT_MAX && currMaxY!=INT_MIN)
                {
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

        for(mrs_natural c = 0; c<column_count;c++)
        {
            x = (total_count==1 ? plot_margin : plot_margin + (width()-plot_margin*2.0)/(total_count-1)*(c+offset));
            y = plot_margin + (height()-plot_margin*2.0)/(max-min)*(max-(*data)(r,c));

            if(y>currMaxY)
            {
                currMaxY = y;
            }

            if(y<currMinY)
            {
                currMinY = y;
            }

            if(x!=currX)
            {

                if(prevMaxY != INT_MIN)
                {
                    painter.drawLine(prevX,prevMaxY,currX,currMinY);
                }
                if(currMinY!=INT_MAX && currMaxY!=INT_MIN)
                {
                    painter.drawLine(currX,currMinY,currX,currMaxY);
                }

                prevMaxY = currMaxY;
                prevMinY = currMinY;
                prevX = currX;

                currX = x;
                if(c!=column_count-1)
                {
                    currMinY = INT_MAX;
                    currMaxY = INT_MIN;
                }
            }

        }
        if(currMinY!=INT_MAX && currMaxY!=INT_MIN)
        {
            if(prevMinY!=INT_MAX && prevMaxY!=INT_MIN)
            {
                painter.drawLine(prevX,prevMaxY,currX,currMinY);
            }
            painter.drawLine(currX,currMinY,currX,currMaxY);
        }
    }
}

void CustomizedPlot::plotStick()
{

    QPainter painter(this);

    mrs_natural row_count = data->getRows();
    mrs_natural column_count = data->getCols();

    mrs_real max = (data->getSize() > 0 ? data->maxval() : 1);
    mrs_real min = (data->getSize() > 0 ? data->minval() : -1);

    if(max==min)
    {
        max++;
        min--;
    }

    mrs_natural total_count = data_size*m_debugger->getTickCount();
    mrs_natural offset = total_count-column_count;

    for(mrs_natural r = 0; r<row_count;r++)
    {
        QPen pen;
        //if(row_count<plot_width && column_count!=0)
        //{
        //    pen.setWidth(static_cast<int>(std::ceil(plot_width/total_count)));
        //}
        //else
        //{
            pen.setWidth(1);
        //}
        pen.setColor(QColor::fromHsvF( (qreal)r / row_count, 1, 1));
        painter.setPen(pen);
        mrs_natural minY = INT_MAX;
        mrs_natural maxY = INT_MIN;
        mrs_natural currX = plot_margin;

        mrs_natural x, y;

        for(mrs_natural c = 0; c<offset;c++)
        {
            x = (total_count==1 ? plot_margin : plot_margin + (width()-plot_margin*2.0)/(total_count-1)*c);
            y = plot_margin +  (height()-plot_margin*2.0)/(max-min)*max;

            if(y>maxY)
            {
                maxY = y;
            }

            if(y<minY)
            {
                minY = y;
            }

            if(x!=currX)
            {
                if(minY!=INT_MAX && maxY!=INT_MIN)
                {
                    if(maxY>height()/2)
                    {
                        painter.drawLine(currX,height()-plot_margin,currX,maxY);
                    }
                    if(minY<height()/2)
                    {
                        painter.drawLine(currX,minY,currX,height()-plot_margin);
                    }
                }
                currX = x;
                minY = INT_MAX;
                maxY = INT_MIN;
            }
        }

        for(mrs_natural c = 0; c<column_count;c++)
        {
            x = (total_count==1 ? plot_margin : plot_margin + (width()-plot_margin*2.0)/(total_count-1)*(c+offset));
            mrs_real y = plot_margin +  (height()-plot_margin*2.0)/(max-min)*(max-(*data)(r,c));

            if(y>maxY)
            {
                maxY = y;
            }

            if(y<minY)
            {
                minY = y;
            }

            if(x!=currX)
            {
                if(minY!=INT_MAX && maxY!=INT_MIN)
                {
                    if(maxY>height()/2)
                    {
                        painter.drawLine(currX,height()-plot_margin,currX,maxY);
                    }
                    if(minY<height()/2)
                    {
                        painter.drawLine(currX,minY,currX,height()-plot_margin);
                    }
                }
                currX = x;
                if(c!=column_count-1)
                {
                    minY = INT_MAX;
                    maxY = INT_MIN;
                }
            }
        }
        if(minY!=INT_MAX && maxY!=INT_MIN)
        {
            if(maxY>0)
            {
                painter.drawLine(currX,0,currX,maxY);
            }
            if(minY<0)
            {
                painter.drawLine(currX,minY,currX,0);
            }
        }
    }
}

void CustomizedPlot::plotWidgetOutline()
{
    QPainter painter(this);

    QPen pen;
    pen.setColor(QColor(0, 0, 255));
    pen.setWidth(1);
    painter.setPen(pen);

    painter.drawRect(plot_margin, plot_margin, width()-2*plot_margin, height()-2*plot_margin);
}

CustomizedTrackWidget::CustomizedTrackWidget( DebugController *debugger, QWidget * parent ):
  QWidget(parent),
  m_debugger(debugger),
  m_display_port(false),
  m_port_type(UndefinedFlowDirection),
  m_system(0)
{
  setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

  m_display_type_selector = new QComboBox;
  m_display_type_selector->addItems( QStringList()
                                     << "Table" << "Points" << "Sticks"
                                     << "Line" << "Curve" /*<< "Spectrogram"*/ );

  m_auto_scale_btn = new QToolButton;
  m_auto_scale_btn->setText("S");
  m_auto_scale_btn->setToolTip("Auto Scale");

  m_table = new MarsyasQt::RealvecTableWidget;
  m_table->setEditable(false);

  m_plotter = new CustomizedPlot(m_debugger);

  m_stack = new QStackedLayout();
  m_stack->addWidget(m_table);
  m_stack->addWidget(m_plotter);

  m_select_button = new QPushButton();
  m_select_button->resize(8,80);
  m_select_button->setText("s\ne\nl\ne\nc\nt");

  connect(m_select_button,SIGNAL(clicked()),this,SLOT(emitCurrTrackWidget()));

  // Toolbar

  QStringList style_names;
  style_names << "Table" << "Points" << "Sticks"
              << "Line" << "Curve"
              //<< "Spectrogram"
              ;

  QMenu *style_menu = new QMenu(this);
  int style_idx = 0;
  foreach (const QString & style_name, style_names)
  {
    QAction * action = style_menu->addAction(style_name);
    m_style_menu_mapper.setMapping(action, style_idx);
    connect(action, SIGNAL(triggered()), &m_style_menu_mapper, SLOT(map()));
    style_idx++;
  }

  connect(&m_style_menu_mapper, SIGNAL(mapped(int)),
          this, SLOT(setDisplayType(int)));

  QToolButton *style_btn = new QToolButton;
  style_btn->setText("D");
  style_btn->setToolTip("Display Mode");
  style_btn->setMenu(style_menu);
  style_btn->setPopupMode(QToolButton::InstantPopup);

  // Layout

  QVBoxLayout *tool_layout = new QVBoxLayout();
  tool_layout->addWidget(style_btn);
  tool_layout->addWidget(m_auto_scale_btn);
  tool_layout->addStretch();

  QHBoxLayout *layout = new QHBoxLayout();
  layout->setContentsMargins(0,0,0,0);
  layout->setSpacing(0);
  layout->addWidget(m_select_button);
  layout->addLayout(m_stack);
  layout->addLayout(tool_layout);

  setLayout(layout);

  connect( m_display_type_selector, SIGNAL(activated(int)), this, SLOT(setDisplayType(int)) );
  connect( m_auto_scale_btn, SIGNAL(clicked()), this, SLOT(autoScale()) );

  m_display_type_selector->setCurrentIndex(Line);
  setDisplayType(Line);
}

CustomizedTrackWidget::~CustomizedTrackWidget()
{
  // must manually delete m_plot because it's destructor accesses m_plotter

  delete m_plotter;
  delete m_table;
  delete m_stack;
  delete m_display_type_selector;
  delete m_auto_scale_btn;

}

//plot curves or image
void CustomizedTrackWidget::setDisplayType( int type )
{
  if (type == Table)
  {
    m_stack->setCurrentWidget(m_table);
  }
  else
  {

    refresh(true);
    m_stack->setCurrentWidget(m_plotter);
    m_plotter->setType(type);
  }
}

//refresh plot and update display path
void CustomizedTrackWidget::displayControl( MarSystem * system, const QString & path )
{
  if (!m_display_port && m_system == system && m_path == path)
    return;

  MarControlPtr control = system->getControl( path.toStdString() );

  m_display_port = false;
  m_path = path;
  m_system = system;

  refresh(true);

  QString abs_path = QString::fromStdString(control->path());
  emit pathChanged(abs_path);
}

//refresh plot and update display path
void CustomizedTrackWidget::displayPort( MarSystem * system,
                                 FlowDirection port )
{
  if (m_display_port && m_system == system && m_port_type == port)
    return;

  m_display_port = true;
  m_port_type = port;
  m_system = system;
  m_path = QString::fromStdString(system->getAbsPath());

  refresh(true);

  QString display_path = QString::fromStdString(system->path());
  if (port == Input)
    display_path += " [input]";
  else
    display_path += " [output]";
  emit pathChanged(display_path);
}

//refresh from control or port and replot
void CustomizedTrackWidget::refresh(bool doAutoScale)
{
  if (m_path.isEmpty())
  {
    return;
  }
  if (m_display_port)
    refreshFromPort();
  else
    refreshFromControl();

  m_plotter->update();
}
//fit range and replot
void CustomizedTrackWidget::autoScale()
{
  //if (m_plot) {
  //  m_plot->fitRange();
  //  m_plotter->replot();
  //}
}

void CustomizedTrackWidget::emitCurrTrackWidget()
{

    emit currTrackWidget(this);
}

//clear plot and replot
void CustomizedTrackWidget::clear()
{
  m_path.clear();
  m_system = 0;
  clearPlot();
  m_plotter->update();
  emit pathChanged(QString());
}

//set data from control
void CustomizedTrackWidget::refreshFromControl()
{
  Q_ASSERT(m_system);
  Q_ASSERT(!m_path.isEmpty());

  MarControlPtr control = m_system->getControl( m_path.toStdString() );
  if (control.isInvalid())
  {
    qWarning() << "CustomizedTrackWidget: invalid control path:" << m_path;
    clearPlot();
    return;
  }
  const realvec & data = m_debugger->m_controls_acumulated[m_path.toStdString()].data;
  m_plotter->setDataSize(m_debugger->m_controls_acumulated[m_path.toStdString()].data_size);

  m_table->setData(data);

  m_data = data;
  m_plotter->setData(&data);
}

//set data from port and set data
void CustomizedTrackWidget::refreshFromPort()
{
  Q_ASSERT(m_system);
  Q_ASSERT(!m_path.isEmpty());

  const realvec * data = 0;

  switch(m_port_type)
  {
    case Input:
      data = &m_debugger->m_ports_acummulated[m_path.toStdString()].data_input;
      m_plotter->setDataSize(m_debugger->m_ports_acummulated[m_path.toStdString()].input_data_size);
      std::cout<<"input data size:"<<m_debugger->m_ports_acummulated[m_path.toStdString()].input_data_size<<"\n";
      break;
    case Output:
      data = &m_debugger->m_ports_acummulated[m_path.toStdString()].data_output;
      m_plotter->setDataSize(m_debugger->m_ports_acummulated[m_path.toStdString()].output_data_size);
      std::cout<<"output data size:"<<m_debugger->m_ports_acummulated[m_path.toStdString()].output_data_size<<"\n";
      break;
    default:
      break;
  }

  if (!data)
  {
    clearPlot();
    return;
  }

  m_table->setData(*data);

  m_data = *data;
  m_plotter->setData(data);
}

//reset m_data
//set data of m_table and clear m_plot
void CustomizedTrackWidget::clearPlot()
{
  m_data = realvec();
  m_table->setData( realvec() );
  m_plotter->setData(0);
}
