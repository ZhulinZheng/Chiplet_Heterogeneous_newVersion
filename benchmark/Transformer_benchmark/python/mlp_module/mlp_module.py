import torch
import torch.nn as nn
import math
import os
from pathlib import Path
import numpy as np
import time

class GELU(nn.Module):
    def __init__(self, approximate: str = 'none'):
        super(GELU, self).__init__()
        self.approximate = approximate

    def forward(self, x):
        if self.approximate == 'none':
            return 0.5 * x * (1.0 + torch.erf(x / math.sqrt(2.0)))
        elif self.approximate == 'tanh':
            return 0.5 * x * (1 + torch.tanh(math.sqrt(2 / math.pi) * (x + 0.044715 * torch.pow(x, 3))))

class Mlp(nn.Module):
    def __init__(self, in_features, hidden_features=None, out_features=None, act_layer=GELU, drop=0.):
        super().__init__()
        out_features = out_features or in_features
        hidden_features = hidden_features or in_features
        self.fc1 = nn.Linear(in_features, hidden_features)
        self.act = act_layer()
        self.fc2 = nn.Linear(hidden_features, out_features)
        self.drop = nn.Dropout(drop)

    def forward(self, x):
        x = self.fc1(x)
        x = self.act(x)
        x = self.drop(x)
        x = self.fc2(x)
        x = self.drop(x)
        return x

dim = 128
mlp_ratio = 4
drop = 0.

device = torch.device("cuda" if torch.cuda.is_available() else "cpu")

file_path = '../../input_mlp.txt'

for i in range(2):
    input_numbers = np.array([])
    file_path_test = Path(file_path)

    # 等待文件存在
    while not file_path_test.exists():
        time.sleep(1)

    print(f"文件 {file_path} 存在。")

    # 读取输入数据
    with open(file_path, 'r') as f:
        for line in f:
            input_numbers = np.append(input_numbers, float(line.strip()))  # 将每行的数字转换为浮点数

    # 删除文件
    try:
        os.remove(file_path)
        print(f"文件 {file_path} 已被删除。")
    except OSError as e:
        print(f"删除文件时发生错误: {e}")

    # 检查并 reshape 输入数据
    if input_numbers.size == 50 * 128:
        input_numbers = input_numbers.reshape(1, 50, 128).astype(np.float32)  # 确保数据为 float32
    else:
        print(f"数据大小不正确，无法 reshape 为 (1, 50, 128)，当前大小为 {input_numbers.size}")
        continue

    # 转换输入数据为 tensor
    input_tensor = torch.tensor(input_numbers, dtype=torch.float32).to(device)

    # 创建模型实例并进行推理
    model = Mlp(dim, int(dim * mlp_ratio), dim, drop=drop).to(device)
    output = model(input_tensor)

    # 输出结果并保存到 txt 文件
    output_data = output.detach().cpu().numpy().reshape(-1).tolist()

    with open('../../output_mlp.txt', 'w') as f:
        for number in output_data:
            f.write(f"{number}\n")  # 将每个输出数字写入文件中，每个数字一行
