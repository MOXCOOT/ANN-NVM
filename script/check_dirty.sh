#!/bin/bash
# filepath: check_pmem_dirty.sh

# 获取PEMEM_PATH路径
PEMEM_PATH="/mnt/pmem2-ext4-dax/pool.pmem"

echo "检查持久内存池: $PEMEM_PATH"

# 检查文件是否存在
if [ ! -f "$PEMEM_PATH" ]; then
    echo "信息: 持久内存池文件不存在，继续执行"
    exit 0  # 文件不存在，继续执行后续操作
fi

# 使用pmempool info命令检查池状态
if ! command -v pmempool &> /dev/null; then
    echo "警告: pmempool命令未找到，无法检查dirty状态，继续执行"
    exit 0
fi

# 检查是否为dirty状态
DIRTY_STATUS=$(sudo pmempool info $PEMEM_PATH | grep -i "dirty")

if [ -n "$DIRTY_STATUS" ]; then
    echo "警告: 持久内存池处于dirty状态，退出脚本"
    sudo rm /mnt/pmem2-ext4-dax/*.pmem
    exit 1  # 池是dirty状态，退出脚本
else
    echo "持久内存池状态正常，继续执行"
    exit 0  # 池状态正常，继续执行后续操作
fi