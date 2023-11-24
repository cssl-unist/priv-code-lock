import sys, os, subprocess, logging, re, multiprocessing
# Read file
# [0]  find isolation sw on!
# [0] pc: XXXXX -> XXX parsing
# subprocess run addr2line
# result write in new file

defaultLogFile = "logs/spike.log"
getAddrFile = "logs/spike_function_addr.log"
defaultOutFile = "logs/add2lineResult.log"   
path_vmlinux = "work/riscv-linux/vmlinux"




logging.basicConfig(stream = sys.stdout, level=logging.DEBUG)
logger = logging.getLogger('run-eval')


def call_addr2line(addrs):
    pattern = '0x([0-9a-f]+): ([a-zA-Z_0-9]+) at (.*)$'
    pattern_inline = ' (\(inlined by\)) ([a-zA-Z_0-9]+) at (.*)$'
    print(len(addrs))
    logs = []
    cmd = ['addr2line']
    cmd += ["-f"]
    cmd += ["-a"]
    cmd += ["-p"]
    cmd += ["-i"]
    cmd += ["-e"]
    cmd += ["./"+path_vmlinux]
    cmd += addrs
    #logger.info('running:' + ' '.join(cmd))
    proc = subprocess.run(cmd,stderr=subprocess.STDOUT, stdout=subprocess.PIPE)
    #logs += "["
    #logs += addrs[0]
    #logs += "] "
    output = proc.stdout.decode('utf-8').split('\n')
    while len(output) != 0:
        head = output[0]
        output = output[1:]
        if ("(inlined by)" in head):
            raise Exception()
        elif not head:
            break
        matched = re.match(pattern, head)
        if not matched:
            if (head.split()[1] == '??'):
                addr = head.split(":")[0][2:]
                fname = '??'
                location = '??'
            else:
                raise Exception()
        else:
            addr = matched.group(1)
            fname = matched.group(2)
            location = matched.group(3)
        entry = [addr, (fname, location)]
        while "(inlined by)" in output[0]:
            head = output[0]
            output = output[1:]
            
            matched = re.match(pattern_inline, head)
            fname = matched.group(2)
            location = matched.group(3)
            entry.append((fname, location))
        logs.append(entry)

    return logs


def runAddr2line(log, outFileName):

    addrs = list(map(lambda x: x[0], log))
    insns = list(map(lambda x: x[1], log))
    

    logger.info("found {} instruction step".format(len(log)))
    num_logs = len(log)
    len_split = 10000
    splited = []
    for i in range(0,num_logs//len_split):
        if (i+1)*len_split < num_logs:
            len((addrs[i * len_split:(i+1)*len_split]))
            splited.append(addrs[i * len_split:(i+1)*len_split])
        else:
            splited.append(addrs[i * len_split:])
    #fnames = []

    with multiprocessing.Pool(10) as p:
        fnames = p.map(call_addr2line, splited)
    aa = []
    for f in fnames:
        aa += f
    fnames = aa

    #for s in splited:
    #    fnames += call_addr2line(s)

    logStr = ''
    logNotfound = ''
    delete_sw = False
    non_inline_opcodes = set([])
    for i in range(1, len(fnames)):
        '''
        if logs[i-1][0] == 'ffffffe000d9aa82':
            delete_sw = True
        elif logs[i-1][0] == '80001b4' and delete_sw == True :
            delete_sw = False
        elif delete_sw :
            continue
        elif logs[i-1][1] != logs[i][1] : 
            if insns[i-1].split(':')[-1].strip() == ('jal' or 'jalr' or 'jr' or 'j') :
                logStr += '[J-type] {}: {} | Instr: {}  \n'.format(logs[i-1][0], logs[i-1][1], insns[i-1])
            else:
                 logStr += '{}: {} | Instr: {}  \n'.format(logs[i-1][0], logs[i-1][1], insns[i-1])
        ''' 
        addr = fnames[i-1][0]
        outer_ftn_name_before = fnames[i-1][-1][0]
        outer_ftn_name_after = fnames[i][-1][0]
        inner_ftn_name_before = fnames[i-1][1][0]
        inner_ftn_name_after = fnames[i][1][0]
        sp = insns[i-1]
        mnemonic = sp[1].split()[0].strip()
        committed = sp[2].strip()
        
        if inner_ftn_name_before != inner_ftn_name_after:
            if outer_ftn_name_before != outer_ftn_name_after:
                inlined_jump = False
                jump_symbol = '  ->'
                non_inline_opcodes.add(mnemonic)
                
            else:
                inlined_jump = True
                jump_symbol = '(i)->'
            
            thisLog = 'op: {:>8} | committed: {:>3} | pc: {:>10} | {:>30} {:>} {:>30} '.format(
                mnemonic, committed, addrs[i-1], inner_ftn_name_before, 
            jump_symbol, inner_ftn_name_after)
            if (inlined_jump):
                thisLog += '| {:>30} {:>} {:>30} |\n'.format(outer_ftn_name_before, '->', outer_ftn_name_after)
            else:
                thisLog += '\n'
            logStr += thisLog
            if (mnemonic == 'li' and not inlined_jump and addrs[i-1] != 'ffffffe000d9b756'):
                    print(thisLog[:-1])
    print('non_inline_opcodes: {}'.format(non_inline_opcodes))
    with open(outFileName, 'w') as fw:
        fw.write(logStr)
        
    print("runAddr2line done\n")


def find_isolation_sw_on(logFile, outFileName):
    sw = False
    log = []
    count = 0
    with open(logFile) as f:
        lines = f.readlines()
        for line in lines:
            count += 1
            sp = line.split("::")
            if sp[0] == "Seon":
                addr = sp[1]
                if len(sp) > 2 :
                    entry = (addr, sp[2:])
                    log.append(entry)
                else:
                    print("Error log need to fix line number is ")       
                    print(count)
                    print("\n code: n")
                    print(sp)
    #with open(getAddrFile, 'w') as fw:
    #    fw.write(str(log))

 

    return log


# def test():
#     #pattern = '0x([0-9a-f]+): ([a-zA-Z_]+) at (.*)$'
#     pattern = '(\(inlined by\)) ([a-zA-Z_]+) at (.*)$'
#     #inp = '0xffffffe000c370ee: arch_local_irq_save at /home/hyungon/elsa/priv-code-lock/freedom-u-sdk/riscv-linux/arch/riscv/include/asm/irqflags.h:42'
#     inp = '(inlined by) __printk_safe_enter at /home/hyungon/elsa/priv-code-lock/freedom-u-sdk/riscv-linux/kernel/printk/printk_safe.c:352'
#     res = re.match(pattern, inp)
#     print(res.group(1))
#     print(res.group(2))
#     print(res.group(3))


if __name__ == '__main__':
    # test()
    # exit()
    if len(sys.argv) > 2:
        logFile = sys.argv[1]
        outFile = sys.argv[2]
    elif len(sys.argv) == 2:
        logFile = sys.argv[1]
        outFile = defaultOutFile
    else:
        logfile = defaultLogFile
        outFile = defaultOutFile
        
    log = find_isolation_sw_on(logFile, outFile)
    runAddr2line(log, outFile)
 
