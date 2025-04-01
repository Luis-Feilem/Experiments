import os
from .technologies.kafka.kafka_manager import KafkaManager
from .technologies.zeromq_p2p.zeromq_p2p_manager import ZeroMQP2PManager

class TechnologyManagerInterface:
    def setup_tech(self):
        pass
    
    def reset_tech(self):
        pass
    
    def teardown_tech(self):
        pass

class TechnologyManager:
    
    def __init__(self, tech_path):
        self.tech_path = tech_path
        self.tech_name = os.path.basename(tech_path)
        if self.tech_name == "kafka":
            self.tech_manager = KafkaManager()
        elif self.tech_name == "zeromq_p2p":
            self.tech_manager = ZeroMQP2PManager()

    def setup_tech(self):
        self.tech_manager.setup_tech()
    
    def teardown_tech(self):
        self.tech_manager.teardown_tech()
        
    def reset_tech(self):
        self.tech_manager.reset_tech()
            
    def teardown_kafka(self):
        self.tech_manager.teardown_tech()
        
    def validate_technology(self):
        print(f"Inspecting files in {self.tech_path}...")
        for f in [self.base_dockerfile(), self.publisher_dockerfile(), self.consumer_dockerfile()]:
            print(f"Validating {f}...")
            if not os.path.exists(f):
                raise ValueError(f"Missing {f} in {self.tech_path}")
        return True
    
    def base_dockerfile(self):
        return os.path.join(self.tech_path, "Dockerfile.base")
    
    def publisher_dockerfile(self):
        return os.path.join(self.tech_path, "Dockerfile.publisher")
    
    def consumer_dockerfile(self):
        return os.path.join(self.tech_path, "Dockerfile.consumer")