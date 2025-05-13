# LibGPM

LibGPM 包含了我们的 CUDA 库的源代码，该库为 GPU 加速的可恢复应用程序提供了用户友好的接口。

## 库

### LibGPM (libgpm.cuh)

- （**host**）void *gpm_map_file(const char *path, size_t &len, bool create)
  - 如果 create 为 true，则在 path 处创建大小为 len 字节的内存映射文件。
  - 如果 create 为 false，则打开 path 处的内存映射文件，并将文件大小存储在 len 中。输入的 len 应为 0。
  - 返回指向 GPU 上持久化区域（大小为 len）的指针。
- （**host**）cudaError_t gpm_unmap(void \*addr, size_t len)
  - 解除起始指针为 addr、大小为 len 的内存映射变量的映射，并持久化所有数据。
  - 成功时返回 cudaSuccess，否则返回错误码。
- （**host**）void gpm_persist_begin(void)
  - 关闭 DDIO，允许后续 GPU 内核访问 PM 时进行内核内持久化。
- （**host**）void gpm_persist_end(void)
  - 打开 DDIO，不再保证内核内持久化。
- （**device**）void gpm_persist()
  - 保证调用线程对 PM 的所有先前写入都已持久化。
  - 通过 `__threadfence_system()` 实现。
- （**device**，**host**）cudaError_t gpm_memcpy_nodrain(void *gpmdest, const void *src, size_t len, cudaMemcpyKind kind)
  - 将 src 的数据以 len 字节大小复制到 gpmdest，但不持久化该区域。kind 变量指示 memcpy 类型（如 cudaMemcpyDeviceToDevice、cudaMemcpyHostToDevice 等）。
- （**device**，**host**）cudaError_t gpm_memset_nodrain(void \*gpmdest, unsigned char value, size_t len)
  - 从指针 gpmdest 开始，对大小为 len 的区域逐字节赋值 value，但不持久化该区域。
- （**device**，**host**）cudaError_t gpm_memcpy(void *pmemdest, const void *src, size_t len, cudaMemcpyKind kind)
  - 将 src 的数据以 len 字节大小复制到 gpmdest，完成后保证持久化。kind 变量指示 memcpy 类型（如 cudaMemcpyDeviceToDevice、cudaMemcpyHostToDevice 等）。
  - 不保证失败原子性。
- （**device**，**host**）cudaError_t pmem_memset(void \*pmemdest, int c, size_t len)
  - 从指针 gpmdest 开始，对大小为 len 的区域逐字节赋值 c，完成后保证持久化。
  - 不保证失败原子性。

### LibGPMLog (libgpmlog.cuh)

- TODO

### LibGPMCP (libgpmcp.cuh)

- TODO

## 源代码

本文件夹包含 6 个文件，说明如下：

- [libgpm.cuh](libgpm.cuh) - 包含 GPM 的主要实现细节，支持在 PMEM 上分配/释放。
- [libgpmlog.cuh](libgpmlog.cuh) - 包含 GPM 中日志（HCL 及常规）相关实现。
- [libgpmcp.cuh](libgpmcp.cuh) - 包含 GPM 中检查点相关实现。
- [bandwidth_analysis.cuh](bandwidth_analysis.cuh) - 包含带宽测量用的辅助定义。
- [change-ddio.h](change-ddio.h) - 包含用于打开/关闭 DDIO 的函数。GPM 对这些函数的封装见 libgpm。[1]
- [gpm-helper.cuh](gpm-helper.cuh) - 包含其他文件需要的通用辅助函数。

参考文献：
[1] Characterizing and Optimizing Remote Persistent Memory with RDMA and NVM. 作者：Xingda Wei, Xiating Xie, Rong Chen, Haibo Chen, Binyu Zang. 发表在：Usenix ATC'2021
