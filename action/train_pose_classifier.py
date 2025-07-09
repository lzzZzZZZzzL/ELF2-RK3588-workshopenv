import pandas as pd
import numpy as np
from sklearn.model_selection import train_test_split
from sklearn.ensemble import RandomForestClassifier
from sklearn.metrics import classification_report
from sklearn.preprocessing import StandardScaler
import joblib
import os

# 动作类别映射
ACTION_MAPPING = {
    1: "lying",
    2: "sitting", 
    3: "squatting",
    4: "standing"
}

def load_data(filepath):
    """加载并预处理数据"""
    df = pd.read_csv(filepath)
    
    # 检查数据列数
    if len(df.columns) != 52:
        raise ValueError("数据格式不正确，应为52列(17关键点*3 + 1标签)")
    
    # 分离特征和标签
    X = df.iloc[:, :-1].values  # 前51列为特征
    y = df.iloc[:, -1].values   # 最后一列为标签
    
    # 只保留已知类别的数据
    known_mask = np.isin(y, [1, 2, 3, 4])
    X = X[known_mask]
    y = y[known_mask]
    
    return X, y

def preprocess_features(X): # 特征预处理
    scaler = StandardScaler()
    X_scaled = scaler.fit_transform(X)
    return X_scaled, scaler

def train_model(X, y): # 训练随机森林分类器
    # 划分训练集和测试集
    X_train, X_test, y_train, y_test = train_test_split(
        X, y, test_size=0.2, random_state=42)
    
    # 初始化模型
    model = RandomForestClassifier(
        n_estimators=100,
        max_depth=10,
        random_state=42,
        class_weight='balanced'
    )
    
    # 训练模型
    model.fit(X_train, y_train)
    
    # 评估模型
    y_pred = model.predict(X_test)
    print("模型评估报告:")
    print(classification_report(y_test, y_pred, target_names=ACTION_MAPPING.values()))
    
    return model

def save_model(model, scaler, output_dir="models"):
    """保存模型和预处理对象"""
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
    
    joblib.dump(model, os.path.join(output_dir, "pose_classifier.pkl"))
    joblib.dump(scaler, os.path.join(output_dir, "scaler.pkl"))
    print(f"模型已保存到 {output_dir} 目录")

def main():
    # 加载数据
    try:
        X, y = load_data("data_total.csv")
    except Exception as e:
        print(f"数据加载失败: {e}")
        return
    
    # 预处理特征
    X_scaled, scaler = preprocess_features(X)
    
    # 训练模型
    model = train_model(X_scaled, y)
    
    # 保存模型
    save_model(model, scaler)
    
    print("训练完成!")

if __name__ == "__main__":
    main()