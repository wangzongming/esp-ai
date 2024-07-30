FROM node:18.0.0

# 设置环境变量
ENV SERVER_PATH=/server

# 设置工作目录
WORKDIR $SERVER_PATH

# 设置使用国内源
RUN sed -i 's/archive.ubuntu.com/mirrors.aliyun.com/g' /etc/apt/sources.list
# 安装解析https的相关包
RUN apt update && apt install -y apt-transport-https ca-certificates
# 添加国内的launchpad源
RUN echo "deb https://launchpad.proxy.ustclug.org/ubuntu-toolchain-r/test/ubuntu xenial main" >> /etc/apt/sources.list

# 安装pm2，使用pm2作为进程守护工具启动node服务
RUN yarn global add pm2

# 把当前目录下的所有文件拷贝到镜像的工作目录下
COPY . $SERVER_PATH

# 安装依赖
RUN yarn

# 暴露端口
EXPOSE 8080

# 创建一个挂载点，用于映射宿主机目录
VOLUME $SERVER_PATH

# 镜像运行后执行的node服务启动命令
# pm2启动服务时默认在后台运行，但是docker运行时需保持一个进程在前台，所以要加上--no-daemon参数
CMD ["pm2", "start", "index.js", "-i", "max", "--no-daemon"]


# docker 运行
# docker run -itd -p 8088:8080 -v /esp-ai-server:/server --name esp-ai esp-ai-server:1.15.5  /bin/bash
# 配置修改
# 打开 /esp-ai-server/index.js，后自行修改
