import glob, re, string, commands, sys, os
import shutil, fnmatch, platform, common

deco = common.deco

#################################################################################

def find(word):
 def _find(path):
  with open(path, "rb") as fp:
   for n, line in enumerate(fp):
    if word in line:
     yield n+1, line
 return _find

#################################################################################

class result:
 def __init__(self,path,lineno,text):
  self.path = path
  self.lineno = lineno
  self.text = text

###############################################################################

def search_at_root(word, root,extlist):
 finder = find(word)
 results = list()
 for root, dirs, files in os.walk(root):
  for f in files:
   path = os.path.join(root, f)
   spl = os.path.splitext(path)
   ext = spl[1]
   not_obj = (spl[0].find("/obj/")==-1) and (spl[0].find("/pluginobj/")==-1)
   if not_obj:
    is_in_list = ext in extlist
    if is_in_list:
     for line_number, line in finder(path):
      line = line.replace("\n","")
      res = result(path,line_number,line)
      results.append(res)
 return results 


#################################################################################
def makeModuleList(as_str):
  pathspl = string.split(as_str)
  pathlist = ""
  for p in pathspl:
    pathlist += "%s " % (p)
  return string.split(pathlist)

#################################################################################

def visit(word,visitor,subdirs,extensions):
  print "searching modules<%s>" % subdirs
  modulelist = makeModuleList(subdirs)
  extlist = string.split(extensions)
  for module in modulelist:
   results = search_at_root(word,module,extlist)
   have_results = len(results)!=0
   if have_results:
     visitor.onPath(module)
     for item in results:
       visitor.onItem(item)
