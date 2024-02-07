from building import *
import rtconfig

# get current directory
cwd     = GetCurrentDir()
# The set of source files associated with this SConscript file.
src     = []
src += Glob(cwd+"/common/src/*.c")
if GetDepend(['PKG_USING_FILEX_DEMO_MEM']):
    src += Glob(cwd+"/common/src/fx_ram_driver.c")
    src += Glob(cwd+"/common/src/fx_utility_memory_copy.c")
if GetDepend(['PKG_USING_FILEX_DEMO_BLK']):
    src += Glob("fx_user_driver.c")


path    = [cwd + '/common/inc']
path += [cwd + '/ports/generic/inc']
group = DefineGroup('Filex', src, depend = ['PKG_USING_FILEX'], CPPPATH = path)

Return('group')
