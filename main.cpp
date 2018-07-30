#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QStringList>
#include <QRegExp>
#include <iostream>
#include <hamlib/rotator.h>
#include  <QList>
#include <QMap>
#include <QHash>


using namespace  std;
const int MAX_DISTANCE = 10000; //km
const QString myLoc = "ko91ct";
const QString DIR_WITH_REPORTS = "output";
const QString CALLSIGN = "Callsign: ";
const QString QTH_LOCATOR = "QTH Locator: ";
const QString BAND_INFO_SEPARATOR = "------------------------------------------------";


struct qrb_param {
  double qrb;
  int az;
  qrb_param()
  {
    qrb=0;
    az=0;
  }
};

struct callsign_param
{
  QString call;
  QString qra;
  qrb_param qrb_info;
  callsign_param()
  {
    call="";
    qra="";
  }

};



bool checkDistance(const QString& loc1, const QString loc2, qrb_param &param);
callsign_param getCallSignParamFromFile(const QString filePath);
QSet<QString> getBandSet(const QString &filePath);
QString getBandFromStr(const QString& bandInfoStr);

int main(int argc, char *argv[])
{
  QCoreApplication a(argc, argv);


  //  QString testReqExp = "  145 MHz     114           16    3        29336";
  QMap<QString, QList<callsign_param> > bandMap;// = new QHash<QString, QList<callsign_param> >();


  QDir recoredDir(DIR_WITH_REPORTS+"");
  QStringList allFiles = recoredDir.entryList(QDir::Filter::Files,QDir::SortFlag::NoSort);

  QFile file(myLoc + " " + QString::number(MAX_DISTANCE)+"km");
  file.open(QIODevice::WriteOnly | QIODevice::Text);

  foreach (QString path, allFiles) {
    path = DIR_WITH_REPORTS+"/"+path;
    callsign_param cp = getCallSignParamFromFile(path);
    if(checkDistance(myLoc, cp.qra, cp.qrb_info))
    {
      QSet<QString> bandSet = getBandSet(path);
      foreach(QString band, bandSet)
      {
        bandMap[band].append(cp);
      }
    }
  }

  for(QMap<QString, QList<callsign_param> >::const_iterator it = bandMap.constBegin(); it != bandMap.constEnd(); ++it)
  {
    QList<callsign_param> listParam = it.value();

    QString strToFile = it.key() + " BAND. Calls: " + QString::number(listParam.count()) + " ***********************************************\n";
    file.write(strToFile.toUtf8());

    foreach (callsign_param cp, listParam) {
      QString result_str = cp.call + " " + cp.qra + " " + QString::number(cp.qrb_info.qrb) + " " + QString::number(cp.qrb_info.az) + " " +  "\n";
      file.write(result_str.toUtf8());
    }
  }

  file.close();

  cout << "END";
  return a.exec();
}

bool checkDistance(const QString& loc1, const QString loc2, qrb_param &param)
{
  double lon1(0),lat1(0), lon2(0), lat2(0);
  int result = locator2longlat(&lon1, &lat1, loc1.toStdString().c_str());
  if(result != RIG_OK)
  {
    std::cout << "Error calculate lon/lat for locator: "  << loc1.toStdString() << endl;
    return false;
  }
  result = locator2longlat(&lon2, &lat2, loc2.toStdString().c_str());
  if(result != RIG_OK)
  {
    std::cout << "Error calculate lon/lat for locator: "  << loc1.toStdString() << endl;
    return false;
  }

  double length(0),az(0);
  result = qrb(lon1, lat1, lon2, lat2, &length, &az);
  if(result != RIG_OK)
  {
    std::cout << "Error calculate qrb for locator: "  << loc1.toStdString() +" " + loc2.toStdString()<< endl;
    return false;
  }
  if(length <= MAX_DISTANCE)
  {
    param.qrb = length;
    param.az = az;
    return true;
  }
  return false;
}

callsign_param getCallSignParamFromFile(const QString filePath)
{
  QFile file(filePath);
  if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    cout<<"Error open file: " + filePath.toStdString()<< endl;
  }
  QString call = file.readLine();
  call = call.split(CALLSIGN).at(1);
  call = call.replace(call.length()-1,1,"");
  QString qth = file.readLine();
  qth = qth.split(QTH_LOCATOR).at(1);
  qth = qth.replace(qth.length()-1,1, "");

  callsign_param cp;
  if(!qth.isEmpty() && !call.isEmpty())
  {
    cp.call=call;
    cp.qra=qth;
  }
  file.seek(0);
  if(file.isOpen()) file.close();
  return cp;
}

QSet<QString> getBandSet(const QString& filePath)
{
  QSet<QString> bandSet;
  QFile file(filePath);
  if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    cout<<"Error open file: " + filePath.toStdString()<< endl;
    return bandSet;
  }
  QTextStream in(&file);

  while(!in.atEnd()) {
    QString line = in.readLine();
    if(line.compare(BAND_INFO_SEPARATOR) == 0 ) break;
    QString band = getBandFromStr(line);
    if(!band.isEmpty())
    {
      bandSet.insert(band);
    }
  }
  if(file.isOpen())file.close();
  return bandSet;
}

QString getBandFromStr(const QString& bandInfoStr)
{
  QRegExp re("([\\s]*)([\\d]*|[\\d*\\.]*)([\\s]*)(MHz|GHz)([\\s]*)([\\d]*)(.*)");
  if(re.exactMatch(bandInfoStr))
  {
    int QSO = re.cap(6).toInt();
    if(QSO > 0)
    {
      return re.cap(2);
    }
  }
  return "";
}
