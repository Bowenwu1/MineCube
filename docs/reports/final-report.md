# Final Report

项目地址：[MineCube on Github](https://github.com/longjj/MineCube)

## Members

| 学号       | 姓名   | Github                                   |
| -------- | ---- | ---------------------------------------- |
| 15331229 | 罗剑杰  | [Johnny Law](https://longjj.com/)        |
| 15331310 | 吴博文  | [Bob Wu](https://github.com/Bowenwu1)    |
|          |      | [Jarvis](https://github.com/Ace-0)       |
|          |      | [Mr.Gu 菇生](https://github.com/mgsweet)   |
|          |      | [Hiyoung.Tsui](https://github.com/15331335) |



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
- Model import/export

### Bonus

- Sky Box (天空盒)
- Display Text（显示文字）
- Complex Lighting （复杂光照: Gamma矫正）
- Cloth Simulation（织物模拟）
-  Gravity System and Collision Detection (重力系统与碰撞检测) （织物那里）
- 3D拾取

## 实现功能点简介

> 
>
> Basic只介绍该点在项目中的应用，Bonus要介绍具体实现原理
>
> 每个点都要附上结果截图
>
> 

### Camera Roaming





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





### Display Text



### Complex Lighting （复杂光照: Gamma矫正）

人眼看到颜色的亮度更倾向于顶部的灰阶，监视器使用的也是一种指数关系（电压的2.2次幂），所以物理亮度通过监视器能够被映射到顶部的非线性亮度，这个非线性映射的确可以让亮度在我们眼中看起来更好，但当渲染图像时，会产生一个问题：我们在应用中配置的亮度和颜色是基于监视器所看到的，这样所有的配置实际上是非线性的亮度/颜色配置。



![Gamma 校正曲线图](https://learnopengl-cn.github.io/img/05/02/gamma_correction_gamma_curves.png)

Gamma 校正( Gamma Correction )的思路是在最终的颜色输出上应用监视器Gamma的倒数。再看上面的Gamma 曲线图，你会有一个短划线，它是监视器Gamma曲线的翻转曲线。我们在颜色显示到监视器的时候把每个颜色输出都加上这个翻转的Gamma曲线，这样应用了监视器 Gamma 以后最终的颜色将会变为线性的。我们所得到的中间色调就会更亮，所以虽然监视器使它们变暗，但是我们又将其平衡回来了。



只需要在 Blinn-Phong shader 的片段着色器最后加上下面代码既可以实现 Gamma 校正。

```
void main()
{
    // do super fancy lighting 
    [...]
    // apply gamma correction
    float gamma = 2.2;
    fragColor.rgb = pow(fragColor.rgb, vec3(1.0/gamma));
}
```

实验结果如下：

![gamma_correction](http://or5jajfqs.bkt.clouddn.com/MineCube/gamma_correction.jpg)



### Cloth Simulation





###  Gravity System and Collision Detection (重力系统与碰撞检测) （织物那里）





### 3D拾取







## 遇到的问题和解决方案

* 撤销操作的实现
  这里我们需要将每个对于方块的操作以某种方式保存至内存中，但是Operation多种多样，很难定义一个普适的模型去保存Operation。我们这里利用了C++11中的`lambda`表达式去定义一个Operation，通过一个**execute**的`lambda`表达式和一个**undo**的`lambda`表达式完整定义一个操作，再将其保存至一个`stack`中，完成了撤销操作的需求。

  ```Cpp
  BasicOperation(const function<void()> & execute, const function<void()> & undo);
  ```

* 大量立方体渲染导致的渲染性能下降

  我们的初始方案是让每一个小立方体独自渲染自己，我们很快就会因为绘制调用过多而达到性能瓶颈，最多只能渲染出一个 10x10x10 的大立方体，这样远远不能达到创作的要求。与绘制顶点本身相比，使用 glDrawArrays 或 glDrawElements 函数告诉 GPU 去绘制你的顶点数据会消耗更多的性能，因为 OpenGL 在绘制顶点数据之前需要做很多准备工作（比如告诉GPU该从哪个缓冲读取数据，从哪寻找顶点属性，而且这些都是在相对缓慢的 CPU 到 GPU 总线上进行的）。所以，即便渲染顶点非常快，命令GPU去渲染却未必。

  我们采用了是**实例化( Instancing )**的计算机图形学方法，将数据**一次性发送给GPU**，然后在顶层（CubeManager）使用一个绘制函数让 OpenGL 利用这些数据绘制多个物体，从而极大地提升了渲染的效率。目前项目可以流畅渲染 20x20x20 的立方体，当编译出来的是 release 版本的程序的时候， **30x30x30 的大立方体也可以流畅支持。**

  ![](http://or5jajfqs.bkt.clouddn.com/MineCube/architecture.jpg)

* 阴影支持 bias 过大

  使用作业的 Phong shader 的时候，因为小立方体之间的间隔较小，shader 中的 bias 设置得过大 (0.005) , 使得一些部分的渲染因为 bias 过大而认为没有处于阴影之中。


![error_shadow](http://or5jajfqs.bkt.clouddn.com/MineCube/error_shadow.jpg)



​	将 bias 适当调小后让阴影可以正常渲染。	

![correct shadow](http://or5jajfqs.bkt.clouddn.com/MineCube/temp_shadow.jpg)

## 小组成员分工

- 罗剑杰 [@Johnny Law](https://longjj.com/)
  - Cmake 配置
  - Shadow Map 实现
  - blinn-phong 光照实现
  - Gamma 校正
  - 底层逻辑设计 
  - 实例化渲染 
  - 面剔除
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
