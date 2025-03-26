import docker
from functools import wraps

class ContainerManager:
    """
    Manages Docker containers for publishers and consumers.
    Methods
    -------
    __init__():
        Initializes the ContainerManager with a Docker client and an empty container list.
    return_container_ids(method):
        Decorator that adds container IDs to the return value of the decorated method.
    validate_container(method):
        Decorator that validates the existence of a container by its ID before executing the decorated method.
    validate_publisher_config(config):
        Validates the configuration for a publisher.
    start_publisher(config, tech_name, paused=True):
        Starts a publisher container with the given configuration and technology name. Optionally pauses the container.
    validate_consumer_config(config):
        Validates the configuration for a consumer.
    start_consumer(config, tech_name, paused=True):
        Starts a consumer container with the given configuration and technology name. Optionally pauses the container.
    wake_all():
        Unpauses all containers.
    wake_container(container_id):
        Unpauses a specific container by its ID.
    stop_all():
        Stops all containers.
    stop_container(container_id):
        Stops a specific container by its ID.
    remove_all():
        Removes all containers and clears the container list.
    remove_container(container_id):
        Removes a specific container by its ID and updates the container list.
    wait_for_all():
        Waits for all containers to finish.
    wait_for_container(container_id):
        Waits for a specific container to finish by its ID.
    """
    
    
    def __init__(self, network_name="benchmark_network"):
        self.client = docker.from_env()
        self.containers = []
        try:
            self.network = self.client.networks.get(network_name)
            self.network_name = network_name
        except docker.errors.NotFound:
            self.network = self.client.networks.create(network_name, driver="bridge")
            self.network_name = network_name
        
    @staticmethod
    def return_container_ids(method):
        @wraps(method)
        def wrapper(self, *args, **kwargs):
            # Execute the original method
            result = method(self, *args, **kwargs)

            # Add container IDs to the return value (or create one if result is None)
            container_ids = [container.name for container in self.containers]
            if result is None:
                return container_ids
            elif isinstance(result, tuple):
                return (*result, container_ids)
            else:
                return result, container_ids
        return wrapper
        
    @staticmethod
    def validate_container(method):
        @wraps(method)
        def wrapper(self, container_id, *args, **kwargs):
            container = next((c for c in self.containers if c.id == container_id), None)
            if container is None:
                raise ValueError(f"Container ID '{container_id}' not found")
            return method(self, container, *args, **kwargs)
        return wrapper
        
    # @return_container_ids
    def start_publisher(self, tech_name, pub_id, topics, pub_rate, n_messages=None, duration=None, paused = True, mode = None):
        if (n_messages is None and duration is None) or (n_messages is not None and duration is not None):
            raise ValueError("One and only one of 'n_messages' and 'duration' must be passed.")
        print(f"Starting publisher {pub_id} on topics {topics} using {tech_name}")
        try:
            container_name = f"{tech_name}-{pub_id}"
            # publisher_endpoint = "0.0.0.0" if publisher_endpoint == container_name else publisher_endpoint
            environment={
                "TECHNOLOGY": tech_name,
                "CONTAINER_ID": pub_id,
                # "PUBLISHER_ENDPOINT": publisher_endpoint,
                "TOPICS": ','.join(topics),
                "MESSAGES": n_messages,
                "DURATION": duration,
                "UPDATE_EVERY": pub_rate
            }
            print(f"Environment: {environment}")
            print(f"Starting container from image {tech_name}_publisher in mode {mode}")
            container = self.client.containers.run(
                name=container_name,
                image=f"{tech_name}_publisher",
                environment=environment,
                network=self.network_name,
                detach=True,
                command=[mode]
            )
            if paused:
                container.pause()
            self.containers.append(container)
            print(f"Created container {container.name}")
        except docker.errors.DockerException as e:
            raise ValueError(f"Failed to start publisher {pub_id}") from e
        return container.name
    
    # @return_container_ids
    def start_consumer(self, tech_name, con_id, topics, backlog_size = None, paused = True, mode = None):
        print(f"Starting consumer {con_id} subscribed to topics {topics} with backlog_size {backlog_size} using {tech_name}")
        try:
            environment = {
                "TECHNOLOGY": tech_name,
                "CONTAINER_ID": con_id,
                # "CONSUMER_ENDPOINT": f"{tech_name}-{config['endpoint']}",
                "TOPICS": ','.join(topics),
                "BACKLOG_SIZE": backlog_size
            }
            container = self.client.containers.run(
                name=f"{tech_name}_{con_id}",
                image=f"{tech_name}_consumer",
                environment=environment,
                network=self.network_name,
                detach=True,
                command=[mode]
            )
            if paused:
                container.pause()
            self.containers.append(container)
            print(f"Created container {container.name}")
        except docker.errors.DockerException as e:
            raise ValueError(f"Failed to start consumer {con_id}") from e
        return container.name

    def wake_all(self):
        print("Waking all containers...")
        for container in self.containers:
            container.unpause()
            
    @validate_container
    def wake_container(self, container_id):
        print(f"Waking container {container_id}...")
        container = self.client.containers.get(container_id)
        container.unpause()

    def stop_all(self):
        print("Stopping all containers...")
        for container in self.containers:
            container.stop()
            
    @validate_container
    def stop_container(self, container_id):
        print(f"Stopping container {container_id}...")
        container = self.client.containers.get(container_id)
        container.stop()
        
    @return_container_ids
    def remove_all(self):
        print("Removing all containers...")
        for container in self.containers:
            container.remove()
        self.containers = []
        
    @validate_container
    @return_container_ids
    def remove_container(self, container_id):
        print(f"Removing container {container_id}...")
        container = self.client.containers.get(container_id)
        container.remove()
        self.containers = [c for c in self.containers if c.id != container_id]
        
    def wait_for_all(self):
        print("Waiting for all containers to finish...")
        for container in self.containers:
            container.wait()
    
    @validate_container
    def wait_for_container(self, container_id):
        print(f"Waiting for container {container_id} to finish...")
        container = self.client.containers.get(container_id)
        container.wait()
    
    def is_healthy(self, container_id):
        status = self.client.containers.get(container_id).attrs['Health']
        return status == 'healthy'
