#ifndef MARSYASQT_INSPECTOR_DEBUG_CONTROLLER_INCLUDED
#define MARSYASQT_INSPECTOR_DEBUG_CONTROLLER_INCLUDED

#include <QObject>

#include <marsyas/system/MarSystem.h>
#include <marsyas/realvec.h>
#include <marsyas/debug/file_io.h>
#include <marsyas/debug/recorder.h>
#include <marsyas/debug/debugger.h>



class DebugController : public QObject
{
  Q_OBJECT
public:

  struct controlInformation{
    int tick_offset;
    Marsyas::realvec data;

    controlInformation(): tick_offset(0), data(){

   }

    controlInformation(int offset): tick_offset(offset), data(){

    }
  };

  struct portInformation{
    int tick_offset;
    Marsyas::realvec data_input;
    Marsyas::realvec data_output;

    portInformation(): tick_offset(0), data_input(), data_output(){

    }
    portInformation(int offset): tick_offset(offset), data_input(), data_output(){

    }
  };

  DebugController( QObject * parent = 0 );
  Marsyas::MarSystem *system() const { return m_system; }
  void setSystem( Marsyas::MarSystem * system );
  bool setRecording( const QString & fileName );
  bool endOfRecording()
  {
    return m_reader ? m_reader->eof() : true;
  }
  const Marsyas::Debug::Record * currentState()
  {
    return m_recorder ? &m_recorder->record() : 0;
  }
  const Marsyas::Debug::BugReport & report() const
  {
    return m_report;
  }

public slots:
  void tick( int count = 1 );
  void rewind();

signals:
  void ticked();
  void tickCountChanged(int count);
  void recordingChanged(const QString & filename);



private:
  Marsyas::MarSystem * m_system;
  Marsyas::Debug::FileReader *m_reader;
  Marsyas::Debug::Recorder *m_recorder;
  Marsyas::Debug::BugReport m_report;
  int m_tick_count;

public:
  std::map<std::string, controlInformation> m_controls_acumulated;
  std::map<std::string, portInformation> m_ports_acummulated;

protected:
  void appendRealvec(const Marsyas::realvec & from, Marsyas::realvec & to);
  void initControlInfomation(Marsyas::MarSystem * m_system);
  void initPortInfomation(Marsyas::MarSystem * m_system);

};


#endif // MARSYASQT_INSPECTOR_DEBUG_CONTROLLER_INCLUDED
