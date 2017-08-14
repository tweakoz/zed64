#!/usr/bin/env python
import os
import localopts
print localopts.ISE_BIN_DIR()
os.system("%s/ise zed64/zed64.xise"%localopts.ISE_BIN_DIR())
