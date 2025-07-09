import cv2
import os
import subprocess
import sys
import numpy as np
import joblib
import ast
import warnings
import shutil
warnings.filterwarnings("ignore")  # 忽略警告

# 动作类别映射
ACTION_MAPPING = {
    1: "lying",
    2: "sitting", 
    3: "squatting",
    4: "standing"
}

def main():
    if len(sys.argv) < 2:
        print("用法: python3 pose_detect_single.py 图片路径")
        return

    # 配置路径
    RKNN_DEMO_PATH = "/home/elf/action/rknn_yolov8_pose_demo/rknn_yolov8_pose_demo"
    MODEL_PATH = "/home/elf/action/rknn_yolov8_pose_demo/model/yolov8_pose.rknn"
    LIB_PATH = "/home/elf/action/rknn_yolov8_pose_demo/lib"
    CLASSIFIER_PATH = "/home/elf/action/rknn_yolov8_pose_demo/model/pose_classifier.pkl"
    SCALER_PATH = "/home/elf/action/rknn_yolov8_pose_demo/model/scaler.pkl"

    # 加载模型
    try:
        classifier = joblib.load(CLASSIFIER_PATH)
        scaler = joblib.load(SCALER_PATH)
        print("模型加载成功")
    except Exception as e:
        print(f"模型加载失败: {str(e)}")
        return

    img_path = sys.argv[1]
    if not os.path.exists(img_path):
        print(f"图片不存在: {img_path}")
        return

    # 设置环境变量
    env = os.environ.copy()
    env['LD_LIBRARY_PATH'] = LIB_PATH
    
    # 运行姿态检测
    result = subprocess.run(
        [RKNN_DEMO_PATH, MODEL_PATH, img_path],
        env=env,
        capture_output=True,
        text=True
    )
    
    if result.returncode != 0:
        print("姿态检测失败")
        print(result.stderr)
        return


    
    # 解析所有人体关键点和检测框
    all_keypoints = []
    all_boxes = []
    for line in result.stdout.split('\n'):
        line = line.strip()
        if not line:
            continue
            
        # 处理关键点行    
        if line.startswith("Keypoints:"):
            try:
                coords_str = line.split("Keypoints:")[1].strip()
                coord_list = ast.literal_eval(coords_str)
                keypoints = []
                for point in coord_list:
                    keypoints.extend(point)
                all_keypoints.append(keypoints)
            except:
                continue
                
        # 处理检测框行 (假设格式为"Box: [x1,y1,x2,y2]")
        elif line.startswith("Box:"):
            try:
                box_str = line.split("Box:")[1].strip()
                box = ast.literal_eval(box_str)
                all_boxes.append(box)
            except:
                continue
    
    if not all_keypoints:
        print("未检测到关键点")
        return

    # 加载可视化结果图片
    result_img = None
    if os.path.exists("out.png"):
        result_img = cv2.imread("out.png")

    try:
        # 处理每个检测到的人体
        for i, keypoints in enumerate(all_keypoints):
            print(f"\n第{i+1}个人体关键点数据({len(keypoints)}个值):")
            print(keypoints)

            # 转换为numpy数组并reshape
            keypoints = np.array(keypoints).reshape(-1, 3)  # 每个关键点(x,y,score)
            print(f"处理后关键点形状: {keypoints.shape}")
            
            # 检查维度是否为51 (17*3)
            if len(keypoints.flatten()) != 51:
                print(f"错误: 需要51个特征值(17个关键点的x,y,score)，实际得到{len(keypoints.flatten())}个")
                continue
                
            # 使用完整的51维数据进行标准化
            keypoints_scaled = scaler.transform(keypoints.flatten().reshape(1, -1))
            print(f"标准化后数据形状: {keypoints_scaled.shape}")
            
            # 预测动作
            predicted_action = classifier.predict(keypoints_scaled)[0]
            proba = classifier.predict_proba(keypoints_scaled)[0]
            
            action_text = ACTION_MAPPING.get(predicted_action, "unknown")
            print(f"\n第{i+1}个人体预测结果: {action_text}")
            print(f"各类别概率: {dict(zip(classifier.classes_, proba))}")

    except Exception as e:
        print(f"\n第{i+1}个人体预测过程中出错: {str(e)}")
    
    # 保存最终结果
    if os.path.exists("out.png"):
        output_path = os.path.splitext(img_path)[0] + "_result.png"
        shutil.move("out.png", output_path)
        print(f"\n结果已保存到: {output_path}")
        
if __name__ == "__main__":
    main()
