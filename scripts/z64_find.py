#!/usr/bin/python
import os, sys, string
import search as search
from common import deco

decor = deco()

#################################################################################
class visitor:
  def __init__(self):
    pass
  def onPath(self,pth):
    print "/////////////////////////////////////////////////////////////"
    print "// path : %s" % pth
    print "/////////"
  def onItem(self,item):
    print "%-*s : line %-*s : %s" % (40, decor.magenta(item.path), 5, decor.white(str(item.lineno)), decor.yellow(item.text))
#################################################################################

if __name__ == "__main__":
  #########################
  if not len(sys.argv) >= 2:
    print("usage: word [module]")
    sys.exit(1)
  #########################
  modulelist = None
  if len(sys.argv) == 3:
    modulelist = search.makeModuleList(sys.argv[2])
  #########################
  wsdir = os.environ["Z64ROOT"]
  os.chdir(wsdir)
  find_word = sys.argv[1]
  print("searching for word<%s>" % find_word)
  subdirs =  "nexys4 xilinx_lib 6502sim verilog-6502"
  extensions  = " .v .cpp .hpp .inl .py"

  search.visit(find_word,visitor(),subdirs,extensions)
