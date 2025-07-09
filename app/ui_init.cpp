/*****************************************************************************/
/* 头文件                                                                     */
/*****************************************************************************/
#include "ui_init.h"
#include "widget.h"
#include <QObject>

/*****************************************************************************/
/* 函数定义                                                                   */
/*****************************************************************************/
void UIInit::initUI(Widget *widget)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(widget);
    mainLayout->setSpacing(10);

    /* 设置字体 */
    QFont font;
    font.setPointSize(16);

    /* BH1750数据区域上方的分隔线 */
    QFrame *topLine = new QFrame(widget);
    topLine->setFrameShape(QFrame::HLine);
    topLine->setFrameShadow(QFrame::Sunken);
    topLine->setLineWidth(1);
    topLine->setMidLineWidth(1);
    topLine->setFixedHeight(4);
    mainLayout->addWidget(topLine);

    /* BH1750数据区域和按钮布局 */
    QHBoxLayout *bh1750ButtonLayout = new QHBoxLayout();

    /* BH1750数据区域 */
    QHBoxLayout *bh1750Layout = new QHBoxLayout();
    QLabel *bh1750Title = new QLabel("BH1750数据", widget);
    bh1750Title->setFont(font);
    bh1750Title->setStyleSheet("font-weight: bold");
    bh1750Layout->addWidget(bh1750Title, 1);

    QLabel *lightLabel = new QLabel("光照强度：", widget);
    lightLabel->setFont(font);
    bh1750Layout->addWidget(lightLabel, 1);

    widget->lightDisplay = new QLabel("0.0 lx", widget);
    QFont numberFont("Times New Roman", 16);
    widget->lightDisplay->setFont(numberFont);
    bh1750Layout->addWidget(widget->lightDisplay, 1);

    /* 人脸注册按钮 */
    QPushButton *faceRegisterButton = new QPushButton("人脸注册", widget);
    faceRegisterButton->setFont(font);
    faceRegisterButton->setFixedSize(125, 40);
    faceRegisterButton->setEnabled(true);
    QObject::connect(faceRegisterButton, SIGNAL(clicked()), widget, SLOT(registerFace()));
    bh1750Layout->addWidget(faceRegisterButton, 1);

    /*人脸考勤按钮*/
    widget->faceAttendanceButton = new QPushButton("人脸考勤", widget);
    widget->faceAttendanceButton->setFont(font);
    widget->faceAttendanceButton->setFixedSize(125, 40);
    widget->faceAttendanceButton->setEnabled(true);
    QObject::connect(widget->faceAttendanceButton, SIGNAL(clicked()), widget, SLOT(toggleFaceAttendance()));
    bh1750Layout->addWidget(widget->faceAttendanceButton, 1);

    /* 动作识别按钮 */
    widget->actionRecognitionButton = new QPushButton("动作识别", widget);
    widget->actionRecognitionButton->setFont(font);
    widget->actionRecognitionButton->setFixedSize(125, 40);
    widget->actionRecognitionButton->setEnabled(true);
    widget->actionRecognitionButton->setStyleSheet("QPushButton { color: black; }"
                                                  "QPushButton:disabled { color: gray; }");
    QObject::connect(widget->actionRecognitionButton, SIGNAL(clicked()), widget, SLOT(togglePoseScript()));
    bh1750Layout->addWidget(widget->actionRecognitionButton, 1);

    bh1750ButtonLayout->addLayout(bh1750Layout);

    mainLayout->addLayout(bh1750ButtonLayout);
    mainLayout->addSpacing(5);

    /* 添加分隔线 */
    QFrame *line1 = new QFrame(widget);
    line1->setFrameShape(QFrame::HLine);
    line1->setFrameShadow(QFrame::Sunken);
    line1->setLineWidth(1);
    line1->setMidLineWidth(1);
    line1->setFixedHeight(4);
    mainLayout->addWidget(line1);

    /* DHT11数据区域 */
    QHBoxLayout *dhtLayout = new QHBoxLayout();
    QLabel *dhtTitle = new QLabel("DHT11数据", widget);
    dhtTitle->setFont(font);
    dhtTitle->setStyleSheet("font-weight: bold");
    dhtLayout->addWidget(dhtTitle, 1);

    QLabel *dhtTempLabel = new QLabel("温度：", widget);
    dhtTempLabel->setFont(font);
    dhtLayout->addWidget(dhtTempLabel, 1);

    widget->dhtTempDisplay = new QLabel("0.0 °C", widget);
    widget->dhtTempDisplay->setFont(numberFont);
    dhtLayout->addWidget(widget->dhtTempDisplay, 1);

    QLabel *dhtHumidLabel = new QLabel("湿度：", widget);
    dhtHumidLabel->setFont(font);
    dhtLayout->addWidget(dhtHumidLabel, 1);

    widget->dhtHumidDisplay = new QLabel("0.0 %RH", widget);
    widget->dhtHumidDisplay->setFont(numberFont);
    dhtLayout->addWidget(widget->dhtHumidDisplay, 1);

    mainLayout->addLayout(dhtLayout);
    mainLayout->addSpacing(5);

    /* 添加分隔线 */
    QFrame *line2 = new QFrame(widget);
    line2->setFrameShape(QFrame::HLine);
    line2->setFrameShadow(QFrame::Sunken);
    line2->setLineWidth(1);
    line2->setMidLineWidth(1);
    line2->setFixedHeight(4);
    mainLayout->addWidget(line2);

    /* 底部布局，包含警告消息显示区域和右侧布局（阈值设置、默认按钮和时间显示） */
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->setSpacing(10);

    /* 控件用于显示警告信息 */
    widget->warningTextEdit = new QTextEdit(widget);
    QFont warningFont = font;       //复制原来的字体
    warningFont.setPointSize(14);   //设置字体大小为14
    widget->warningTextEdit->setFont(warningFont); //使用新的字体
    widget->warningTextEdit->setStyleSheet("background-color: #FFF7F7; border: 1px solid #FF0000; padding: 5px;");
    widget->warningTextEdit->setReadOnly(true);
    widget->warningTextEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    widget->warningTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    bottomLayout->addWidget(widget->warningTextEdit, 3);

    /* 垂直布局来包含阈值设置、默认按钮和时间显示 */
    QVBoxLayout *rightLayout = new QVBoxLayout();
    rightLayout->setSpacing(10);

    /* 阈值设置区域 */
    QVBoxLayout *thresholdLayout = new QVBoxLayout();
    thresholdLayout->setSpacing(5);  //设置行间距

    /* BH1750光照强度阈值 */
    QHBoxLayout *luxTitleLayout = new QHBoxLayout();
    QLabel *luxLabel = new QLabel("光照强度阈值 (lx):", widget);
    luxLabel->setFont(font);
    luxTitleLayout->addWidget(luxLabel);
    thresholdLayout->addLayout(luxTitleLayout);

    /* 阈值上下限输入行 */
    QHBoxLayout *luxMinMaxLayout = new QHBoxLayout();
    QLabel *luxMinLabel = new QLabel("下限:", widget);
    luxMinLabel->setFont(font);
    luxMinMaxLayout->addWidget(luxMinLabel);
    widget->luxMinInput = new QLineEdit(widget);
    widget->luxMinInput->setFont(numberFont);
    widget->luxMinInput->setAlignment(Qt::AlignLeft);
    widget->luxMinInput->setFixedWidth(80);
    widget->luxMinInput->setText("150");
    luxMinMaxLayout->addWidget(widget->luxMinInput);

    QLabel *luxMaxLabel = new QLabel("上限:", widget);
    luxMaxLabel->setFont(font);
    luxMinMaxLayout->addWidget(luxMaxLabel);
    widget->luxMaxInput = new QLineEdit(widget);
    widget->luxMaxInput->setFont(numberFont);
    widget->luxMaxInput->setAlignment(Qt::AlignLeft);
    widget->luxMaxInput->setFixedWidth(80);
    widget->luxMaxInput->setText("500");
    luxMinMaxLayout->addWidget(widget->luxMaxInput);
    thresholdLayout->addLayout(luxMinMaxLayout);

    /* DHT11温度阈值 */
    QHBoxLayout *tempTitleLayout = new QHBoxLayout();
    QLabel *tempLabel = new QLabel("温度阈值 (°C):", widget);
    tempLabel->setFont(font);
    tempTitleLayout->addWidget(tempLabel);
    thresholdLayout->addLayout(tempTitleLayout);

    /* 阈值上下限输入行 */
    QHBoxLayout *tempMinMaxLayout = new QHBoxLayout();
    QLabel *tempMinLabel = new QLabel("下限:", widget);
    tempMinLabel->setFont(font);
    tempMinMaxLayout->addWidget(tempMinLabel);
    widget->tempMinInput = new QLineEdit(widget);
    widget->tempMinInput->setFont(numberFont);
    widget->tempMinInput->setAlignment(Qt::AlignLeft);
    widget->tempMinInput->setFixedWidth(80);
    widget->tempMinInput->setText("17");
    tempMinMaxLayout->addWidget(widget->tempMinInput);

    QLabel *tempMaxLabel = new QLabel("上限:", widget);
    tempMaxLabel->setFont(font);
    tempMinMaxLayout->addWidget(tempMaxLabel);
    widget->tempMaxInput = new QLineEdit(widget);
    widget->tempMaxInput->setFont(numberFont);
    widget->tempMaxInput->setAlignment(Qt::AlignLeft);
    widget->tempMaxInput->setFixedWidth(80);
    widget->tempMaxInput->setText("27");
    tempMinMaxLayout->addWidget(widget->tempMaxInput);
    thresholdLayout->addLayout(tempMinMaxLayout);

    /* DHT11湿度阈值 */
    QHBoxLayout *humidTitleLayout = new QHBoxLayout();
    QLabel *humidLabel = new QLabel("湿度阈值 (%RH):", widget);
    humidLabel->setFont(font);
    humidTitleLayout->addWidget(humidLabel);
    thresholdLayout->addLayout(humidTitleLayout);

    /* 阈值上下限输入行 */
    QHBoxLayout *humidMinMaxLayout = new QHBoxLayout();
    QLabel *humidMinLabel = new QLabel("下限:", widget);
    humidMinLabel->setFont(font);
    humidMinMaxLayout->addWidget(humidMinLabel);
    widget->humidMinInput = new QLineEdit(widget);
    widget->humidMinInput->setFont(numberFont);
    widget->humidMinInput->setAlignment(Qt::AlignLeft);
    widget->humidMinInput->setFixedWidth(80);
    widget->humidMinInput->setText("40");
    humidMinMaxLayout->addWidget(widget->humidMinInput);

    QLabel *humidMaxLabel = new QLabel("上限:", widget);
    humidMaxLabel->setFont(font);
    humidMinMaxLayout->addWidget(humidMaxLabel);
    widget->humidMaxInput = new QLineEdit(widget);
    widget->humidMaxInput->setFont(numberFont);
    widget->humidMaxInput->setAlignment(Qt::AlignLeft);
    widget->humidMaxInput->setFixedWidth(80);
    widget->humidMaxInput->setText("70");
    humidMinMaxLayout->addWidget(widget->humidMaxInput);
    thresholdLayout->addLayout(humidMinMaxLayout);

    rightLayout->addLayout(thresholdLayout);

    /* 阈值设置区域上方的分隔线 */
    QFrame *thresholdLine = new QFrame(widget);
    thresholdLine->setFrameShape(QFrame::HLine);
    thresholdLine->setFrameShadow(QFrame::Sunken);
    thresholdLine->setLineWidth(1);
    thresholdLine->setMidLineWidth(1);
    thresholdLine->setFixedHeight(4);
    rightLayout->addWidget(thresholdLine);

    /* 恢复默认警告阈值按钮和时间显示在同一行 */
    QHBoxLayout *buttonTimeLayout = new QHBoxLayout();
    QPushButton *resetThresholdsButton = new QPushButton("默认阈值设置", widget);
    resetThresholdsButton->setFont(font);
    buttonTimeLayout->addWidget(resetThresholdsButton, 1);
    QObject::connect(resetThresholdsButton, SIGNAL(clicked()), widget, SLOT(resetThresholds())); //使用 QObject::connect

    widget->timeDisplay = new QLabel(widget);
    widget->timeDisplay->setFont(font);
    widget->timeDisplay->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    buttonTimeLayout->addWidget(widget->timeDisplay, 1);

    rightLayout->addLayout(buttonTimeLayout);

    //设置阈值区域和按钮时间区域的垂直拉伸比例，总共7行
    rightLayout->setStretch(0, 6);  //thresholdLayout
    rightLayout->setStretch(2, 1);  //buttonTimeLayout

    /* 将右侧布局添加到底部布局 */
    bottomLayout->addLayout(rightLayout, 2);

    mainLayout->addLayout(bottomLayout);

    /* 设置各传感器部分的布局权重 */
    mainLayout->setStretchFactor(bh1750ButtonLayout, 1);
    mainLayout->setStretchFactor(dhtLayout, 1);
    mainLayout->setStretchFactor(bottomLayout, 8);

    widget->setWindowTitle("传感器数据监控");
    widget->resize(960, 520);
}
