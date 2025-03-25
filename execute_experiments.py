import sys
from core.orchestrator.benchmark_manager import BenchmarkManager


if __name__ == "__main__":
    config_path = "benchmark_scenarios.json"
    benchmark_manager = BenchmarkManager(config_path, metrics_interval=0.1)
    mode = None
    if len(sys.argv) > 1:
        mode = sys.argv[1]
    i = 0
    while i < len(sys.argv):
        print(f"[BenchmarkManager] Argument {i}: {sys.argv[i]}")
        i += 1
    print(f"[BenchmarkManager] Executing benchmark in mode {mode}")
    benchmark_manager.run(mode=mode)
    