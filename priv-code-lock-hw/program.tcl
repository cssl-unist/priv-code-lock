open_hw
connect_hw_server
open_hw_target
current_hw_device [get_hw_devices xcvu9p_0]
set_property PROBES.FILE {} [lindex [get_hw_devices xcvu9p_0] 0]
set_property PROGRAM.FILE {/home/hyungon/elsa/etri-2020/priv-code-lock/freedom/builds/vcu118-u500devkit/obj/VCU118Shell.bit} [lindex [get_hw_devices xcvu9p_0] 0]
#set_property PROGRAM.FILE {/home/hyungon/elsa/etri-2020/freedom-play/freedom/builds/vcu118-u500devkit/obj/VCU118Shell.bit} [lindex [get_hw_devices xcvu9p_0] 0]
#set_property PROGRAM.FILE {/home/hyungon/elsa/etri-2020/freedom-play/freedom/builds/u500vcu118devkit/obj/U500VCU118DevKitFPGAChip.bit} [lindex [get_hw_devices xcvu9p_0] 0]
program_hw_devices [lindex [get_hw_devices xcvu9p_0] 0]
close_hw
