# SelfLearningTask

## 编译
1. mkdir Build
2. cd Build & cmake .. -G "Visual Studio 15 2017 Win64" (以 VS 2017 为例)
3. 设置 Editor 或 WinEditor 作为启动项,前者为 Imgui 后者为 Win32
4. 运行

## 操作
- 默认模式为旋转模式
- 鼠标左键：旋转视角
- 鼠标右键：拖动视角
- left alt + 鼠标左键：旋转光源, x 方向旋转 theta, y 方向旋转 phi 
- 鼠标滚轮：视角缩放
- left shift + `：切换到自由模式
- 进入自由模式后 WSAD 和鼠标移动控制视角和前进方向
- Esc：从自由模式切换回旋转模式
- TAB：切换模型
- Win32 暂不支持自由模式
- P：切换阴影, 支持 hard shadow, PCF, PCSS
- Q：切换剔除模式，支持 背面剔除，前面剔除，无剔除