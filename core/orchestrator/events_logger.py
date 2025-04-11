import docker
import csv
import json
import os

class ContainerEventsLogger:
    def __init__(self, tech_name, scenario_name, scenario_config, separator = ';', log_level = "STUDY"):
        self.tech_name = tech_name
        self.scenario_name = scenario_name
        self.log_file = os.path.join("logs", scenario_config, tech_name, f"{scenario_name}_events.csv")
        self.client = docker.from_env()
        self.fieldnames = [
            "timestamp", 
            "container_name", 
            "actor", 
            "message", 
        ]
        self.separator = separator
        self.logs = []
        self.log_level = log_level

    def collect_logs(self):
        self.logs = [] # ensure idempotency
        containers = self.client.containers.list(all=True, filters={"name": f"{self.tech_name}-*"})
        print(f"[EL] Collecting logs from {len(containers)} containers...")
        for container in containers:
            try:
                logs = container.logs(timestamps=True).decode("utf-8").strip().split("\n")
                for log in logs:
                    # else continue
                    if f"[{self.log_level}]" in log:
                        self.logs.append(self._parse_log(log, container.name))
            except Exception as e:
                print(f"[EL] Error collecting logs from container {container.id}: {e}")
        
    def write_logs(self):
        with open(self.log_file, mode='w', encoding='utf-8') as file:
            file.write(self.separator.join(self.fieldnames) + "\n")
            file.writelines(self.logs)
        print(f"[EL] Logs saved to {self.log_file}")

    def _parse_log(self, log, container_name):
        row = ""
        info_msg = log.split(f"[{self.log_level}]")
        row += info_msg[0].strip() # timestamp
        row += self.separator + container_name
        row += self.separator + info_msg[1].strip().split("]")[0][1:] # actor
        row += self.separator + ']'.join(info_msg[1].strip().split("]")[1:]).strip() # message
        row += "\n"
        return row
