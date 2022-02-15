# Utility module

import sys

from .CppHeaderParser import CppHeader

if __name__ == "__main__":
    for arg in sys.argv[1:]:
        print(CppHeader(arg).to_json())
