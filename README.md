Repo to manage the experiments run on benchmarking messaging technologies.

The repo structure is as follows:

README.md: this file
run_experiment.bat: executable to run the experiments
config.json: configuration file to adjust the parameters of the benchmark (technologies, test scenarios, iterations, output format)
benchmark_driver.exe: orchestrator of the experiments. The run_experiments.bat file uses this, and is responsible for starting any processes, creating producers, topics, consumers, etc.
test_scenarios folder: contains the files necessary to configure the use cases to experiment with as well as the results produced by benchmarking each of the candidate technologies
technologies folder: contains a folder for each candidate technology. These folders contain the files necessary to setup, run, and teardown the experiments.
logs folder: contains log files from running the experiments
agg_results folder: contains post-processed data based on the results of testing each technology on each scenario.

