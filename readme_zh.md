
<div align="center"> 
<a name="readme-top"></a>

![logo.png](https://xiaomingio.top/esp-ai/images/logo.png)

<h1>ESP-AI</h1>

硬件接入AI最简单、最低成本的方案<br/>The simplest and lowest cost solution for any item to access AI


[![NPM version][npm-image]][npm-url] 
[![NPM downloads][download-image]][download-url]
[![][bundlephobia-image]][bundlephobia-url] 

[Changelog](https://xiaomingio.top/esp-ai/change-logs.html) · 
[中文文档](https://xiaomingio.top/esp-ai/) · 
[English Docs](https://xiaomingio.top/esp-ai/)

![](https://raw.githubusercontent.com/andreasbm/readme/master/assets/lines/rainbow.png)

[npm-image]: https://img.shields.io/npm/v/esp-ai.svg?style=flat-square
[npm-url]: https://npmjs.org/package/esp-ai
[download-url]: https://npmjs.org/package/esp-ai
[bundlephobia-image]: https://badgen.net/bundlephobia/minzip/esp-ai?style=flat-square
[download-image]: https://img.shields.io/npm/dm/esp-ai.svg?style=flat
[bundlephobia-url]: https://bundlephobia.com/package/esp-ai 
   
<!-- ![logo.png](./imgs/ESP-AI.png) -->

![logo.png](https://xiaomingio.top/esp-ai/images/ESP-AI.png)

</div>

<a href="./readme_english.md">English</a>
 
 
> 为你的开发板提供全套的AI对话方案，包括但不限于 `ESP32` 系列开发板的 `IAT(ASR)+LLM+TTS` 集成方案。依赖式注入到项目，不影响现有项目。

对于开发机器人的对话功能，您仅需准备好 `IAT(ASR)`、`LLM`、`TTS` 服务，其他的事情交给 `ESP-AI`。


本项目服务端代码基于 `Nodejs`，硬件代码基于 `Arduino`/`IDF`。

开源不易，右上角点个 `Star` 支持一下下吧 ~


# 🖥 网站主页 

- [中文文档](https://xiaomingio.top/esp-ai/)
- [English](https://xiaomingio.top/esp-ai/en)


# ✨ 特性

- [x] 可定制的离线语音唤醒
- [x] IAT(ASR) ➡️ LLM/RAG ➡️ TTS
- [x] 用户指令识别(家电控制、唱歌等)
- [x] 配置化
- [x] 插件化
- [x] 服务与客户端为一对多关系
- [x] 服务端鉴权 
- [x] 流式数据交互 
- [x] 开箱即用

# 🧐 下一步 

- [ ] 🤔 唤醒精准度提升
- [ ] 🤔 提供无代码接入方案
- [ ] 🤔 用户意图推理过程加入AI（如： 帮我关灯、快开灯，都将识别为: "开灯" 指令）
- [ ] 🤔 提供免费测试服务
- [ ] 🤔 唤醒词在线生成
- [ ] 🤔 客户端 OTA 支持
- [ ] 🤔 其他语言编写插件的方法（避免只能使用nodejs进行开发插件）
- [ ] 🤔 提供专用开发板（避免当前的复杂接线）

# 📦 安装

### 服务端
```bash
docker run -itd -p 8080:8080 -v /esp-ai-server/index.js:/server/index.js --name esp-ai-server registry.cn-shanghai.aliyuncs.com/xiaomingio/esp-ai:1.0.0
```
### 客户端
在发布页面下载依赖后烧录到开发板中即可，详情见：[客户端安装](https://xiaomingio.top/esp-ai/start.html#%E5%AE%A2%E6%88%B7%E7%AB%AF)

# 🔨 仅几行代码为您的机器人注入灵魂

下面分别展示在只需要对话的情况下，你需要写的`Node.js`和`Arduino`代码。

<!-- <img src="./imgs/arduino.png" />
<img src="./imgs/nodejs.png" /> -->

<img src="https://xiaomingio.top/esp-ai/images/arduino.png" />
<img src="https://xiaomingio.top/esp-ai/images/nodejs.png" />

# 🏪 交流群
QQ 交流群: 854445223

 
# 🎥 案例视频
[bilibili](https://www.bilibili.com/video/BV1xS421o7hi/#reply1505985392)



## 🤝 贡献 [![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg?style=flat-square)](http://makeapullrequest.com)

让我们一起打造一个更好的 `esp-ai`。

我们热忱欢迎大家踊跃投稿。请随时通过[Pull Requests](https://github.com/wangzongming/esp-ai/pulls)或[GitHub Issues](https://github.com/wangzongming/esp-ai//issues)分享您的想法。


<table>
<tr>
  <td> 
<a href="https://next.ossinsight.io/widgets/official/compose-recent-top-contributors?repo_id=820274347" target="_blank" style="display: block" align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="https://next.ossinsight.io/widgets/official/compose-recent-top-contributors/thumbnail.png?repo_id=820274347&image_size=auto&color_scheme=dark" width="280" height="auto">
    <img alt="Top Contributors of wangzongming/esp-ai - Last 28 days" src="https://next.ossinsight.io/widgets/official/compose-recent-top-contributors/thumbnail.png?repo_id=820274347&image_size=auto&color_scheme=light" width="280" height="auto">
  </picture>
</a>
 
  </td>
  <td rowspan="2"> 
    <a href="https://next.ossinsight.io/widgets/official/compose-last-28-days-stats?repo_id=820274347" target="_blank" style="display: block" align="center">
    <picture>
        <source media="(prefers-color-scheme: dark)" srcset="https://next.ossinsight.io/widgets/official/compose-last-28-days-stats/thumbnail.png?repo_id=820274347&image_size=auto&color_scheme=dark" width="655" height="auto">
        <img alt="Performance Stats of wangzongming/esp-ai - Last 28 days" src="https://next.ossinsight.io/widgets/official/compose-last-28-days-stats/thumbnail.png?repo_id=820274347&image_size=auto&color_scheme=light" width="655" height="auto">
    </picture>
    </a> 
  </td>
</tr>
<tr>
  <td> 
<a href="https://next.ossinsight.io/widgets/official/compose-recent-active-contributors?repo_id=820274347&limit=30" target="_blank" style="display: block" align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="https://next.ossinsight.io/widgets/official/compose-recent-active-contributors/thumbnail.png?repo_id=820274347&limit=30&image_size=auto&color_scheme=dark" width="273" height="auto">
    <img alt="Active Contributors of wangzongming/esp-ai - Last 28 days" src="https://next.ossinsight.io/widgets/official/compose-recent-active-contributors/thumbnail.png?repo_id=820274347&limit=30&image_size=auto&color_scheme=light" width="273" height="auto">
  </picture>
</a> 
  </td>
</tr>
</table>

# 🌍 Star 地理分布
 
<a href="https://next.ossinsight.io/widgets/official/analyze-repo-stars-map?repo_id=820274347&activity=stars" target="_blank" style="display: block" align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="https://next.ossinsight.io/widgets/official/analyze-repo-stars-map/thumbnail.png?repo_id=820274347&activity=stars&image_size=auto&color_scheme=dark" width="721" height="auto">
    <img alt="Star Geographical Distribution of wangzongming/esp-ai" src="https://next.ossinsight.io/widgets/official/analyze-repo-stars-map/thumbnail.png?repo_id=820274347&activity=stars&image_size=auto&color_scheme=light" width="100%" height="auto">
  </picture>
</a> 