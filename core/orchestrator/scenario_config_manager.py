import json
import itertools

EXCLUSIVE_MSG = "numberOfMessages"
EXCLUSIVE_TIME = "testDurationS"

class ScenarioConfigManager:
    
    def __init__(self, config_file):
        self.assignment_strats = [
            "producerAssignmentStrategy",
            "consumerAssignmentStrategy"
        ]
        self.common_parts = [
            "numProducers",
            "numConsumers", 
            "numTopics", 
            "parallelSubscriptionsPerTopic",
            "messageSizeBytes",
            "producerWaitInMicroSeconds",
            "backlogSizeMessages"
        ]
        self.network_parts = [
            "bandwidthMbps",
            "latencyMs",
            "packetLossPerc",
            "jitterMs"
        ]
        self.exclusive_parts = [
            EXCLUSIVE_TIME,
            EXCLUSIVE_MSG
        ]
        print(f"[SCM] Scenario config file: {config_file}")
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
        for strat in self.assignment_strats:
            generators[strat] = iter(self.config[strat])
        generators[exclusive_part] = self.iter_over_range(**self.config["exclusive"][exclusive_part])
        for scenario_config in itertools.product(*generators.values()):
            scenario = dict(zip(generators.keys(), scenario_config))
            yield scenario
    
    @staticmethod
    def generate_scenario_name(scenario):
        name_parts = []

        # Common messaging identifiers
        p = str(scenario.get("numProducers", "P")).replace('.','_')
        c = str(scenario.get("numConsumers", "C")).replace('.','_')
        t = str(scenario.get("numTopics", "T")).replace('.','_')
        pc = str(scenario.get("parallelSubscriptionsPerTopic", "PC")).replace('.','_')
        b = str(scenario.get("messageSizeBytes", "B")).replace('.','_')
        w = str(scenario.get("producerWaitInMicroSeconds", "W")).replace('.','_')
        bm = str(scenario.get("backlogSizeMessages", "BM")).replace('.','_')
        name_parts.append(f"{p}p{c}c{t}t{pc}pc{b}b{w}us{bm}bm")

        # Exclusive mode
        if EXCLUSIVE_TIME in scenario:
            name_parts.append(f"{int(scenario[EXCLUSIVE_TIME])}s")
        elif EXCLUSIVE_MSG in scenario:
            name_parts.append(f"{int(scenario[EXCLUSIVE_MSG])}m")

        # Network-related identifiers
        bw = str(scenario.get("bandwidthMbps", "BW")).replace('.','_')
        lat = str(scenario.get("latencyMs", "L")).replace('.','_')
        pl = str(scenario.get("packetLossPerc", "PL")).replace('.','_')
        jit = str(scenario.get("jitter", "J")).replace('.','_')
        name_parts.append(f"{bw}mbps{lat}ms{pl}pl{jit}j")

        return "-".join(name_parts)
    
    @staticmethod
    def get_producerAssignmentStrategy(scenario):
        return scenario['producerAssignmentStrategy']
    
    @staticmethod
    def get_consumerAssignmentStrategy(scenario):
        return scenario['consumerAssignmentStrategy']
    
    @staticmethod
    def get_numProducers(scenario):
        return scenario['numProducers']
 
    @staticmethod
    def get_numConsumers(scenario):
        return scenario['numConsumers']
 
    @staticmethod
    def get_numTopics(scenario):
        return scenario['numTopics']
 
    @staticmethod
    def get_parallelSubscriptionsPerTopic(scenario):
        return scenario['parallelSubscriptionsPerTopic']

    @staticmethod
    def get_messageSizeBytes(scenario):
        return scenario['messageSizeBytes']

    @staticmethod
    def get_producerWaitInMicroSeconds(scenario):
        return scenario['producerWaitInMicroSeconds']

    @staticmethod
    def get_backlogSizeMessages(scenario):
        return scenario['backlogSizeMessages']


    @staticmethod
    def get_bandwidthMbps(scenario):
        return scenario['bandwidthMbps']

    @staticmethod
    def get_latencyMs(scenario):
        return scenario['latencyMs']

    @staticmethod
    def get_packetLossPerc(scenario):
        return scenario['packetLossPerc']

    @staticmethod
    def get_jitterMs(scenario):
        return scenario['jitterMs']


    @staticmethod
    def get_numberOfMessages(scenario):
        if 'numberOfMessages' in scenario:
            return scenario['numberOfMessages']
        else:
            return None

    @staticmethod
    def get_testDurationS(scenario):
        if 'testDurationS' in scenario:
            return scenario['testDurationS']
        else:
            return None
