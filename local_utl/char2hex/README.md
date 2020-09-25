char2hex
========

本程序用于将字符串转为十六进制值。在 BMYBBS 源代码中，大量的中文字符使用 GBK 编码内置在源代码中，这导致在以 UTF-8 为主的 \*NIX 操作系统或者主流的编辑器中都需要额外留意。

鉴于 GBK 编码带来的种种不便，可以使用本程序将中文使用 hex 方式编码后再嵌入源代码，同时辅以注释，这样即可将源代码转换为 UTF-8 方式保存。

典型使用方法：

```bash
/path/to/char2hex foo
/path/to/char2hex `echo 中文测试 | iconv -t gbk`
for i in `iconv -t gbk /file/of/lines/to/be/parsed`; do /path/to/char2hex $i; done
```

