from building import *
Import('rtconfig')

src   = []
cwd   = GetCurrentDir()

# add gp2y10 src files.
if GetDepend('PKG_USING_GP2Y10'):
    src += Glob('src/gp2y10.c')
    src += Glob('src/sensor_sharp_gp2y10.c')

if GetDepend('PKG_USING_GP2Y10_SAMPLE'):
    src += Glob('examples/gp2y10_sample.c')
    src += Glob('examples/sensor_gp2y10_sample.c')

# add gp2y10 include path.
path  = [cwd + '/inc']

# add src and include to group.
group = DefineGroup('gp2y10', src, depend = ['PKG_USING_DHTXX'], CPPPATH = path)

Return('group')