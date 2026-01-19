import torch
import torch.nn as nn
import os
from pathlib import Path
import numpy as np
import time

def drop_path(x, drop_prob: float = 0.0, training: bool = False):
    if drop_prob == 0.0 or not training:
        return x
    keep_prob = 1 - drop_prob
    shape = (x.shape[0], ) + (1, ) * (x.ndim - 1)
    random_tensor = keep_prob + torch.rand(shape, dtype=x.dtype, device=x.device)
    random_tensor.floor_()
    output = x.div(keep_prob) * random_tensor
    return output

class DropPath(nn.Module):
    def __init__(self, drop_prob=None):
        super(DropPath, self).__init__()
        self.drop_prob = drop_prob

    def forward(self, x):
        return drop_path(x, self.drop_prob, self.training)

device = torch.device("cuda" if torch.cuda.is_available() else "cpu")

file_path = '../../input_droppath.txt'
for i in range(2):
    input_numbers=np.array([])
    file_path_test = Path(file_path)
    # 检查文件是否存在
    while not file_path_test.exists():
        # print(f"文件 {file_path} 不存在。")
        time.sleep(1)
    
    print(f"文件 {file_path} 存在。")
    # 读取输入数据
    with open(file_path, 'r') as f:
        for line in f:
            input_numbers = np.append(input_numbers, float(line.strip()))  # 将每行的数字转换为浮点数
            
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

    # 转换输入数据为tensor
    input_tensor = torch.tensor(input_numbers).to(device)

    # 创建模型实例并进行推理
    model = DropPath(drop_prob=0).to(device)
    output = model(input_tensor)

    # 输出结果并保存到txt文件
    output_data = output.detach().cpu().numpy()
    output_data = output_data.reshape(-1).tolist()
    
    with open('../../output_droppath.txt', 'w') as f:
        for number in output_data:
            f.write(f"{number}\n")  # 将每个输出数字写入文件中，每个数字一行