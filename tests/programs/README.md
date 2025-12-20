# 测试程序目录

这个目录包含了项目开发过程中使用的各种测试和调试程序，它们独立于主程序，用于验证特定功能和组件。

## 📁 目录结构

```
tests/programs/
├── CMakeLists.txt              # 测试程序构建配置
├── README.md                   # 本文档
├── debug_async_main.cpp        # 调试异步加载主程序
├── test_async_step.cpp         # 异步STEP加载测试
├── test_opencascade.cpp        # OpenCASCADE基础功能测试
├── vtk_ply_test.cpp           # VTK点云加载测试
└── vtk_test_main.cpp          # VTK主测试程序
```

## 🧪 测试程序说明

### 1. test_opencascade.cpp
**功能**: 测试OpenCASCADE基础功能
- 验证OpenCASCADE库是否正确链接
- 测试STEP文件读取功能
- 验证几何体解析和网格生成

**编译**: `TestOpenCASCADE.exe`
**用法**: 
```bash
TestOpenCASCADE.exe                           # 基础功能测试
TestOpenCASCADE.exe path/to/file.step        # 测试STEP文件加载
```

### 2. test_async_step.cpp
**功能**: 测试异步STEP文件加载
- 简化版的异步加载实现
- 测试多线程STEP文件处理
- 验证进度更新和错误处理

**编译**: `TestAsyncSTEP.exe`
**用法**: 运行后通过GUI选择STEP文件进行测试

### 3. vtk_ply_test.cpp
**功能**: 测试VTK点云文件加载
- 验证VTK库配置
- 测试PLY文件读取
- 调试路径和编码问题

**编译**: `TestVTKPLY.exe`
**用法**: 自动测试预设的PLY文件

### 4. vtk_test_main.cpp
**功能**: VTK 3D渲染主测试程序
- 完整的VTK 3D界面测试
- 点云加载和显示
- 3D交互功能验证

**编译**: `TestVTKMain.exe`
**用法**: 运行后通过GUI加载点云文件

### 5. debug_async_main.cpp
**功能**: 调试异步加载的简化主程序
- 专门用于调试异步STEP加载
- 简化的界面，便于问题定位
- 详细的调试输出

**编译**: `DebugAsyncMain.exe`
**用法**: 运行后点击按钮加载MPX3500.STEP文件

## 🔨 编译方法

### 方法1: 独立编译（推荐）
```bash
cd tests/programs
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64 -DQt6_DIR="K:/Kapps/Qt/6.10.1/msvc2022_64/lib/cmake/Qt6"
cmake --build . --config Debug
```

### 方法2: 作为主项目的一部分
在主项目的CMakeLists.txt中添加：
```cmake
add_subdirectory(tests/programs)
```

## 📋 测试流程

### 基础功能验证
1. **OpenCASCADE测试**:
   ```bash
   cd build/bin/tests
   ./TestOpenCASCADE.exe ../../data/model/MPX3500.STEP
   ```

2. **VTK功能测试**:
   ```bash
   ./TestVTKPLY.exe
   ```

3. **异步加载测试**:
   ```bash
   ./TestAsyncSTEP.exe
   ```

### 调试流程
1. **简化调试**:
   ```bash
   ./DebugAsyncMain.exe
   ```

2. **完整界面测试**:
   ```bash
   ./TestVTKMain.exe
   ```

## 🔧 开发用途

### 功能验证
- 在开发新功能前，先用这些程序验证基础组件
- 隔离问题，确定是库配置问题还是代码逻辑问题

### 性能测试
- 测试不同STEP文件的加载性能
- 对比同步和异步加载的效果
- 验证内存使用和CPU占用

### 调试辅助
- 当主程序出现问题时，用简化版本定位问题
- 测试特定的代码路径和边界条件

## 📝 注意事项

1. **依赖关系**: 这些程序依赖主项目的配置文件和库设置
2. **路径配置**: 确保`config/paths.cmake`中的路径正确
3. **测试数据**: 某些程序需要特定的测试文件
4. **版本兼容**: 与主程序使用相同的库版本

## 🚀 添加新测试程序

如果需要添加新的测试程序：

1. 在此目录创建新的.cpp文件
2. 在CMakeLists.txt中添加对应的可执行文件配置
3. 更新本README文档
4. 确保遵循现有的命名和结构约定

---

**维护者**: 开发团队  
**最后更新**: 2025年12月20日