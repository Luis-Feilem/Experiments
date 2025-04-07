import os
import json
from .technologies.kafka_manager import KafkaManager
from .technologies.zeromq_p2p_manager import ZeroMQP2PManager
from .scenario_manager import ScenarioManager
from .container_manager import ContainerManager
from .metrics_collector import MetricsCollector
from .events_logger import ContainerEventsLogger
from .scenario_config_manager import ScenarioConfigManager, EXCLUSIVE_MSG, EXCLUSIVE_TIME

TECHNOLOGIES_DIR = "technologies"
SCENARIOS_DIR = "test_scenarios"

class BenchmarkManager:
    
    def __init__(self, config_path, metrics_interval=2.0):
        self.interval = metrics_interval
        with open(config_path, 'r', encoding='utf-8') as file:
            self.config = json.load(file)
        self.scenario_config_files = self.config['scenario_batch']
        self.scenario_batch_name = ""
        self.scm = None
        self.cm = ContainerManager()
        self.tm = None

    def run(self, mode = None):
        for scenario_batch in self.config['scenario_batch']:
            self.run_config(scenario_batch, mode)
    
    def run_config(self, scenario_batch, mode = None):
        self.scm = ScenarioConfigManager(os.path.join(SCENARIOS_DIR, scenario_batch))
        self.scenario_batch_name = scenario_batch.split("json")[0]
        print(f"[BM] Using scenario_config from {self.scenario_batch_name}")
        technologies = self.config['technologies']
        for tech_name in technologies:
            if tech_name == "kafka":
                self.tm = KafkaManager(os.path.join(TECHNOLOGIES_DIR,tech_name))
            elif tech_name == "zeromq_p2p":
                self.tm = ZeroMQP2PManager(os.path.join(TECHNOLOGIES_DIR, tech_name))
            else:
                print(f"[BM] Unknown technology: {tech_name}")
                return
            print(f"[BM] Validating technology {tech_name}...")
            if not self.tm.validate_technology():
                raise ValueError(f"Invalid technology: {tech_name}")
            print(f"[BM] Setting up {tech_name} extra resources...")
            self.tm.setup_tech()
            print(f"[BM] Running experiments for technology {tech_name} in mode {mode}...")
            for scenario_messages in self.scm.iter_valid_combinations(EXCLUSIVE_MSG):
                self.execute_experiment(tech_name, scenario_messages, mode)
            for scenario_time in self.scm.iter_valid_combinations(EXCLUSIVE_TIME):
                self.execute_experiment(tech_name, scenario_time, mode)
            self.tm = None

    def execute_experiment(self, tech_name, scenario_config, mode = None):
        scenario_name = ScenarioConfigManager.generate_scenario_name(scenario_config)
        metrics = MetricsCollector(tech_name, scenario_name, self.scenario_batch_name, interval=self.interval)
        try:
            print(f"[BM] Using technology {tech_name} to run scenario {scenario_name} ...")
            print(f"[BM] Starting and pausing all containers in mode {mode}...")
            sm = ScenarioManager(scenario_config)
            for p_id, p_config in sm.publisher_configs().items():
                print(f"[BM] starting publisher with config {p_config}")
                container_id = self.cm.start_publisher(
                    tech_name = tech_name,
                    **p_config,
                    mode = mode
                )
                # if not container_manager.is_healthy(container_id):
                #     raise ValueError(f"Publisher {pub_config['id']} failed to start correctly.")

            for c_id, c_config in sm.consumer_configs().items():
                print(f"[BM] starting consumer with config {c_config}")
                container_id = self.cm.start_consumer(
                    tech_name, 
                    **c_config, 
                    mode = mode
                )
                # if not container_manager.is_healthy(container_id):
                #     raise ValueError(f"Consumer {sub_config['id']} failed to start correctly.")
            
            metrics.start()    
            print("[BM] All containers started. Unpausing...")
            self.cm.wake_all()
            print("[BM] All containers running...")
            self.cm.wait_for_all()
            metrics.stop()
            events_logger = ContainerEventsLogger(tech_name, scenario_name, self.scenario_batch_name)
            events_logger.collect_logs()
            events_logger.write_logs()

        finally:
            print("[BM] Cleaning up...")
            self.cm.stop_all()
            self.cm.remove_all()
            self.tm.reset_tech()
            print(f"[BM] Producer and Consumer containers removed, {self.tm.tech_name} reset completed")