# Fast Gaussian Blur
实时高斯模糊， CPU算法4为Photopea的实现方案
* 原图
<div>
  <img src="./Images/800x200.png" width="800px"/> 
</div>

## CPU方案
Demo中给出了4种算法的实现，窗口标题为算法所耗时间
* 算法1
<div>
  <img src="./Images/CPU/standard.png" width="800px"/> 
</div>

* 算法2
<div>
   <img src="./Images/CPU/algorithm2.png" width="800px"/> 
</div>

* 算法3
<div>
  <img src="./Images/CPU/algorithm3.png" width="800px"/> 
</div>

* 算法4
<div>
  <img src="./Images/CPU/algorithm4.png" width="800px"/> 
</div>

## GPU方案
<div>
  <img src="./Images/GPU/blurGPU.gif" width="800px"/> 
</div>
<div>
  <img src="./Images/GPU/blurGPU46.png" width="800px"/> 
</div>
<div>
  <img src="./Images/GPU/BlurGPU93.png" width="800px"/> 
</div>
<br>
<br>

## 参考资料及其他资料
http://blog.ivank.net/fastest-gaussian-blur.html

https://www.peterkovesi.com/papers/FastGaussianSmoothing.pdf

https://blog.csdn.net/fightingforcv/article/details/51785681
