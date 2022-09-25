#!/usr/bin/env python3

import argparse,sys

def pmm_check(args):

    alloc_list = []
    print("Executing checking pmm",file=sys.stderr)
    def backtrace(addr):
        
        print(hex(addr),file=sys.stderr,flush=True)
        for alloc_addr in alloc_list:
            print(hex(alloc_addr),file=sys.stderr,end=" ",flush=True)
        print("",flush=True)

    for line in sys.stdin.read().splitlines():
        if args.verbose:
            print(line)
        l = line.split(',')
        op = l[0]
        try:
            addr = int(l[1],base=16)
        except ValueError:
            continue
        except IndexError:
            continue

        if op == 'A':
            if addr in alloc_list:
                # backtrace(addr)
                raise SystemError("Double alloc error. Please recheck your malloc implementation.")
            alloc_list.append(addr)
            
        elif op == 'F':
            if addr not in alloc_list:
                # backtrace(addr)
                raise SystemError("Double free error. Please recheck your malloc implementation.")
            alloc_list.remove(addr)
def main(args):
    if args.mode == 'pmm':
        pmm_check(args)
    else:
        return
    print("Checking passed!")

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--mode', type=str, dest="mode", required=True)
    parser.add_argument('--verbose', '-v', action='store_true')
    main(parser.parse_args())
