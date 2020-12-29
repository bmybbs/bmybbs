#!/bin/sh
# 微信小程序的 npm 构建只会将 dayjs/index.js 放置在 miniprogram_npm/dayjs 下面，
# 因此需要手动建立目录引入本地化和扩展组件

mkdir -p miniprogram_npm/dayjs/locale
mkdir -p miniprogram_npm/dayjs/plugin

cp node_modules/dayjs/locale/zh-cn.js           miniprogram_npm/dayjs/locale/
cp node_modules/dayjs/plugin/localizedFormat.js miniprogram_npm/dayjs/plugin/
cp node_modules/dayjs/plugin/relativeTime.js    miniprogram_npm/dayjs/plugin/

