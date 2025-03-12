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
    
    
    
    def __init__(self):
        self.client = docker.from_env()
        self.containers = []
        
    
    def return_container_ids(self, method):
        @wraps(method)
        def wrapper(self, *args, **kwargs):
            # Execute the original method
            result = method(self, *args, **kwargs)

            # Add container IDs to the return value (or create one if result is None)
            container_ids = [container.id for container in self.containers]
            if result is None:
                return container_ids
            elif isinstance(result, tuple):
                return (*result, container_ids)
            else:
                return result, container_ids
        return wrapper
        
    def validate_container(self, method):
        @wraps(method)
        def wrapper(self, container_id, *args, **kwargs):
            container = next((c for c in self.containers if c.id == container_id), None)
            if container is None:
                raise ValueError(f"Container ID '{container_id}' not found")
            return method(self, container, *args, **kwargs)
        return wrapper

    def validate_publisher_config(self, method):
        @wraps(method)
        def wrapper(self, config, *args, **kwargs):
            for i in ['id', 'topics', 'messages', 'update_every']:
                if i not in config:
                    raise ValueError(f"Invalid publisher config: missing '{i}'")
            return method(self, config, *args, **kwargs)
        return wrapper
        
    @validate_publisher_config
    @return_container_ids
    def start_publisher(self, config, tech_name, paused = True):
        print(f"Starting publisher {config['id']} on topics {config['topics']} using {tech_name}")
        try:
            container = self.client.containers.run(
                name=f"{tech_name}_{config['id']}",
                image=f"{tech_name}-publisher",
                environment={
                    "CONTAINER_ID": config['id'],
                    "TOPICS": ','.join(config['topics']),
                    "MESSAGES": config['messages'],
                    "UPDATE_EVERY": config['update_every']
                },
                detach=True
            )
            self.containers.append(container)
            if paused:
                container.pause()
        except docker.errors.DockerException as e:
            raise ValueError(f"Failed to start publisher {config['id']}") from e
        return container.id

    def validate_consumer_config(self, method):
        @wraps(method)
        def wrapper(self, config, *args, **kwargs):
            for i in ['id', 'topics']:
                if i not in config:
                    raise ValueError(f"Invalid consumer config: missing '{i}'")
            return method(self, config, *args, **kwargs)
        return wrapper
    
    @validate_consumer_config
    @return_container_ids
    def start_consumer(self, config, tech_name, paused = True):
        print(f"Starting consumer {config['id']} subscribed to topics {config['topics']} using {tech_name}")
        try:
            container = self.client.containers.run(
                name=f"{tech_name}_{config['id']}",
                image=f"{tech_name}-consumer",
                environment={
                    "CONTAINER_ID": config['id'],
                    "TOPICS": ','.join(config['topics'])
                },
                detach=True
            )
            self.containers.append(container)
            if paused:
                container.pause()
        except docker.errors.DockerException as e:
            raise ValueError(f"Failed to start consumer {config['id']}") from e
        return container.id

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
        status = self.client.containers.get(container_id).attrs['State']['Health']['Status']
        return status == 'healthy'
