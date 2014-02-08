env = Environment()

env.ParseConfig('pkg-config --cflags --libs gl glu glew')

env['LIBS'].append('glut')
env['CCFLAGS'].extend(['-std=c++11', '-g', '-O0'])

env.Program(['main.cpp', 'program.cpp'])

