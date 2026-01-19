import torch
import torch.nn as nn
import torch.nn.functional as F
from functools import partial
import os
from pathlib import Path
import numpy as np
import time

class Attention(nn.Module):
    def __init__(self, dim, num_heads, qkv_bias=True, qk_scale=None, attn_drop=0., proj_drop=0.):
        super().__init__()
        self.num_heads = num_heads
        head_dim = dim // num_heads
        self.scale = qk_scale or head_dim ** -0.5

        self.qkv = nn.Linear(dim, dim * 3, bias=qkv_bias)
        self.attn_drop = nn.Dropout(attn_drop)
        self.proj = nn.Linear(dim, dim)
        self.proj_drop = nn.Dropout(proj_drop)
        self.softmax = nn.Softmax(dim=-1)

    def forward(self, x):
        B, N, C = x.shape
        print(f"Input shape: {x.shape}")  # Print input shape
        qkv = self.qkv(x).reshape(B, N, 3, self.num_heads, C // self.num_heads).permute(2, 0, 3, 1, 4)
        q, k, v = qkv[0], qkv[1], qkv[2]
        q = q * self.scale
        attn = (q @ k.transpose(-2, -1))
        attn = self.softmax(attn)
        attn = self.attn_drop(attn)
        x = (attn @ v).transpose(1, 2).reshape(B, N, C)
        x = self.proj(x)
        x = self.proj_drop(x)
        print(f"Output shape: {x.shape}")  # Print output shape
        return x

dim = 128
num_heads = 8
drop = 0.


# 选择设备：GPU 或 CPU
device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
file_path='../../input_attention.txt'

for i in range(2):
    input_numbers = np.array([])
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
    
    # 转换输入数据为 tensor，并确保数据类型为浮点数
    input_tensor = torch.tensor(input_numbers, dtype=torch.float32).to(device)

    # 创建模型实例并加载模型参数
    # 注意：需要先定义 Attention 类和其参数
    model = Attention(dim, num_heads=num_heads, qkv_bias=True, attn_drop=0., proj_drop=drop).to(device)

    # 设置模型为推理模式
    model.eval()

    # 进行推理，确保关闭梯度计算
    with torch.no_grad():
        output = model(input_tensor)

    # 输出结果并保存到txt文件
    output_data = output.detach().cpu().numpy()
    output_data = output_data.reshape(-1).tolist()
    
    with open('../../output_attention.txt', 'w') as f:
        for number in output_data:
            f.write(f"{number}\n")  # 将每个输出数字写入文件中，每个数字一行

