from building import *
Import('rtconfig')

src   = []
cwd   = GetCurrentDir()

# add dhtxx src files.
if GetDepend('PKG_USING_DHTXX'):
    src += Glob('src/dhtxx.c')
    src += Glob('src/sensor_asair_dhtxx.c')

if GetDepend('PKG_USING_DHTXX_SAMPLE'):
    src += Glob('examples/dhtxx_sample.c')
    src += Glob('examples/sensor_dhtxx_sample.c')

# add dhtxx include path.
path  = [cwd + '/inc']

# add src and include to group.
group = DefineGroup('dhtxx', src, depend = ['PKG_USING_DHTXX'], CPPPATH = path)

Return('group')