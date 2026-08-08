# `basicpp` Python Bindings

Python FFI wrapper for the BASIC++ v6.5.2 engine (`libbasicpp`).

## Quickstart

```python
import basicpp

# Initialize BASIC++ Engine Instance
vm = basicpp.VM(ram_mb=640)

# Execute BASIC Statements
vm.exec('PRINT "Hello from Python + BASIC++!"')
vm.exec('LET X = 100')

# Evaluate Expressions
result = vm.eval('X * 4 + SQR(16)')
print("Evaluation Result:", result) # Output: 404.0

# Cleanup Engine Context
vm.close()
```
