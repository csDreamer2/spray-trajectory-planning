# STEP转STL转换故障排除指南

## 问题诊断

### 1. FreeCAD参数错误
**错误信息**: `unrecognised option '--python-script'`

**解决方案**: 已修复，现在使用正确的参数：
- FreeCADCmd: 直接运行Python脚本
- FreeCAD GUI: 使用 `-c` 参数

### 2. 转换过程检查

#### 检查FreeCAD安装
```bash
# 测试FreeCADCmd是否工作
"K:\Kapps\FreeCAD\bin\FreeCADCmd.exe" test_freecad_simple.py
```

#### 检查STEP文件
- 确保STEP文件存在且可读
- 文件大小合理（不要太大，避免内存不足）
- 文件格式正确

### 3. 转换参数优化

当前使用的转换参数：
```python
linear_deflection = 0.1   # 线性偏差
angular_deflection = 0.1  # 角度偏差
```

如果转换失败，可以尝试：
- 增大偏差值（0.5, 1.0）- 降低精度但提高成功率
- 减小偏差值（0.05, 0.01）- 提高精度但可能失败

### 4. 内存和性能

大型STEP文件转换可能需要：
- 足够的系统内存（建议8GB+）
- 较长的转换时间（几分钟到几十分钟）
- 足够的磁盘空间

### 5. 备选方案

如果自动转换失败：

#### 方案A: 手动转换
1. 打开FreeCAD GUI
2. 文件 → 打开 → 选择STEP文件
3. 选择导入的对象
4. 文件 → 导出 → STL格式
5. 保存到相同目录

#### 方案B: 在线转换
- https://www.convertio.co/step-stl/
- https://anyconv.com/step-to-stl-converter/

#### 方案C: 其他CAD软件
- SolidWorks
- Fusion 360
- Inventor

### 6. 调试信息

程序会输出详细的转换日志：
```
尝试FreeCAD路径: K:/Kapps/FreeCAD/bin/FreeCADCmd.exe
开始转换STEP文件: data/model/MPX3500.STEP
找到对象数量: X
添加形状: ObjectName
STL文件已保存: data/model/MPX3500.stl
网格点数: XXXX
网格面数: XXXX
SUCCESS
```

### 7. 常见问题

#### 问题: 转换卡住不动
**原因**: STEP文件太大或太复杂
**解决**: 
- 等待更长时间
- 简化STEP文件
- 增大网格偏差参数

#### 问题: 内存不足
**原因**: 系统内存不够
**解决**:
- 关闭其他程序
- 增大虚拟内存
- 使用更简单的STEP文件

#### 问题: STL文件损坏
**原因**: 转换过程中断或参数不当
**解决**:
- 重新转换
- 调整网格参数
- 检查原始STEP文件

### 8. 测试步骤

1. **启动程序**: `SprayTrajectoryPlanning.exe`
2. **导入STEP**: 文件 → 导入车间模型 → 选择STEP文件
3. **观察日志**: 查看控制台输出的转换信息
4. **检查结果**: 转换成功后会自动加载3D模型
5. **验证功能**: 点击"机械臂控制"测试动画

### 9. 性能优化建议

- 使用SSD硬盘提高I/O性能
- 确保有足够的RAM
- 关闭不必要的后台程序
- 使用较新版本的FreeCAD

### 10. 联系支持

如果问题仍然存在：
1. 收集错误日志
2. 记录STEP文件信息（大小、来源）
3. 记录系统配置（内存、CPU）
4. 提供详细的错误描述