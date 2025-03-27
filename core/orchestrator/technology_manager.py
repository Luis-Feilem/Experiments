import os

class TechnologyManager:
    """
    TechnologyManager is responsible for managing and validating Dockerfiles for different technology components.
    Attributes:
        tech_path (str): The file path to the technology directory.
        tech_name (str): The name of the technology, derived from the tech_path.
    Methods:
        __init__(tech_path):
            Initializes the TechnologyManager with the given technology path.
        validate_technology():
            Raises a ValueError if any of the required Dockerfiles are missing.
        base_dockerfile():
            Returns the path to the base Dockerfile.
        publisher_dockerfile():
            Returns the path to the publisher Dockerfile.
        consumer_dockerfile():
            Returns the path to the consumer Dockerfile.
    """
    
    def __init__(self, tech_path):
        self.tech_path = tech_path
        self.tech_name = os.path.basename(tech_path)

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