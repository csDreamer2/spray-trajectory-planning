# 喷涂轨迹规划系统

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/Platform-Windows-blue.svg)](https://www.microsoft.com/windows)
[![Qt](https://img.shields.io/badge/Qt-6.10+-green.svg)](https://www.qt.io/)
[![VTK](https://img.shields.io/badge/VTK-9.2-orange.svg)](https://vtk.org/)
[![OpenCASCADE](https://img.shields.io/badge/OpenCASCADE-7.8-red.svg)](https://www.opencascade.com/)

> 基于Qt、VTK和OpenCASCADE的工业级喷涂轨迹规划系统，支持复杂3D模型的STEP文件加载、点云处理和轨迹生成。

## 🚀 快速开始

### 🆕 新开发者入门

**第一次参与项目？请先阅读这些文档：**

1. **📖 [开发者协作指南](docs/开发者协作指南.md)** - 🔥 **必读文档**
   - 完整的开发环境配置
   - 代码获取和分支管理
   - 开发流程和规范
   - 提交和PR流程

2. **🛠️ [开发环境部署指南](docs/开发环境部署指南.md)** - 详细的环境搭建
   ```bash
   # 详细的环境搭建指南
   docs/开发环境部署指南.md
   ```

3. **🔧 运行环境配置助手**（这个慎用，还是自己配好点）
   ```bash
   # Windows环境配置脚本
   scripts/setup_environment.bat
   ```

4. **✅ 验证环境配置**
   ```bash
   # 环境验证脚本
   scripts/verify_setup.bat
   ```

5. **🏗️ 编译项目**
   ```bash
   mkdir build
   cd build
   cmake .. -G "Visual Studio 17 2022" -A x64 -DQt6_DIR="K:/Kapps/Qt/6.10.1/msvc2022_64/lib/cmake/Qt6"
   cmake --build . --config Debug
   ```

### 核心功能

- ✅ **STEP模型导入** - 支持复杂工业模型的加载和解析
- ✅ **3D可视化** - 基于VTK的高性能3D渲染
- ✅ **点云处理** - 支持多种点云格式的加载和处理
- ✅ **轨迹规划** - 智能喷涂轨迹生成算法
- ✅ **异步处理** - 大文件加载不阻塞UI界面

## 🔧 最新修复

### STEP导入崩溃问题已彻底解决 ✅

**问题**: 导入STEP模型时程序在生成网格后崩溃  
**原因**: UI集成和线程处理方式问题  
**解决**: 使用STEPModelTreeWidget异步处理机制  

**测试验证**:
```bash
# 运行修复验证测试
测试STEP修复方案.bat
```

**详细信息**: 参见 [STEP导入崩溃问题最终修复方案](docs/STEP导入崩溃问题最终修复方案.md)

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
- **OpenCASCADE 7.7**: 工业级CAD几何内核
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
│   ├── 开发者协作指南.md    # 新开发者必读
│   └── ...                # 技术文档
├── scripts/                # 辅助脚本
│   ├── setup_environment.bat    # 环境配置助手
│   └── verify_setup.bat        # 环境验证脚本
├── tests/                  # 测试脚本
│   ├── programs/           # 测试程序（独立于主程序）
│   └── ...                # py和bat的测试脚本
├── src/                    # 源代码
│   ├── UI/                 # 用户界面
│   ├── Data/               # 数据处理
│   ├── Core/               # 核心功能
│   └── ...
├── data/                   # 测试数据目录
│   ├── model/              # 3D模型文件 (STEP, STL, OBJ)
│   └── pointclouds/        # 点云数据文件 (PCD, PLY, XYZ)
├── build/                  # 构建输出目录
└── CMakeLists.txt          # 主构建配置
```

## 📊 数据文件说明

### 🎯 3D模型文件 (`data/model/`)

支持的3D模型格式：
- **STEP文件** (*.step, *.stp) - 🔥 **推荐格式**，完整CAD几何信息
- **STL文件** (*.stl) - 网格模型格式
- **OBJ文件** (*.obj) - 通用3D模型格式

**使用方法**：
1. 将3D模型文件放入 `data/model/` 目录
2. 在程序中通过 "文件 -> 打开" 菜单加载
3. 支持异步加载，大文件请耐心等待进度条

### ☁️ 点云数据文件 (`data/pointclouds/`)

支持的点云格式：
- **PCD文件** (*.pcd) - 🔥 **推荐格式**，PCL库标准格式
- **PLY文件** (*.ply) - 通用点云格式  
- **XYZ文件** (*.xyz) - 简单ASCII点云格式
- **LAS文件** (*.las) - 激光扫描数据格式

**使用方法**：
1. 将点云文件放入 `data/pointclouds/` 目录
2. 在程序中通过 "点云 -> 加载" 菜单导入
3. 系统将基于点云数据生成喷涂轨迹

**数据要求**：
- 坐标系：右手坐标系
- 单位：毫米(mm)或米(m)
- 点密度：1-5mm间距（根据喷涂精度）
- 文件命名：`工件名称_日期_分辨率.格式`

### 📝 数据准备建议

**测试模型分类**：
- 🔸 **小型模型** (< 1MB) - 快速功能测试
- 🔹 **中型模型** (1-10MB) - 功能验证
- 🔶 **大型模型** (> 10MB) - 性能测试

**点云数据分类**：
- 🔸 **简单几何体** - 立方体、圆柱体（算法验证）
- 🔹 **复杂曲面** - 汽车零件、机械部件
- 🔶 **完整工件** - 大规模扫描数据（性能测试）

> 💡 **提示**: 由于Git文件大小限制，实际的模型和点云文件不包含在仓库中。请根据需要添加测试数据。

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

### 📂 准备测试数据

在开始测试前，请准备测试数据：

1. **3D模型文件** - 放入 `data/model/` 目录
   ```bash
   # 建议的测试文件
   data/model/
   ├── simple_cube.step      # 简单几何体
   ├── complex_part.step     # 复杂零件
   └── large_assembly.step   # 大型装配体
   ```

2. **点云数据文件** - 放入 `data/pointclouds/` 目录
   ```bash
   # 建议的点云文件
   data/pointclouds/
   ├── test_object_1mm.pcd   # 高精度点云
   ├── scan_data_5mm.pcd     # 标准精度点云
   └── large_scan.pcd        # 大规模点云
   ```

### 🔍 功能测试

1. **基础功能测试**
   ```bash
   build/bin/Debug/SprayTrajectoryPlanning.exe
   ```

2. **STEP文件加载测试**
   - 通过 "文件 -> 打开" 导入 `data/model/` 中的STEP文件
   - 验证异步加载和进度显示
   - 检查3D渲染效果和交互操作

3. **点云处理测试**
   - 通过 "点云 -> 加载" 导入 `data/pointclouds/` 中的点云文件
   - 验证点云可视化效果
   - 测试轨迹规划功能

4. **性能测试**
   - 大型STEP文件加载时间
   - 点云处理速度
   - 内存使用情况
   - UI响应性

### 🧪 测试脚本

```bash
# UI事件循环修复验证
测试UI事件循环修复.bat

# TransferRoots阻塞问题验证
测试TransferRoots修复.bat

# 异步加载性能测试
测试异步加载.bat
```

## 📚 开发文档

### 🚀 新开发者入门
- **[开发者协作指南](docs/开发者协作指南.md)** - 🔥 **新手必读！** 完整的开发流程和环境配置
- **[开发环境部署指南](docs/开发环境部署指南.md)** - 详细的环境搭建步骤

### 👨‍💼 项目维护
- **[代码审核与合并指南](docs/代码审核与合并指南.md)** - PR审核和代码质量管理

### 🔧 核心技术文档
- [异步STEP加载优化方案](docs/异步STEP加载优化方案.md) - 核心技术实现
- [主程序UI事件循环阻塞问题修复](docs/主程序UI事件循环阻塞问题修复.md) - 重要bug修复
- [VTK集成指南](docs/vtk-integration-guide.md) - VTK集成和使用
- [OpenCASCADE集成指南](docs/opencascade-integration-guide.md) - OpenCASCADE集成
- [性能优化报告](docs/STEP加载性能优化.md) - 系统性能优化

### 📋 问题排查
- [故障排除指南](docs/Troubleshooting_Guide.md) - 常见问题解决
- [快速测试指南](docs/Quick_Test_Guide.md) - 功能验证方法

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

我们欢迎所有形式的贡献！无论你是想修复bug、添加新功能，还是改进文档，都请先阅读我们的开发指南。

### 📖 开发者必读文档

#### 🚀 **新手入门**
- **[开发者协作指南](docs/开发者协作指南.md)** - 完整的开发流程指南
  - 环境配置详细步骤
  - 代码获取和分支管理
  - 代码规范和提交标准
  - 常见问题解决方案

#### 👨‍💼 **项目维护者**
- **[代码审核与合并指南](docs/代码审核与合并指南.md)** - PR审核和管理指南
  - 代码审核标准和流程
  - 合并策略选择
  - 审核工具使用
  - 质量保证措施

### 🔄 快速开始贡献

1. **📚 阅读文档**
   ```bash
   # 新开发者必读
   docs/开发者协作指南.md
   
   # 环境配置指南
   docs/开发环境部署指南.md
   ```

2. **🍴 Fork并克隆项目**
   ```bash
   # Fork项目到你的GitHub账户
   # 然后克隆到本地
   git clone https://github.com/csDreamer2/spray-trajectory-planning.git
   cd spray-trajectory-planning
   
   # 添加上游仓库
   git remote add upstream https://github.com/csDreamer2/spray-trajectory-planning.git
   ```

3. **🌿 创建功能分支**
   ```bash
   # 同步最新代码
   git checkout main
   git pull upstream main
   
   # 创建新分支
   git checkout -b feature/你的功能名称
   ```

4. **💻 开发和测试**
   ```bash
   # 按照开发者协作指南进行开发
   # 运行测试确保代码质量
   scripts/run_tests.bat
   ```

5. **📤 提交Pull Request**
   - 遵循提交信息规范
   - 填写详细的PR描述
   - 等待代码审核

### 📏 代码规范

- **语言标准**: C++17
- **编码风格**: Qt编码标准
- **命名约定**: 
  - 类名: `PascalCase`
  - 函数名: `camelCase`
  - 成员变量: `m_camelCase`
- **注释**: 使用Doxygen格式
- **线程安全**: 确保多线程代码的安全性

### 🏷️ 提交类型

```bash
feat:     新功能
fix:      Bug修复  
docs:     文档更新
style:    代码格式调整
refactor: 重构
perf:     性能优化
test:     测试相关
chore:    构建或工具变动
```

### 🎯 贡献领域

我们特别欢迎以下方面的贡献：

- 🐛 **Bug修复** - 修复已知问题
- ✨ **新功能** - 添加实用功能
- 📈 **性能优化** - 提升系统性能
- 📚 **文档改进** - 完善项目文档
- 🧪 **测试增强** - 增加测试覆盖
- 🌐 **国际化** - 多语言支持
- 🎨 **UI/UX改进** - 用户体验优化

## 📞 技术支持

- **文档**: 查看 `docs/` 目录下的详细文档
- **Issues**: 在GitHub上提交问题报告
- **讨论**: 参与项目讨论区

## 📄 许可证

本项目采用 [MIT License](LICENSE) 许可证。

---

**版本**: v1.0  
**最后更新**: 2025-12-20  