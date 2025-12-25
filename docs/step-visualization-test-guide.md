# STEP模型可视化功能测试指南

## 测试步骤

### 1. 启动程序
```bash
cd build/bin/Debug
./SprayTrajectoryPlanning.exe
```

### 2. 导入STEP模型
1. 点击菜单 "文件 -> 导入STEP模型" 或按 `Ctrl+S`
2. 选择你的STEP文件（例如：MPX3500.step）
3. 等待加载完成

### 3. 观察控制台输出

**成功的输出应该包含：**

```
STEPModelTreeWidget: 已保存根节点引用
MainWindow: STEP模型树加载成功，准备创建VTK可视化
MainWindow: 获取加载的根节点: 成功
MainWindow: 根节点名称: MPX3500 ROBOT
MainWindow: 根节点子节点数: 8
MainWindow: 延迟100ms后创建VTK可视化
MainWindow: 开始创建VTK可视化...
VTKWidget: 开始为STEP模型树创建Actors
VTKWidget: 根节点名称: MPX3500 ROBOT
VTKWidget: 根节点子节点数: 8
VTKWidget: 为节点创建Actor: MPX3500 BASE_L-Type 层级: 2
VTKWidget: PolyData转换成功，点数: XXXX
VTKWidget: Actor创建并添加成功: MPX3500 BASE_L-Type
...
VTKWidget: STEP模型树Actors创建完成
MainWindow: VTK可视化创建成功
```

### 4. 测试可视化控制

#### 测试1：单个零件显示/隐藏
1. 在右侧"STEP模型树"面板中找到任意零件
2. 取消勾选该零件的复选框
3. **预期结果**：3D视图中该零件消失
4. 重新勾选复选框
5. **预期结果**：3D视图中该零件重新显示

#### 测试2：装配体联动
1. 找到一个装配体节点（有子节点的节点）
2. 取消勾选装配体
3. **预期结果**：
   - 装配体及其所有子零件都消失
   - 所有子节点的复选框自动取消勾选
4. 重新勾选装配体
5. **预期结果**：
   - 装配体及其所有子零件都显示
   - 所有子节点的复选框自动勾选

#### 测试3：3D视图交互
1. 使用鼠标左键拖动旋转视图
2. 使用鼠标滚轮缩放
3. 点击"重置视图"按钮
4. 点击"适应场景"按钮

## 预期行为

### 正常情况
- ✅ 模型树正确显示所有装配体和零件
- ✅ 3D视图显示所有零件（不同颜色）
- ✅ 勾选/取消勾选立即生效
- ✅ 父子节点联动正常
- ✅ 3D视图可以正常交互

### 控制台输出
- ✅ 没有"根节点为空"的警告
- ✅ 没有"无法从Item获取节点"的警告
- ✅ 看到"Actor创建并添加成功"的日志
- ✅ 看到"VTK可视化创建成功"的日志

## 故障排查

### 问题1：根节点为空
**症状**：
```
MainWindow: 获取加载的根节点: 失败
```

**原因**：`m_loadedRootNode`没有正确设置

**解决**：检查`updateModelFromWorkerResult`是否正确保存了根节点

### 问题2：无法从Item获取节点
**症状**：
```
STEPModelTreeWidget: 加载的根节点为空
STEPModelTreeWidget: 无法从Item获取节点
```

**原因**：节点指针存储或恢复有问题

**解决**：检查`buildQtModelItemFromNode`和`getNodeFromItem`的实现

### 问题3：Actor创建失败
**症状**：
```
VTKWidget: PolyData转换成功，点数: 0
```

**原因**：Shape为空或网格化失败

**解决**：检查Shape是否有效，网格化参数是否合理

### 问题4：程序崩溃
**症状**：程序在创建Actor时崩溃

**可能原因**：
1. 访问了已释放的内存
2. OpenCASCADE异常
3. VTK渲染器未初始化

**解决**：
1. 检查所有shared_ptr是否有效
2. 添加try-catch捕获异常
3. 确保VTK组件正确初始化

## 性能测试

### 小型模型（<10个零件）
- 加载时间：< 5秒
- 创建Actor时间：< 1秒
- 交互响应：流畅

### 中型模型（10-50个零件）
- 加载时间：5-30秒
- 创建Actor时间：1-5秒
- 交互响应：流畅

### 大型模型（>50个零件）
- 加载时间：30秒-数分钟
- 创建Actor时间：5-30秒
- 交互响应：可能有延迟

## 调试技巧

### 1. 启用详细日志
程序已经包含详细的qDebug输出，直接查看控制台即可

### 2. 检查内存使用
使用任务管理器监控内存使用，大型模型可能占用数GB内存

### 3. 单步调试
在以下关键点设置断点：
- `STEPModelTreeWidget::updateModelFromWorkerResult`
- `MainWindow::OnImportSTEPModel`的lambda
- `VTKWidget::LoadSTEPModelWithTree`
- `VTKWidget::CreateActorsForSTEPNode`

### 4. 验证数据结构
在调试器中检查：
- `m_loadedRootNode`是否有效
- `rootNode->children.size()`是否正确
- `node->shape.IsNull()`是否为false
- `node->actor`是否已创建

## 已知限制

1. **大型模型性能**：超过100个零件可能需要较长时间
2. **内存占用**：每个零件都有独立Actor，内存占用较大
3. **颜色方案**：当前使用简单的层级颜色，可能不够直观

## 下一步改进

1. **性能优化**：
   - 延迟创建Actor（仅创建可见零件）
   - 使用LOD技术
   - 实例化渲染

2. **功能增强**：
   - 零件高亮
   - 零件选择
   - 自定义颜色
   - 透明度调整

3. **用户体验**：
   - 搜索功能
   - 过滤功能
   - 右键菜单
   - 批量操作
