# FlowChar

![](https://img.shields.io/badge/version-v1.0-9cf)

一个用来从伪代码生成纯字符格式流程图的小工具。

## 背景

前几天学习红黑树，插入和删除的操作过于复杂，心想画个小流程图好让笔记看上去更直观些，又懒得用画图工具，于是就一个字符一个字符地敲，敲完发现其实也没比用画图工具轻松多少。后来我就想搞一个小工具，只要输入一些很简单的伪代码就能生成一张纯字符格式的流程图，于是就有了这个仓库。

## 示例

伪代码：

```
be born;
while (alive) {
    if (happy) {
        smile;
    }
    else {
        try to be happy;
    }
}
die;
```

流程图：

```
              +-------------+                   
              |   be born   |                   
              +-------------+                   
                     |                          
                     V                          
             N /-----------\                    
+--------------|   alive   |<------------------+
|              \-----------/                   |
|                    | Y                       |
|                    V                         |
|            Y /-----------\ N                 |
|         +----|   happy   |----+              |
|         |    \-----------/    |              |
|         |                     |              |
|         V                     V              |
|   +-----------+    +---------------------+   |
|   |   smile   |    |   try to be happy   |   |
|   +-----------+    +---------------------+   |
|         |                     |              |
|         +--------->O<---------+              |
|                    |                         |
|                    V                         |
|                    O-------------------------+
|                                               
|                                               
|               +---------+                     
+-------------->|   die   |                     
                +---------+                     
```

[*更多示例*](./demo.md)

