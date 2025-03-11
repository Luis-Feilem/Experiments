import json


class ScenarioManager:
    """
    ScenarioManager is a class responsible for managing and validating the configuration
    of a scenario, which includes publishers and consumers.
    Attributes:
        config_path (str): The path to the configuration file.
        config (dict): The loaded configuration data.
    Methods:
        __init__(config_path):
            Initializes the ScenarioManager with the given configuration path and loads the configuration.
        load_config():
            Loads the configuration from the file specified by config_path and validates it.
        validate_config(config):
            Validates the configuration to ensure it contains the required 'publishers' and 'consumers' sections.
        get_publishers():
            Returns the list of publishers from the configuration.
        get_consumers():
            Returns the list of consumers from the configuration.
        get_duration():
            Returns the duration from the configuration, defaulting to 5000 if not specified.
    """
    def __init__(self, config_path):
        self.config_path = config_path
        self.config = self.load_config()

    def load_config(self):
        with open(self.config_path, 'r', encoding='utf-8') as file:
            config = json.load(file)
        self.validate_config(config)
        return config

    def validate_config(self, config):
        if 'publishers' not in config or 'consumers' not in config:
            raise ValueError("Invalid config: missing 'publishers' or 'consumers'")

        for pub in config['publishers']:
            if 'id' not in pub or 'topics' not in pub:
                raise ValueError(f"Invalid publisher config: {pub}")

        for sub in config['consumers']:
            if 'id' not in sub or 'topics' not in sub:
                raise ValueError(f"Invalid consumer config: {sub}")

    def get_publishers(self):
        return self.config['publishers']

    def get_consumers(self):
        return self.config['consumers']

    def get_duration(self):
        return self.config.get('duration', 5000)
