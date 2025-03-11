import os

class TechnologyManager:
    """
    A class to manage technology configurations and validations.
    Attributes:
    ----------
    tech_path : str
        The path to the technology directory.
    Methods:
    -------
    validate_technology():
        Validates the presence of a Dockerfile in the technology directory.
    get_tech_name():
        Returns the name of the technology based on the directory name.
    """
    
    def __init__(self, tech_path):
        self.tech_path = tech_path
        self.validate_technology()

    def validate_technology(self):
        dockerfile = os.path.join(self.tech_path, "Dockerfile")
        if not os.path.exists(dockerfile):
            raise ValueError(f"Missing Dockerfile in {self.tech_path}")

    def get_tech_name(self):
        return os.path.basename(self.tech_path)
