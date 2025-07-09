import numpy as np
import joblib
from sklearn.preprocessing import StandardScaler

# 动作类别映射
ACTION_MAPPING = {
    1: "lying",
    2: "sitting", 
    3: "squatting",
    4: "standing"
}

class PoseClassifier:
    def __init__(self, model_path="models/pose_classifier.pkl", scaler_path="models/scaler.pkl"):
        """初始化分类器"""
        try:
            self.model = joblib.load(model_path)
            self.scaler = joblib.load(scaler_path)
            print("模型加载成功")
        except Exception as e:
            raise Exception(f"模型加载失败: {e}")

    def predict(self, X, confidence_threshold=0.6):
        """
        预测动作类别
        参数:
            X: 输入数据 (n_samples, 51)
            confidence_threshold: 判断为unknown的置信度阈值
        返回:
            预测结果列表 (包含"unknown")
        """
        # 数据预处理
        X_scaled = self.scaler.transform(np.array(X).reshape(-1, 51))
        
        # 获取预测概率
        proba = self.model.predict_proba(X_scaled)
        
        # 获取最可能的类别和置信度
        pred_classes = self.model.classes_
        max_proba = np.max(proba, axis=1)
        pred_labels = np.argmax(proba, axis=1)
        
        # 应用阈值判断
        results = []
        for label, prob in zip(pred_labels, max_proba):
            if prob >= confidence_threshold:
                results.append(ACTION_MAPPING.get(pred_classes[label], "unknown"))
            else:
                results.append("unknown")
        
        return results

def main():
    # 示例用法
    classifier = PoseClassifier()
    
    # 示例数据 
    example_data = np.random.rand(2, 51)  # 2个样本, 51个特征
    
    # 预测
    predictions = classifier.predict(example_data)
    for i, pred in enumerate(predictions):
        print(f"样本{i+1}预测结果: {pred}")

if __name__ == "__main__":
    main()