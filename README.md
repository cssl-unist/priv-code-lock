# priv-code-lock

## Before getting started

- Use ssh-agent to register your ssh key so that you are not asked to put your passphrase repeatedly.

## Tested envrioment.
- Ubuntu 18.04
- AMD Xilinx Vivado 2018.02

## Steps to reproduce

```
eval "$(ssh-agent -s)"
ssh-add ~/.ssh/id_rsa
```

## Notes

- `priv-code-lock-sw` is an extension of fredom-u-sdk. Please follow README.md under priv-code-lock-sw directory to build.
- `priv-code-lock-hw` is a modified rocketchip, please follow follow README.md under priv-code-lock-hw folder.
- Please check `FPGA_manual.md` under `priv-code-lock-hw` directory for the details about the FPGA dev board we used for testing..


## Authors
- Seon Ha (UNIST)  <seonha@unist.ac.kr>
- Minsang Yu (UNIST)  <ms1021@unist.ac.kr>
- Hyungon Moon (UNIST) <hyungon@unist.ac.kr>
- Jongeun Lee (UNIST) <jlee@unist.ac.kr>

## Publications
```
@ARTICLE{10151854,
    author={Ha, Seon and Yu, Minsang and Moon, Hyungon and Lee, Jongeun},
    journal={IEEE Access}, 
    title={Kernel Code Integrity Protection at the Physical Address Level on RISC-V}, 
    year={2023},
    volume={11},
    number={},
    pages={62358-62367},
    doi={10.1109/ACCESS.2023.3285876}}

```
