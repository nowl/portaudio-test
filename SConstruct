# -*- mode: python -*-

env = Environment()

t = env.Program(target = 'test',
                source = ['audio.c',
                          'mempool.c',
                          'generators.c',
                          'test.c'])

env.Append(CCFLAGS = ['-g', '-Wall'])
env.Append(CPPPATH = ['/usr/local/include'])
env.Append(LIBPATH = ['/usr/local/lib'])
env.Append(LIBS = ['asound', 'portaudio', 'pthread', 'rt'])

Default(t)
