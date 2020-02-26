# Fast Gaussian Blur
实时高斯模糊， CPU算法4为Adobe Photoshop的实现方案
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
T.T gif录出来的效果太差了. 有前辈知道好用的gif录制工具还请告知。

这个gif是拿GifCam录的。Screen2Gif也试过了。。还没这个效果好。。
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