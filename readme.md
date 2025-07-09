# 智能环境监测与行为识别系统

## 项目概述
本项目是一个集环境监测、人脸识别和行为分析于一体的智能系统，主要应用于智能家居、安防监控等领域。系统通过多种传感器采集环境数据，结合计算机视觉技术实现人脸识别和危险行为检测，并通过物联网平台实现远程监控。

## 功能特点
1. **环境监测**
   - 实时采集光照强度、温湿度等环境数据
   - 阈值报警功能，当环境参数异常时触发报警

2. **人脸识别**
   - 员工人脸注册与管理
   - 实时人脸检测与识别
   - 考勤记录与日志

3. **行为分析**
   - 基于YOLOv8的人体姿态估计
   - 自定义危险行为识别
   - 实时报警与记录

4. **物联网集成**
   - 与OneNet平台对接
   - 数据远程监控
   - 手机APP远程控制

## 硬件要求
- 开发板：RK3588/RK3566等支持NPU的嵌入式平台
- 摄像头：支持USB或MIPI接口
- 传感器：
  - BH1750光照传感器
  - DHT11温湿度传感器
- LED指示灯

## 软件依赖
- 操作系统：Ubuntu
- Python 3.9
- 主要Python库：
  - OpenCV
  - dlib
  - scikit-learn
  - RKNN-Toolkit2
- C++环境：
  - QT
  - CMake

## 使用说明
1. **人脸注册**
   - 运行`face_register.py`进行员工人脸注册
   - 注册信息将保存在`face/data/register_photos`和`face/data/feature.csv`

2. **考勤识别**
   - 运行`face_recognize.py`启动人脸识别
   - 识别结果将记录在`face/data/attendance.log`

3. **行为监测**
   - 系统自动检测危险行为并记录
   - 报警信息显示在QT界面

4. **环境监测**
   - 实时显示环境数据
   - 可通过QT界面设置报警阈值

## 项目结构说明
```
├── action/            # 行为识别模块
│   ├── models/        # 行为分类模型
│   ├── rknn_yolov8_pose_demo/  # YOLOv8 RKNN推理程序
│   ├── logs/          # 行为识别日志
│   └── pose_infer_app.py  # 主程序
├── app/               # QT界面程序
│   ├── *.cpp          # 源代码
│   └── *.h            # 头文件
├── driver/            # 传感器驱动
│   ├── bh1750/        # 光照传感器
│   ├── dht11/         # 温湿度传感器
│   └── led/           # LED控制
├── face/              # 人脸识别模块
│   ├── data/          # 人脸数据和日志
│   ├── weights/       # 模型权重
│   ├── face_recognize.py  # 人脸识别
│   └── face_register.py   # 人脸注册
└── onenet/            # OneNet物联网平台集成
    ├── CMakeLists.txt # 构建配置
    └── src/           # 源代码
```
