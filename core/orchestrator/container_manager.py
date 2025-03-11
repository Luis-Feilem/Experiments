import docker

class ContainerManager:
    """
    Manages Docker containers for publishers and consumers.
    Methods
    -------
    __init__():
        Initializes the ContainerManager with a Docker client and an empty list of containers.
    start_publisher(config, tech_name):
        Starts a publisher container with the specified configuration and technology name.
        Parameters:
        config (dict): Configuration dictionary containing 'id', 'topics', 'rate', and 'message_size'.
        tech_name (str): The technology name to use for the publisher image.
    start_consumer(config, tech_name):
        Starts a consumer container with the specified configuration and technology name.
        Parameters:
        config (dict): Configuration dictionary containing 'id' and 'topics'.
        tech_name (str): The technology name to use for the consumer image.
    stop_all():
        Stops and removes all running containers managed by this instance.
    """
    
    def __init__(self):
        self.client = docker.from_env()
        self.containers = []

    def start_publisher(self, config, tech_name):
        print(f"Starting publisher {config['id']} on topics {config['topics']} using {tech_name}")
        container = self.client.containers.run(
            image=f"{tech_name}-publisher-image",
            environment={
                "TOPICS": ','.join(config['topics']),
                "RATE": config.get('rate', 1000),
                "MESSAGE_SIZE": config.get('message_size', 256)
            },
            detach=True
        )
        self.containers.append(container)

    def start_consumer(self, config, tech_name):
        print(f"Starting consumer {config['id']} subscribed to topics {config['topics']} using {tech_name}")
        container = self.client.containers.run(
            image=f"{tech_name}-consumer-image",
            environment={
                "TOPICS": ','.join(config['topics'])
            },
            detach=True
        )
        self.containers.append(container)

    def stop_all(self):
        print("Stopping all containers...")
        for container in self.containers:
            container.stop()
            container.remove()
        self.containers = []
