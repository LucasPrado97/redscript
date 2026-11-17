#!/usr/bin/env python3
"""Compares two final.redscripts blobs table by table (redscript output is not byte-stable from run to run).
Usage: compare_blobs.py a.redscripts b.redscripts"""
import struct, sys, collections

TABLES = ["data", "names", "tweakdb_indexes", "resources", "definitions", "strings"]

def load(path):
    b = open(path, "rb").read()
    magic, version, flags = struct.unpack_from("<4sII", b, 0)
    assert magic == b"REDS", path
    hdr = {"version": version, "flags": flags, "size": len(b)}
    for i, t in enumerate(TABLES):
        hdr[t] = struct.unpack_from("<III", b, 32 + 12 * i)  # offset, count, crc32
    off, count, _ = hdr["definitions"]
    defs = []
    for i in range(count):
        name, parent, doff, size, typ, u1, u2, u3 = struct.unpack_from("<IIIIBBBB", b, off + 20 * i)
        defs.append((name, parent, typ, u1, u2, u3, size, b[doff:doff + size]))
    return hdr, defs

(ha, da), (hb, db) = load(sys.argv[1]), load(sys.argv[2])
print(f"size        {ha['size']} / {hb['size']}  {'same' if ha['size'] == hb['size'] else 'DIFFERENT'}")
for t in TABLES:
    (_, ca, xa), (_, cb, xb) = ha[t], hb[t]
    print(f"{t:16} count {ca} / {cb}  crc {'same' if xa == xb else 'different'}")
same_slot = sum(1 for x, y in zip(da, db) if x == y)
same_shape = sum(1 for x, y in zip(da, db) if x[:7] == y[:7])
print(f"definitions identical at the same index: {same_slot} of {len(da)}; same name/parent/type/size: {same_shape}")
diff = [(i, x, y) for i, (x, y) in enumerate(zip(da, db)) if x != y]
kinds = collections.Counter(x[2] for _, x, _ in diff)
print("differing definitions by type id:", dict(kinds))
nbytes = sum(sum(1 for p, q in zip(x[7], y[7]) if p != q) for _, x, y in diff if len(x[7]) == len(y[7]))
print(f"bytes differing inside same-size definitions: {nbytes}")
print("multiset of (header, body) equal:", collections.Counter(da) == collections.Counter(db))
