#!/bin/env python

top = '.'
out = 'build'

def options(opt):
  opt.load('compiler_cxx')

  opt.add_option('--test', action='store', default='*', dest='test')

def configure(conf):
  conf.load('compiler_cxx')

  conf.check_cxx(header_name='boost/smart_ptr.hpp', mandatory=True)

def build(bld):
  bld.recurse('vendor hx tests')

def test(ctx):
  print
  for test in ctx.path.ant_glob(out + '/tests/test_' + ctx.options.test):
    print('T: ' + test.name)
    ctx.exec_command(test.abspath())

def tests(ctx):
  from waflib import Options
  Options.commands = ['configure', 'build', 'test'] + Options.commands
