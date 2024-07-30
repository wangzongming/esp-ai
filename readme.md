

# ESP-AI [![npm](https://img.shields.io/npm/v/esp-ai.svg)](https://www.npmjs.com/package/esp-ai) [![npm](https://img.shields.io/npm/dm/esp-ai.svg?style=flat)](https://www.npmjs.com/package/esp-ai)

![logo.png](./imgs/ESP-AI.png)

<a href="./readme_zh.md">简体中文</a>

> Provides a complete AI dialogue solution for your development board, including but not limited to the `IAT(ASR)+LLM+TTS` integration solution for the `ESP32` series development boards. It is injected into the project as a dependency without affecting existing projects.

For developing the dialogue functionality of robots, you only need to prepare the `IAT(ASR)`, `LLM`, and `TTS` services, and leave the rest to `ESP-AI`.

The server-side code of this project is based on Node.js, and the hardware code is based on Arduino IDE.

Open source is not easy, click the `Star` button in the upper right corner to show your support~

# Website Homepage

- [中文网站](https://xiaomingio.top/esp-ai/)
- [English](https://xiaomingio.top/esp-ai/en)

# Features

- [x] Customizable offline voice wake-up
- [x] IAT(ASR) ➡️ LLM/RAG ➡️ TTS
- [x] User command recognition (home appliance control, singing, etc.)
- [x] Configurable
- [x] Plugin-based
- [x] The service and client have a one-to-many relationship
- [x] Server authentication
- [x] Streaming data interaction 
- [x] Ready to use



# Next Steps

- [ ] Provide a no-code access solution
- [ ] Integrate AI into the user intent inference process (e.g., "Turn off the light", "Quickly turn on the light", both recognized as the "turn on the light" command)
- [ ] Offer both free and paid services
- [ ] Online generation of wake words
- [ ] Support for writing plugins in other languages (avoiding the need to use Node.js exclusively for plugin development)
- [ ] Provide dedicated development boards (to avoid the current complex wiring)

# Inject Soul into Your Robot with Just a Few Lines of Code

Below are the `Node.js` and `Arduino` codes you need to write if you only require dialogue functionality.

<img src="./imgs/arduino.png" />
<img src="./imgs/nodejs.png" />

# Discussion Group
QQ Discussion Group: 854445223

# Detailed Usage Tutorial
[Connecting Hardware to Large Language Models (LLM) Made Simple~](https://juejin.cn/post/7384704245495234594)

# Case Study Video
[bilibili](https://www.bilibili.com/video/BV1xS421o7hi/#reply1505985392)