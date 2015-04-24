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
    int data_size;
    Marsyas::realvec data;

    controlInformation(): data_size(1), data() {}

    controlInformation(int size): data_size(size), data() {}

    int& getDataSize() { return data_size; }
  };

  struct portInformation{
    int input_data_size;
    int output_data_size;
    Marsyas::realvec data_input;
    Marsyas::realvec data_output;

    int& getInputDataSize() { return input_data_size; }
    int& getOutputDataSize() { return output_data_size; }

    portInformation(): input_data_size(1), output_data_size(1), data_input(), data_output() {}

    portInformation(int input_size, int output_size): input_data_size(input_size), output_data_size(output_size), data_input(), data_output() {}
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

  const int getTickCount() const { return m_tick_count; }

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
