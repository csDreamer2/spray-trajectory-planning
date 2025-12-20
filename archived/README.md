# 归档文件说明

## 📁 目录概述

这个目录包含了项目开发过程中已经废弃或不再使用的代码和文档，主要是Unity集成相关的内容。

## 🗂️ 目录结构

```
archived/
├── README.md                   # 本说明文档
├── Unity/                      # Unity项目文件（已废弃）
│   ├── SpraySimulation/        # Unity项目主体
│   ├── Assets/                 # Unity资源文件
│   ├── *.cs                   # Unity C#脚本
│   └── README.md              # Unity项目说明
├── unity-integration/          # Unity集成源码（已废弃）
│   ├── UnityWidget.cpp        # Unity窗口组件
│   ├── UnityWidget.h          # Unity窗口组件头文件
│   ├── QtUnityBridge.cpp      # Qt-Unity通信桥
│   └── QtUnityBridge.h        # Qt-Unity通信桥头文件
└── unity-docs/                # Unity相关文档（已废弃）
    ├── PointCloud_Unity_Integration_Guide.md
    ├── unity-*.md             # 各种Unity集成文档
    ├── vtk-migration-guide.md # VTK迁移指南
    └── workpiece-loading-fix-guide.md
```

## 🚫 废弃原因

### Unity集成方案的问题
1. **窗口嵌入困难**: Unity窗口嵌入Qt界面导致布局问题和交互困难
2. **技术复杂性**: Qt-Unity TCP通信增加了系统复杂性
3. **维护成本**: 需要同时维护Qt和Unity两套代码
4. **性能问题**: 跨进程通信带来的性能开销
5. **部署复杂**: 需要同时部署Qt程序和Unity应用

### 转向VTK的优势
1. **原生集成**: VTK与Qt原生集成，无需额外进程
2. **性能优异**: 直接在Qt界面中渲染，性能更好
3. **代码统一**: 全部使用C++，维护更简单
4. **功能完整**: VTK提供完整的3D可视化功能
5. **部署简单**: 单一可执行文件，部署更容易

## 📅 时间线

- **2024年初**: 开始Unity集成尝试
- **2024年中期**: 发现Unity窗口嵌入问题
- **2024年下半年**: 转向VTK解决方案
- **2024年12月**: 完成VTK集成，Unity方案正式废弃
- **2025年12月20日**: 将Unity相关文件归档

## 🔍 技术参考价值

虽然Unity方案已经废弃，但这些文件仍有一定的参考价值：

### Unity集成技术
- Qt-Unity进程间通信实现
- Unity窗口嵌入技术探索
- TCP Socket通信机制
- Unity C#脚本与Qt C++交互

### 问题解决经验
- 跨平台窗口嵌入的挑战
- 进程间通信的性能考量
- 3D渲染引擎选择的决策过程
- 技术方案迁移的经验教训

## ⚠️ 注意事项

1. **不要恢复使用**: 这些文件已经从主项目中移除，不应该重新集成
2. **仅供参考**: 可以作为技术研究和学习的参考材料
3. **版本兼容**: 这些代码基于较早的Unity和Qt版本，可能存在兼容性问题
4. **依赖缺失**: 相关的Unity项目和依赖可能已经不完整

## 📚 相关文档

如需了解当前的VTK集成方案，请参考：
- `docs/vtk-integration-success.md` - VTK集成成功报告
- `docs/vtk-migration-complete.md` - VTK迁移完成总结
- `src/UI/VTKWidget.h` - VTK组件实现
- `docs/开发环境部署指南.md` - 当前开发环境配置

---

**归档日期**: 2025年12月20日  
**归档原因**: Unity集成方案废弃，转向VTK解决方案  
**技术决策**: 基于性能、维护性和部署简便性考虑