# priv-code-lock

## Before getting started

- Use ssh-agent to register your ssh key so that you are not asked to put your passphrase repeatedly.

# Tested envrioment.
- ubuntu 18.04


## Reproduce

```
eval "$(ssh-agent -s)"
ssh-add ~/.ssh/id_rsa
```


# Software: priv-code-lock-sw

- This is extension of fredom-u-sdk follow README.md under priv-code-lock-sw folder.
 


# Hardware: priv-code-lock-hw
- we modified rocketchip, follow README.md under priv-code-lock-hw folder.

## Need to software 
- vivado 2018.02 (Xlinx)



## FPGA detail
- See follow FPGA_manual.md under priv-code-lock-hw folder.

