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

