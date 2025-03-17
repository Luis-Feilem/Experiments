import docker
import csv
import json
import os

class ContainerEventsLogger:
    def __init__(self, tech_name, scenario_name):
        self.tech_name = tech_name
        self.scenario_name = scenario_name
        self.log_file = os.path.join("logs", tech_name, f"{scenario_name}_events.csv")
        self.client = docker.from_env()
        self.fieldnames = [
            "timestamp", 
            "container_name", 
            "event_type", 
            "message_id", 
            "message_size", 
            "payload"
        ]

    def collect_logs(self):
        """Collect logs from Docker containers after experiment completion."""
        containers = self.client.containers.list(all=True, filters={"name": f"{self.tech_name}-*"})
        print(f"Collecting logs from {len(containers)} containers...")

        with open(self.log_file, mode='w', newline='', encoding='utf-8') as file:
            writer = csv.DictWriter(file, fieldnames=self.fieldnames)
            writer.writeheader()

            for container in containers:
                try:
                    logs = container.logs().decode("utf-8").strip().split("\n")
                    for log in logs:
                        event = self._parse_log(log, container.name)
                        if event:
                            writer.writerow(event)
                except Exception as e:
                    print(f"Error collecting logs from container {container.id}: {e}")

        print(f"Logs saved to {self.log_file}")

    def _parse_log(self, log, container_name):
        """
        Parse the container's log output into a structured format.
        Expected log format:
        {"timestamp": "...", "event_type": "...", "message_id": "...", "message_size": "...", "payload": "..."}
        """

        try:
            data = json.loads(log)

            # Ensure all expected fields are present
            if all(key in data for key in ["timestamp", "event_type", "message_id", "message_size", "payload"]):
                return {
                    "timestamp": data["timestamp"],
                    "container_name": container_name,
                    "event_type": data["event_type"],
                    "message_id": data["message_id"],
                    "message_size": data["message_size"],
                    "payload": data["payload"][:100]  # Truncate payload to 100 characters for readability
                }

        except json.JSONDecodeError:
            # If it's not valid JSON, ignore or handle as needed
            pass
        
        return None

# Example usage:
# logger = ContainerEventsLogger(tech_name="zeromq", scenario_name="1pub1sub1topic10msg5000ms")
# logger.collect_logs()
