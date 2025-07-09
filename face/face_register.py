import csv
import logging
import os
import time
from datetime import datetime
import cv2
import dlib
import numpy as np
from PIL import Image, ImageDraw, ImageFont
import argparse

class Employee: # 员工信息类
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
        filename='/home/elf/face/data/face_register.log',
        encoding='utf-8'
    )
except Exception as e:
    print(f"注册日志初始化出错: {str(e)}")
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
            logging.error(f"人脸注册运行识别，缺少必要文件: {file}")
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

def face_register(employee=None): # 人脸注册
    if not check_required_files():
        print("人脸注册运行失败，缺少必要文件！")
        logging.info(f"注册失败")
        return

    # 如命令行没输入员工信息则交互输入
    if employee is None:
        while True:
            emp_id = input("请输入员工ID: ").strip()
            if not emp_id:
                print("ID不能为空！")
                continue
                
            emp_name = input("请输入员工姓名: ").strip()
            if not emp_name:
                print("姓名不能为空！")
                continue
                
            employee = Employee(emp_id, emp_name)
            break

    # 保证目录存在
    register_photo_dir = '/home/elf/face/data/register_photos/'
    os.makedirs(register_photo_dir, exist_ok=True)

    # 初始化开发板摄像头
    try:
        cap = cv2.VideoCapture('/dev/video11')
        if not cap.isOpened():
            logging.error("无法打开摄像头")
            print("无法打开摄像头！")
            logging.info(f"注册失败")
            return

        # 设置分辨率
        cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
        cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)
    except Exception as e:
        logging.error(f"摄像头初始化失败: {str(e)}")
        print("摄像头初始化失败！")
        logging.info(f"注册失败")
        return

    print(f"\n请正对摄像头，系统将自动采集3张照片...")

    # 采集3张照片
    photos = []
    features = []
    
    for i in range(3):
        print(f"准备拍摄第 {i+1} 张照片...")
        ret, frame = cap.read()
        if not ret:
            print("拍照失败，请重试！")
            cap.release()
            logging.info(f"注册失败")
            return

        # 保证检测到人脸
        dets = face_detector(frame, 1)
        if len(dets) == 0:
            print(f"第 {i+1} 张照片未检测到人脸")
            continue
            
        try:
            # 人脸画框，标注员工信息
            face = dets[0]
            l, t, r, b = face.left(), face.top(), face.right(), face.bottom()
            
            # 边框颜色
            cv2.rectangle(frame, (l, t), (r, b), (0, 255, 0), 3)
            
            # 员工信息
            info_text = f"ID: {employee.id}\n姓名: {employee.name}"
            text_x = r + 20
            text_y = t + 40
    
            # 添加标注背景
            cv2.rectangle(frame, 
                        (text_x - 10, text_y - 10),
                        (text_x + 250, text_y + 60),
                        (245, 245, 245), -1)
    
            # 标注员工信息
            frame = cv2_put_cn_text(frame, info_text, (text_x, text_y), (255, 0, 0), 30)
    
            # 保存照片
            photo_path = f"{register_photo_dir}{employee.id}_{i+1}.jpg"
            cv2.imwrite(photo_path, frame, [cv2.IMWRITE_JPEG_QUALITY, 95])
            #logging.info(f"保存照片: {photo_path} (员工: {employee.name})")
            photos.append(photo_path)
            print(f"已拍摄第 {i+1} 张照片")
            
            # 提取人脸特征
            face_shape = face_sp(frame, face)
            face_descriptor = face_feature_model.compute_face_descriptor(frame, face_shape)
            features.append([x for x in face_descriptor])
                
        except Exception as e:
            print(f"人脸特征提取失败: {str(e)}")
            continue
            
        if i < 2:  # 延时1秒保证照片丰富性
            time.sleep(1)
    
    cap.release()
    
    # 检查三张照片是否都能提取特征
    if len(features) < 3:
        print("未能成功提取3张照片的人脸特征，注册失败！")
        # 清理已保存的照片
        for photo in photos:
            try:
                os.remove(photo)
            except:
                pass
        logging.info(f"注册失败")
        return
        
    # 计算三张照片的特征是否差异过大
    for i in range(len(features)-1):
        for j in range(i+1, len(features)):
            dist = np.linalg.norm(np.array(features[i]) - np.array(features[j]))
            if dist > 0.5: # 差异阈值
                print(f"3张照片的人脸特征差异过大，注册失败！")
                # 清理已保存的照片
                for photo in photos:
                    try:
                        os.remove(photo)
                    except:
                        pass
                logging.info(f"注册失败")
                return
    
    # 保存特征数据
    with open('/home/elf/face/data/feature.csv', 'a', newline="", encoding='utf-8-sig') as f:
        csv_f = csv.writer(f)
        for feature in features:
            feature_str = ','.join([str(x) for x in feature])
            csv_f.writerow([employee.id, employee.name, feature_str])
    
    for i in range(3):
        photo_path = f"{register_photo_dir}{employee.id}_{i+1}.jpg"
        logging.info(f"保存照片: {photo_path} (员工: {employee.name})")

    print(f"\n{employee.name} 注册成功！")


if __name__ == "__main__":
    try:
        # 确保数据目录存在
        if not os.path.exists('/home/elf/face/data'):
            os.makedirs('/home/elf/face/data')
        
        # 保证日志存在
        if not os.path.exists('/home/elf/face/data/attendance.log'):
            with open('/home/elf/face/data/attendance.log', 'w', newline='', encoding='utf-8') as f:
                writer = csv.writer(f)
                writer.writerow(['时间', '姓名'])
            logging.info("缺少日志文件，已创建新的日志文件")
        
        logging.info("员工注册系统启动")
        print("=== 人脸注册系统 ===")

        # 解析命令行参数
        parser = argparse.ArgumentParser(description='人脸注册系统')
        parser.add_argument('--id', type=str, help='员工ID')
        parser.add_argument('--name', type=str, help='员工姓名')
        args = parser.parse_args()

        if args.id and args.name:
            print(f"获取到员工信息: ID={args.id}, 姓名={args.name}")
            employee = Employee(args.id, args.name)
            face_register(employee) # 运行人脸注册
        else:
            face_register() # 运行人脸注册
        
    except Exception as e:
        logging.error(f"系统运行异常: {str(e)}")
        print(f"\n系统错误: {str(e)}")
    finally:
        logging.info("系统正常退出")
        print("\n系统已安全退出")
