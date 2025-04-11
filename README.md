# Messaging Benchmark Framework

This project provides an extensible framework to benchmark messaging technologies in controlled, repeatable conditions. The system is designed for internal experiments to evaluate latency, throughput, and overhead under various network and scenario configurations.

---

## 🧭 Project Purpose

This tool exists to support **internal, comparative evaluation** of message-passing technologies. It enables structured experimentation across combinations of:
- Message sizes and frequencies
- Numbers of producers, consumers, and topics
- Subscription patterns and topic multiplexing
- Network conditions (latency, jitter, bandwidth, loss)

The output is a basis for comparing **performance**, **overhead**, and **behavioral characteristics** across technologies.

---

## 🎯 High-Level Goals

1. **Isolate and measure technology overhead**  
   Capture the cost of using Kafka, ZeroMQ, etc., beyond the base network transport.

2. **Support modular experimentation**  
   New scenarios and technologies can be tested without modifying core logic.

3. **Automate end-to-end test orchestration**  
   Given a configuration file, the system launches the required containers, injects environment, collects logs, and tears down after execution.

---

## 🧱 Design Overview

### Architecture Principles

- **Separation of Concerns**
  - Core logic defines interfaces and orchestration, not implementation specifics.
  - Tech-specific code lives in loadable modules (`.so`, `.dll`).

- **Dynamic Factory + Shared Libs**
  - Messaging implementations register themselves at runtime via factories.

- **Technology Isolation via Docker**
  - One image per publisher/consumer pair per tech.
  - Shared `Dockerfile.base` for build dependencies.

- **Interface-Based Extensibility**
  - `IPublisher` and `IConsumer` define the contract.
  - Payloads are lightweight (`label + vector<double>`).

- **Scenario-as-Data**
  - Experiment dimensions (e.g. producers, size, rate, etc.) come from JSON.
  - The orchestrator uses these configs to coordinate container deployment.

---

## 🧪 Experiment Flow

1. Define a test scenario in JSON (see `test_scenarios/quick_test.json` for template).
2. The orchestrator reads the config, generates combinations, and launches the matching containers.
3. All containers are paused at startup, then synchronized and unpaused together.
4. Metrics and events are logged.
5. Containers terminate on poison-pill signals and are then cleaned up.

---

## 🗂 Project Structure (Core-Only)

    core/
    ├── applications/               # PublisherApp and ConsumerApp (main executables)
    ├── factory/                    # Factory pattern logic for dynamic tech binding
    ├── interfaces/                 # Core abstractions: IPublisher, IConsumer, Payload
    ├── logger/                     # Logger implementation with level-based control
    ├── orchestrator/              # Python modules for scenario execution and orchestration
    │   ├── benchmark_manager.py       # Main entry point for experiment lifecycle
    │   ├── container_manager.py       # Docker container handling logic
    │   ├── scenario_manager.py        # Scenario instantiation from JSON
    │   ├── scenario_config_manager.py# Dimension iteration and validation
    │   └── ... (metrics/events/logging)
    ├── Dockerfile.base             # Base image with C++ build dependencies
    ├── Dockerfile.publisher        # Publisher-specific image (extends base)
    ├── Dockerfile.consumer         # Consumer-specific image (extends base)

    technologies/
    ├── kafka/                      # Kafka implementation of IPublisher/IConsumer
    ├── zeromq/                     # ZeroMQ implementation
    └── ...                         # Additional tech modules

    test_scenarios/
    ├── quick_test.json             # Scenario config: topics, producers, rate, etc.
    └── ...                         # Optional experimental configurations


---

## ⚙️ Technologies

Each messaging technology lives in its own subdirectory under `technologies/`. Each implementation must:
- Extend `IPublisher` and `IConsumer`
- Register itself via the factory
- Compile into a shared object

Each also gets its own Dockerfiles for consumer/publisher images.

---

## 📌 Notes

- Message size and frequency control is currently implemented via environment variables.
- Both `DURATION` and `NUMBER_OF_MESSAGES` are supported, but only one should be set per scenario.
- The orchestrator uses Docker Python API — make sure there is access to the Docker daemon.

---

## 🧠 For Extension

When adding a new tech or extending the scenario model:
- Follow the interface and registration pattern — nothing in `core/` should need to change.
- Keep Docker images lean: start from `Dockerfile.base`, add only what’s necessary.
- Update the orchestrator only if scenario structure or orchestration behavior needs to evolve.

---

