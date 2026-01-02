# AiMulator

## Introduction
AiMulator is a simulator that provides a processing-in-memory (PIM) simulation, specifically SK hynix AiM.
We extend the AiM simulation from [Ramulator 2.0](https://github.com/CMU-SAFARI/ramulator2).

## Table of Contents
- [Build & Run](#build--run)
- [Simulation Overview](#simulation-overview)
- [Future Work](#future-work)
- [Contributors](#contributors)
- [Acknowledgment](#acknowledgment)
- [Reference](#reference)
- [License](#license)

## Build & Run
We follow the dependencies of Ramulator 2.0.
- g++-12
- clang++-15
Also, CMake from Ramulator 2.0 will automatically prepare the following external libraries.
- [argparse](https://github.com/p-ranav/argparse)
- [spdlog](https://github.com/gabime/spdlog)
- [yaml-cpp](https://github.com/jbeder/yaml-cpp)

### Getting Started
Clone the repository.
```bash
$ git clone https://github.com/comsys-lab/AiMulator.git
$ cd ./AiMulator/
```

#### Build
To build the executable, run:
```bash
$ mkdir build
$ cd build
$ cmake ..
$ make -j
$ cp ./aimulator ../aimulator
$ cd ..
```

#### Run
Run the executable with the configuration file specified by `-f`.
```bash
$ ./aimulator -f ./configs/example_aim.yaml
```

## Simulation Overview

## Future Work

## Contributors
- Jongmin Kim {jmkim99@korea.ac.kr}

## Reference
- [Kwon et al., JSSC, Jan. 2023](https://ieeexplore.ieee.org/document/9882182)
- [Lee et al., ISSC'22](https://ieeexplore.ieee.org/document/9731711)
- [Kwon et al., HotChips'22](https://ieeexplore.ieee.org/document/9895629)
- [Kwon et al., HotChips'23](https://ieeexplore.ieee.org/document/10254717)
- [Kim et al., HotChips'24](https://ieeexplore.ieee.org/document/10664793)

## Acknowledgement

## License