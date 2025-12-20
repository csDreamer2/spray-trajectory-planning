# Unity启动问题修复指南

## 🔧 问题描述
点击"初始化Unity"按钮时出现错误：找不到Unity可执行文件

## ✅ 解决方案
已修改`src/UI/UnityWidget.cpp`，现在程序会：
1. 自动查找系统中安装的Unity编辑器
2. 启动Unity编辑器并打开SpraySimulation项目
3. 提供清晰的操作指导

## 🚀 测试步骤

### 1. 启动程序
```bash
cd build/bin/Debug
./SprayTrajectoryPlanning.exe
```

### 2. 点击"初始化Unity引擎"按钮
- 程序会自动查找Unity编辑器
- 如果找到，会启动Unity并打开项目
- 如果未找到，会显示手动启动指导

### 3. Unity编辑器操作
1. 等待Unity编辑器完全加载
2. 确保SpraySimulation项目已打开
3. 在Unity中点击▶️播放按钮
4. 等待Qt程序显示"Unity已连接"状态

### 4. 测试点云加载
1. 在Qt程序中点击"导入工件"
2. 选择测试文件：`test_data/sample_cube.ply`
3. 观察Unity中是否显示点云

## 🔍 故障排除

### 如果Unity编辑器未自动启动
**可能原因**：
- Unity未安装或路径不在预期位置
- Unity版本不兼容

**解决方法**：
1. 手动打开Unity Hub
2. 添加项目：`Unity/SpraySimulation`
3. 打开项目并点击播放

### 如果连接失败
**检查项**：
- Unity场景中是否有QtCommunication脚本
- 防火墙是否阻止端口12346
- Unity Console是否有错误信息

### 常见Unity编辑器路径
程序会自动搜索以下路径：
- `C:/Program Files/Unity/Hub/Editor/2022.3.*/Editor/Unity.exe`
- `C:/Program Files/Unity/Hub/Editor/2023.*/Editor/Unity.exe`
- `C:/Program Files (x86)/Unity/Editor/Unity.exe`
- `C:/Program Files/Unity/Editor/Unity.exe`

## 📋 验证清单
- [ ] 点击"初始化Unity引擎"不再报错
- [ ] Unity编辑器成功启动
- [ ] SpraySimulation项目正确加载
- [ ] 点击Unity播放按钮后Qt显示"已连接"
- [ ] 能够成功导入和显示点云

## 🎯 预期结果
修复后的流程应该是：
1. 点击按钮 → Unity编辑器启动
2. Unity加载项目 → 点击播放
3. Qt显示连接成功 → 可以导入点云
4. 点云在Unity中正确显示

---
**修复日期**: 2024-12-18  
**修复文件**: `src/UI/UnityWidget.cpp`  
**状态**: ✅ 已修复