#include "wifiqserial.h"
#include <QDebug>
#include <iostream>
#include <regex>

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

#include <QCryptographicHash>
#include <QTextCodec>

WifiQSerial* WifiQSerial::_wifi_qserial = nullptr;

WifiQSerial::WifiQSerial()
{


}

WifiQSerial::~WifiQSerial()
{
    if(m_pSerial) {
        delete m_pSerial; m_pSerial = nullptr;
    }
}


WifiQSerial* WifiQSerial::GetInstance()
{
    if(_wifi_qserial == nullptr) {
        _wifi_qserial = new WifiQSerial;
    }
    return _wifi_qserial;
}

void WifiQSerial::at_init()
{
    m_pSerial = new QSerialPort;
    m_pSerial->setPortName("/dev/ttyS4");
    m_pSerial->open(QIODevice::ReadWrite);
    m_pSerial->setBaudRate(QSerialPort::Baud115200);
    m_pSerial->setDataBits(QSerialPort::Data8);
    m_pSerial->setParity(QSerialPort::NoParity);
    m_pSerial->setStopBits(QSerialPort::OneStop);
    m_pSerial->setFlowControl(QSerialPort::NoFlowControl);
    connect(m_pSerial, SIGNAL(readyRead()), this, SLOT(onAcceptWifiSerialAck()));
    m_bIsConnected = WIFI_READY;
    m_pSendDataPackageSize = 1000;
    //this->at_cwmode(STATION);
}


void WifiQSerial::at_cifsr()
{
    m_nAt = AT_CIFSR;
    m_pRecvData.clear();
    QByteArray writeData;
    writeData.append(QString("AT+CIFSR\r\n"));
    m_pSerial->write(writeData);
}

void WifiQSerial::at_rst()
{
    m_bIsConnected = false;
    m_nAt = AT_RST;
    QByteArray writeData;
    writeData.append(QString("AT+RST\r\n"));
    m_pSerial->write(writeData);
}

/***
 * 设置WIFI模块工作方式
 * 返回值: 0、成功　>0、失败
 */
void WifiQSerial::at_cwmode(int mode)
{
    m_nAt = AT_CWMODE;
    QByteArray writeData;
    writeData.append(QString("AT+CIPMODE=%1\r\n").arg(mode));
    qDebug() << "write data" << writeData;
    m_pSerial->write(writeData);
}
/***
 * 加入网络
 * 参数１、wifi名字
 * 参数２、wifi密码
 * 返回值: 0、成功　>0、失败
 */
void WifiQSerial::at_cwjap(const char* wifi_name, const char* wifi_pswd)
{
    m_nAt = AT_CWJAP;
    QByteArray writeData;
    writeData.append(QString("AT+CWJAP=\"%1\",\"%2\"\r\n").arg(wifi_name).arg(wifi_pswd));
    m_pSerial->write(writeData);
}
/***
 * 退出网络
 */
void WifiQSerial::at_cwqap()
{
    m_nAt = AT_CWQAP;
    QByteArray cmdData;
    cmdData.append(QString("AT+CWQAP\r\n"));
    m_pSerial->write(cmdData);
}

/***
 * 获得连接状态
 * 返回值: 0、成功　>0、失败
 */
void WifiQSerial::send()
{
    m_nAt = AT_CIP_STATUS;
    m_bServerIsOk = false;
    QByteArray cmdData;
    cmdData.append(QString("AT+CIPSTATUS\r\n"));
    m_pSerial->write(cmdData);
}

/***
 *
 * 连接服务器
 */
void WifiQSerial::at_cipstart()
{
    m_nAt = AT_CIP_CONNECT;
    QByteArray connectData;
    connectData.append(QString("AT+CIPSTART=\"TCP\",\"www.baidu.com\",8800\r\n"));
    m_pSerial->write(connectData);
}

/***
 * 发送数据
 */

int WifiQSerial::at_cipsendcmd()
{
    if(m_bServerIsOk == true) {
        qDebug() << ":-----------> AT+CIPSEND";
        m_nAt = AT_CIP_SEND_CMD;
        QByteArray lengthData;
        if(m_pSendDataPackageCount == 1) {
            m_pSendDataCurrentLength = m_pSendData.length();
        }
        else {
            if(m_pSendDataCurrentCount < (m_pSendDataPackageCount - 1)) {
                m_pSendDataCurrentLength = m_pSendDataPackageSize;
            }
            else {
                m_pSendDataCurrentLength = m_pSendData.length() % m_pSendDataPackageSize;
            }
        }
        lengthData.append(QString("AT+CIPSEND=%1\r\n").arg(m_pSendDataCurrentLength));
        m_pSerial->write(lengthData);
        return 0;
    }
    return -1;
}

void WifiQSerial::at_cipsenddata()
{
    qDebug() << ":----> start send data";
    m_nAt = AT_CIP_SEND_DATA;
    QString writeString = QString::fromStdString(m_pSendData.toStdString()).mid(m_pSendDataCurrentCount * m_pSendDataPackageSize, m_pSendDataCurrentLength);
    m_pSendDataCurrentCount++;
    m_pSerial->write(QByteArray::fromStdString(writeString.toStdString()));
    m_pRecvData.clear();
    m_bRecvDataFlag = true;
}


void WifiQSerial::onAcceptWifiSerialAck()
{
    std::string ackString = m_pSerial->readAll().toStdString();
    if(!QString::fromStdString(ackString).isEmpty()) {
        qDebug() << QString::fromStdString(ackString);
    }
    if(m_nAt == AT_CIFSR) {
        m_pRecvData.append(ackString.c_str());
    }
    else if(m_nAt == AT_CWMODE) {
        if(ackString == "AT+CIPMODE=1\r\r\n\r\nOK\r\n"){
            qDebug() << "AT_CWMODE ok";
            this->at_rst();
        }
        else {
            qDebug() << "AT_CWMODE error";
        }
    }
    else if(m_nAt == AT_RST || m_nAt == AT_CWJAP)
    {
        if(ackString == "WIFI DISCONNECT\r\n"){
            m_bIsConnected = WIFI_DISCONNECT;
        }
        if(ackString == "WIFI GOT IP\r\n") {
            m_bIsConnected = WIFI_CONNECT;
        }
    }
    else if(m_nAt == AT_CIP_STATUS) {
        if(ackString.find("STATUS:3") != std::string::npos) {
            m_bServerIsOk = true;
            this->at_cipsendcmd();
        }
        else  {
            this->at_cipstart();
        }
    }
    else if(m_nAt == AT_CIP_CONNECT) {
        if(ackString == "CONNECT\r\n\r\nOK\r\n") {
            m_bServerIsOk = true;
            this->at_cipsendcmd();
        }
        else {
            m_bServerIsOk = false;
        }

    }
    else if(m_nAt == AT_CIP_SEND_CMD) {
        if(ackString.find("OK\r\n> ")  == std::string::npos) {
             //qDebug() << "AT_CIP_SEND_CMD error";
             m_bRecvDataFlag = false;
        }
        else {
            this->at_cipsenddata();
        }
    }
    else if(m_nAt == AT_CIP_SEND_DATA) {
        if(m_bRecvDataFlag) {
            m_pRecvData.append(ackString.c_str());
        }
        if(ackString.find("SEND OK") != std::string::npos && m_pSendDataCurrentCount < m_pSendDataPackageCount) {
            this->at_cipsendcmd();
        }
    }
}

int WifiQSerial::isConnected() const
{
    return m_bIsConnected;
}

bool WifiQSerial::isServerOK() const
{
    return m_bServerIsOk;
}

void WifiQSerial::setSendDeviceUploadData(QString lcokMac)
{
    //生成json字符串
    QJsonObject json;
    json.insert("lock_mac", lcokMac);

    QJsonDocument document;
    document.setObject(json);
    QByteArray byte_array = document.toJson(QJsonDocument::Compact);

    m_pSendData.clear();
    m_pSendData.append("POST /net-facelock/deviceupload HTTP/1.1\r\n");
    m_pSendData.append("Host: lock.wavewisdom-bj.com\n");
    m_pSendData.append("User-Agent: Fiddler\n");
    m_pSendData.append("Content-Type: application/json\n");
    m_pSendData.append("Content-Length: ");
    m_pSendData.append(QString::number(byte_array.length()));
    m_pSendData.append("\n\n");
    m_pSendData.append(byte_array);
    m_pSendData.append("\r\n\r\n");

    qDebug() << m_pSendData;
    qDebug() << __FUNCTION__ << m_pSendData.length();
    int count  = m_pSendData.length() / m_pSendDataPackageSize;
    m_pSendDataPackageCount = (m_pSendData.length() % m_pSendDataPackageSize) ? ++count : count;
    m_pSendDataCurrentCount = 0;
    qDebug() << "m_pSendDataPackageCount" << m_pSendDataPackageCount;
}

QString WifiQSerial::byteArrayToHexString(QByteArray bytes)
{
    QString retString(bytes.toHex().toUpper());
    int len = retString.length() / 2;
    for(int i = 0; i < len; i++) {
        qDebug() << i;
        retString.insert(2 * i + i -1, "");
    }
    return retString;
}

void WifiQSerial::setSendRecordUploadData(QString apiKey, QString apiSecret, QByteArray faceImage, int lockType, int userType, QString lockId, QString userName, int openResult, QString time)
{
    try {
        m_pRecvData.clear();
        qDebug() << ":-----> " << __FUNCTION__;
        //生成json字符串
        QJsonObject json;
        json.insert("api_key", apiKey);
        json.insert("api_secret", apiSecret);
        if(faceImage.isEmpty()) {
            json.insert("image_base64", "");
            json.insert("image_md5", "");
        }
        else {
            QByteArray image_base64 = faceImage.toBase64();
            json.insert("image_base64", QString::fromStdString(image_base64.toStdString()));
            QByteArray bb =  QCryptographicHash::hash(image_base64, QCryptographicHash::Md5);
            json.insert("image_md5", QString::fromStdString(bb.toHex().toStdString()));
        }
        json.insert("lock_type", lockType);
        json.insert("lock_user_type", userType);
        json.insert("lock_id", lockId);
        json.insert("user_name", userName);
        json.insert("open_lock_result", openResult);
        json.insert("open_time", time);

        QJsonDocument document;
        document.setObject(json);
        QByteArray byte_array = document.toJson(QJsonDocument::Compact);

        m_pSendData.clear();
        m_pSendData.append("POST /net-facelock/uploadrecord HTTP/1.1\r\n");
        m_pSendData.append("Host: lock.wavewisdom-bj.com\n");
        m_pSendData.append("User-Agent: Fiddler\n");
        m_pSendData.append("Content-Type: application/json\n");
        m_pSendData.append("Content-Length: ");
        m_pSendData.append(QString::number(byte_array.length()));
        m_pSendData.append("\n\n");
        m_pSendData.append(byte_array);
        m_pSendData.append("\r\n\r\n");

        qDebug() << "send data length " << m_pSendData.length();
        int count  = m_pSendData.length() / m_pSendDataPackageSize;
        m_pSendDataPackageCount = (m_pSendData.length() % m_pSendDataPackageSize) ? ++count : count;
        m_pSendDataCurrentCount = 0;
        qDebug() << "m_pSendDataPackageCount" << m_pSendDataPackageCount;
    }
    catch(...) {

    }
}

QByteArray WifiQSerial::getRecvData()
{
    return m_pRecvData;
}
