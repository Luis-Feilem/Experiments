import json


class ScenarioManager:
    """
    ScenarioManager is a class responsible for managing scenarios based on a configuration file.
    It provides methods to load, validate, and retrieve information about publishers and consumers.
    Attributes:
        config_path (str): Path to the configuration file.
        config (dict): Loaded configuration data.
    Methods:
        __init__(config_path=None):
            Initializes the ScenarioManager with an optional configuration file path.
        load_config(config_path=None):
            Loads the configuration from the specified path or the instance's config_path.
            Raises ValueError if no config_path is provided.
        validate_config(config=None):
            Validates the configuration, ensuring required sections and fields are present.
            Raises ValueError if the configuration is invalid.
        validate_publisher(pub):
            Validates a publisher configuration, ensuring required fields are present.
            Raises ValueError if any required field is missing.
        validate_consumer(consumer):
            Validates a consumer configuration, ensuring required fields are present.
            Raises ValueError if any required field is missing.
        get_publishers():
            Returns the list of publishers from the configuration.
        get_publishers_ids():
            Returns a list of publisher IDs from the configuration.
        get_publisher_by_id(pub_id):
            Returns the publisher configuration with the specified ID, or None if not found.
        get_consumers():
            Returns the list of consumers from the configuration.
        get_consumers_ids():
            Returns a list of consumer IDs from the configuration.
        get_consumer_by_id(sub_id):
            Returns the consumer configuration with the specified ID, or None if not found.
    """
    
    
    def __init__(self, config_path = None):
        self.config_path = config_path
        self.config = None
        if config_path is not None:
            self.load_config(config_path)

    def load_config(self, config_path = None):
        if config_path is None:
            config_path = self.config_path
        if self.config_path is None:
            raise ValueError("No config_path provided")
        with open(config_path, 'r', encoding='utf-8') as file:
            config = json.load(file)
        self.config = config
        return self.config

    def validate_config(self, config = None):
        if config is None:
            config = self.config
        if config is None:
            raise ValueError(f"No config provided ({config}) nor loaded.")
        if 'publishers' not in config:
            raise ValueError(f"Invalid config: missing 'publishers' section ({config})")
        if 'consumers' not in config:
            raise ValueError(f"Invalid config: missing 'consumers' section ({config})")
        for pub in config['publishers']:
            self.validate_publisher(pub)
        for sub in config['consumers']:
            self.validate_consumer(sub)
        return True
            
    def validate_publisher(self, pub):
        for i in ['id', 'topics', 'messages', 'update_every']:
            if i not in pub:
                raise ValueError(f"Publisher config missing '{i}': {pub}")
        return True
    
    def validate_consumer(self, consumer):
        for i in ['id', 'topics']:
            if i not in consumer:
                raise ValueError(f"Consumer config missing '{i}': {consumer}")
        return True

    def get_publishers(self):
        return self.config['publishers']
    
    def get_publishers_ids(self):
        return [pub['id'] for pub in self.config['publishers']]
    
    def get_publisher_by_id(self, pub_id):
        for pub in self.config['publishers']:
            if pub['id'] == pub_id:
                return pub
        return None

    def get_consumers(self):
        return self.config['consumers']
    
    def get_consumers_ids(self):
        return [sub['id'] for sub in self.config['consumers']]

    def get_consumer_by_id(self, sub_id):
        for sub in self.config['consumers']:
            if sub['id'] == sub_id:
                return sub
        return None
