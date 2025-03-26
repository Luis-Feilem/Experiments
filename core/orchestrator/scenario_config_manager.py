import json
import itertools
import pandas as pd

class ScenarioConfigManager:
    
    def __init__(self, config_file):
        self.common_parts = [
            "numProducers",
            "numConsumers", 
            "numTopics", 
            "parallelChannelsPerTopic",
            "messageSizeBytes",
            "producerRatePerSecond",
            "backlogSizeMessages"
        ]
        self.network_parts = [
            "bandwidthMbps",
            "latencyMs",
            "packetLossPerc",
            "jitterMs"
        ]
        self.exclusive_parts = [
            "testDurationS",
            "numberOfMessages"
        ]
        with open(config_file, 'r', encoding='utf-8') as file:
            self.config = json.load(file)
        
    @staticmethod
    def iter_over_range(lower, upper, step, step_operator, midpoint=None, step2=None, step_operator2=None):
        current = lower - step if step_operator== '+' else lower / step # hack to yield `lower` as the first value
        while current <= upper:
            if midpoint and step2 and current >= midpoint:
                current = current + step2 if step_operator2 == '+' else current * step2
            else:
                current = current + step if step_operator == '+' else current * step
            if current > upper:
                break
            yield current
    
    def iter_valid_combinations(self, exclusive_part):
        generators = {}
        for common_part in self.common_parts:
            generators[common_part] = self.iter_over_range(**self.config["common"][common_part])
        for network_part in self.network_parts:
            generators[network_part] = self.iter_over_range(**self.config["network"][network_part])
        generators[exclusive_part] = self.iter_over_range(**self.config["exclusive"][exclusive_part])
        for scenario_config in itertools.product(*generators.values()):
            scenario = dict(zip(generators.keys(), scenario_config))
            yield scenario
    
    def generate_scenario_name(self, scenario):
        name_parts = []

        # Common messaging identifiers
        p = scenario.get("numProducers", "P")
        c = scenario.get("numConsumers", "C")
        t = scenario.get("numTopics", "T")
        pc = scenario.get("parallelChannelsPerTopic", "PC")
        b = scenario.get("messageSizeBytes", "B")
        r = scenario.get("producerRatePerSecond", "R")
        bm = scenario.get("backlogSizeMessages", "BM")
        name_parts.append(f"{p}p{c}c{t}t{pc}pc{b}b_{r}r{bm}bm")

        # Exclusive mode
        if "testDurationS" in scenario:
            name_parts.append(f"{scenario['testDurationS']}s")
        elif "numberOfMessages" in scenario:
            name_parts.append(f"{scenario['numberOfMessages']}m")

        # Network-related identifiers
        bw = scenario.get("bandwidthMbps", "BW")
        lat = scenario.get("latencyMs", "L")
        pl = scenario.get("packetLossPerc", "PL")
        jit = scenario.get("jitter", "J")
        name_parts.append(f"{bw}mbps_{lat}ms_{pl}pl_{jit}j")

        return "_".join(name_parts)