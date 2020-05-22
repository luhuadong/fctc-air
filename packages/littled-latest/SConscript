from building import *
Import('rtconfig')

src   = []
cwd   = GetCurrentDir()

# add littled src files.
if GetDepend('PKG_USING_LITTLED'):
    src += Glob('src/littled.c')

if GetDepend('PKG_USING_LITTLED_SAMPLE'):
    src += Glob('examples/littled_sample.c')

# add littled include path.
path  = [cwd + '/inc']

# add src and include to group.
group = DefineGroup('littled', src, depend = ['PKG_USING_LITTLED'], CPPPATH = path)

Return('group')
