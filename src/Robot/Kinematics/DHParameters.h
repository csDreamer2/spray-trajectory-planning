#ifndef DHPARAMETERS_H
#define DHPARAMETERS_H

#include <QString>
#include <QVector>
#include <Eigen/Dense>
#include <cmath>

// 定义M_PI（Windows MSVC兼容性）
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * @brief DH参数结构体
 * 
 * 存储单个关节的Denavit-Hartenberg参数
 */
struct DHParameter {
    QString jointName;          // 关节名称 (J1, J2, ...)
    QString partName;           // STEP模型部件名称 (NAUO1, NAUO2, ...)
    
    double a;                   // 连杆长度 (mm)
    double d;                   // 连杆偏移 (mm)
    double alpha;               // 连杆扭角 (弧度)
    double theta_offset;        // 关节零位偏移 (弧度)
    
    double min_angle;           // 最小关节角 (弧度)
    double max_angle;           // 最大关节角 (弧度)
    double max_velocity;        // 最大角速度 (弧度/秒)
    
    char joint_type;            // 关节类型: 'R' (旋转) 或 'P' (平移)
    
    DHParameter() 
        : a(0), d(0), alpha(0), theta_offset(0),
          min_angle(-M_PI), max_angle(M_PI), max_velocity(M_PI),
          joint_type('R') {}
    
    DHParameter(const QString& name, const QString& part,
                double a_val, double d_val, double alpha_val, double theta_off,
                double min_a, double max_a, double max_v, char type)
        : jointName(name), partName(part),
          a(a_val), d(d_val), alpha(alpha_val), theta_offset(theta_off),
          min_angle(min_a), max_angle(max_a), max_velocity(max_v),
          joint_type(type) {}
};

/**
 * @brief Aubo i5H 机器人的DH参数配置
 */
class AuboI5HDHParameters {
public:
    AuboI5HDHParameters();
    
    /**
     * @brief 获取所有DH参数
     */
    const QVector<DHParameter>& getParameters() const { return m_parameters; }
    
    /**
     * @brief 获取指定关节的DH参数
     */
    const DHParameter& getParameter(int jointIndex) const;
    
    /**
     * @brief 获取关节数量
     */
    int getJointCount() const { return m_parameters.size(); }
    
    /**
     * @brief 根据部件名称获取关节索引
     */
    int getJointIndexByPartName(const QString& partName) const;
    
    /**
     * @brief 计算DH变换矩阵
     * @param jointIndex 关节索引
     * @param theta 关节角度 (弧度)
     * @return 4x4齐次变换矩阵
     */
    Eigen::Matrix4d computeDHTransform(int jointIndex, double theta) const;
    
    /**
     * @brief 计算正向运动学 (从基座到末端执行器)
     * @param jointAngles 所有关节的角度 (弧度)
     * @return 末端执行器的4x4齐次变换矩阵
     */
    Eigen::Matrix4d computeForwardKinematics(const QVector<double>& jointAngles) const;
    
    /**
     * @brief 计算从基座到指定关节的变换矩阵
     * @param jointIndex 关节索引
     * @param jointAngles 所有关节的角度 (弧度)
     * @return 该关节的4x4齐次变换矩阵
     */
    Eigen::Matrix4d computeTransformToJoint(int jointIndex, const QVector<double>& jointAngles) const;
    
    /**
     * @brief 验证关节角度是否在范围内
     */
    bool isJointAngleValid(int jointIndex, double theta) const;
    
    /**
     * @brief 限制关节角度在有效范围内
     */
    double clampJointAngle(int jointIndex, double theta) const;

private:
    QVector<DHParameter> m_parameters;
    
    /**
     * @brief 初始化Aubo i5H的DH参数
     * 
     * 注意: 这些是基于标准6轴协作机器人的典型参数。
     * 实际的Aubo i5H参数应该从官方文档获取并更新。
     */
    void initializeAuboI5HParameters();
};

/**
 * @brief 矩阵工具函数
 */
namespace MatrixUtils {
    /**
     * @brief 创建旋转矩阵 (绕Z轴)
     */
    Eigen::Matrix4d rotationZ(double angle);
    
    /**
     * @brief 创建旋转矩阵 (绕X轴)
     */
    Eigen::Matrix4d rotationX(double angle);
    
    /**
     * @brief 创建平移矩阵 (沿Z轴)
     */
    Eigen::Matrix4d translationZ(double distance);
    
    /**
     * @brief 创建平移矩阵 (沿X轴)
     */
    Eigen::Matrix4d translationX(double distance);
    
    /**
     * @brief 从齐次变换矩阵提取位置 (mm)
     */
    Eigen::Vector3d extractPosition(const Eigen::Matrix4d& transform);
    
    /**
     * @brief 从齐次变换矩阵提取欧拉角 (弧度)
     */
    Eigen::Vector3d extractEulerAngles(const Eigen::Matrix4d& transform);
    
    /**
     * @brief 打印矩阵用于调试
     */
    void printMatrix(const Eigen::Matrix4d& matrix, const QString& name = "Matrix");
}

#endif // DHPARAMETERS_H
