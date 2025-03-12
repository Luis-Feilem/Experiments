import os
import json
from .technology_manager import TechnologyManager
from .scenario_manager import ScenarioManager
from .container_manager import ContainerManager


TECHNOLOGIES_DIR = "technologies"
SCENARIOS_DIR = "test_scenarios"

class BenchmarkManager:
    
    
    def __init__(self, config_path):
        with open(config_path, 'r', encoding='utf-8') as file:
            self.config = json.load(file)

    def run(self):
        scenarios = self.config['scenarios']
        technologies = self.config['technologies']

        for tech_name in technologies:
            print(f"Running experiments for technology {tech_name}...")
            for scenario_name in scenarios:
                print(f"Running experiment for scenario {scenario_name}...")
                self.execute_experiment(tech_name, scenario_name)

    def execute_experiment(self, tech_name, scenario_name):
        technology_dir = os.path.join(TECHNOLOGIES_DIR, tech_name)
        tech_manager = TechnologyManager(technology_dir)
        print(f"Validating technology {tech_name}...")
        if not tech_manager.validate_technology():
            raise ValueError(f"Invalid technology: {tech_name}")

        scenario_dir = os.path.join(SCENARIOS_DIR, scenario_name)
        scenario_config = os.path.join(scenario_dir, "config.json")
        scenario_manager = ScenarioManager(scenario_config)
        if not scenario_manager.validate_config():
            raise ValueError(f"Invalid scenario: {scenario_name}")
        scenario_manager.load_config()
        container_manager = ContainerManager()

        try:
            print(f"Using technology {tech_name} to run scenario {scenario_name} ...")
            print("Starting and pausing all containers...")
            for pub_config in scenario_manager.get_publishers():
                container_id = container_manager.start_publisher(pub_config, tech_name)[0]
                # if not container_manager.is_healthy(container_id):
                #     raise ValueError(f"Publisher {pub_config['id']} failed to start correctly.")

            for sub_config in scenario_manager.get_consumers():
                container_id = container_manager.start_consumer(sub_config, tech_name)[0]
                # if not container_manager.is_healthy(container_id):
                #     raise ValueError(f"Consumer {sub_config['id']} failed to start correctly.")
                
            print("All containers started. Unpausing...")
            container_manager.wake_all()
            print("All containers running...")
            container_manager.wait_for_all()

        finally:
            print("Cleaning up...")
            container_manager.stop_all()
            container_manager.remove_all()