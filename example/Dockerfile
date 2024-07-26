FROM node:18.0.0

# 设置环境变量
ENV SERVER_PATH=/server

# 设置工作目录
WORKDIR $SERVER_PATH

# 安装pm2，使用pm2作为进程守护工具启动node服务
RUN yarn global add pm2

# 把当前目录下的所有文件拷贝到镜像的工作目录下
COPY . $SERVER_PATH

# 安装依赖
RUN yarn

# 暴露端口
EXPOSE 8080

# 镜像运行后执行的node服务启动命令
# pm2启动服务时默认在后台运行，但是docker运行时需保持一个进程在前台，所以要加上--no-daemon参数
CMD ["pm2", "start", "src/index.js", "-i max", "--no-daemon"]
