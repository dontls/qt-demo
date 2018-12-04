#ifndef WIFIQSERIAL_H
#define WIFIQSERIAL_H

#include <QObject>
#include <QSerialPort>
#include <QThread>
#include <QRegExp>

class WifiQSerial : QObject
{
    Q_OBJECT
public:
    WifiQSerial();
    virtual ~WifiQSerial();
    static WifiQSerial* GetInstance();

    enum {
        STATION = 1,
        AP,
        AP_STATION
    };

    enum {
        WIFI_READY,
        WIFI_CONNECT,
        WIFI_DISCONNECT
    };

    enum {
        AT_RST,
        AT_CWMODE,
        AT_CWLAP,
        AT_CWJAP,
        AT_CWQAP,
        AT_CIP_CONNECT,
        AT_CIP_STATUS,
        AT_CIP_CLOSE,
        AT_CIP_SEND_CMD,
        AT_CIP_SEND_DATA,
        AT_CIFSR
    };
public:
    void at_cwjap(const char* wifi_name, const char* wifi_pswd);
    void at_cwqap();
    void send();
    int at_cipsendcmd();
    void at_cipsenddata();
    void at_init();
    void at_cifsr();
    void at_rst();

    int  isConnected() const;
    bool isServerOK() const;
    void setSendDeviceUploadData(QString lcokMac);
    void setSendRecordUploadData(QString apiKey, QString apiSecret, QByteArray faceImage, int lockType, int userType, QString lockId, QString userName, int openResult, QString time);
    QByteArray getRecvData();
private:
    void at_cipstart();
    void at_cwmode(int mode);
    QString byteArrayToHexString(QByteArray bytes);
private:
    QSerialPort* m_pSerial;
    int          m_nAt;
    QByteArray   m_pSendData;
    QByteArray   m_pRecvData;
    bool         m_bRecvDataFlag;
    int          m_bIsConnected;
    bool         m_bServerIsOk;
    int          m_pSendDataPackageCount;
    int          m_pSendDataPackageSize;
    int          m_pSendDataCurrentCount;
    int          m_pSendDataCurrentLength;
    static WifiQSerial*    _wifi_qserial;
signals:
    void signalWifi(QString);

private slots:
    void onAcceptWifiSerialAck();
};

#endif // WIFIQSERIAL_H
