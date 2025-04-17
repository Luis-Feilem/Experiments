import docker
import polars as pl
import os

class ContainerEventsLogger:
    def __init__(self, tech_name, scenario_name, scenario_config, separator = ';', log_level = "STUDY"):
        self.tech_name = tech_name
        self.scenario_name = scenario_name
        self.log_file = os.path.join("logs", scenario_config, tech_name, f"{scenario_name}_events.parquet")
        self.client = docker.from_env()
        self.fieldnames = [
            "timestamp", 
            "container_name", 
            "actor", 
            "message", 
        ]
        self.separator = separator
        self.logs = []
        # self.logs_schema = {
        #     "timestamp": str,
        #     "container_name": str,
        #     "actor": str, # optional
        #     "message": str,
        # }
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
                    parsed = self._parse_log(log, container.name)
                    if parsed:
                        self.logs.append(parsed)
            except Exception as e:
                print(f"[EL] Error collecting logs from container {container.id}: {e}")
        
    def write_logs(self):
        if not self.logs:
            print(f"[EL] No logs to save.")
            return
        df = pl.DataFrame(self.logs)
        df.write_parquet(self.log_file)
        # with open(self.log_file, mode='w', encoding='utf-8') as file:
        #     file.write(self.separator.join(self.fieldnames) + "\n")
        #     file.writelines(self.logs)
        print(f"[EL] Logs saved to {self.log_file}")

    def _parse_log(self, log_line, container_name):
        if not self.log_level in log_line:
            return None
        try:
            timestamp_part, rest = log_line.split(f"[{self.log_level}]", 1)
            timestamp = timestamp_part.strip()

            # Optional: standardize to ISO format (if needed)
            # timestamp = datetime.fromisoformat(timestamp).isoformat()

            # Attempt to parse actor + message if present
            if "]" in rest:
                actor_part, message = rest.strip().split("]", 1)
                actor = actor_part.strip("[ ")
                message = message.strip()
            else:
                actor = None
                message = rest.strip()

            return {
                "timestamp": timestamp,
                "container_name": container_name,
                "actor": actor,
                "message": message
            }

        except Exception as e:
            print(f"[EL] Failed to parse log line: {log_line} — {e}")
            return None
