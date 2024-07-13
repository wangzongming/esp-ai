
# ESP-AI 插件开发文档

插件分为3类
- IAT 语音识别，用于将用户说的话转为文字
- LLM 大语言模型，当然也可以对接知识库等，主要用于推理用户问题
- TTS 用于将文字转为语音


欢迎大家开发自己的专属插件，插件发布后可以在 `issues` 或者交流群中提交自己的插件，插件合规时将会把插件挂到插件列表中。

# 插件开发流程

1. 代码编写
2. 使用 `npm publish` 发布到 npm 仓库（如果只是内部使用可省略）
3. 提交 `issues` 到 `esp-ai` 仓库，审核通过将插件挂到插件列表中（如果只是内部使用可省略）


# IAT 插件开发指南

**命名规则**：esp-ai-plugin-iat-xxx  [必须用这种规则]

详情见案例仓库：https://github.com/wangzongming/esp-ai-plugin-iat-example

文档补充中...

# LLM 插件开发指南

**命名规则**：esp-ai-plugin-llm-xxx  [必须用这种规则]

详情见案例仓库：https://github.com/wangzongming/esp-ai-plugin-llm-example

文档补充中...


# TTS 插件开发指南

**命名规则**：esp-ai-plugin-tts-xxx  [必须用这种规则]

详情见案例仓库：https://github.com/wangzongming/esp-ai-plugin-tts-ttson

文档补充中...
