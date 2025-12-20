# Scripts Directory

This directory contains batch scripts for environment setup and verification.

## 📁 Files

### Environment Scripts
- **`setup_environment.bat`** - Interactive environment configuration helper
- **`verify_setup.bat`** - Comprehensive environment verification
- **`debug_paths.bat`** - Debug script for troubleshooting path issues
- **`test_encoding.bat`** - Test script for encoding verification

## 🚀 Usage

### From Project Root Directory
```cmd
# Run environment setup
scripts\setup_environment.bat

# Verify environment
scripts\verify_setup.bat

# Debug path issues
scripts\debug_paths.bat

# Test encoding
scripts\test_encoding.bat
```

### From Scripts Directory
All scripts automatically change to the project root directory, so they can be run from anywhere:

```cmd
cd scripts
setup_environment.bat
verify_setup.bat
```

## 🔧 What Each Script Does

### setup_environment.bat
- Guides you through environment configuration
- Validates tool installations (VTK, OpenCASCADE)
- Generates `config/paths.cmake` configuration file
- Tests CMake configuration

### verify_setup.bat
- Checks all required tools (CMake, Git, Visual Studio)
- Validates project directory structure
- Tests CMake configuration with Qt paths
- Verifies third-party library paths
- Checks for test data files

### debug_paths.bat
- Shows current working directory
- Lists all project directories
- Displays configuration file contents
- Helps troubleshoot path-related issues

### test_encoding.bat
- Tests if batch file encoding is working correctly
- Displays various character types
- Helps verify Windows terminal compatibility

## ⚠️ Requirements

- Windows 10/11
- Visual Studio 2019/2022
- CMake 3.16+
- Git
- Qt 6.10+
- VTK 9.2 (compiled)
- OpenCASCADE 7.8 (compiled)

## 🐛 Troubleshooting

If scripts fail to find directories or files:

1. **Run from project root**: Make sure you're in `K:\vsCodeProjects\qtSpraySystem\`
2. **Check paths**: Run `debug_paths.bat` to see what's being detected
3. **Verify structure**: Ensure all required directories exist (src, docs, config, etc.)
4. **Check permissions**: Make sure you have read/write access to the project directory

## 📝 Notes

- All scripts use English to avoid encoding issues
- Scripts automatically change to project root directory using `cd /d "%~dp0\.."`
- Error messages include detailed debugging information
- Visual Studio detection uses `vswhere.exe` for accuracy