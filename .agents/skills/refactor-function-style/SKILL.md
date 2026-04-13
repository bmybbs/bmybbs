---
name: refactor-non-prototype-function-style
description: Refactor non-prototype function declarations to modern style.
---

只处理用户在调用本 SKILL 时传入的文件。

## 如何转换

代码中存在混合风格的函数声明。只需要处理 non-prototype 风格的（如下示例，忽略空格、缩进或者换行带来的差别）：

```c
int
foo(a, b)
int a;
char *b;
{
    // function body
}
```

需要替换成：

```c
int foo(int a, char *b)
{
    // function body
}
```

实际代码中函数名、参数名、参数数量、参数类型和示例里均有差异。保持原有的函数名、参数名，仅仅将声明参数类型的位置改变到函数原型中。

重构完成后没有必要执行 build 或者任何操作，用户会人工检查。
