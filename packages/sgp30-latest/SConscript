from building import *
Import('rtconfig')

src   = []
cwd   = GetCurrentDir()

# add sgp30 src files.
if GetDepend('PKG_USING_SGP30'):
    src += Glob('src/sgp30.c')
    src += Glob('src/sensor_sensirion_sgp30.c')

if GetDepend('PKG_USING_SGP30_SAMPLE'):
    src += Glob('examples/sgp30_sample.c')
    src += Glob('examples/sensor_sgp30_sample.c')

# add sgp30 include path.
path  = [cwd + '/inc']

# add src and include to group.
group = DefineGroup('sgp30', src, depend = ['PKG_USING_SGP30'], CPPPATH = path)

Return('group')