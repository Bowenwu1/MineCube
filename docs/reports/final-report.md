# Final Report

项目地址：[MineCube on Github](https://github.com/longjj/MineCube)

## Members

| 学号     | 姓名   | Github                                      |
| -------- | ------ | ------------------------------------------- |
|          |        | [Johnny Law](https://longjj.com/)           |
| 15331310 | 吴博文 | [Bob Wu](https://github.com/Bowenwu1)       |
|          |        | [Jarvis](https://github.com/Ace-0)          |
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

![presentation](./docs/imgs/demo.gif)

![](./docs/imgs/example/Dinosaur.jpg)

![](./docs/imgs/example/Warrior.jpg)

![](./docs/imgs/example/YellowDuck.jpg)

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
- Simple lighting and shading(phong)
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





### Simple lighting and shading(phong)





### Texture mapping





### Shadow mapping





### Cloth Simulation





### Model import/export

可以将创作的结果保存/载入，这里我们没有利用传统的`obj`格式模型，而是将模型格式定义为JSON

通过调用`nlohmann::json`生成JSON字符串，随后写入文件，完成模型导出

读入模型文件，使用`nlohmann::json`解析JSON字符串，将其中的信息恢复成CPP Object即完成了模型导入的过程。



### Sky Box 





### Display Text





### Cloth Simulation





### 3D拾取





### 几何着色器



## 遇到的问题和解决方案

* 撤销操作的实现
  这里我们需要将每个对于方块的操作以某种方式保存至内存中，但是Operation多种多样，很难定义一个普适的模型去保存Operation。我们这里利用了C++11中的`lambda`表达式去定义一个Operation，通过一个**execute**的`lambda`表达式和一个**undo**的`lambda`表达式完整定义一个操作，再将其保存至一个`stack`中，完成了撤销操作的需求。

  ```Cpp
  BasicOperation(const function<void()> & execute, const function<void()> & undo);
  ```

  



## 小组成员分工

- 罗剑杰 [@Johnny Law](https://longjj.com/)
- 吴博文 [@Bob Wu](https://github.com/Bowenwu1)
  * 初版底层模块构建
  * 方块增删改查
  * 模型导入导出
  * 撤销操作的实现
- 王治鋆 [@Jarvis](https://github.com/Ace-0)
- 邱兆丰 [@Mr.Gu 菇生](https://github.com/mgsweet)
- 徐海洋 [@Hiyoung.Tsui](https://github.com/15331335)



---

**Notice**

需要用到的图片到时再打包

字数先随意发挥，等大家都写完，再整体调整