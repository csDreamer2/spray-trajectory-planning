# STEP导入崩溃修复方案 - 更新版

## 问题描述

用户报告在使用"导入STEP模型"功能时，程序在网格生成完成后约2秒钟会发生闪退。通过测试发现，崩溃发生在`STEPModelTree: Found 1 free shapes`之后，返回码`-1073741571`（0xC00000FD）表示栈溢出。

## 根本原因

经过深入分析，发现问题的根本原因是：

### 1. 栈溢出问题
`parseSTEPLabel`函数是递归的，对于大型STEP文件（150MB），递归深度可能非常深，导致栈溢出崩溃。

### 2. 缺乏递归保护
原始代码没有递归深度限制，对于复杂的装配体结构，可能会无限递归。

### 3. 异常处理不足
在递归解析过程中，如果某个节点出现问题，会导致整个解析过程崩溃。

## 修复方案

### 1. 添加递归深度限制

```cpp
// 添加最大递归深度限制
const int MAX_RECURSION_DEPTH = 100;

void STEPModelTree::parseSTEPLabel(const TDF_Label& label, 
                                  std::shared_ptr<STEPTreeNode> parent, 
                                  int level,
                                  int maxDepth)
{
    // 检查递归深度限制，防止栈溢出
    if (level > maxDepth) {
        qWarning() << "Maximum recursion depth reached at level" << level;
        return;
    }
    // ... 其他代码
}
```

### 2. 增强异常处理

为每个解析步骤添加try-catch保护：

```cpp
// 保护每个自由形状的解析
for (int i = 1; i <= freeShapes.Length(); i++) {
    try {
        TDF_Label label = freeShapes.Value(i);
        parseSTEPLabel(label, m_rootNode, 1, MAX_RECURSION_DEPTH);
    } catch (const std::exception& e) {
        qWarning() << "Exception parsing free shape" << i << ":" << e.what();
        // 继续处理其他形状
    }
}
```

### 3. 添加CPU时间让出

```cpp
// 每处理一定数量的节点就让出CPU时间
static int processedCount = 0;
processedCount++;
if (processedCount % 50 == 0) {
    QThread::msleep(1); // 让出1ms CPU时间
}
```

### 4. 改进资源清理

```cpp
STEPModelTree::~STEPModelTree() 
{
    try {
        // 递归清理所有节点的OpenCASCADE引用
        if (m_rootNode) {
            clearNodeReferences(m_rootNode);
            m_rootNode.reset();
        }
        
        // 清理XCAF工具引用
        m_shapeTool.Nullify();
        m_colorTool.Nullify();
        m_layerTool.Nullify();
        
        // 关闭STEP文档
        if (!m_stepDocument.IsNull()) {
            Handle(XCAFApp_Application) app = XCAFApp_Application::GetApplication();
            if (!app.IsNull()) {
                app->Close(m_stepDocument);
            }
            m_stepDocument.Nullify();
        }
    } catch (...) {
        // 捕获所有异常，避免析构函数抛出异常
    }
}
```

## 测试方案

### 1. 最小OpenCASCADE测试
```bash
tests/minimal_occt_test.bat
```
只测试OpenCASCADE的基本STEP读取功能，验证库本身是否正常。

### 2. 安全STEP测试
```bash
tests/safe_step_test.bat
```
测试带有保护措施的STEP模型树解析，包括递归深度限制。

### 3. 单独STEP树测试
```bash
tests/step_tree_only_test.bat
```
只测试STEP模型树组件，不涉及VTK显示。

## 使用方法

1. **先测试最小功能**：
   ```bash
   cd tests
   minimal_occt_test.bat
   ```
   如果这个测试失败，说明OpenCASCADE库配置有问题。

2. **测试安全版本**：
   ```bash
   safe_step_test.bat
   ```
   这个版本有递归深度限制，应该不会栈溢出。

3. **测试主程序**：
   如果上述测试都通过，再测试完整的主程序功能。

## 预期效果

- 消除栈溢出导致的崩溃
- 即使遇到异常，也能继续处理其他部分
- 提供更详细的调试信息
- 对大型STEP文件有更好的处理能力

## 注意事项

1. 递归深度限制可能会导致某些深层嵌套的组件不被解析
2. 增加了更多的调试输出，便于问题排查
3. 对于超大文件，解析时间可能会较长

## 后续优化建议

1. **实现迭代式解析**：将递归改为迭代，彻底避免栈溢出
2. **分块处理**：对大型文件进行分块处理
3. **内存优化**：优化内存使用，避免同时加载过多数据
4. **进度反馈**：提供更准确的进度信息