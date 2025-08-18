const fs = require('fs');
const path = require('path');
const log = require("./log.js");

class AudioSaver {
    /**
     * 初始化音频保存器
     * @param {string} deviceId - 设备ID
     * @param {string} baseDir - 基础保存目录
     */
    constructor(deviceId, baseDir = path.join(__dirname, 'audio_analysis')) {
        this.config = {
            enabled: false,
            baseDir,
            timestamp: null,
            rawStream: null,
            processedStream: null,
            rawFilePath: '',
            processedFilePath: '',
            deviceId: (deviceId || 'unknown_device').replace(/[^a-zA-Z0-9_-]/g, '_')
        };
    }

    /**
     * 启用或关闭音频保存
     * @param {boolean} enable - true 启用，false 关闭
     */
    toggle(enable = false) {
        try {
            // 若当前状态与目标状态一致，直接返回
            if (this.config.enabled === enable) {
                this.config.enabled && log.info(`音频保存状态未变（当前: ${enable}）`);
                return;
            }

            if (enable) {
                this.config.enabled = true;
                this.initialize();
            } else {
                this.config.enabled = false;
                this.close();
            }
        } catch (error) {
            log.error(`音频保存操作失败: ${error.message}`);
            this.config.enabled = false;
            throw error;
        }
    }

    /**
     * 初始化音频保存配置
     */
    initialize() {
        // 仅在未初始化且启用状态下执行
        if (this.config.rawStream || !this.config.enabled) return;

        // 确保目录存在
        this.ensureDirectory();

        // 检查目录可写性
        this.checkDirectoryWritable();

        // 初始化文件路径
        this.initFilePaths();

        // 创建写入流
        this.createWriteStreams();

        // 测试写入
        this.testWrite();

        log.info(`音频保存已启用: 原始文件=${this.config.rawFilePath}, 处理后文件=${this.config.processedFilePath}`);
    }

    /**
     * 确保保存目录存在
     */
    ensureDirectory() {
        if (!fs.existsSync(this.config.baseDir)) {
            try {
                fs.mkdirSync(this.config.baseDir, {recursive: true, mode: 0o755});
                log.info(`已创建音频保存目录: ${this.config.baseDir}`);
            } catch (err) {
                throw new Error(`创建目录失败: ${err.message} (路径: ${this.config.baseDir})`);
            }
        }
    }

    /**
     * 检查目录是否可写
     */
    checkDirectoryWritable() {
        try {
            fs.accessSync(this.config.baseDir, fs.constants.W_OK);
            log.info(`音频保存目录验证通过: ${this.config.baseDir}`);
        } catch (err) {
            throw new Error(`目录不可写: ${this.config.baseDir} (错误: ${err.message})`);
        }
    }

    /**
     * 初始化文件路径
     */
    initFilePaths() {
        this.config.timestamp = new Date().getTime();
        const fileNamePrefix = `device_${this.config.deviceId}_${this.config.timestamp}`;
        this.config.rawFilePath = path.join(this.config.baseDir, `${fileNamePrefix}_raw.mp3`);
        this.config.processedFilePath = path.join(this.config.baseDir, `${fileNamePrefix}_processed.mp3`);
    }

    /**
     * 创建写入流
     */
    createWriteStreams() {
        // 创建原始音频文件写入流
        log.info(`创建原始音频流: ${this.config.rawFilePath}`);
        this.config.rawStream = fs.createWriteStream(this.config.rawFilePath);
        this.config.rawStream.on('error', (err) => {
            log.error(`原始音频流错误: ${err.message}`);
        });

        // 创建处理后音频文件写入流
        log.info(`创建处理后音频流: ${this.config.processedFilePath}`);
        this.config.processedStream = fs.createWriteStream(this.config.processedFilePath);
        this.config.processedStream.on('error', (err) => {
            log.error(`处理后音频流错误: ${err.message}`);
        });
    }

    /**
     * 测试写入功能
     */
    testWrite() {
        try {
            const testBuffer = Buffer.from('TEST_INIT', 'utf-8');
            // 同步写入测试数据
            fs.writeFileSync(this.config.rawFilePath, testBuffer);
            // 同步清空测试数据
            fs.truncateSync(this.config.rawFilePath, 0);
            log.info(`音频保存初始化完成: 原始文件=${this.config.rawFilePath}`);
        } catch (err) {
            // 清理测试文件
            if (fs.existsSync(this.config.rawFilePath)) {
                fs.unlinkSync(this.config.rawFilePath);
            }
            throw new Error(`测试写入失败: ${err.message}`);
        }
    }

    /**
     * 关闭所有音频流
     */
    close() {
        // 记录关闭前的状态（是否曾经启用）
        const wasEnabled = this.config.enabled;

        this.closeStream(this.config.rawStream, this.config.rawFilePath, '原始');
        this.closeStream(this.config.processedStream, this.config.processedFilePath, '处理后');

        this.config.rawStream = null;
        this.config.processedStream = null;

        // 仅当曾经启用过时，才打印关闭日志
        if (wasEnabled) {
            log.info(`音频保存已禁用: device=${this.config.deviceId}`);
        }
    }

    /**
     * 关闭单个流
     * @param {stream.Writable} stream - 要关闭的流
     * @param {string} filePath - 文件路径
     * @param {string} streamType - 流类型描述
     */
    closeStream(stream, filePath, streamType) {
        // 增加对已销毁流的判断
        if (stream && !stream.destroyed) {
            try {
                if (!stream.closed) {
                    stream.end(() => {
                        log.info(`关闭${streamType}音频流: ${filePath}`);
                    });
                }
            } catch (err) {
                log.error(`关闭${streamType}音频流失败: ${err.message}`);
            }
        }
    }

    /**
     * 写入原始音频数据
     * @param {Buffer} data - 音频数据
     * @returns {boolean} 写入结果（未初始化时返回false）
     */
    writeRaw(data) {
        // 校验数据类型
        if (!(data instanceof Buffer)) {
            log.error(`写入原始音频失败: 数据必须为Buffer类型（实际类型: ${typeof data}）`);
            return false;
        }

        // 未启用 或 流未初始化时，直接返回不报错
        if (!this.config.enabled || !this.config.rawStream) {
            // 仅在调试模式下输出，避免干扰正常日志
            log.info(`原始音频流未初始化或功能未启用，跳过写入（长度: ${data?.length || 0}）`);
            return false;
        }

        // 确认流处于可写状态
        if (this.config.rawStream.closed || this.config.rawStream.destroyed) {
            log.info(`原始音频流已关闭，跳过写入（长度: ${data?.length || 0}）`);
            return false;
        }

        // 正常写入逻辑
        const result = this.config.rawStream.write(data);
        log.info(`写入 rawStream: 长度=${data.length}, 写入结果=${result}`);

        if (!result) {
            this.config.rawStream.once('drain', () => {
                log.info(`rawStream 缓存已清空`);
            });
        }

        return result;
    }

    /**
     * 写入处理后的音频数据
     * @param {Buffer} data - 处理后的音频数据
     */
    writeProcessed(data) {
        log.info(`处理后音频输入跟踪: 数据长度=${data?.length || 0} bytes, 保存功能状态=${this.config.enabled}`);

        if (!(data instanceof Buffer)) {
            log.error(`写入处理后音频失败: 数据必须为Buffer类型（实际类型: ${typeof data}）`);
            return false;
        }

        if (!data) {
            log.info(`跳过写入: 处理后音频数据为空`);
            return;
        }

        if (!this.config.enabled) {
            log.info(`跳过写入: 音频保存功能未启用`);
            return;
        }

        if (!this.config.processedStream) {
            log.info(`跳过写入: 处理后音频流未初始化`);
            return;
        }

        try {
            // 正常写入逻辑
            const result = this.config.processedStream.write(data);
            log.info(`已写入处理后音频: 长度=${data.length} bytes, 写入结果=${result}`);

            if (!result) {
                this.config.processedStream.once('drain', () => {
                    log.info(`processedStream 缓存已清空`);
                });
            }
        } catch (err) {
            log.error(`处理后音频写入失败: ${err.message}`);
        }
    }

    /**
     * 标记原始音频保存完成
     */
    finishRaw() {
        if (this.config.enabled && this.config.rawStream) {
            this.config.rawStream.end(() => {
                log.info(`原始音频保存完成: device=${this.config.deviceId}, timestamp=${this.config.timestamp}`);
            });
            this.config.rawStream = null;
        }
    }

    /**
     * 标记处理后音频保存完成
     */
    finishProcessed() {
        if (this.config.enabled && this.config.processedStream) {
            this.config.processedStream.end(() => {
                log.info(`处理后音频保存完成: device=${this.config.deviceId}, timestamp=${this.config.timestamp}`);
            });
            this.config.processedStream = null;
        }
    }

    /**
     * 启用或禁用保存功能
     * @param {boolean} enabled - 是否启用
     */
    setEnabled(enabled) {
        this.config.enabled = enabled;
    }

    /**
     * 检查是否启用
     * @returns {boolean} 是否启用
     */
    isEnabled() {
        return this.config.enabled;
    }
}

module.exports = AudioSaver;
