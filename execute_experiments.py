from core.orchestrator.benchmark_manager import BenchmarkManager


if __name__ == "__main__":
    config_path = "benchmark_scenarios.json"
    benchmark_manager = BenchmarkManager(config_path)
    benchmark_manager.run()