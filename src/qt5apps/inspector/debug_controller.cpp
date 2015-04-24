#include "debug_controller.h"

#include <QDebug>

using namespace Marsyas;

DebugController::DebugController( QObject * parent ):
  QObject(parent),
  m_system(0),
  m_reader(0),
  m_recorder(0),
  m_tick_count(0),
  m_controls_acumulated(),
  m_ports_acummulated() {}

void DebugController::initControlInfomation(Marsyas::MarSystem * m_system)
{
    m_controls_acumulated.clear();
    for(auto cit=m_system->controls().cbegin();cit!=m_system->controls().cend();++cit)
    {
        if(!cit->second->hasType<mrs_string>() && !cit->second->hasType<mrs_complex>())
        {
          m_controls_acumulated.insert
          (std::pair<std::string,controlInformation>(cit->first,controlInformation()));
        }

    }
}

void DebugController::initPortInfomation(Marsyas::MarSystem * m_system)
{
    std::string m_path = m_system->path();
    portInformation pInfor;
    m_ports_acummulated.insert(std::pair<std::string,portInformation>(m_path,pInfor));

    std::vector<MarSystem*> children = m_system->getChildren();
    for (MarSystem *child : children)
      initPortInfomation(child);
}

void DebugController::appendRealvec(const Marsyas::realvec & from, Marsyas::realvec & to)
{
    int fromRow = from.getRows();
    int fromCol = from.getCols();
    int toCol = to.getCols();
    to.stretch(fromRow,fromCol+toCol);
    for(int c=0; c<fromCol; ++c)
    {
        for(int r=0; r<fromRow; ++r)
        {
            to(r,c+toCol) = from(r,c);
        }
    }
}


void DebugController::setSystem( Marsyas::MarSystem * system )
{
  if (m_system == system)
    return;

  delete m_recorder;
  m_recorder = 0;

  m_system = system;

  if (m_system)
    m_recorder = new Debug::Recorder(m_system);

  m_tick_count = 0;

  initControlInfomation(m_system);
  initPortInfomation(m_system);

  emit tickCountChanged(m_tick_count);
}

bool DebugController::setRecording(const QString &filename)
{
  if (filename.isEmpty())
    return false;

  Debug::FileReader *reader = new Debug::FileReader(filename.toStdString());
  if (!reader->isOpen())
  {
    qWarning() << "*** Recording file could not be opened:" << filename;
    delete reader;
    return false;
  }

  delete m_reader;
  m_reader = reader;

  emit recordingChanged(filename);

  return true;
}

void DebugController::tick( int count )
{
  if (count < 1)
  {
    qWarning() << "*** Warning: tick count less than 1.";
    return;
  }

  m_report.clear();

  if (m_system)
  {
    Q_ASSERT(m_recorder);

    m_recorder->clear();

    for (int i = 0; i < count; ++i)
    {
      m_system->tick();
      m_recorder->commit();

      for(auto it=m_controls_acumulated.begin();it!=m_controls_acumulated.end();++it)
      {
        if(it->first.find("mrs_realvec")!=std::string::npos)
        {
            mrs_realvec ctrlData = m_system->getControl(it->first)->to<mrs_realvec>();
            appendRealvec(ctrlData,it->second.data);
            if(i == 0)
            {
                it->second.getDataSize() = ctrlData.getCols();
            }
        }
        else
        {

            mrs_real controlVal = 0.0;
            if(it->first.find("mrs_real")!=std::string::npos)
            {
                controlVal = m_system->getControl(it->first)->to<mrs_real>();
            }
            else if(it->first.find("mrs_natural")!=std::string::npos)
            {
                controlVal = m_system->getControl(it->first)->to<mrs_natural>();
            }
            else if(it->first.find("mrs_bool")!=std::string::npos)
            {
                controlVal = m_system->getControl(it->first)->to<mrs_bool>();
            }
            it->second.data.stretch(it->second.data.getSize()+1);
            it->second.data(0,it->second.data.getSize()-1) = controlVal;
        }
      }


      for(auto it=m_ports_acummulated.begin();it!=m_ports_acummulated.end();++it)
      {

        const Marsyas::Debug::Record::Entry * entry = this->currentState()->entry(it->first);
        if(entry)
        {

            if(i == 0)
            {
                it->second.input_data_size = entry->input.getCols();
                it->second.output_data_size = entry->output.getCols();
            }

            appendRealvec(entry->input,it->second.data_input);
            appendRealvec(entry->output,it->second.data_output);
        }
      }
      m_tick_count += count;

    }

    if (m_reader && !m_reader->eof())
    {
      Marsyas::Debug::Record file_record;
      bool ok = m_reader->read( file_record );
      if (ok)
        Debug::compare(m_recorder->record(), file_record, m_report);
    }

    emit ticked();
    emit tickCountChanged(m_tick_count);
  }
}

void DebugController::rewind()
{
  if (m_reader)
    m_reader->rewind();
  if (m_recorder)
    m_recorder->clear();
  m_report.clear();
  m_tick_count = 0;


  for(auto it=m_controls_acumulated.begin();it!=m_controls_acumulated.end();++it)
  {
    it->second.data=realvec();
    it->second.data_size = 1;
  }

  for(auto it=m_ports_acummulated.begin();it!=m_ports_acummulated.end();++it)
  {

    it->second.data_input=realvec();
    it->second.data_output=realvec();
    it->second.input_data_size = 1;
    it->second.output_data_size = 1;
  }

  emit tickCountChanged(m_tick_count);
}
