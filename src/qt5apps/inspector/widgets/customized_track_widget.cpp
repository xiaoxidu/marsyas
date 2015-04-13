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

using namespace track;

CustomizedTrackWidget::CustomizedTrackWidget( DebugController *debugger, QWidget * parent ):
  QWidget(parent),
  m_debugger(debugger),
  m_display_port(false),
  m_port_type(UndefinedFlowDirection),
  m_system(0)//,
  //m_plot(0)
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

  m_plotter = new track::CustomizedPlot;

  m_stack = new QStackedLayout();
  m_stack->addWidget(m_table);
  m_stack->addWidget(m_plotter);

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
  if (m_path.isEmpty()){
    return;
  }
  if (m_display_port)
    refreshFromPort();
  else
    refreshFromControl();

  //if (m_plot && doAutoScale)
  //  m_plot->fitRange();

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

//clear plot and replot
void CustomizedTrackWidget::clear()
{
  m_path.clear();
  m_system = 0;
  clearPlot();
  //m_plotter->replot();
  m_plotter->update();
  emit pathChanged(QString());
}

//set data from control
void CustomizedTrackWidget::refreshFromControl()
{
  Q_ASSERT(m_system);
  Q_ASSERT(!m_path.isEmpty());

  MarControlPtr control = m_system->getControl( m_path.toStdString() );
  if (control.isInvalid()) {
    qWarning() << "CustomizedTrackWidget: invalid control path:" << m_path;
    clearPlot();
    return;
  }

  //const realvec & data = control->to<mrs_realvec>();
  const realvec & data = m_debugger->m_controls_acumulated[m_path.toStdString()].data;

  m_table->setData(data);

  m_data = data;
  m_plotter->setData(&data);
  //m_plotter->plotData();
  //if (m_plot)
  //  m_plot->setData(&m_data);
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
        data = &m_debugger->m_ports_acummulated[m_path.toStdString()].data_input; break;
      case Output:
        data = &m_debugger->m_ports_acummulated[m_path.toStdString()].data_output; break;
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
  //m_plotter->plotData();
  //if (m_plot)
  //  m_plot->setData(&m_data);
}

//reset m_data
//set data of m_table and clear m_plot
void CustomizedTrackWidget::clearPlot()
{
  m_data = realvec();
  m_table->setData( realvec() );
  m_plotter->setData(0);
  //m_plotter->plotData();
  //if (m_plot)
  //  m_plot->clear();

}
