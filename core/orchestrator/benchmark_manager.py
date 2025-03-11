import json
import time
from technology_manager import TechnologyManager
from scenario_manager import ScenarioManager
from container_manager import ContainerManager

class BenchmarkManager:
    """
    Manages the execution of benchmark scenarios using different technologies.
    Attributes:
        config (dict): Configuration loaded from the provided config_path.
    Methods:
        __init__(config_path):
            Initializes the BenchmarkManager with the configuration file path.
        run():
            Executes the benchmark scenarios using the specified technologies.
    """
    
    def __init__(self, config_path):
        with open(config_path, 'r', encoding='utf-8') as file:
            self.config = json.load(file)

    def run(self):
        scenarios = self.config['scenarios']
        technologies = self.config['technologies']

        for tech_path in technologies:
            tech_manager = TechnologyManager(tech_path)
            tech_name = tech_manager.get_tech_name()

            for scenario_path in scenarios:
                scenario_manager = ScenarioManager(scenario_path)
                container_manager = ContainerManager()

                try:
                    print(f"Running scenario {scenario_path} using technology {tech_name}")
                    for pub_config in scenario_manager.get_publishers():
                        container_manager.start_publisher(pub_config, tech_name)

                    for sub_config in scenario_manager.get_consumers():
                        container_manager.start_consumer(sub_config, tech_name)

                    duration = scenario_manager.get_duration() / 1000
                    print(f"Running test for {duration} seconds...")
                    time.sleep(duration)

                finally:
                    container_manager.stop_all()
