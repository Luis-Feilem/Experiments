
# 📋 Project Summary: ZeroMQ Benchmarking Project

---
## 🏆 Project Goals and Structure

### **Goal:**
- Build a benchmarking system to test different messaging technologies (starting with ZeroMQ) under defined test scenarios.
- Ensure a flexible, extendable architecture using C++ interfaces and design patterns (Factory, Adapter).
- Collect performance metrics like throughput, latency, CPU, and memory usage.
- Automate deployment and execution using Docker and a custom Orchestrator.

### **Project Structure:**
```
/project_root
│── CMakeLists.txt      <-- Main project build file
│── core/
│   ├── interfaces/
│       ├── IPublisher.hpp
│       ├── IConsumer.hpp
│── technologies/
│   ├── zeromq/
│       ├── CMakeLists.txt
│       ├── ZeroMQPublisher.hpp
│       ├── ZeroMQPublisher.cpp
│       ├── ZeroMQConsumer.hpp
│       ├── ZeroMQConsumer.cpp
│       ├── ZeroMQPublisherTest.cpp
│       ├── ZeroMQConsumerTest.cpp
│── test_scenarios/
│   ├── scenario_1/
│       ├── config.yaml
│       ├── dataset.csv
│── vcpkg/  <-- Located outside the project, used for dependencies
│── logs/   <-- Output logs from experiments
```

### **Code Architecture:**
- **`core/interfaces/`** → Defines common Publisher and Consumer interfaces.
- **`technologies/zeromq/`** → ZeroMQ-specific implementation of interfaces.
- **`test_scenarios/`** → Define configurations and test cases (to be automated).
- **`logs/`** → Output logs for benchmarking results and performance.

---
## ✅ What’s Working
✅ Correct setup of `vcpkg` and toolchain for dependency management.  
✅ Successful configuration and linking of ZeroMQ library (`libzmq-mt-4_3_5.lib`).  
✅ CMake building cleanly without linker errors.  
✅ Compiler handling ZeroMQ symbols correctly.  
✅ Working CMake files with proper absolute paths and target inclusion.  
✅ Basic test structure in place.  

---
## 🐛 Known Issues/Bugs
❌ Publisher and Consumer tests not producing output or connecting.  
❌ Consumer not receiving messages from the publisher.  
❌ Suspected socket connection issue due to Windows network settings.  
❌ Possible Windows firewall interference with inter-process communication.  

---
## 🚧 Recent Fixes Attempted
✔️ Changed `tcp://*` to `tcp://127.0.0.1` for better Windows compatibility.  
✔️ Added `std::flush` to force console output.  
✔️ Added retry loop in Consumer to improve connection stability.  
✔️ Introduced delay after Publisher bind to give Consumer time to connect.  

---
## 🏗️ High-Level Design: Docker + Orchestrator

✅ **Separate Docker Containers for:**
- **Publisher** → Containerized instance that sends messages to the network.  
- **Consumer** → Containerized instance that subscribes and processes messages.  
- **Logger** → Collects and aggregates all messages from containers.  
- **Monitor** → Collects CPU, memory, and network usage.  

✅ **Orchestrator Role:**
- Reads configuration files from `test_scenarios/`.  
- Spawns Publisher and Consumer containers dynamically.  
- Ensures proper connection between containers.  
- Collects logs and performance data post-execution.  
- Cleans up containers after completion.  

✅ **Test Scenarios:**
- Configured in `test_scenarios/` using YAML/JSON files.  
- Defines parameters like:  
  - Number of Publishers and Consumers  
  - Topics  
  - Message size and frequency  
  - Duration of test  

---
## 🚀 Planned Improvements
🔹 Add structured logging for better debugging visibility.  
🔹 Create more flexible configuration for different messaging patterns (Pub/Sub, Req/Rep).  
🔹 Set up Docker infrastructure:
- Dockerfile for Publisher  
- Dockerfile for Consumer  
- Docker network for communication  
🔹 Create an orchestrator to automate testing scenarios and container lifecycle.  
🔹 Extend to other messaging technologies after ZeroMQ (e.g., Kafka, MQTT).  

---
## ✅ Next Steps
1. Confirm if connection issues are resolved.  
2. If it still fails, check network firewall settings (Windows Defender).  
3. Set up Dockerfiles for Publisher and Consumer.  
4. Prepare for orchestrator and automation design.  
5. Add structured logging for easier debugging.  

---
## 📝 Additional Notes from Latest Files
- The `root_CMakeLists.txt` correctly manages ZeroMQ dependencies and ensures proper toolchain setup.
- The `zeromq_CMakeLists.txt` defines library dependencies, handles include paths, and correctly links ZeroMQ.
- The Publisher and Consumer code appears logically sound but will need deeper inspection to address the connection issue.

---
**Next Session:** Dive into debugging the Publisher-Consumer connection issue or move on to Docker setup — your call! 😎
