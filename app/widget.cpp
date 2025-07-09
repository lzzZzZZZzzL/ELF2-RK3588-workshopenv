/*****************************************************************************/
/* 头文件                                                                     */
/*****************************************************************************/
#include "widget.h"

/*****************************************************************************/
/* 函数定义                                                                   */
/*****************************************************************************/
/* 程序启动时调用 */
Widget::Widget(QWidget *parent)
    : QWidget(parent), dht11_fd(-1), bh1750_fd(-1),
      dht11WarningShown(false), bh1750WarningShown(false),
      isPoseScriptRunning(false), actionScriptPaused(false),
      isFaceAttendanceRunning(false)
{
    poseScriptProcess = new QProcess(this);
    faceAttendanceProcess = new QProcess(this);
    UIInit::initUI(this);
    initDevices();

    //在程序启动时读取阈值文件
    if (!readThresholdFile())
    {
        //文件不存在或格式错误，使用默认值并写入文件
        writeThresholdFile();
    }

    //更新UI显示的阈值
    luxMinInput->setText(QString::number(luxMin));
    luxMaxInput->setText(QString::number(luxMax));
    tempMinInput->setText(QString::number(tempMin));
    tempMaxInput->setText(QString::number(tempMax));
    humidMinInput->setText(QString::number(humidMin));
    humidMaxInput->setText(QString::number(humidMax));

    //初始化摄像头状态文件
    initializeCameraStateFile();

    //BH1750数据更新定时器 (1秒)
    QTimer *bh1750Timer = new QTimer(this);
    connect(bh1750Timer, SIGNAL(timeout()), this, SLOT(updateBH1750Data()));
    bh1750Timer->start(1000);

    //DHT11数据更新定时器 (2秒)
    QTimer *dht11Timer = new QTimer(this);
    connect(dht11Timer, SIGNAL(timeout()), this, SLOT(updateDHT11Data()));
    dht11Timer->start(2000);

    //启动定时器更新时间
    QTimer *timeTimer = new QTimer(this);
    connect(timeTimer, SIGNAL(timeout()), this, SLOT(updateTime()));
    timeTimer->start(1000);

    //添加初始化完成的提示信息到日志和界面
    QString initMessage = "Qt程序初始化完成";
    writeOperationLog(initMessage);

    QDateTime initTime = QDateTime::currentDateTime();
    QString initTimeStr = initTime.toString("yyyy-MM-dd hh:mm:ss");
    QString initMessageWithTime = QString(">> %1 %2").arg(initTimeStr).arg(initMessage);
    warningTextEdit->append(initMessageWithTime);

    //设置窗口位置为屏幕左上角
    alignToScreenCorner();

    //创建定时器，每两秒写入数据到文件
    QTimer *dataWriteTimer = new QTimer(this);
    connect(dataWriteTimer, SIGNAL(timeout()), this, SLOT(writeDataToFile()));
    dataWriteTimer->start(2000);

    //连接阈值输入框的信号到槽函数
    connect(luxMinInput, &QLineEdit::textChanged, this, &Widget::updateThresholds);
    connect(luxMaxInput, &QLineEdit::textChanged, this, &Widget::updateThresholds);
    connect(tempMinInput, &QLineEdit::textChanged, this, &Widget::updateThresholds);
    connect(tempMaxInput, &QLineEdit::textChanged, this, &Widget::updateThresholds);
    connect(humidMinInput, &QLineEdit::textChanged, this, &Widget::updateThresholds);
    connect(humidMaxInput, &QLineEdit::textChanged, this, &Widget::updateThresholds);
}

/* 程序结束时调用 */
Widget::~Widget()
{
    if (isPoseScriptRunning)
    {
        poseScriptProcess->terminate();
        poseScriptProcess->waitForFinished();
    }

    if (isFaceAttendanceRunning)
    {
        faceAttendanceProcess->terminate();
        faceAttendanceProcess->waitForFinished();
    }

    if (dht11_fd >= 0) ::close(dht11_fd);
    if (bh1750_fd >= 0) ::close(bh1750_fd);

    updateCameraStateFile("0"); //程序结束时更新摄像头状态文件状态为0

    setWindowFlags(Qt::Widget);
    setWindowTitle("传感器数据监控");

    /* 关闭物联网模块 */
    QProcess process;
    process.start("killall", QStringList() << "mqtt");  //执行killall命令
    process.waitForFinished();  //等待进程结束

    writeThresholdFile();   //在程序结束时写入阈值文件
}

/* 初始化设备 */
void Widget::initDevices()
{
    dht11_fd = open(DHT11_DEV_NAME, O_RDWR);    //打开DHT11设备
    if(dht11_fd < 0)
    {
        QString warningMessage = "DHT11设备未找到！";
        writeOperationLog(warningMessage);

        //获取当前时间并显示在界面上
        QDateTime currentTime = QDateTime::currentDateTime();
        QString timeString = currentTime.toString("yyyy-MM-dd hh:mm:ss");
        QString warningMessageWithTime = QString(">> %1 %2").arg(timeString).arg(warningMessage);
        warningTextEdit->append(warningMessageWithTime);


        if(!dht11WarningShown)      //显示警告弹窗
        {
            QMessageBox msgBox(QMessageBox::Warning, "警告", warningMessage, QMessageBox::NoButton, this);
            QFont font = msgBox.font();
            font.setPointSize(14);
            msgBox.setFont(font);
            msgBox.exec();
            dht11WarningShown = true;
        }
    }

    //查找BH1750和MPU6050设备
    DIR *dir;
    struct dirent *entry;
    dir = opendir("/dev");
    if(dir == nullptr)
    {
        QString warningMessage = "无法打开设备目录！";
        writeOperationLog(warningMessage);

        //获取当前时间并显示在界面上
        QDateTime currentTime = QDateTime::currentDateTime();
        QString timeString = currentTime.toString("yyyy-MM-dd hh:mm:ss");
        QString warningMessageWithTime = QString(">> %1 %2").arg(timeString).arg(warningMessage);
        warningTextEdit->append(warningMessageWithTime);
        return;
    }

    while((entry = readdir(dir)) != nullptr)
    {
        if(strstr(entry->d_name, "bh1750"))
        {
            QString dev_path = QString("/dev/") + QString::fromUtf8(entry->d_name);
            bh1750_fd = open(dev_path.toStdString().c_str(), O_RDWR);
            if(bh1750_fd >= 0)
                break;
        }
    }
    closedir(dir);

    if(bh1750_fd < 0)
    {
        QString warningMessage = "BH1750设备未找到！";
        writeOperationLog(warningMessage);

        //获取当前时间并显示在界面上
        QDateTime currentTime = QDateTime::currentDateTime();
        QString timeString = currentTime.toString("yyyy-MM-dd hh:mm:ss");
        QString warningMessageWithTime = QString(">> %1 %2").arg(timeString).arg(warningMessage);
        warningTextEdit->append(warningMessageWithTime);

        if(!bh1750WarningShown)     //显示警告弹窗
        {
            QMessageBox msgBox(QMessageBox::Warning, "警告", warningMessage, QMessageBox::NoButton, this);
            QFont font = msgBox.font();
            font.setPointSize(14);
            msgBox.setFont(font);
            msgBox.exec();
            bh1750WarningShown = true;
        }
    }
}

/* 更新BH1750数据 */
void Widget::updateBH1750Data()
{
    if (bh1750_fd >= 0)
    {
        unsigned short light_data;
        if (read(bh1750_fd, &light_data, sizeof(unsigned short)) >= 0)
        {
            float lux = static_cast<float>(light_data) / 1.2f;
            lightDisplay->setText(QString::number(static_cast<double>(lux), 'f', 1) + " lx");

            //获取当前时间
            QDateTime currentTime = QDateTime::currentDateTime();
            QString timeString = currentTime.toString("yyyy-MM-dd hh:mm:ss");

            //从输入框获取当前阈值
            float luxMin = luxMinInput->text().toFloat();
            float luxMax = luxMaxInput->text().toFloat();

            //判断当前光照强度是否在阈值范围内
            bool isTooBright = lux > luxMax;
            bool isTooDark = lux < luxMin;

            if(isTooBright || isTooDark)
            {
                QString warningMessage;
                if (isTooBright)
                {
                    warningMessage = QString("Too Bright! 光照强度:%1 lx 上限:%2 lx").arg(lux, 0, 'f', 2).arg(luxMax, 0, 'f', 2);
                }
                else
                {
                    warningMessage = QString("Too Dark! 光照强度:%1 lx 下限:%2 lx").arg(lux, 0, 'f', 2).arg(luxMin, 0, 'f', 2);
                }

                //添加警告信息到QTextEdit控件
                QString warningMessageWithTime = QString(">> %1 %2").arg(timeString).arg(warningMessage);
                warningTextEdit->append(warningMessageWithTime);

                writeOperationLog(warningMessage);  //写入操作日志

                QStringList lines = warningTextEdit->toPlainText().split('\n');
                if(lines.size() > 12)   //限制警告信息的最大行数为12行
                {
                    QString newText = lines.mid(1).join('\n');
                    warningTextEdit->setPlainText(newText);
                }
            }
        }
    }
}

/* 更新DHT11数据 */
void Widget::updateDHT11Data()
{
    if (dht11_fd >= 0)
    {
        unsigned char DHT11_data[5];
        if(ioctl(dht11_fd, DHT11_READ_DATA, DHT11_data) >= 0)
        {
            if(DHT11_data[4] == 1)
            {
                //获取温度和湿度值
                float temperature = DHT11_data[2] + static_cast<float>(DHT11_data[3]) / 10.0f;
                float humidity = DHT11_data[0] + static_cast<float>(DHT11_data[1]) / 10.0f;

                dhtTempDisplay->setText(QString::number(temperature, 'f', 1) + "°C");
                dhtHumidDisplay->setText(QString::number(humidity, 'f', 1) + "%RH");

                //获取当前时间
                QDateTime currentTime = QDateTime::currentDateTime();
                QString timeString = currentTime.toString("yyyy-MM-dd hh:mm:ss");

                //从输入框获取当前阈值
                float tempMin = tempMinInput->text().toFloat();
                float tempMax = tempMaxInput->text().toFloat();
                float humidMin = humidMinInput->text().toFloat();
                float humidMax = humidMaxInput->text().toFloat();

                //判断温湿度是否超出阈值
                bool isTooHot = temperature > tempMax;
                bool isTooCold = temperature < tempMin;
                bool isTooWet = humidity > humidMax;
                bool isTooDry = humidity < humidMin;

                //若超出阈值，则生成警告信息
                if(isTooHot || isTooCold || isTooWet || isTooDry)
                {
                    QString warningMessage;

                    if(isTooHot)
                    {
                        warningMessage = QString("Too Hot! 温度:%1℃ 上限:%2℃").arg(temperature, 0, 'f', 1).arg(tempMax, 0, 'f', 1);
                    }
                    else if(isTooCold)
                    {
                        warningMessage = QString("Too Cold! 温度:%1℃ 下限:%2℃").arg(temperature, 0, 'f', 1).arg(tempMin, 0, 'f', 1);
                    }
                    else if(isTooWet)
                    {
                        warningMessage = QString("Too Wet! 湿度:%1%RH 上限:%2%RH").arg(humidity, 0, 'f', 1).arg(humidMax, 0, 'f', 1);
                    }
                    else if(isTooDry)
                    {
                        warningMessage = QString("Too Dry! 湿度:%1%RH 下限:%2%RH").arg(humidity, 0, 'f', 1).arg(humidMin, 0, 'f', 1);
                    }

                    //添加警告信息到QTextEdit控件
                    QString warningMessageWithTime = QString(">> %1 %2").arg(timeString).arg(warningMessage);
                    warningTextEdit->append(warningMessageWithTime);

                    writeOperationLog(warningMessage);      //写入操作日志

                    QStringList lines = warningTextEdit->toPlainText().split('\n');
                    if (lines.size() > 12)  //限制警告信息的最大行数为12行
                    {
                        QString newText = lines.mid(1).join('\n');
                        warningTextEdit->setPlainText(newText);
                    }
                }
            }
        }
    }
}

/* 更新时间显示 */
void Widget::updateTime()
{
    QDateTime currentTime = QDateTime::currentDateTime();
    QString timeString = currentTime.toString("yyyy-MM-dd hh:mm:ss");
    timeDisplay->setText(timeString);
}

/* 将窗口左上角对齐到屏幕左上角 */
void Widget::alignToScreenCorner()
{
    move(0, 0);
    resize(980, 540);
}

/* 写入操作日志 */
void Widget::writeOperationLog(const QString &logMessage)
{
    //获取当前时间
    QDateTime currentTime = QDateTime::currentDateTime();
    QString timeString = currentTime.toString("yyyy-MM-dd hh:mm:ss");
    QString monthStr = currentTime.toString("yyyyMM");    //用于目录名，例如 202506

    //构建日志文件路径，路径格式为/home/elf/sensor/log/202506log.txt
    QString logFilePath = QString("/home/elf/sensor/log/%1log.txt").arg(monthStr);

    //确保目录存在
    QDir dir(logFilePath.section('/', 0, -2));  //提取目录部分
    if (!dir.exists())
    {
        dir.mkpath(".");  //创建目录
    }

    //构建要写入的日志行
    QString logLine = QString("%1 %2").arg(timeString).arg(logMessage);

    //打开并写入日志文件
    QFile logFile(logFilePath);
    if(logFile.open(QIODevice::Append | QIODevice::Text))
    {
        QTextStream out(&logFile);
        out << logLine << "\n";
        logFile.close();
    }
    else
    {
        QMessageBox::warning(this, "错误", QString("无法打开日志文件 %1").arg(logFilePath));
    }
}

/* 将环境参数写入文件 */
void Widget::writeDataToFile()
{
    //获取当前时间
    QDateTime currentTime = QDateTime::currentDateTime();
    QString timeString = currentTime.toString("yyyy-MM-dd hh:mm:ss");
    QString dateStr = currentTime.toString("yyyyMMdd");     //用于文件名，例如 20250608
    QString monthStr = currentTime.toString("yyyyMM");      //用于目录名，例如 202506

    //构建传感器数据文件路径，例如 /home/elf/sensor/data/202506/20250608.txt
    QString dataFilePath = QString("/home/elf/sensor/data/%1/%2.txt").arg(monthStr).arg(dateStr);

    //获取传感器数据
    float temperature = dhtTempDisplay->text().replace("°C", "").toFloat();
    float humidity = dhtHumidDisplay->text().replace("%RH", "").toFloat();
    float lightIntensity = lightDisplay->text().replace(" lx", "").toFloat();

    //构建要写入的数据行
    QString dataLine = QString("%1 %2℃ %3%RH %4lx")
        .arg(timeString)
        .arg(temperature, 0, 'f', 2)
        .arg(humidity, 0, 'f', 2)
        .arg(lightIntensity, 0, 'f', 2);

    //确保目录存在
    QDir dir(dataFilePath.section('/', 0, -2));  //提取目录部分
    if(!dir.exists())
    {
        dir.mkpath(".");  //创建目录
    }

    //打开并写入传感器数据文件
    QFile dataFile(dataFilePath);
    if(dataFile.open(QIODevice::Append | QIODevice::Text))
    {
        QTextStream out(&dataFile);
        out << dataLine << "\n";
        dataFile.close();
    }
    else
    {
        QMessageBox::warning(this, "错误", QString("无法打开文件 %1").arg(dataFilePath));
    }

    //更新索引文件，内容为当前日期的年月日
    QString indexFilePath = INDEX_FILE_PATH;
    QFile indexFile(indexFilePath);
    if(indexFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&indexFile);
        out << dateStr;  //写入当前日期，例如 20250608
        indexFile.close();
    }
    else
    {
        QMessageBox::warning(this, "错误", QString("无法打开文件 %1").arg(indexFilePath));
    }
}

/* 动作识别 */
void Widget::togglePoseScript()
{
    QString scriptPath = POSE_RECO_FILE_PATH;

    if (!QFile::exists(scriptPath))
    {
        QString warningMessage = "找不到/无法打开动作识别脚本";
        QString formattedWarningMessage = QString(">> %1 %2").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")).arg(warningMessage);

        warningTextEdit->append(formattedWarningMessage);
        writeOperationLog(warningMessage);
        return;
    }

    if (!isPoseScriptRunning)
    {
        poseScriptProcess->start("python", QStringList() << scriptPath);
        if (poseScriptProcess->waitForStarted())
        {
            isPoseScriptRunning = true;
            actionRecognitionButton->setText("停止动作识别");

            //更新摄像头状态文件状态
            updateCameraStateFile("1");
            QString message = "动作识别已开启";
            QString formattedMessage = QString(">> %1 %2").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")).arg(message);
            warningTextEdit->append(formattedMessage);
            writeOperationLog(message);

            //连接输出信号到槽函数
            connect(poseScriptProcess, &QProcess::readyReadStandardOutput,
                    this, &Widget::handlePoseScriptOutput);

            //连接错误信号到槽函数
            connect(poseScriptProcess, &QProcess::readyReadStandardError, this, [=]() {
                QByteArray errorData = poseScriptProcess->readAllStandardError();
                QString errorMessage = QString::fromUtf8(errorData);
                QString formattedErrorMessage = QString(">> %1 动作识别脚本错误:\n%2").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")).arg(errorMessage);
                warningTextEdit->append(formattedErrorMessage);
                writeOperationLog(errorMessage);
            });
        }
        else
        {
            QByteArray errorData = poseScriptProcess->readAllStandardError();
            QString errorMessage = QString("无法启动动作识别脚本:\n%1").arg(QString::fromUtf8(errorData));
            QString formattedErrorMessage = QString(">> %1 %2").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")).arg(errorMessage);
            warningTextEdit->append(formattedErrorMessage);
            writeOperationLog(errorMessage);
            actionRecognitionButton->setText("动作识别");
        }
    }
    else
    {
        poseScriptProcess->terminate();
        if (poseScriptProcess->waitForFinished())
        {
            isPoseScriptRunning = false;
            actionRecognitionButton->setText("动作识别");

            //更新摄像头状态文件状态
            updateCameraStateFile("0");

            QString message = "动作识别已关闭";
            QString formattedMessage = QString(">> %1 %2").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")).arg(message);
            warningTextEdit->append(formattedMessage);
            writeOperationLog(message);
        }
        else
        {
            QString errorMessage = "无法停止动作识别脚本";
            QString formattedErrorMessage = QString(">> %1 %2").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")).arg(errorMessage);
            warningTextEdit->append(formattedErrorMessage);
            writeOperationLog(errorMessage);
        }
    }
}

/* 设置LED为亮 */
void Widget::turnOnLED()
{
    int fd = open(LED_NAME, O_RDWR);
    if (fd < 0)
    {
        perror("Open LED Failed!\n");
        return;
    }
    ioctl(fd, SET_LED_ON);
    ::close(fd);
}

/* 设置LED为亮 */
void Widget::turnOffLED()
{
    int fd = open(LED_NAME, O_RDWR);
    if (fd < 0)
    {
        perror("Open LED Failed!\n");
        return;
    }
    ioctl(fd, SET_LED_OFF);
    ::close(fd);
}

/* 处理动作识别脚本输出 */
void Widget::handlePoseScriptOutput()
{
    while (poseScriptProcess->canReadLine())
    {
        QString output = poseScriptProcess->readLine().trimmed();
        QString formattedOutputMessage = QString(">> %1 动作识别脚本输出: %2")
            .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
            .arg(output);

        warningTextEdit->append(formattedOutputMessage);
        writeOperationLog(output);

        //检查是否是触发信号
        if(output == "ACTION_TRIGGER:1")
        {
            turnOnLED();  //亮灯
        } 
        else
        {
            turnOffLED();  //灭灯
        }
    }
}

/* 人脸考勤 */
void Widget::toggleFaceAttendance()
{
    QString scriptPath = FACE_RECO_FILE_PATH;

    if(!QFile::exists(scriptPath))
    {
        QString warningMessage = "找不到/无法打开人脸考勤脚本";
        QString formattedWarningMessage = QString(">> %1 %2").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")).arg(warningMessage);

        warningTextEdit->append(formattedWarningMessage);
        writeOperationLog(warningMessage);
        return;
    }

    //断开之前的信号连接（如果存在），避免重复连接
    disconnect(faceAttendanceProcess, &QProcess::readyReadStandardOutput, this, nullptr);
    disconnect(faceAttendanceProcess, &QProcess::readyReadStandardError, this, nullptr);

    faceAttendanceProcess->start("python", QStringList() << scriptPath);

    if(faceAttendanceProcess->waitForStarted())
    {
        QString message = "人脸考勤已开启";
        QString formattedMessage = QString(">> %1 %2").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")).arg(message);
        warningTextEdit->append(formattedMessage);
        writeOperationLog(message);

        //连接输出信号到槽函数，用于捕获脚本的输出
        connect(faceAttendanceProcess, &QProcess::readyReadStandardOutput, this, [=]() {
            QByteArray outputData = faceAttendanceProcess->readAllStandardOutput();
            if(!outputData.isEmpty())  //检查是否有数据
            {
                QString outputMessage = QString::fromUtf8(outputData);
                QString formattedOutputMessage = QString(">> %1 人脸考勤脚本输出:\n%2").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")).arg(outputMessage);
                warningTextEdit->append(formattedOutputMessage);
                writeOperationLog(outputMessage);
            }
        });

        //连接错误信号到槽函数，用于捕获脚本的错误输出
        connect(faceAttendanceProcess, &QProcess::readyReadStandardError, this, [=]() {
            QByteArray errorData = faceAttendanceProcess->readAllStandardError();
            if(!errorData.isEmpty())  //检查是否有数据
            {
                QString errorMessage = QString::fromUtf8(errorData);
                QString formattedErrorMessage = QString(">> %1 人脸考勤脚本错误:%2").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")).arg(errorMessage);
                warningTextEdit->append(formattedErrorMessage);
                writeOperationLog(errorMessage);
            }
        });

        //脚本启动后立即返回，不再关注其关闭状态
        return;
    }
    else
    {
        QByteArray errorData = faceAttendanceProcess->readAllStandardError();
        QString errorMessage = QString("无法启动人脸考勤脚本: %1").arg(QString::fromUtf8(errorData));
        QString formattedErrorMessage = QString(">> %1 %2").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")).arg(errorMessage);
        warningTextEdit->append(formattedErrorMessage);
        writeOperationLog(errorMessage);
        faceAttendanceButton->setText("人脸考勤");
        isFaceAttendanceRunning = false;  //启动失败，重置状态标志
    }
}

/* 处理人脸考勤脚本错误 */
void Widget::handleFaceAttendanceError(QProcess::ProcessError error)
{
    QString errorMessage;
    switch (error)
    {
        case QProcess::FailedToStart:
            errorMessage = "人脸考勤脚本启动失败";
            break;
        case QProcess::Crashed:
            errorMessage = "人脸考勤脚本崩溃";
            break;
        case QProcess::Timedout:
            errorMessage = "人脸考勤脚本启动超时";
            break;
        case QProcess::WriteError:
            errorMessage = "向人脸考勤脚本写入数据时出错";
            break;
        case QProcess::ReadError:
            errorMessage = "从人脸考勤脚本读取数据时出错";
            break;
        default:
            errorMessage = "未知错误";
    }

    QString formattedErrorMessage = QString(">> %1 %2").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")).arg(errorMessage);
    warningTextEdit->append(formattedErrorMessage);
    writeOperationLog(errorMessage);

    if (error == QProcess::Crashed)     //如果脚本崩溃，重置按钮状态
    {
        isFaceAttendanceRunning = false;
        faceAttendanceButton->setText("人脸考勤");
    }
}

/* 人脸注册 */
void Widget::registerFace()
{
    FaceRegisterDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted)
    {
        QString id = dialog.getID();
        QString name = dialog.getName();

        QString scriptPath = FACE_REGI_FILE_PATH;
        if (!QFile::exists(scriptPath))
        {
            QString warningMessage = "找不到/无法打开人脸注册脚本";
            QString formattedWarningMessage = QString(">> %1 %2").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")).arg(warningMessage);
            warningTextEdit->append(formattedWarningMessage);
            writeOperationLog(warningMessage);
            return;
        }

        QStringList arguments;
        arguments << scriptPath << "--id" << id << "--name" << name;

        QProcess *registerProcess = new QProcess(this);

        connect(registerProcess, &QProcess::readyReadStandardError, [=]()
        {
            QByteArray errorData = registerProcess->readAllStandardError();
            QString errorMessage = QString::fromUtf8(errorData);
            QString formattedErrorMessage = QString(">> %1 人脸注册脚本错误:\n%2").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")).arg(errorMessage);
            warningTextEdit->append(formattedErrorMessage);
            writeOperationLog(errorMessage);
        });

        registerProcess->start("python", arguments);
        if(!registerProcess->waitForStarted())
        {
            QByteArray errorData = registerProcess->readAllStandardError();
            QString errorMessage = QString("无法启动人脸注册脚本: %1").arg(QString::fromUtf8(errorData));
            QString formattedErrorMessage = QString(">> %1 %2").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")).arg(errorMessage);
            warningTextEdit->append(formattedErrorMessage);
            writeOperationLog(errorMessage);
            registerProcess->deleteLater();
            return;
        }

        if(!registerProcess->waitForFinished())
        {
            QString errorMessage = "人脸注册脚本执行失败";
            QString formattedErrorMessage = QString(">> %1 %2").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")).arg(errorMessage);
            warningTextEdit->append(formattedErrorMessage);
            writeOperationLog(errorMessage);
            registerProcess->deleteLater();
            return;
        }

        QByteArray outputData = registerProcess->readAllStandardOutput();
        QString outputMessage = QString::fromUtf8(outputData);
        QString formattedOutputMessage = QString(">> %1 人脸注册脚本输出:\n%2").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")).arg(outputMessage);
        warningTextEdit->append(formattedOutputMessage);
        writeOperationLog(outputMessage);

        registerProcess->deleteLater();
    }
}

/* 更新摄像头状态文件内容 */
void Widget::updateCameraStateFile(const QString& statusLine)
{
    QString cameraStateFilePath = CAMERA_STATE_FILE_PATH;
    QFile cameraStateFile(cameraStateFilePath);

    //确保目录存在
    QDir dir(cameraStateFilePath.section('/', 0, -2));  //提取目录部分
    if(!dir.exists())
    {
        dir.mkpath(".");  //创建目录
    }

    if(cameraStateFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&cameraStateFile);
        out << statusLine;  //写入状态行（0或1）
        cameraStateFile.close();
    }
    else
    {
        QString errorMessage = QString("无法打开摄像头状态文件 %1").arg(cameraStateFilePath);
        QMessageBox::warning(this, "错误", errorMessage);
        writeOperationLog(errorMessage);
    }
}

/* 重设阈值按钮 */
void Widget::resetThresholds()
{
    //BH1750光照强度阈值
    luxMin = luxMinDefault;
    luxMax = luxMaxDefault;
    luxMinInput->setText(QString::number(luxMin));
    luxMaxInput->setText(QString::number(luxMax));

    //DHT11温度阈值
    tempMin = tempMinDefault;
    tempMax = tempMaxDefault;
    tempMinInput->setText(QString::number(tempMin));
    tempMaxInput->setText(QString::number(tempMax));

    //DHT11湿度阈值
    humidMin = humidMinDefault;
    humidMax = humidMaxDefault;
    humidMinInput->setText(QString::number(humidMin));
    humidMaxInput->setText(QString::number(humidMax));

    writeThresholdFile();   //写入阈值文件
}

/* 初始化摄像头状态文件内容为0 */
void Widget::initializeCameraStateFile()
{
    updateCameraStateFile("0");
}

/* 读取阈值设置文件 */
bool Widget::readThresholdFile()
{
    QFile thresholdFile(THRESHOLD_FILE_PATH);
    if(!thresholdFile.exists()) //文件不存在
    {
        return false;
    }

    if(!thresholdFile.open(QIODevice::ReadOnly | QIODevice::Text))  //打开文件失败
    {
        return false;
    }

    QTextStream in(&thresholdFile);
    QStringList lines = in.readAll().split('\n');
    thresholdFile.close();

    if(lines.size() < 1)    //文件内容格式错误
    {
        return false;
    }

    QStringList values = lines[0].split(' ');
    if(values.size() != 6)  //文件内容格式错误
    {
        return false;
    }

    //读取并设置阈值
    luxMin = values[0].toFloat();
    luxMax = values[1].toFloat();
    tempMin = values[2].toFloat();
    tempMax = values[3].toFloat();
    humidMin = values[4].toFloat();
    humidMax = values[5].toFloat();

    return true;
}

/* 写入阈值设置文件 */
void Widget::writeThresholdFile()
{
    //提取目录部分并确保目录存在
    QDir dir(QString(THRESHOLD_FILE_PATH).section('/', 0, -2));  //提取目录部分
    if(!dir.exists())
    {
        dir.mkpath(".");  //创建目录
    }

    //打开文件并写入阈值
    QFile thresholdFile(THRESHOLD_FILE_PATH);
    if(!thresholdFile.open(QIODevice::WriteOnly | QIODevice::Text)) //打开文件失败
    {
        return;
    }

    QTextStream out(&thresholdFile);
    out << luxMin << " " << luxMax << " " << tempMin << " " << tempMax << " " << humidMin << " " << humidMax;
    thresholdFile.close();
}

/* 更新阈值文件 */
void Widget::updateThresholds()
{
    bool ok;
    luxMin = luxMinInput->text().toFloat(&ok);
    if(!ok){ qDebug() << "无效的光照强度下限值"; return; }

    luxMax = luxMaxInput->text().toFloat(&ok);
    if(!ok){ qDebug() << "无效的光照强度上限值"; return; }

    tempMin = tempMinInput->text().toFloat(&ok);
    if(!ok){ qDebug() << "无效的温度下限值"; return; }

    tempMax = tempMaxInput->text().toFloat(&ok);
    if(!ok){ qDebug() << "无效的温度上限值"; return; }

    humidMin = humidMinInput->text().toFloat(&ok);
    if(!ok){ qDebug() << "无效的湿度下限值"; return; }

    humidMax = humidMaxInput->text().toFloat(&ok);
    if(!ok){ qDebug() << "无效的湿度上限值"; return; }

    writeThresholdFile();   //更新阈值文件
}
