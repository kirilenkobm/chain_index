#!/usr/bin/env python3
"""Just a collection of functions."""
import sys
import ctypes
import os

___author__ = "Bogdan Kirilenko"
__email__ = "kirilenk@mpi-cbg.de"

MODES = {"index", "extract"}


class chain_index:
    """Just a collection of functions."""
    def __init__(self):
        """Connect binaries."""
        self.LOCATION = os.path.dirname(__file__)

        self.lib_location = os.path.join(self.LOCATION, "chain_index_lib.so")
        try:
            self.sh_lib = ctypes.CDLL(self.lib_location)
        except OSError:
            sys.stderr.write("ERROR!\n")
            sys.stderr.write(f"Shared library {self.lib_location}")
            sys.stderr.write(" not found!\n")
            self.__build_lib()

        self.sh_lib.make_index.argtypes = [ctypes.POINTER(ctypes.c_uint64),
                                           ctypes.POINTER(ctypes.c_uint64),
                                           ctypes.POINTER(ctypes.c_uint64),
                                           ctypes.c_uint64,
                                           ctypes.c_char_p]
        self.sh_lib.make_index.restype = ctypes.c_int

        self.sh_lib.get_s_byte.argtypes = [ctypes.c_char_p,
                                           ctypes.c_uint64,
                                           ctypes.POINTER(ctypes.c_uint64),
                                           ctypes.POINTER(ctypes.c_uint64)]
        self.sh_lib.get_s_byte.restype = ctypes.c_uint64

    def __build_lib(self):
        """Build shared library."""
        import subprocess  # yes, I know
        sys.stderr.write("Building library...")
        source_file = os.path.join(self.LOCATION, "chain_index_lib.c")
        if not os.path.isfile(source_file):
            sys.exit("ERROR! Shared lib source not found!")

        cmd = f"gcc -fPIC -shared -o {self.lib_location} {source_file}"
        rc = subprocess.call(cmd, shell=True)
        if rc == 0:
            sys.stderr.write("Build sucessful.\n")
            self.sh_lib = ctypes.CDLL(self.lib_location)
        else:
            sys.exit("Error! Build failed!")
        
    def make_index(self, chain_file, table):
        """Create chain index file.
        
        Chain file - file to create index for.
        Table - binary index table path.
        """
        chain_ids, start_bytes, to_nexts = [], [], []
        byte_num, to_next = 0, 0

        f = open(chain_file, "rb")
        for line in f:
            if not line.startswith(b"chain"):
                # just count these bytes
                byte_num += len(line)
                to_next += len(line)
                continue
            # if we're here -> this is a chain header
            to_nexts.append(to_next)
            header = line.decode("utf-8").rstrip()
            chain_id = header.split()[-1]
            chain_ids.append(int(chain_id))
            start_bytes.append(int(byte_num))
            byte_num += len(line)
            to_next = len(line)
        f.close()
        arr_size = len(chain_ids)
        to_nexts.append(to_next)
        del to_nexts[0]

        # pass it to a shared library
        c_chain_ids = (ctypes.c_uint64 * (arr_size + 1))()
        c_chain_ids[:-1] = chain_ids

        c_s_bytes = (ctypes.c_uint64 * (arr_size + 1))()
        c_s_bytes[:-1] = start_bytes

        c_offsets = (ctypes.c_uint64 * (arr_size + 1))()
        c_offsets[:-1] = to_nexts

        c_arr_size = ctypes.c_uint64(arr_size)
        c_table_path = ctypes.c_char_p(str(table).encode())

        _ = self.sh_lib.make_index(c_chain_ids,
                                   c_s_bytes,
                                   c_offsets,
                                   c_arr_size,
                                   c_table_path)

    def extract_chain(self, chain_file, index, chain_id):
        """Improtable function to extract a chain."""
        c_index_path = ctypes.c_char_p(index.encode())
        c_chain_id = ctypes.c_uint64(chain_id)
        c_sb = ctypes.c_uint64(0)
        c_of = ctypes.c_uint64(0)

        _ = self.sh_lib.get_s_byte(c_index_path,
                                   c_chain_id,
                                   ctypes.byref(c_sb),
                                   ctypes.byref(c_of))
        if c_sb.value == c_of.value == 0:
            sys.stderr.write(f"Error, chain {chain_id} ")
            sys.stderr.write("not found\n")
            return ""
        f = open(chain_file, "rb")
        f.seek(c_sb.value)
        chain = f.read(c_of.value).decode("utf-8")
        f.close()
        return chain


def eprint(msg, end="\n"):
    """Write to stderr."""
    sys.stderr.write(msg + end)


def print_help(exe):
    """Show help message and quit."""
    eprint(f"Usage: {exe} [MODE: {MODES}] [OPTIONS]")
    eprint("Options for mode index: ")
    eprint("1) chain_file -> file to be indexed")
    eprint("2) table path -> where index shall be saved")
    eprint("")
    eprint("Options for mode extract: ")
    eprint("1) chain_file -> file to search in")
    eprint("2) table path -> pre-created index file")
    eprint("3) chain_id -> id to extract")
    sys.exit(0)


def cli():
    """Use as a stantdalone tool."""
    argv = sys.argv
    try:
        mode = argv[1]
        if mode not in MODES:
            raise ValueError("Wrong mode argument")
        elif mode == "index":
            chain_file = argv[2]
            table = argv[3]
            chain_id = 0
        else:
            # mode is extract
            chain_file = argv[2]
            table = argv[3]
            chain_id = int(argv[4])
    except (IndexError, ValueError):
        print_help(argv[0])
    
    # call requested module
    indexer = chain_index()
    if mode == "index":
        indexer.make_index(chain_file, table)
        print(f"Chain {chain_file} indexed, index saved to {table}")
    elif mode == "extract":
        chain = indexer.extract_chain(chain_file, table, chain_id)
        print(chain)
    pass

if __name__ == "__main__":
    cli()
