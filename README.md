📋 Project Summary: ZeroMQ Benchmarking Project
🏆 Goal:

Build a benchmarking system to test different messaging technologies (starting with ZeroMQ) under defined test scenarios.
Ensure a flexible, extendable architecture using C++ interfaces and design patterns (Factory, Adapter).
Collect performance metrics like throughput, latency, CPU, and memory usage.
Automate deployment and execution using Docker and an Orchestrator.
📁 Project Structure:
lua
Copy
Edit
/project_root
│── CMakeLists.txt      <-- Root CMake file for project-level build config
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
│       ├── ZeroMQPublisherApp.cpp
│       ├── ZeroMQConsumerApp.cpp
│── test_scenarios/
│   ├── scenario_1/
│       ├── config.yaml
│       ├── dataset.csv
│── vcpkg/              <-- Dependency manager (used in Windows)
│── logs/               <-- Output logs from experiments
✅ Core Components:
Interfaces:

IPublisher and IConsumer define the common interface for messaging technologies.
IPublisherApp and IConsumerApp handle app-level lifecycle and configuration.
ZeroMQ-Specific Implementations:

ZeroMQPublisher and ZeroMQConsumer implement the messaging logic for ZeroMQ.
ZeroMQPublisherApp and ZeroMQConsumerApp handle config parsing and message flow.
Docker Setup:

Dockerfile.base → Installs common dependencies and build tools.
Dockerfile.ZeroMQ → Installs ZeroMQ-specific dependencies and builds apps.
Dockerfile.ZeroMQPublisher and Dockerfile.ZeroMQConsumer → Handle runtime execution.
Test Scenarios:

Configured via YAML files.
Define number of publishers/consumers, message size, frequency, etc.
✅ What’s Working:
✅ Correct setup of vcpkg and toolchain for dependency management (Windows).
✅ Clean Docker builds for ZeroMQ using cmake and ninja.
✅ Publisher and Consumer apps now executable in containers.
✅ Established communication between Publisher and Consumer over Docker network.

🏗️ High-Level Design:
✅ Separate Docker Containers:

Publisher → Sends messages to the network.
Consumer → Subscribes and processes messages.
Logger → Collects and aggregates logs from containers.
Monitor → Collects CPU, memory, and network usage.
✅ Orchestrator Role:

Reads config files from test_scenarios/.
Spawns Publisher and Consumer containers dynamically.
Ensures proper connection between containers.
Collects logs and performance data post-execution.
✅ Test Scenarios:

Defined in test_scenarios/ using YAML/JSON files.
Defines parameters like:
Number of publishers/consumers
Topics
Message size and frequency
Duration of test
🚧 Known Issues / Open Points:
❌ First message lost (workaround in place).
❌ Orchestrator still needs to be designed and implemented.
❌ Metrics collection and logging still need to be defined and set up.

🚀 Next Steps:
Design and implement Orchestrator.
Automate deployment and teardown of containers.
Set up logging and performance monitoring.