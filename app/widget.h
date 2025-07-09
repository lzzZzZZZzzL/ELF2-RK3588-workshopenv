#ifndef WIDGET_H
#define WIDGET_H

/*****************************************************************************/
/* 头文件                                                                    */
/*****************************************************************************/
#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>
#include <QMessageBox>
#include <QDateTime>
#include <QFrame>
#include <QString>
#include <QLineEdit>
#include <QPushButton>
#include <QFormLayout>
#include <QTextEdit>
#include <QFile>
#include <QTextStream>
#include <QIODevice>
#include <QDesktopWidget>
#include <QDir>
#include <QProcess>
#include <QDebug>

#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/ioctl.h>

#include "ui_init.h"
#include "facedialog.h"

/*****************************************************************************/
/* 宏定义                                                                     */
/*****************************************************************************/
#define DHT11_IOC_MAGIC 'k'
#define DHT11_READ_DATA _IOWR(DHT11_IOC_MAGIC, 1, unsigned char)
#define DHT11_DEV_NAME "/dev/dht11"

#define LED_NAME "/dev/my_device"   //LED设备路径
#define LED_IOC_MAGIC 'm'
#define SET_LED_ON _IO(LED_IOC_MAGIC, 0)
#define SET_LED_OFF _IO(LED_IOC_MAGIC, 1)

#define THRESHOLD_FILE_PATH     "/home/elf/sensor/threshold.txt"            //阈值设置文件路径
#define INDEX_FILE_PATH         "/home/elf/sensor/data/index.txt"           //索引文件路径
#define CAMERA_STATE_FILE_PATH  "/home/elf/sensor/data/camera_state.txt"    //摄像头状态文件路径
#define FACE_RECO_FILE_PATH     "/home/elf/face/face_recognize.py"          //人脸考勤程序路径
#define FACE_REGI_FILE_PATH     "/home/elf/face/face_register.py"           //人脸注册程序路径
#define POSE_RECO_FILE_PATH     "/home/elf/action/pose_infer_app.py"        //动作识别程序路径

/*****************************************************************************/
/* 声明                                                                      */
/*****************************************************************************/
class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void updateBH1750Data();        //更新光强数据
    void updateDHT11Data();         //更新温湿度数据
    void updateTime();              //更新时间
    void resetThresholds();         //阈值设置初始化
    void writeDataToFile();         //数据记录以及日志写入
    void togglePoseScript();        //动作识别
    void toggleFaceAttendance();    //人脸考勤槽函数
    void registerFace();            //人脸注册槽函数
    void handleFaceAttendanceError(QProcess::ProcessError error); //处理人脸考勤错误的槽函数,防止qt程不正常退出
    void handlePoseScriptOutput();  //处理动作识别脚本输出

private:
    friend class UIInit;

    void initUI();
    void initDevices();
    void alignToScreenCorner();
    void writeOperationLog(const QString &logMessage);
    void updateCameraStateFile(const QString& statusLine); //更新摄像头状态文件
    void initializeCameraStateFile(); //初始化摄像头状态文件
    bool readThresholdFile(); //读取阈值文件
    void writeThresholdFile(); //写入阈值文件
    void updateThresholds();
    void turnOnLED();
    void turnOffLED();

    QLabel *lightDisplay;
    QLabel *dhtTempDisplay, *dhtHumidDisplay;
    QLabel *timeDisplay;
    QLabel *warningDisplay;
    QTextEdit *warningTextEdit;

    QLineEdit *luxMinInput, *luxMaxInput;
    QLineEdit *tempMinInput, *tempMaxInput;
    QLineEdit *humidMinInput, *humidMaxInput;

    QPushButton *actionRecognitionButton;   //动作识别按钮
    QPushButton *faceAttendanceButton;      //人脸考勤按钮

    QProcess *poseScriptProcess;            //动作识别脚本进程
    QProcess *faceAttendanceProcess;        //人脸考勤进程

    QTimer *timer;

    int dht11_fd;
    int bh1750_fd;

    bool dht11WarningShown;
    bool bh1750WarningShown;
    bool isPoseScriptRunning;             //动作识别脚本运行状态
    bool actionScriptPaused;              //动作识别脚本是否被暂停的标志
    bool isFaceAttendanceRunning;        //人脸考勤运行状态标志

    //默认阈值
    float luxMinDefault = 200.0f;     //光照强度下限默认值
    float luxMaxDefault = 500.0f;     //光照强度上限默认值
    float tempMinDefault = 17.0f;     //温度下限默认值
    float tempMaxDefault = 28.0f;     //温度上限默认值
    float humidMinDefault = 40.0f;    //湿度下限默认值
    float humidMaxDefault = 70.0f;    //湿度上限默认值

    //当前阈值
    float luxMin = luxMinDefault;
    float luxMax = luxMaxDefault;
    float tempMin = tempMinDefault;
    float tempMax = tempMaxDefault;
    float humidMin = humidMinDefault;
    float humidMax = humidMaxDefault;
};

#endif
