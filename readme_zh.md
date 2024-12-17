
<div align="center"> 
<a name="readme-top"></a>
  
<div style="background:#fff;border-radius: 12px;width:300px;">
  <img src="https://espai.fun/images/logo.png"/> 
</div>

<h1>ESP-AI</h1>

硬件接入AI最简单、最低成本的方案<br/>The simplest and lowest cost solution for any item to access AI


[![NPM version][npm-image]][npm-url] 
[![NPM downloads][download-image]][download-url]
[![][bundlephobia-image]][bundlephobia-url] 

[Changelog](https://espai.fun/change-logs.html) · 
[中文文档](https://espai.fun/) · 
[English Docs](https://espai.fun/en)

![](https://raw.githubusercontent.com/andreasbm/readme/master/assets/lines/rainbow.png)

[npm-image]: https://img.shields.io/npm/v/esp-ai.svg?style=flat-square
[npm-url]: https://npmjs.org/package/esp-ai
[download-url]: https://npmjs.org/package/esp-ai
[bundlephobia-image]: https://badgen.net/bundlephobia/minzip/esp-ai?style=flat-square
[download-image]: https://img.shields.io/npm/dm/esp-ai.svg?style=flat
[bundlephobia-url]: https://bundlephobia.com/package/esp-ai 
    
 
<div style="background:#fff;border-radius: 12px;">
<img src="https://espai.fun/images/ESP-AI.png"/>
</div>


</div>

<a href="./readme_english.md">English</a>
 
 
> 为你的开发板提供全套的AI对话方案，包括但不限于 `ESP32` 系列开发板的 `IAT(ASR)+LLM+TTS` 集成方案。依赖式注入到项目，不影响现有项目。

对于开发机器人的对话功能，您仅需准备好 `IAT(ASR)`、`LLM`、`TTS` 服务，其他的事情交给 `ESP-AI`。


本项目服务端代码基于 `Nodejs`，硬件代码基于 `Arduino`/`IDF`。

开源不易，右上角点个 `Star` 支持一下下吧 ~


# 🖥 网站主页 

- [中文文档](https://espai.fun/)
- [English](https://espai.fun/en)

# 🖥 开放平台 

基于 ESP-AI 向企业以及个人提供端服务、与管理服务的平台。[去看看开放平台](https://dev.espai.fun/)。
提供免费的 ASR、TTS、LLM 服务，在平台中仅需 15s 音频您即可克隆出自定义音色。


# ✨ 特性

- ✔️ 可定制的离线语音唤醒词，且内置多种唤醒方式（语音、按钮、串口、天问asrpro）
- ✔️ 完整对话链接 IAT(ASR) ➡️ LLM/RAG ➡️ TTS
- ✔️ TTS/LLM 都做了快速响应算法，在考虑服务费用的基础上尽力以最快速度响应用户
- ✔️ 支持会话打断
- ✔️ 用户指令识别(家电控制、唱歌等)，并且可以根据上下文动态响应指令
- ✔️ 配置化
- ✔️ 插件化，可以利用插件接入任何LLM/TTS/IAT
- ✔️ 服务与客户端为一对多关系，并且可以为每一个客户端(硬件)独立分配一套配置
- ✔️ 连接支持鉴权 
- ✔️ 全链流式数据交互 
- ✔️ 开发者平台提供：免费服务、可视化配置 ... 
- ✔️ 客户端配网页面提供
- ✔️ 轻松应付大并发场景(需配合Nginx做负载均衡) 
- ✔️ 开箱即用

# 🧐 下一步 

- [ ] 🤔 内置离线唤醒精准度提升(目前建议使用天问asrpro)  
- [ ] 🤔 唤醒词在线生成 
- [ ] 🤔 其他语言编写插件的方法（避免只能使用nodejs进行开发插件） 

# 📦 安装

### 服务端
```bash
docker run -itd -p 8088:8088 -v /esp-ai-server/index.js:/server/index.js --name esp-ai-server registry.cn-shanghai.aliyuncs.com/xiaomingio/esp-ai:1.0.0
```
### 客户端
在发布页面下载依赖后烧录到开发板中即可，详情见：[客户端安装](https://espai.fun/start.html#%E5%AE%A2%E6%88%B7%E7%AB%AF)

# 🔨 仅几行代码为您的机器人注入灵魂

下面分别展示在只需要对话的情况下，你需要写的`Node.js`和`Arduino`代码。
 
<img src="https://espai.fun/images/arduino.png" />
<img src="https://espai.fun/images/nodejs.png" />

# 🏪 交流群
QQ 交流群: 854445223

 
# 🎥 案例视频
[bilibili](https://www.bilibili.com/video/BV1gE421w7Dw/?share_source=copy_web&vd_source=041c9610a29750f498de1bafe953086b)



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

# 引用
如果本项目有帮助到您的研究，请引用我们：

    @software{ESP-AI,
        title        = {{ESP-AI}},
        author       = {小明IO},
        year         = 2024,
        journal      = {GitHub repository},
        publisher    = {GitHub},
        howpublished = {\url{https://github.com/wangzongming/esp-ai}}
    }