# NanoEnv: The Lightning-Fast Remote Development Environment

## 🚀 Overview

NanoEnv is a **high-performance remote development environment** designed to be **blazing fast**, fully **self-hosted**,
and offer a **SaaS option** for teams that want a **zero-effort** cloud solution. Unlike traditional remote development
solutions that rely on Docker, NanoEnv uses **LXC (Linux Containers) and Lima (for macOS support)** to deliver
near-native performance while maintaining **full isolation and security**.

## 🔥 Key Features

- **⚡ Insanely Fast**: Starts environments **in milliseconds**, not seconds.
- **🛠️ Self-Hosted & SaaS**: Choose between self-hosted **(free & open-source)** or the **SaaS-managed** option.
- **🌎 Multi-Platform**: Supports **Linux (via LXC)** and **macOS (via Lima)**.
- **🔗 No Docker Required**: Runs **natively on the host** without Docker overhead.
- **📡 Remote Access**: Securely connect to your **remote dev environments** from **VS Code, JetBrains, or CLI**.
- **⚙️ API & CLI**: Easily automate environments with **Drogon-powered API** and **CLI11-based command-line interface**.
- **🔒 Secure & Isolated**: Each development environment is **fully sandboxed**.
- **🔌 Extensible & Modular**: Built on a **microservice architecture**, allowing **easy customization**.

## 🏗️ Architecture

NanoEnv follows a **highly modular microservices architecture**, ensuring **clean separation of concerns** while
maximizing **performance and scalability**.

```
├── 📂 core/ (Core container & platform abstraction)
│   ├── 📂 platform/ (OS-specific runtime logic)
│   ├── 📂 containers/ (LXC/Lima-based container management)
│   ├── 📂 storage/ (Persistent storage & volume management)
│   ├── 📂 networking/ (Network isolation & port forwarding)
│   ├── 📂 auth/ (User authentication & permissions)
│
├── 📂 api/ (REST API, built with Drogon)
│   ├── 📂 controllers/ (Handles API requests)
│   ├── 📂 models/ (Data structures & request validation)
│   ├── 📂 middleware/ (Security, rate limiting, etc.)
│
├── 📂 cli/ (Command-line interface, built with CLI11)
│
├── 📂 frontend/ (Web UI for managing environments)
│
├── 📂 deployment/ (Kubernetes, Ansible, Terraform scripts)
```

## ⚡ Installation (No Dependencies Required)

### **1️⃣ Install NanoEnv (Prebuilt Binary)**

#### **Linux/macOS (Automatic Install)**

```sh
curl -L https://nanoenv.io/install.sh | bash
```

This will:

- **Download the correct prebuilt binary** for your OS.
- **Automatically set up LXC (Linux) or Lima (macOS)**.
- **Place `nanoenv` in `/usr/local/bin/`** for global access.

#### **Manual Install**

1. Download the prebuilt binary:
   ```sh
   curl -LO https://nanoenv.io/releases/latest/nanoenv-linux-x86_64
   chmod +x nanoenv-linux-x86_64
   mv nanoenv-linux-x86_64 /usr/local/bin/nanoenv
   ```
2. **(Linux only)** Install LXC:
   ```sh
   sudo apt install lxc
   ```
3. **(macOS only)** Install Lima:
   ```sh
   brew install lima
   limactl start nanoenv
   ```

#### **Windows (WSL2)**

NanoEnv is **not natively supported on Windows**, but can be used with **WSL2**:

```sh
wsl --install
curl -L https://nanoenv.io/install.sh | bash
```

### 🏗️ Building from Source

If you'd like to build NanoEnv from source, follow these steps:

#### **1️⃣ Install Dependencies**

```sh
sudo apt install cmake ninja-build lxc liblxc-dev libdrogon-dev
```

For macOS:

```sh
brew install cmake ninja lima zlib jsoncpp simdjson openssl pigz drogon uuid brotli zstd libpsl
```

#### **2️⃣ Clone the Repository & Build**

```sh
git clone https://github.com/nanoenv/nanoenv.git
cd nanoenv
mkdir build && cd build
cmake .. _DCMAKE_BUILD_TYPE=Release -G Ninja
ninja && ninja install
```

#### **3️⃣ Run the NanoEnv API**

```sh
nanoenv_api --config ../config.yaml
```

#### **4️⃣ Create & Start a Development Environment**

```sh
nanoenv create --name my-env --image ubuntu:22.04
nanoenv start my-env
```

### ☁️ **SaaS (Fully Managed) Option**

Simply sign up at [nanoenv.io](https://nanoenv.io) and launch **instant remote dev environments** in seconds.

## 🖥️ CLI Usage

```sh
nanoenv create --name my-env --image ubuntu:22.04
nanoenv start my-env
nanoenv stop my-env
nanoenv destroy my-env
```

## 📡 API Usage

The API exposes **fully RESTful endpoints** for automation.

```sh
curl -X POST https://api.nanoenv.io/environments \
  -H "Authorization: Bearer YOUR_TOKEN" \
  -d '{"name": "my-env", "image": "ubuntu:22.04"}'
```

## 🏎️ Performance Benchmarks

NanoEnv vs. GitPod & DevContainers:
| **Feature**        | **NanoEnv**   | **GitPod**  | **DevContainers** |
|-------------------|--------------|------------|----------------|
| **Startup Time**   | ⚡ ~200ms | ❌ 4-10s | ❌ 3-7s |
| **Memory Usage**   | ✅ Low | ❌ High | ❌ High |
| **Requires Docker?** | ❌ No | ✅ Yes | ✅ Yes |
| **Self-Hosted?**   | ✅ Yes | ❌ No | ✅ Yes |

## 🔥 Why Choose NanoEnv?

✅ **Faster than GitPod & DevContainers** 🚀  
✅ **No Docker Required** 🛠️  
✅ **Self-Hosted + SaaS Model** 💰  
✅ **Low Overhead & Near-Native Performance** 🏎️

## 🛠️ Contributing

We welcome contributions! To contribute:

1. Fork the repo & create a new branch.
2. Follow the **C++23 coding guidelines** (strict best practices).
3. Submit a pull request!

## 📜 License

NanoEnv is **open-source** and released under the **MIT License**.

## 📢 Stay Updated

🔗 **Website:** [nanoenv.io](https://nanoenv.io)  
📖 **Docs:** [docs.nanoenv.io](https://docs.nanoenv.io)  
🐙 **GitHub:** [github.com/nanoenv](https://github.com/nanoenv)  
💬 **Community:** [Discord](https://discord.nanoenv.io)

---

🚀 **NanoEnv: The Fastest Remote Development Experience** 🏎️

