import csv
import logging
import os
import time
from datetime import datetime
import cv2
import dlib
import numpy as np
from PIL import Image, ImageDraw, ImageFont

class Employee: # 员工信息
    def __init__(self, emp_id, name):
        self.id = emp_id
        self.name = name

    def __str__(self):
        return f"员工ID: {self.id}, 姓名: {self.name}"

# 日志记录
try:
    logging.basicConfig(
        level=logging.INFO,
        format='%(asctime)s - %(levelname)s - %(message)s',
        filename='/home/elf/face/data/face_recognize.log',
        encoding='utf-8'
    )
except Exception as e:
    print(f"日志初始化失败: {str(e)}")
    logging.basicConfig(
        level=logging.INFO,
        format='%(asctime)s - %(levelname)s - %(message)s'
    )

def check_required_files(): # 检查模型文件
    required_files = [
        '/home/elf/face/weights/shape_predictor_68_face_landmarks.dat',
        '/home/elf/face/weights/dlib_face_recognition_resnet_model_v1.dat',
        '/home/elf/face/fonts/songti.ttc'
    ]
    for file in required_files:
        if not os.path.exists(file):
            logging.error(f"缺少必要文件: {file}")
            return False
    return True

# 初始化dlib
face_detector = dlib.get_frontal_face_detector()
face_sp = dlib.shape_predictor('/home/elf/face/weights/shape_predictor_68_face_landmarks.dat')
face_feature_model = dlib.face_recognition_model_v1('/home/elf/face/weights/dlib_face_recognition_resnet_model_v1.dat')

# 中文
def cv2_put_cn_text(img, text, position, text_color=(0, 255, 0), text_size=30):
    text_color_rgb = (text_color[2], text_color[1], text_color[0])
    img = Image.fromarray(cv2.cvtColor(img, cv2.COLOR_BGR2RGB))
    draw = ImageDraw.Draw(img)

    font_style = ImageFont.truetype("/home/elf/face/fonts/songti.ttc", text_size, encoding="utf-8")
    draw.text(position, text, text_color_rgb, font=font_style)

    return cv2.cvtColor(np.asarray(img), cv2.COLOR_RGB2BGR)

def load_employee_face_feature(): # 加载人脸数据
    all_employee = []
    all_face_feature = []

    with open('/home/elf/face/data/feature.csv', 'r', encoding='utf-8-sig') as f:
        csv_f = csv.reader(f)
        for line in csv_f:
            try:
                # 解析数据
                feature_str = line[2].strip('[]') if '[' in line[2] else line[2]
                feature = [float(x) for x in feature_str.split(',')]
                if len(feature) != 128:  # 验证特征维度
                    logging.warning(f"数据维度异常(工号:{line[0]}): {len(feature)}")
                    continue
                
                employee = Employee(line[0], line[1])
                face_feature = np.asarray(feature, dtype=np.float64).reshape((1, -1))
                
                all_employee.append(employee)
                all_face_feature.append(face_feature)
            except Exception as e:
                logging.warning(f"数据解析失败(工号:{line[0]}): {str(e)}")
                continue
                
        if not all_employee:
            logging.error("没有有效的数据")
            raise ValueError("没有有效的数据")
            
        logging.info(f"成功加载 {len(all_employee)} 条数据")

    return all_employee, np.vstack(all_face_feature)

def face_recognize(dist_threshold=0.5): # 人脸识别考勤功能
    if not check_required_files():
        print("人脸识别运行失败，缺少必要文件！")
        logging.info(f"打卡失败")
        return

    # 加载数据
    try:
        all_employee, all_employee_face_feature = load_employee_face_feature()
        if not all_employee:
            print("没有员工数据，请先进行人脸注册！")
            logging.info(f"打卡失败")
            return
    except Exception as e:
        logging.error(f"加载员工数据失败: {str(e)}")
        print("加载员工数据失败！")
        logging.info(f"打卡失败")
        return

    # 创建考勤照片目录
    attendance_photo_dir = '/home/elf/face/data/attendance_photos/'
    os.makedirs(attendance_photo_dir, exist_ok=True)

    try:
        cap = cv2.VideoCapture('/dev/video11')
        if not cap.isOpened():
            logging.error("无法打开摄像头")
            print("无法打开摄像头！")
            logging.info(f"打卡失败")
            return

        # 设置适合开发板的分辨率
        cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
        cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)

        print("请正对摄像头进行拍照识别...")

        # 捕获1张照片
        ret, frame = cap.read()
        if not ret:
            print("拍照失败，请重试！")
            logging.info(f"打卡失败")
            return
    except Exception as e:
        logging.error(f"摄像头异常: {str(e)}")
        print(f"摄像头异常: {str(e)}")
        logging.info(f"打卡失败")
        return

    # 检测人脸
    dets = face_detector(frame, 1)
    if len(dets) == 0:
        print("未检测到人脸！")
        logging.info(f"打卡失败")
        cap.release()
        return

    # 提取特征
    face = dets[0]
    face_shape = face_sp(frame, face)
    face_descriptor = face_feature_model.compute_face_descriptor(frame, face_shape)
    cur_face_feature = np.asarray([x for x in face_descriptor], dtype=np.float64)
    cur_face_feature = cur_face_feature.reshape((1, -1))

    # 计算距离
    distances = np.linalg.norm((cur_face_feature - all_employee_face_feature), axis=1)
    min_dist_index = np.argmin(distances)
    min_dist = distances[min_dist_index]

    # 识别结果
    if min_dist < dist_threshold:
        employee = all_employee[min_dist_index]
        face_rect_color = (0, 255, 0)  # 边框颜色
        
        # 人脸画框，标注员工信息
        l, t, r, b = face.left(), face.top(), face.right(), face.bottom()
        cv2.rectangle(frame, (l, t), (r, b), face_rect_color, 3)
        
        # 标注员工信息
        info_text = f"打卡成功\n姓名: {employee.name}\n工号: {employee.id}"
        text_x = r + 20
        text_y = t
        
        # 添加标注背景
        cv2.rectangle(frame, 
                     (text_x - 10, text_y - 10),
                     (text_x + 250, text_y + 100),
                     (245, 245, 245), -1)
        
        # 文字标注颜色
        frame = cv2_put_cn_text(frame, info_text, (text_x, text_y), (255, 0, 0), 30)

        # 保存识别照片
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        photo_path = f"{attendance_photo_dir}{timestamp}.jpg"
        cv2.imwrite(photo_path, frame, [cv2.IMWRITE_JPEG_QUALITY, 95])
        
        # 记录日志
        log_attendance(employee.name)
        logging.info(f"{employee.name} 打卡成功")
        
        print(f"\n=== 识别成功 ===")
        print(f"姓名: {employee.name}")
        print(f"工号: {employee.id}")
    else:
        print("\n=== 识别失败 ===")

    # 确保资源释放
    if 'cap' in locals() and cap.isOpened():
        cap.release()

def log_attendance(name): # 记录考勤日志
    now = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    try:
        with open('/home/elf/face/data/attendance.log', 'a', newline="", encoding='utf-8') as f:
            writer = csv.writer(f)
            writer.writerow([now, name])
            logging.info(f"考勤记录: {name} 于 {now}")
    except Exception as e:
        logging.error(f"记录考勤失败: {str(e)}")

if __name__ == "__main__":
    try:
        # 确保数据目录存在
        if not os.path.exists('/home/elf/face/data'):
            os.makedirs('/home/elf/face/data')
        
        # 保证日志存在
        attendance_file = '/home/elf/face/data/attendance.log'
        if not os.path.exists(attendance_file):
            with open(attendance_file, 'w', newline='', encoding='utf-8') as f:
                writer = csv.writer(f)
                writer.writerow(['时间', '姓名'])
        
        logging.info("程序启动，开始人脸识别")
        print("=== 人脸识别考勤 ===")
        
        # 运行人脸识别
        face_recognize(dist_threshold=0.5)
        
    except Exception as e:
        logging.error(f"系统运行异常: {str(e)}")
        print(f"\n系统错误: {str(e)}")
    finally:
        logging.info("系统正常退出")
        print("\n系统已安全退出")
