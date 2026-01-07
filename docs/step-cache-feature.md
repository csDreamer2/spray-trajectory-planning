# STEP模型缓存加速功能

## 功能概述

为了解决大型STEP模型（如mpx3500）加载速度慢的问题，实现了基于VTK缓存的快速加载机制。

## 工作原理

### 传统加载流程（慢）
```
STEP文件 → OpenCASCADE解析 → 网格化 → VTK Actor → 显示
         ↑ 耗时最多（几分钟）
```

### 缓存加速流程（快）
```
首次加载:
STEP文件 → OpenCASCADE解析 → 网格化 → VTK Actor → 显示
                                      ↓
                                  保存为.vtp缓存

后续加载:
检查缓存 → 直接读取.vtp → VTK Actor → 显示
         ↑ 只需几秒钟
```

## 使用方法

### 方法1：菜单导入

1. **普通导入**（首次加载）
   - 菜单：`文件` → `导入STEP模型` (Ctrl+S)
   - 完整解析STEP文件
   - 自动创建缓存
   - 保留完整的模型树结构

2. **快速导入**（使用缓存）
   - 菜单：`文件` → `快速导入STEP模型` (Ctrl+Q)
   - 检查缓存是否存在
   - 如果缓存有效，直接加载（秒级）
   - 如果缓存无效，执行普通加载并创建缓存

### 方法2：快捷键

- `Ctrl+S` - 普通导入STEP模型
- `Ctrl+Q` - 快速导入STEP模型

## 缓存机制

### 缓存位置
```
data/cache/
├── mpx3500_a1b2c3d4.vtp
├── i5H_e5f6g7h8.vtp
└── ...
```

### 缓存命名规则
```
格式: {原文件名}_{时间戳hash}.vtp
示例: mpx3500_a1b2c3d4.vtp

时间戳hash: 基于文件修改时间的MD5哈希（前8位）
```

### 缓存有效性检查

缓存在以下情况下会失效：
1. 缓存文件不存在
2. STEP文件被修改（修改时间变化）
3. 缓存文件比STEP文件旧

失效后会自动重新加载并创建新缓存。

## 性能对比

### 大型模型（mpx3500）

| 加载方式 | 首次加载 | 后续加载 | 加速比 |
|---------|---------|---------|--------|
| 普通导入 | ~5分钟 | ~5分钟 | 1x |
| 快速导入 | ~5分钟 | ~5秒 | 60x |

### 中型模型（i5H）

| 加载方式 | 首次加载 | 后续加载 | 加速比 |
|---------|---------|---------|--------|
| 普通导入 | ~30秒 | ~30秒 | 1x |
| 快速导入 | ~30秒 | ~2秒 | 15x |

## 技术细节

### 缓存文件格式

- **格式**: VTP (VTK XML PolyData)
- **内容**: 合并后的所有部件网格数据
- **压缩**: Appended模式，未编码
- **大小**: 通常比STEP文件小50-70%

### 实现细节

#### 保存缓存
```cpp
bool STEPModelTreeWidget::saveToCache(const QString& cachePath)
{
    // 1. 合并所有Actor的PolyData
    vtkSmartPointer<vtkAppendPolyData> appendFilter = ...;
    for (auto actor : m_actorMap) {
        appendFilter->AddInputData(actor->GetMapper()->GetInput());
    }
    
    // 2. 写入VTP文件
    vtkSmartPointer<vtkXMLPolyDataWriter> writer = ...;
    writer->SetFileName(cachePath);
    writer->SetInputData(appendFilter->GetOutput());
    writer->Write();
}
```

#### 加载缓存
```cpp
bool STEPModelTreeWidget::loadFromCache(const QString& cachePath)
{
    // 1. 读取VTP文件
    vtkSmartPointer<vtkXMLPolyDataReader> reader = ...;
    reader->SetFileName(cachePath);
    reader->Update();
    
    // 2. 创建Actor
    vtkSmartPointer<vtkActor> actor = ...;
    actor->SetMapper(mapper);
    
    // 3. 添加到场景
    m_actorMap["CachedModel"] = actor;
}
```

## 限制和注意事项

### 当前限制

1. **简化的模型树**
   - 缓存加载时，模型树只显示一个节点"CachedModel"
   - 无法单独控制各个部件的可见性
   - 适合快速预览，不适合详细编辑

2. **颜色信息丢失**
   - 缓存只保存几何数据
   - 不保存原始的颜色和材质信息
   - 使用默认颜色显示

3. **缓存大小**
   - 大型模型的缓存文件可能较大（几百MB）
   - 需要足够的磁盘空间

### 使用建议

1. **首次加载**
   - 使用"普通导入"获得完整功能
   - 系统会自动创建缓存

2. **快速预览**
   - 使用"快速导入"快速查看模型
   - 适合频繁加载同一模型

3. **详细编辑**
   - 需要控制单个部件时，使用"普通导入"
   - 获得完整的模型树和交互功能

4. **缓存管理**
   - 定期清理`data/cache/`目录
   - 删除不再需要的缓存文件

## 未来改进

### 计划中的功能

1. **保留模型树结构**
   - 在缓存中保存树结构信息
   - 支持部件级别的可见性控制

2. **保存颜色信息**
   - 在缓存中保存颜色和材质
   - 恢复原始外观

3. **增量缓存**
   - 只缓存变化的部分
   - 减少缓存文件大小

4. **压缩优化**
   - 使用更高效的压缩算法
   - 减少磁盘占用

5. **缓存管理界面**
   - 查看所有缓存文件
   - 一键清理过期缓存
   - 显示缓存占用空间

## 故障排除

### 问题1：缓存加载失败

**症状**：提示"缓存加载失败，回退到正常加载"

**原因**：
- 缓存文件损坏
- VTK版本不兼容

**解决**：
1. 删除对应的缓存文件
2. 重新使用"普通导入"

### 问题2：缓存文件过大

**症状**：`data/cache/`目录占用大量空间

**解决**：
1. 手动删除不需要的`.vtp`文件
2. 只保留常用模型的缓存

### 问题3：缓存未生效

**症状**：每次都是慢速加载

**原因**：
- 文件修改时间变化
- 缓存目录不存在

**解决**：
1. 检查`data/cache/`目录是否存在
2. 确保STEP文件没有被修改
3. 查看控制台日志确认缓存状态

## 相关文件

- `src/UI/STEPModelTreeWidget.h` - 缓存功能声明
- `src/UI/STEPModelTreeWidget.cpp` - 缓存功能实现
- `src/UI/MainWindow.cpp` - 快速导入菜单
- `CMakeLists.txt` - VTK IOXML模块配置
- `src/UI/CMakeLists.txt` - UI库链接配置

## 作者
王睿 (浙江大学)

## 完成时间
2025-01-07

## 版本
v1.0 - 初始实现
