# Chain index/unindex

This is a library to index chain files and extract chains with high performance.
The format described in detail at: <https://genome.ucsc.edu/goldenPath/help/chain.html>.

Chain format is suitable for genome alignment representations.
However, in real world cases these files migth be giant, that makes search for some particular chain very problematic.
With this package, you can (i) create a binary index table and, using it, (ii) extract chains
you need very fast.

## Usage

This package might be used as both a library and a standalone tool.

### Standalone tool usage

```txt
Usage: ./chain_index/chain_index.py [MODE: {'index', 'extract'}] [OPTIONS]
Options for mode index: 
1) chain_file -> file to be indexed
2) table path -> where index shall be saved

Options for mode extract: 
1) chain_file -> file to search in
2) table path -> pre-created index file
3) chain_id -> id to extract
(base) hiller-mac-16:chainIndex kirilenk$ 
```

### Usage as library

First, initiate the class:

```python
from chain_index import chain_index
operator = chain_index()
```

Then, call one of two public functions, make_index or extract_chain.
