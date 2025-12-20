# 喷涂轨迹规划系统

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/Platform-Windows-blue.svg)](https://www.microsoft.com/windows)
[![Qt](https://img.shields.io/badge/Qt-6.10+-green.svg)](https://www.qt.io/)
[![VTK](https://img.shields.io/badge/VTK-9.2-orange.svg)](https://vtk.org/)
[![OpenCASCADE](https://img.shields.io/badge/OpenCASCADE-7.8-red.svg)](https://www.opencascade.com/)

> 基于Qt、VTK和OpenCASCADE的工业级喷涂轨迹规划系统，支持复杂3D模型的STEP文件加载、点云处理和轨迹生成。

## 🚀 快速开始

### 新开发者入门

1. **阅读部署指南**
   ```bash
   # 详细的环境搭建指南
   docs/开发环境部署指南.md
   ```

2. **运行环境配置助手**（这个慎用，还是自己配好点）
   ```bash
   # Windows环境配置脚本
   scripts/setup_environment.bat
   ```

3. **验证环境配置**
   ```bash
   # 环境验证脚本
   scripts/verify_setup.bat
   ```

4. **编译项目**
   ```bash
   mkdir build
   cd build
   cmake .. -G "Visual Studio 17 2022" -A x64 -DQt6_DIR="K:/Kapps/Qt/6.10.1/msvc2022_64/lib/cmake/Qt6"
   cmake --build . --config Debug
   ```

### 核心功能

- ✅ **异步STEP文件加载**: 支持大型CAD模型的非阻塞加载
- ✅ **VTK 3D可视化**: 高性能的3D模型渲染和交互
- ✅ **点云处理**: PCL集成的点云数据处理
- ✅ **质量选择**: 快速预览/平衡模式/高质量三种加载模式
- ✅ **进度反馈**: 实时的加载进度和性能统计
- ✅ **系统日志**: 完整的操作日志和错误追踪

## 🛠️ 技术栈

### 核心框架
- **Qt 6.10+**: 现代化的跨平台UI框架
- **VTK 9.2**: 专业的3D可视化工具包
- **OpenCASCADE 7.8**: 工业级CAD几何内核
- **PCL 1.13**: 点云处理库

### 开发工具
- **CMake 3.16+**: 跨平台构建系统
- **Visual Studio 2019/2022**: Windows开发环境
- **Git**: 版本控制系统

## 📁 项目结构

```
qtSpraySystem/
├── config/                 # 配置文件
│   └── paths.cmake         # 第三方库路径配置
├── docs/                   # 项目文档
│   ├── 开发环境部署指南.md  # 新开发者必读
│   └── ...                # 技术文档
├── scripts/                # 辅助脚本
│   ├── setup_environment.bat    # 环境配置助手
│   └── verify_setup.bat        # 环境验证脚本
├── tests/                # 测试脚本
│   └── programs/         # 测试程序（独立于主程序）
│   └── ...                # py和bat的测试脚本
├── src/                    # 源代码
│   ├── UI/                 # 用户界面
│   ├── Data/               # 数据处理
│   ├── Core/               # 核心功能
│   └── ...
├── data/                   # 测试数据
│   └── model/              # 测试模型文件
├── build/                  # 构建输出目录
└── CMakeLists.txt          # 主构建配置
```

## 🔧 配置说明

### 路径配置

所有第三方库的路径都集中在 `config/paths.cmake` 文件中：

```cmake
# 修改这个基础路径为你的工具安装目录
set(TOOLS_BASE_DIR "K:/Tools" CACHE PATH "工具基础安装目录")
```

### 支持的路径结构

```
K:/Tools/                    # 可以是任意盘符和目录
├── vtkQT/
│   └── build/              # VTK编译输出
├── OpenCasCade/
│   └── install/            # OpenCASCADE安装目录
└── ...
```

## 🧪 测试验证

### 功能测试

1. **基础功能测试**
   ```bash
   build/bin/Debug/SprayTrajectoryPlanning.exe
   ```

2. **STEP文件加载测试**
   - 导入 `data/model/MPX3500.STEP`
   - 验证异步加载和进度显示
   - 检查3D渲染效果

3. **性能测试**
   - 大型STEP文件加载时间
   - 内存使用情况
   - UI响应性

### 测试脚本

```bash
# UI事件循环修复验证
测试UI事件循环修复.bat

# TransferRoots阻塞问题验证
测试TransferRoots修复.bat
```

## 📚 开发文档

### 核心文档
- [开发环境部署指南](docs/开发环境部署指南.md) - 新开发者必读
- [异步STEP加载优化方案](docs/异步STEP加载优化方案.md) - 核心技术实现
- [主程序UI事件循环阻塞问题修复](docs/主程序UI事件循环阻塞问题修复.md) - 重要bug修复

### 技术文档
- [VTK集成指南](docs/vtk-integration-guide.md)
- [OpenCASCADE集成指南](docs/opencascade-integration-guide.md)
- [性能优化报告](docs/STEP加载性能优化.md)

## 🐛 常见问题

### 编译问题

**Q: CMake找不到VTK或OpenCASCADE**
```
A: 检查config/paths.cmake中的路径配置是否正确
```

**Q: 编译时出现链接错误**
```
A: 确保所有第三方库都使用相同的编译器版本编译
```

### 运行时问题

**Q: 程序启动时缺少DLL**
```
A: 将VTK、Qt、OpenCASCADE的bin目录添加到系统PATH
```

**Q: STEP文件加载卡住**
```
A: 这是正常现象，大型文件需要几分钟处理时间
   观察系统日志中的进度信息
```

## 🤝 贡献指南

### 开发流程

1. **Fork项目** 并创建功能分支
2. **遵循代码规范** (C++17, Qt编码标准)
3. **添加测试** 验证新功能
4. **更新文档** 说明变更内容
5. **提交Pull Request**

### 代码规范

- 使用C++17标准特性
- 遵循Qt命名约定
- 添加适当的注释和文档
- 确保线程安全

## 📞 技术支持

- **文档**: 查看 `docs/` 目录下的详细文档
- **Issues**: 在GitHub上提交问题报告
- **讨论**: 参与项目讨论区

## 📄 许可证

本项目采用 [MIT License](LICENSE) 许可证。

---

**版本**: v1.0  
**最后更新**: 2025-12-20  
**作者**: 王睿 (浙江大学)  
**维护团队**: 喷涂轨迹规划开发组