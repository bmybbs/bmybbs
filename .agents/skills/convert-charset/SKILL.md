---
name: convert-charset
description: Safely convert legacy-encoded C/C header source to UTF-8-readable form while preserving original runtime bytes in string literals.
---

# Convert Charset

## When to use this skill

当用户明确要求转换某个历史源码文件的文本表示方式时使用此 skill。适用文件通常位于以下文件夹（及子文件夹），并且以 `*.c` 或 `*.h` 结尾：

- include
- libbmy
- libytht
- libythtbbs
- local_utl
- nju09
- src

这里的“转换”不是把整份文件机械地转成 UTF-8，而是：

- 保持程序运行时需要输出的原始字节序列不变。
- 将注释中的文本改写为 UTF-8 可读形式。
- 将代码字符串字面量中原本依赖 GBK/GB2312/GB18030 等编码字节的非 ASCII 字符，改写为对应的 `\xHH` 字节序列。
这些字符不一定是中文，也可能是框线符号、全角符号或其他非 ASCII 字符，例如 `├`。

## How to use this skill

**不要**直接调用 `iconv` 进行全文转换。很多字符串属于界面输出，程序依旧依赖其原始编码字节。直接全文转换会改变运行时输出结果。因此请逐行方式进行转换，不要做字符集以外的变更，例如代码风格。

- 如果该行不包括需要处理的非 ASCII 字符，则不做处理。
- 如果这些字符位于注释中，可以直接转换为 UTF-8。
- 如果这些字符位于正常的代码字符串内（由 `""` 包裹），则不要直接转成 UTF-8 字面量，而应改写为对应的十六进制字节序列；ASCII 部分保持不变。
- 例如原字符串 `"中文abc"` 应改写成 `"\xD6\xD0\xCE\xC4abc"`。
- 例如原字符串 `"├abc"`，如果该符号在原文件中的字节是 `A9 C0`，则应改写为 `"\xA9\xC0abc"`。
- 需要在这一行代码的上方插入 UTF-8 注释，写出原字符串的人类可读形式，便于阅读。
- 如果同一行有多个包含这类字符的字符串，则按顺序在上方逐行添加注释，每行对应一个字符串。

## Reference

以 [gbk.c](references/gbk.c) 作为转换前的代码、[utf.c](references/utf.c) 作为转换后的代码做参考。
