import torch
import torch.nn as nn
import os
from pathlib import Path
import numpy as np
import time

# norm_layer可以是任何归一化层，例如LayerNorm
class NormLayer(nn.Module):
    def __init__(self, normalized_shape, eps=1e-6):
        super(NormLayer, self).__init__()
        self.layer_norm = nn.LayerNorm(normalized_shape, eps=eps)

    def forward(self, x):
        return self.layer_norm(x)


dim = 128
device = torch.device("cuda" if torch.cuda.is_available() else "cpu")

file_path = '../../input_norm.txt'

for i in range(2):
    input_numbers = np.array([])  # 初始化为空数组
    file_path_test = Path(file_path)
    
    # 检查文件是否存在，若不存在则等待
    while not file_path_test.exists():
        # print(f"文件 {file_path} 不存在，等待中...")
        time.sleep(1)  # 每秒检查一次文件是否存在
    
    print(f"文件 {file_path} 存在。")
    
    # 读取输入数据
    with open(file_path, 'r') as f:
        for line in f:
            input_numbers = np.append(input_numbers, float(line.strip()))  # 使用 np.append 追加数据
            
    # 删除文件
    if os.path.exists(file_path):
        os.remove(file_path)
        print(f"文件 {file_path} 已被删除。")
    else:
        print(f"文件 {file_path} 不存在。")
    
    # 检查并 reshape 输入数据
    if input_numbers.size == 50 * 128:
        input_numbers = input_numbers.reshape(1, 50, 128)
    else:
        print(f"数据大小不正确，无法 reshape 为 (1, 50, 128)，当前大小为 {input_numbers.size}")
        continue

    # 转换输入数据为 tensor，确保数据类型一致
    input_tensor = torch.tensor(input_numbers, dtype=torch.float32).to(device)

    # 创建模型实例并进行推理
    model = NormLayer(dim).to(device)
    output = model(input_tensor)

    # 输出结果并保存到txt文件
    output_data = output.detach().cpu().numpy()
    output_data = output_data.reshape(-1).tolist()
    
    with open('../../output_norm.txt', 'w') as f:
        for number in output_data:
            f.write(f"{number}\n")  # 将每个输出数字写入文件中，每个数字一行
