# 自动喷涂轨迹规划系统数据库设计

## 数据库架构概述

系统采用双层数据存储架构：
- **MySQL数据库**：主要数据存储，用于生产环境的持久化数据
- **SQLite本地缓存**：本地缓存和离线数据，提高响应速度

## MySQL数据库表结构

### 1. 用户管理表

#### users - 用户信息表
```sql
CREATE TABLE users (
    id INT PRIMARY KEY AUTO_INCREMENT,
    username VARCHAR(50) UNIQUE NOT NULL,
    password_hash VARCHAR(255) NOT NULL,
    email VARCHAR(100),
    full_name VARCHAR(100),
    role ENUM('admin', 'operator', 'viewer') DEFAULT 'operator',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    last_login TIMESTAMP NULL,
    is_active BOOLEAN DEFAULT TRUE,
    INDEX idx_username (username),
    INDEX idx_role (role)
);
```

#### user_sessions - 用户会话表
```sql
CREATE TABLE user_sessions (
    id INT PRIMARY KEY AUTO_INCREMENT,
    user_id INT NOT NULL,
    session_token VARCHAR(255) UNIQUE NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    expires_at TIMESTAMP NOT NULL,
    ip_address VARCHAR(45),
    user_agent TEXT,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    INDEX idx_token (session_token),
    INDEX idx_user_id (user_id)
);
```

### 2. 工件管理表

#### workpieces - 工件信息表
```sql
CREATE TABLE workpieces (
    id INT PRIMARY KEY AUTO_INCREMENT,
    name VARCHAR(100) NOT NULL,
    description TEXT,
    category VARCHAR(50),
    model_file_path VARCHAR(500),
    model_file_size BIGINT,
    model_file_hash VARCHAR(64),
    dimensions JSON, -- {"length": 1000, "width": 500, "height": 300}
    material VARCHAR(50),
    surface_area DECIMAL(10,2),
    complexity_score DECIMAL(3,2), -- 0.0-1.0
    created_by INT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    is_active BOOLEAN DEFAULT TRUE,
    FOREIGN KEY (created_by) REFERENCES users(id),
    INDEX idx_category (category),
    INDEX idx_name (name),
    INDEX idx_created_by (created_by)
);
```

#### workpiece_batches - 工件批次表
```sql
CREATE TABLE workpiece_batches (
    id INT PRIMARY KEY AUTO_INCREMENT,
    batch_name VARCHAR(100) NOT NULL,
    description TEXT,
    total_workpieces INT DEFAULT 0,
    layout_config JSON, -- 布局配置信息
    space_utilization DECIMAL(5,2), -- 空间利用率百分比
    estimated_time INT, -- 预计加工时间（分钟）
    status ENUM('planning', 'ready', 'processing', 'completed', 'cancelled') DEFAULT 'planning',
    created_by INT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (created_by) REFERENCES users(id),
    INDEX idx_status (status),
    INDEX idx_batch_name (batch_name)
);
```

#### batch_workpieces - 批次工件关联表
```sql
CREATE TABLE batch_workpieces (
    id INT PRIMARY KEY AUTO_INCREMENT,
    batch_id INT NOT NULL,
    workpiece_id INT NOT NULL,
    quantity INT DEFAULT 1,
    position_x DECIMAL(8,2),
    position_y DECIMAL(8,2),
    position_z DECIMAL(8,2),
    rotation_x DECIMAL(6,2),
    rotation_y DECIMAL(6,2),
    rotation_z DECIMAL(6,2),
    processing_order INT,
    status ENUM('pending', 'processing', 'completed', 'failed') DEFAULT 'pending',
    FOREIGN KEY (batch_id) REFERENCES workpiece_batches(id) ON DELETE CASCADE,
    FOREIGN KEY (workpiece_id) REFERENCES workpieces(id),
    INDEX idx_batch_id (batch_id),
    INDEX idx_workpiece_id (workpiece_id),
    INDEX idx_processing_order (processing_order)
);
```

### 3. 轨迹规划表

#### trajectories - 轨迹信息表
```sql
CREATE TABLE trajectories (
    id INT PRIMARY KEY AUTO_INCREMENT,
    workpiece_id INT NOT NULL,
    name VARCHAR(100) NOT NULL,
    description TEXT,
    trajectory_type ENUM('spray', 'move', 'approach', 'retract') DEFAULT 'spray',
    total_points INT DEFAULT 0,
    total_length DECIMAL(10,2), -- 轨迹总长度（mm）
    estimated_time INT, -- 预计执行时间（秒）
    quality_score DECIMAL(3,2), -- 质量评分 0.0-1.0
    coverage_rate DECIMAL(5,2), -- 覆盖率百分比
    parameters JSON, -- 轨迹参数配置
    created_by INT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    is_active BOOLEAN DEFAULT TRUE,
    FOREIGN KEY (workpiece_id) REFERENCES workpieces(id),
    FOREIGN KEY (created_by) REFERENCES users(id),
    INDEX idx_workpiece_id (workpiece_id),
    INDEX idx_trajectory_type (trajectory_type),
    INDEX idx_quality_score (quality_score)
);
```

#### trajectory_points - 轨迹点表
```sql
CREATE TABLE trajectory_points (
    id INT PRIMARY KEY AUTO_INCREMENT,
    trajectory_id INT NOT NULL,
    point_index INT NOT NULL,
    position_x DECIMAL(8,3) NOT NULL,
    position_y DECIMAL(8,3) NOT NULL,
    position_z DECIMAL(8,3) NOT NULL,
    orientation_x DECIMAL(6,3),
    orientation_y DECIMAL(6,3),
    orientation_z DECIMAL(6,3),
    orientation_w DECIMAL(6,3),
    speed DECIMAL(6,2), -- 速度 mm/s
    flow_rate DECIMAL(4,2), -- 流量 0.0-1.0
    spray_width DECIMAL(5,2), -- 喷涂宽度 mm
    dwell_time DECIMAL(4,2), -- 停留时间 秒
    FOREIGN KEY (trajectory_id) REFERENCES trajectories(id) ON DELETE CASCADE,
    INDEX idx_trajectory_id (trajectory_id),
    INDEX idx_point_index (point_index),
    UNIQUE KEY uk_trajectory_point (trajectory_id, point_index)
);
```

### 4. 机器人控制表

#### robot_programs - 机器人程序表
```sql
CREATE TABLE robot_programs (
    id INT PRIMARY KEY AUTO_INCREMENT,
    trajectory_id INT NOT NULL,
    program_name VARCHAR(100) NOT NULL,
    program_type ENUM('JBO', 'OUT', 'INFORM') DEFAULT 'JBO',
    program_content LONGTEXT,
    file_path VARCHAR(500),
    file_size BIGINT,
    checksum VARCHAR(64),
    validation_status ENUM('pending', 'valid', 'invalid') DEFAULT 'pending',
    validation_errors TEXT,
    created_by INT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (trajectory_id) REFERENCES trajectories(id),
    FOREIGN KEY (created_by) REFERENCES users(id),
    INDEX idx_trajectory_id (trajectory_id),
    INDEX idx_program_type (program_type),
    INDEX idx_validation_status (validation_status)
);
```

#### robot_executions - 机器人执行记录表
```sql
CREATE TABLE robot_executions (
    id INT PRIMARY KEY AUTO_INCREMENT,
    program_id INT NOT NULL,
    batch_id INT,
    execution_name VARCHAR(100),
    start_time TIMESTAMP NULL,
    end_time TIMESTAMP NULL,
    status ENUM('queued', 'running', 'paused', 'completed', 'failed', 'cancelled') DEFAULT 'queued',
    progress DECIMAL(5,2) DEFAULT 0.00, -- 进度百分比
    current_point INT DEFAULT 0,
    error_message TEXT,
    quality_metrics JSON, -- 质量指标
    execution_log LONGTEXT,
    created_by INT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (program_id) REFERENCES robot_programs(id),
    FOREIGN KEY (batch_id) REFERENCES workpiece_batches(id),
    FOREIGN KEY (created_by) REFERENCES users(id),
    INDEX idx_program_id (program_id),
    INDEX idx_batch_id (batch_id),
    INDEX idx_status (status),
    INDEX idx_start_time (start_time)
);
```

### 5. 质量管理表

#### quality_predictions - 质量预测表
```sql
CREATE TABLE quality_predictions (
    id INT PRIMARY KEY AUTO_INCREMENT,
    trajectory_id INT NOT NULL,
    overall_score DECIMAL(3,2), -- 总体质量评分
    thickness_uniformity DECIMAL(3,2), -- 厚度均匀性
    coverage_rate DECIMAL(5,2), -- 覆盖率
    defect_count INT DEFAULT 0,
    predicted_issues JSON, -- 预测问题列表
    thickness_map LONGTEXT, -- 厚度分布数据
    coverage_map LONGTEXT, -- 覆盖率分布数据
    simulation_time DECIMAL(6,2), -- 仿真耗时（秒）
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (trajectory_id) REFERENCES trajectories(id) ON DELETE CASCADE,
    INDEX idx_trajectory_id (trajectory_id),
    INDEX idx_overall_score (overall_score)
);
```

#### quality_reports - 质量报告表
```sql
CREATE TABLE quality_reports (
    id INT PRIMARY KEY AUTO_INCREMENT,
    execution_id INT NOT NULL,
    report_type ENUM('prediction', 'actual', 'comparison') DEFAULT 'actual',
    overall_rating ENUM('excellent', 'good', 'acceptable', 'poor') DEFAULT 'acceptable',
    thickness_avg DECIMAL(6,2), -- 平均厚度 μm
    thickness_std DECIMAL(6,2), -- 厚度标准差
    coverage_percentage DECIMAL(5,2), -- 实际覆盖率
    defect_areas JSON, -- 缺陷区域
    inspector_notes TEXT,
    report_file_path VARCHAR(500),
    created_by INT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (execution_id) REFERENCES robot_executions(id),
    FOREIGN KEY (created_by) REFERENCES users(id),
    INDEX idx_execution_id (execution_id),
    INDEX idx_report_type (report_type),
    INDEX idx_overall_rating (overall_rating)
);
```

### 6. 系统配置表

#### system_configs - 系统配置表
```sql
CREATE TABLE system_configs (
    id INT PRIMARY KEY AUTO_INCREMENT,
    config_key VARCHAR(100) UNIQUE NOT NULL,
    config_value TEXT,
    config_type ENUM('string', 'number', 'boolean', 'json') DEFAULT 'string',
    description TEXT,
    is_system BOOLEAN DEFAULT FALSE, -- 是否为系统配置
    updated_by INT,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (updated_by) REFERENCES users(id),
    INDEX idx_config_key (config_key),
    INDEX idx_is_system (is_system)
);
```

#### operation_logs - 操作日志表
```sql
CREATE TABLE operation_logs (
    id INT PRIMARY KEY AUTO_INCREMENT,
    user_id INT,
    operation_type VARCHAR(50) NOT NULL,
    operation_target VARCHAR(100), -- 操作对象
    operation_details JSON, -- 操作详情
    ip_address VARCHAR(45),
    user_agent TEXT,
    result ENUM('success', 'failure', 'warning') DEFAULT 'success',
    error_message TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id),
    INDEX idx_user_id (user_id),
    INDEX idx_operation_type (operation_type),
    INDEX idx_created_at (created_at),
    INDEX idx_result (result)
);
```

## SQLite本地缓存表结构

本地缓存数据库主要用于：
- 离线数据缓存
- 临时数据存储
- 快速查询缓存

### 缓存表设计

```sql
-- 工件缓存表
CREATE TABLE cache_workpieces (
    id INTEGER PRIMARY KEY,
    mysql_id INTEGER UNIQUE,
    name TEXT NOT NULL,
    model_file_path TEXT,
    last_sync TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    is_dirty BOOLEAN DEFAULT FALSE
);

-- 轨迹缓存表
CREATE TABLE cache_trajectories (
    id INTEGER PRIMARY KEY,
    mysql_id INTEGER UNIQUE,
    workpiece_id INTEGER,
    name TEXT NOT NULL,
    points_data BLOB, -- 压缩的轨迹点数据
    last_sync TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    is_dirty BOOLEAN DEFAULT FALSE
);

-- 用户会话缓存
CREATE TABLE cache_user_sessions (
    id INTEGER PRIMARY KEY,
    user_id INTEGER,
    session_data TEXT, -- JSON格式的会话数据
    expires_at TIMESTAMP,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 系统配置缓存
CREATE TABLE cache_configs (
    config_key TEXT PRIMARY KEY,
    config_value TEXT,
    last_sync TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
```

## 数据库索引策略

### 主要索引
1. **主键索引**：所有表的id字段
2. **外键索引**：所有外键关联字段
3. **查询索引**：常用查询字段（用户名、状态、时间等）
4. **复合索引**：多字段组合查询

### 性能优化
1. **分区策略**：按时间分区大表（日志表、执行记录表）
2. **归档策略**：定期归档历史数据
3. **缓存策略**：热点数据Redis缓存
4. **读写分离**：主从数据库配置

## 数据同步策略

### MySQL ↔ SQLite 同步
1. **增量同步**：基于时间戳的增量数据同步
2. **冲突解决**：以MySQL为准，本地修改标记为dirty
3. **离线支持**：本地SQLite支持离线操作
4. **数据校验**：定期校验数据一致性

### 同步触发条件
- 应用启动时
- 网络连接恢复时
- 定时同步（每5分钟）
- 用户手动同步