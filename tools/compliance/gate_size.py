#!/usr/bin/env python3
# FILENAME: gate_size.py
# LICENSE: Copyleft (c) 2026 BASIC++ Community - All Wrongs Reserved
# VERSION: 7.0.0.0
# NEEDED BY: tools/compliance/run_all.sh, CI
# NEEDS: python3 standard library only
# File and function size gate. PROJECT_RULES sets 400 lines as the file
# ceiling, 250 as the target, and 60 lines as the function gate.
# Standard: ISO/IEC 25010:2023 Maintainability > Modularity, Analysability
import os, re, sys, json

FILE_CEIL, FILE_TARGET, FUNC_CEIL = 400, 250, 60
FN = re.compile(r'(?m)^(?!\s)(?!#)(?!//)[A-Za-z_][\w \t\*]*?\s(\w+)\s*\([^;{]*\)\s*\{')

def function_lengths(text):
    out = []
    for m in FN.finditer(text):
        start = text[:m.start()].count('\n')
        i = text.index('{', m.start()); d = 0; j = i
        while j < len(text):
            if text[j] == '{': d += 1
            elif text[j] == '}':
                d -= 1
                if d == 0: break
            j += 1
        out.append((text[:j].count('\n') - start + 1, m.group(1), start + 1))
    return out

def main(root='.'):
    over_file, over_func, files, funcs = [], [], 0, 0
    for dp, _, fns in os.walk(os.path.join(root, 'engine')):
        for f in fns:
            if not f.endswith(('.c', '.h')): continue
            p = os.path.join(dp, f); rel = os.path.relpath(p, root)
            t = open(p, encoding='utf-8', errors='replace').read()
            n = t.count('\n') + 1; files += 1
            if n > FILE_CEIL: over_file.append((n, rel))
            if f.endswith('.c'):
                for ln, name, at in function_lengths(t):
                    funcs += 1
                    if ln > FUNC_CEIL: over_func.append((ln, name, rel, at))
    over_file.sort(reverse=True); over_func.sort(reverse=True)
    for n, rel in over_file[:20]:
        print("%s: %d lines, ceiling %d" % (rel, n, FILE_CEIL))
    for ln, name, rel, at in over_func[:20]:
        print("%s:%d: %s() is %d lines, gate %d" % (rel, at, name, ln, FUNC_CEIL))
    print("GATE size: %d/%d files over %d lines, %d/%d functions over %d lines"
          % (len(over_file), files, FILE_CEIL, len(over_func), funcs, FUNC_CEIL))
    base = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'baseline.json')
    b = json.load(open(base)) if os.path.exists(base) else {}
    lim_f = b.get('size_files_over', 0); lim_n = b.get('size_funcs_over', 0)
    if len(over_file) > lim_f or len(over_func) > lim_n:
        print("FAIL: regressed against baseline (%d files, %d functions)" % (lim_f, lim_n))
        return 1
    return 0

if __name__ == '__main__':
    sys.exit(main(sys.argv[1] if len(sys.argv) > 1 else '.'))
