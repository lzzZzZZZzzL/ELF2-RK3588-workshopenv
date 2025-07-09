import cv2
import os
import subprocess
import time
import shutil
import re
import numpy as np
import joblib
from concurrent.futures import ThreadPoolExecutor
import warnings
import signal
warnings.filterwarnings("ignore")  # 忽略警告

import logging
from datetime import datetime

# 配置日志
logging.basicConfig(
    filename='/home/elf/action/logs/action_recognition.log',
    level=logging.INFO,
    format='%(asctime)s - %(message)s'
)

# 动作类别映射
ACTION_MAPPING = {
    1: "lying",
    2: "sitting", 
    3: "squatting",
    4: "standing"
}

# 危险动作类型
LOG_ACTIONS = {"lying", "standing"}

# 配置路径
RKNN_DEMO_PATH = "/home/elf/action/rknn_yolov8_pose_demo/rknn_yolov8_pose_demo"
MODEL_PATH = "/home/elf/action/rknn_yolov8_pose_demo/model/yolov8_pose.rknn"
LIB_PATH = "/home/elf/action/rknn_yolov8_pose_demo/lib"
CLASSIFIER_PATH = "/home/elf/action/rknn_yolov8_pose_demo/model/pose_classifier.pkl"
SCALER_PATH = "/home/elf/action/rknn_yolov8_pose_demo/model/scaler.pkl"

# 处理间隔配置
PROCESS_INTERVAL = 1  # 处理间隔
EXIT_FLAG = False       # 全局退出标志

# 信号传输
def signal_handler(sig, frame):
    global EXIT_FLAG
    print("\n接收到退出信号，正在清理资源...")
    EXIT_FLAG = True

# 注册信号处理器
signal.signal(signal.SIGINT, signal_handler)  # Ctrl+C
signal.signal(signal.SIGTERM, signal_handler)

# 加载模型
try:
    classifier = joblib.load(CLASSIFIER_PATH)
    scaler = joblib.load(SCALER_PATH)
    print("成功模型")
except Exception as e:
    print(f"加载模型失败: {str(e)}")
    exit(1)

# 创建输出目录
#os.makedirs("raw_frames", exist_ok=True)       # 原始照片
#os.makedirs("processed_frames", exist_ok=True) # 识别结果
#os.makedirs("logs", exist_ok=True)             # 日志文件
os.makedirs("/home/elf/action/raw_frames", exist_ok=True)       # 原始照片
os.makedirs("/home/elf/action/processed_frames", exist_ok=True) # 识别结果
os.makedirs("/home/elf/action/logs", exist_ok=True)             # 日志文件

max_workers = 2     # 最大并发任务数

last_trigger_time = 0
TRIGGER_COOLDOWN = 5  # 触发冷却时间

def send_trigger_signal(): # 发送信号给QT
    global last_trigger_time
    current_time = time.time()
    
    # 检查冷却期
    if current_time - last_trigger_time >= TRIGGER_COOLDOWN:
        # 发送触发信号
        print("ACTION_TRIGGER:1", flush=True)
        #print(f"已发送触发信号给QT程序 (时间: {current_time})")
        last_trigger_time = current_time
    else:
        #print(f"触发冷却中，跳过发送信号 (剩余冷却: {TRIGGER_COOLDOWN - (current_time - last_trigger_time):.1f}秒)")
        print(f"触发冷却中")

def process_image(img_path): # 识别动作
    try:
        env = os.environ.copy()
        env['LD_LIBRARY_PATH'] = LIB_PATH
        
        result = subprocess.run(
            [RKNN_DEMO_PATH, MODEL_PATH, img_path],
            env=env,
            capture_output=True,
            text=True
        )
        
        if result.returncode == 0:
            print(f"识别成功: {img_path}")
            output_text = result.stdout
            
            # 解析关键点坐标
            keypoints = []
            for line in output_text.split('\n'):
                if "Keypoints:" in line:
                    try:
                        coords_str = line.split("Keypoints:")[1].strip()
                        coords = re.findall(r"[-+]?\d*\.\d+|\d+", coords_str)
                        if len(coords) >= 51:  # 确保有足够的数据点
                            keypoints = [float(x) for x in coords[:51]] 
                        else:
                            print(f"警告: 关键点数据不足，需要51个值，实际得到{len(coords)}个")
                    except Exception as e:
                        print(f"解析关键点出错: {str(e)}")
                    break
            
            # 初始化动作变量
            predicted_action = "Unknown"
            
            # 使用识别动作
            if len(keypoints) > 0:
                try:
                    # 转换为numpy数组
                    keypoints = np.array(keypoints).reshape(-1, 3) 
                    
                    # 检查是否完整
                    if len(keypoints.flatten()) != 51:
                        #print(f"错误: 需要51个特征值(17个关键点)，实际得到{len(keypoints.flatten())}个")
                        predicted_action = "InvalidKeypoints"
                    else:
                        # 标准化
                        keypoints_scaled = scaler.transform(keypoints.flatten().reshape(1, -1))
                        
                        # 预测动作
                        predicted_class = classifier.predict(keypoints_scaled)[0]
                        proba = classifier.predict_proba(keypoints_scaled)[0]
                        
                        # 获取动作名称和概率
                        predicted_action = ACTION_MAPPING.get(predicted_class, "Unknown")
                        
                        # 只对特定动作进行输出和记录
                        if predicted_action in LOG_ACTIONS:
                            proba_details = {ACTION_MAPPING.get(cls, str(cls)): prob for cls, prob in zip(classifier.classes_, proba)}
                            
                            # 输出结果
                            print(f"\n检测到动作: {predicted_action}")
                            #print(f"各类别概率: {proba_details}")
                            
                            # 记录日志
                            #logging.info(f"检测到动作: {predicted_action}, 概率详情: {proba_details}")
                            logging.info(f"检测到动作: {predicted_action}")
                            
                            # 发送信号给QT
                            send_trigger_signal()
                    
                except Exception as e:
                    print(f"\n动作预测过程中出错: {str(e)}")
                    predicted_action = "Error"
            
            # 保存图片
            if os.path.exists("out.png"):
                timestamp = time.strftime("%Y%m%d_%H%M%S")
                output_path = f"/home/elf/action/processed_frames/result_{timestamp}.png"
                
                # 添加预测结果到图片
                img = cv2.imread("out.png")
                if img is not None:
                    #cv2.putText(img, f"Action: {predicted_action}", (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 2) # 标注动作
                    cv2.imwrite(output_path, img)
                    print(f"保存结果: {output_path}")
                else:
                    shutil.move("out.png", output_path)
        else:
            print(f"识别失败: {img_path}")
            print(result.stderr)
            
    except Exception as e:
        print(f"处理图片出错: {img_path}, 错误: {str(e)}")

# 线程池处理动作识别
executor = ThreadPoolExecutor(max_workers=max_workers)

# 初始化摄像头
cap = cv2.VideoCapture('/dev/video11')
if not cap.isOpened():
    print("无法打开摄像头")
    exit()

start_time = time.time()
#print(f"程序启动于: {time.ctime(start_time)}")
#print(f"处理间隔: {PROCESS_INTERVAL}秒")
#print("使用以下命令停止程序:")
#print("  kill -15 <PID>")
#print("或发送Ctrl+C信号")

try:
    last_process_time = time.time()
    
    while not EXIT_FLAG:
        ret, frame = cap.read()
        if not ret:
            print("无法获取帧")
            break
        
        current_time = time.time()
        
        # 定时处理
        if current_time - last_process_time >= PROCESS_INTERVAL:
            timestamp = time.strftime("%Y%m%d_%H%M%S")
            img_path = f"/home/elf/action/raw_frames/frame_{timestamp}.jpg"
            cv2.imwrite(img_path, frame)
            print(f"保存原始图片: {img_path}")
            
            # 提交处理任务
            executor.submit(process_image, img_path)
            last_process_time = current_time
        
        time.sleep(0.01)

finally:
    # 清理资源
    #print("正在释放摄像头资源...")
    cap.release()
    
    #print("正在关闭线程池...")
    executor.shutdown(wait=True)
    
    #print("清理OpenCV资源...")
    cv2.destroyAllWindows()
    
    end_time = time.time()
    duration = end_time - start_time
    #print(f"程序运行时间: {duration:.2f}秒")
    print("程序已安全停止")
