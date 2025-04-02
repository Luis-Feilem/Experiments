import os

class TechnologyManager:
    
    def __init__(self, tech_path, network_name = "benchmark_network"):
        self.tech_path = tech_path
        self.tech_name = os.path.basename(tech_path)
        self.network_name = network_name

    def setup_tech(self):
        pass
    
    def teardown_tech(self):
        pass
        
    def reset_tech(self):
        pass
            
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