# BMYBBS API
[![Build Status](https://travis-ci.org/bmybbs/api.svg)](https://travis-ci.org/bmybbs/api) [![Coverity Scan](https://scan.coverity.com/projects/4755/badge.svg)](https://scan.coverity.com/projects/4755) [![Documentation Status](https://readthedocs.org/projects/bmybbs-api-docs/badge/?version=latest)](http://bmybbs-api-docs.readthedocs.org/)

该项目是 [bmybbs](https://github.com/bmybbs/bmybbs) 项目的 API 部分。

## 技术概要

API 是一个纯 C 编写的 HTTP 服务器，并采用 JSON 格式输出。其依赖如下的库

* [Onion](https://github.com/davidmoreno/onion) - 一个用 C 开发的 HTTP 框架。
* [json-c](https://github.com/json-c/json-c) - 一个用 C 开发的 JSON 库。
* [libxml2](http://www.xmlsoft.org/index.html) - xml 解析器，Gnome 项目的一部分。
* [libytht](https://github.com/bmybbs/bmybbs/tree/master/ythtlib)
* [libythtbbs](https://github.com/bmybbs/bmybbs/tree/master/libythtbbs)

## 代码结构

仓库中的代码主要分为两部分，业务处理以及库函数。前者直接处理 URL 请求和响应，后者向前者提供支持。库的部分包括

> api_template.c api_brc.c apilib.c

## 使用

注意 `Makefile` 中链接以及 bmybbs 引用的位置，并请先编译安装 bmybbs。完成后，可以执行：

```
$ make
$ ./bmyapi > api.log 2>&1 &
```

## 其他及支持

接口文档托管在 readthedocs.org，请访问 http://bmybbs-api-docs.readthedocs.org/

若接口返回的 `errcode` 字段不为 0，请参见 **错误码列表**。使用中若有其他问题，请至 bmybbs BMY_Dev 版面或者 Issue 列表提出。

