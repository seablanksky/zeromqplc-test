# Qt 5.12.12 + ZeroMQ 进程间通信示例

本工程包含两个 Qt 控制台进程：

- `ipc_server`：ZeroMQ `REP` 服务端。
- `ipc_client`：ZeroMQ `REQ` 客户端。

默认 endpoint：

- Linux/macOS：`ipc:///tmp/qt_zeromq_demo.ipc`
- Windows：`tcp://127.0.0.1:5555`

说明：ZeroMQ 官方 `zmq_ipc(7)` 文档说明 `ipc://` 传输目前只在提供 UNIX domain socket 的系统上实现。因此在 Windows + Qt/MSVC 环境下，示例默认使用本机 TCP 作为同机进程间通信替代。如果需要真正的 `ipc://`，请在 Linux/macOS 上运行，或确认你的 libzmq 版本和系统明确支持该传输。

## 目录结构

```text
zeromq_ipc_demo.pro
common/
  zeromq_common.pri
  zmq_helpers.h
  zmq_helpers.cpp
ipc_server/
  ipc_server.pro
  main.cpp
ipc_client/
  ipc_client.pro
  main.cpp
```

## ZeroMQ 依赖

工程使用 ZeroMQ C API，只需要 `zmq.h`、导入库和运行时动态库。

默认查找目录：

```text
third_party/libzmq/include/zmq.h
third_party/libzmq/lib/libzmq.lib
```

也可以通过环境变量指定：

```powershell
$env:ZMQ_ROOT="D:\libs\libzmq"
$env:ZMQ_LIB_NAME="libzmq"
```

如果你的库文件名是 `zmq.lib`，则设置：

```powershell
$env:ZMQ_LIB_NAME="zmq"
```

运行时请确保 `libzmq.dll` 在可执行文件同目录，或已经加入 `PATH`。

## 编译

使用 Qt 5.12.12 的 qmake：

```powershell
C:\Qt\Qt5.12.12\5.12.12\msvc2015_64\bin\qmake.exe zeromq_ipc_demo.pro
nmake
```

如果使用 Qt Creator，直接打开 `zeromq_ipc_demo.pro`，选择 Qt 5.12.12 MSVC Kit 构建。

## 运行

先启动服务端：

```powershell
.\ipc_server\release\ipc_server.exe
```

再启动客户端：

```powershell
.\ipc_client\release\ipc_client.exe
```

自定义 endpoint：

```powershell
.\ipc_server\release\ipc_server.exe tcp://127.0.0.1:6000
.\ipc_client\release\ipc_client.exe tcp://127.0.0.1:6000 10
```

Linux/macOS 上使用真正的 IPC endpoint：

```bash
./ipc_server/ipc_server ipc:///tmp/qt_zeromq_demo.ipc
./ipc_client/ipc_client ipc:///tmp/qt_zeromq_demo.ipc 10
```
