# Final Report

项目地址：[MineCube on Github](https://github.com/longjj/MineCube)

## Members

| 学号     | 姓名   | Github                                      |
| -------- | ------ | ------------------------------------------- |
| 15331229 | 罗剑杰 | [Johnny Law](https://longjj.com/)           |
| 15331310 | 吴博文 | [Bob Wu](https://github.com/Bowenwu1)       |
| 15331304 | 王治鋆 | [Jarvis](https://github.com/Ace-0)          |
|          |        | [Mr.Gu 菇生](https://github.com/mgsweet)    |
|          |        | [Hiyoung.Tsui](https://github.com/15331335) |



## MineCube简介

> **A sample voxel editor based on OpenGL 3.3+**, inspired by [MagicaVoxel](https://ephtracy.github.io/).
>
> Support Windows 10 and Mac OX currently.
>
> A final project originally of 5 undergraduate students for the course Computer Graphics, SYSU.
>
> **A good OpenGL learning example** for the green hand in Computer Graphics, while maybe not the best practice in the related field.

MineCube是一款受到[MagicaVoxel](https://ephtracy.github.io/)启发的而开发的体素编辑器，通过操作小方块创造任何你的所想！

**我们的成果！**

![presentation](../imgs/demo.gif)

![](../imgs/example/Dinosaur.jpg)

![](../imgs/example/Warrior.jpg)

![](../imgs/example/YellowDuck.jpg)

## 开发环境

**Windows / Mac** + **OpenGL 3.3+**

## 第三方库

- [GLAD](https://github.com/Dav1dde/glad)
- [GLFW Master branch](https://github.com/glfw/glfw)
- [GLM 0.9.8.5](https://github.com/g-truc/glm/releases/tag/0.9.8.5)
- [imgui v1.60](https://github.com/ocornut/imgui/releases/tag/v1.60)
- [nlohmann::json v3.1.2](https://github.com/nlohmann/json/releases/tag/v3.1.2)

## 实现功能

>
>
>可能有遗漏，继续补充
>
>

### Basic

- Camera Roaming
- Simple lighting and shading(blinn phong)
- Texture mapping
- Shadow mapping
- Cloth Simulation
- Model import/export

### Bonus

- Sky Box (天空盒)
- Display Text（显示文字）
- Complex Lighting （复杂光照: Gamma矫正）
- Cloth Simulation（织物模拟）
- 3D拾取
- 几何着色器

## 实现功能点简介

> 
>
> Basic只介绍该点在项目中的应用，Bonus要介绍具体实现原理
>
> 每个点都要附上结果截图
>
> 

### Camera Roaming

按`V`键可以切换FPS模式，进行自由移动和观察。使用`WASD`移动位置，鼠标移动来控制观察方向。

![](https://minecube-1257119828.cos.ap-guangzhou.myqcloud.com/camera-roaming.png)

### Simple lighting and shading(blinn-phong)

在场景的右上方有一个光源，对整个物体实现 blinn-phong 的光照效果，使得层次结构更加真实。

![blinn-phong](http://or5jajfqs.bkt.clouddn.com/MineCube/blinn-phong.jpg)

### Texture mapping





### Shadow mapping

当部分小立方体被挖去之后，在适当的位置出现阴影，使得层次结构更加真实。

![](http://or5jajfqs.bkt.clouddn.com/MineCube/shadow.jpg)



### Model import/export

可以将创作的结果保存/载入，这里我们没有利用传统的`obj`格式模型，而是将模型格式定义为JSON

通过调用`nlohmann::json`生成JSON字符串，随后写入文件，完成模型导出

读入模型文件，使用`nlohmann::json`解析JSON字符串，将其中的信息恢复成CPP Object即完成了模型导入的过程。



### Sky Box 

用简单对立方体贴图对方式实现。



### Display Text

使用现代对文本渲染方法，利用 freetype 库导入字体并进行纹理贴图混合。



### Cloth Simulation

采用弹簧质点模型，用粒子系统的方式对质点的速度、受力、位置进行模拟。




### 3D拾取

使用`Ray-OBB`的方法进行拾取。分为以下3步：

1. 将camera射线变换到世界坐标系。
2. 求射线与物体相交的面和距离。
3. 得到最短距离，产生这个距离的即是拾取到的方块。

射线与`OBB`的碰撞检测方法：根据射线进入和离开该物体的顺序来判断。如下，分别是射线**不穿过**和**穿过**的情形：

![](https://minecube-1257119828.cos.ap-guangzhou.myqcloud.com/picking.png)

在所有被射线穿过的方块中，距离最近的（即最前面的）方块就是被拾取的方块。

*注：如果首次运行时该功能出现异常，可能是特定屏幕下的问题，随意改变一次窗口尺寸即可恢复正常。*



### 几何着色器



## 遇到的问题和解决方案

* 撤销操作的实现
  这里我们需要将每个对于方块的操作以某种方式保存至内存中，但是Operation多种多样，很难定义一个普适的模型去保存Operation。我们这里利用了C++11中的`lambda`表达式去定义一个Operation，通过一个**execute**的`lambda`表达式和一个**undo**的`lambda`表达式完整定义一个操作，再将其保存至一个`stack`中，完成了撤销操作的需求。

  ```Cpp
  BasicOperation(const function<void()> & execute, const function<void()> & undo);
  ```

* 着色器对调试
  在进行一些纹理贴图时容易出现渲染问题，在调试时难以追踪渲染管线中对数据变化。解决办法是使用 renderDoc 软件进行单帧捕获，从而调试渲染过程中的调用与数据问题。



## 小组成员分工

- 罗剑杰 [@Johnny Law](https://longjj.com/)
  - Cmake 配置
  -  Shadow 实现
  -  blinn-phong 光照实现
  - Gamma 校正
  - 底层框架设计 
  - 实例化渲染 
  - Face Culling
- 吴博文 [@Bob Wu](https://github.com/Bowenwu1)
  * 初版底层模块构建
  * 方块增删改查
  * 模型导入导出
  * 撤销操作的实现
- 王治鋆 [@Jarvis](https://github.com/Ace-0)
  - Camera Roaming 实现
  - Shader 实现
  - 上层基本CRUD操作
  - 3D拾取
- 邱兆丰 [@Mr.Gu 菇生](https://github.com/mgsweet)
- 徐海洋 [@Hiyoung.Tsui](https://github.com/15331335)
  * 织物模拟（粒子系统）
  * 文本渲染（现代 freetype 库方法）
  * 天空盒（立方体纹理贴图）

---

**Notice**

需要用到的图片到时再打包

字数先随意发挥，等大家都写完，再整体调整
