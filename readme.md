
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

<h2>
<a href="./readme_zh.md">👉简体中文</a>
</h2>

> Provides a complete AI dialogue solution for your development board, including but not limited to the `IAT(ASR)+LLM+TTS` integration solution for the `ESP32` series development boards. It is injected into the project as a dependency without affecting existing projects.

For developing the dialogue functionality of robots, you only need to prepare the `IAT(ASR)`, `LLM`, and `TTS` services, and leave the rest to `ESP-AI`.

The server-side code of this project is based on `Node.js`, and the hardware code is based on `Arduino`/`IDF`.

Open source is not easy, click the `Star` button in the upper right corner to show your support~

# 🖥 Website

- [中文网站](https://xiaomingio.top/esp-ai/)
- [English](https://xiaomingio.top/esp-ai/en)

# ✨ Features

- [x] Customizable offline voice wake-up
- [x] IAT(ASR) ➡️ LLM/RAG ➡️ TTS
- [x] User command recognition (home appliance control, singing, etc.)
- [x] Configurable
- [x] Plugin-based
- [x] The service and client have a one-to-many relationship
- [x] Server authentication
- [x] Streaming data interaction 
- [x] Ready to use



# 🧐 Next Steps

- [ ] 🤔 Provide a no-code access solution
- [ ] 🤔 Integrate AI into user intent inference (e.g., "turn off the light" and "quickly turn on the light" will both be recognized as the "turn on the light" command)
- [ ] 🤔 Offer free and paid services
- [ ] 🤔 Online generation of wake words
- [ ] 🤔 Methods for writing plugins in other languages (to avoid only using Node.js to develop plugins)
- [ ] 🤔 OTA
- [ ] 🤔 Provide a dedicated development board (to avoid current complex wiring

# 📦 Install


### Server
```bash
docker run -itd -p 8080:8080 -v /esp-ai-server/index.js:/server/index.js --name esp-ai-server registry.cn-shanghai.aliyuncs.com/xiaomingio/esp-ai:1.0.0
```
### Client
Download the dependency on the release page and burn it to the development board, see details: <a src="https://xiaomingio.top/esp-ai/start.html#%E5%AE%A2%E6%88%B7%E7%AB%AF">Client Install</a>

# 🔨 Inject Soul into Your Robot with Just a Few Lines of Code

Below are the `Node.js` and `Arduino` codes you need to write if you only require dialogue functionality.

<img src="./imgs/arduino.png" />
<img src="./imgs/nodejs.png" />


# 🏪 Discussion Group
QQ Discussion Group: 854445223
 
# 🎥 Case Study Video
[bilibili](https://www.bilibili.com/video/BV1xS421o7hi/#reply1505985392)



## 🤝 Contributing [![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg?style=flat-square)](http://makeapullrequest.com)

Let's build a better `esp-ai` together.

We warmly invite contributions from everyone. Feel free to share your ideas through [Pull Requests](https://github.com/wangzongming/esp-ai/pulls) or [GitHub Issues](https://github.com/wangzongming/esp-ai//issues).
 
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


# 🌍 Star geographical distribution
<a href="https://next.ossinsight.io/widgets/official/analyze-repo-stars-map?repo_id=820274347&activity=stars" target="_blank" style="display: block" align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="https://next.ossinsight.io/widgets/official/analyze-repo-stars-map/thumbnail.png?repo_id=820274347&activity=stars&image_size=auto&color_scheme=dark" width="721" height="auto">
    <img alt="Star Geographical Distribution of wangzongming/esp-ai" src="https://next.ossinsight.io/widgets/official/analyze-repo-stars-map/thumbnail.png?repo_id=820274347&activity=stars&image_size=auto&color_scheme=light" width="100%" height="auto">
  </picture>
</a> 